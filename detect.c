/*
    Copyright (C) 2012 Lauri Kasanen
    Copyright (C) 2018 Genesis Cloud Ltd.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, version 3 of the License.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "radeontop.h"
#include <pciaccess.h>
#include <dirent.h>

struct bits_t bits;
uint64_t vramsize;
uint64_t gttsize;
unsigned int sclk_max = 0; // kilohertz
unsigned int mclk_max = 0; // kilohertz
const void *area;

// function pointers to the right backend
int (*getgrbm)(uint32_t *out);
int (*getvram)(uint64_t *out);
int (*getgtt)(uint64_t *out);
int (*getsclk)(uint32_t *out);
int (*getmclk)(uint32_t *out);

static void find_pci(short bus, struct pci_device *pci_dev) {
	int ret = pci_system_init();
	if (ret)
		die(_("Failed to init pciaccess"));

	struct pci_device *dev;
	struct pci_id_match match;

	match.vendor_id = 0x1002;
	match.device_id = PCI_MATCH_ANY;
	match.subvendor_id = PCI_MATCH_ANY;
	match.subdevice_id = PCI_MATCH_ANY;
	match.device_class = 0;
	match.device_class_mask = 0;
	match.match_data = 0;

	struct pci_device_iterator *iter = pci_id_match_iterator_create(&match);

	while ((dev = pci_device_next(iter))) {
		pci_device_probe(dev);
		if ((dev->device_class & 0x00ffff00) != 0x00030000 &&
			(dev->device_class & 0x00ffff00) != 0x00038000)
			continue;
		if (bus < 0 || bus == dev->bus) {
			*pci_dev = *dev;
			break;
		}
	}

	pci_iterator_destroy(iter);

	if (!dev)
		die(_("Can't find Radeon cards"));

	pci_system_cleanup();
}

static int getgrbm_pci(uint32_t *out) {
	*out = *(uint32_t *)((const char *) area + 0x10);
	return 0;
}

static void open_pci(struct pci_device *gpu_device) {
	// Warning: gpu_device is a copy of a freed struct. Do not access any pointers!
	int reg = 2;
	if (getfamily(gpu_device->device_id) >= BONAIRE)
		reg = 5;

	if (!gpu_device->regions[reg].size) die(_("Can't get the register area size"));

//	printf("Found area %p, size %lu\n", area, dev->regions[reg].size);

	int mem = open("/dev/mem", O_RDONLY);
	if (mem < 0) die(_("Cannot access GPU registers, are you root?"));

	area = mmap(NULL, MMAP_SIZE, PROT_READ, MAP_PRIVATE, mem,
			gpu_device->regions[reg].base_addr + 0x8000);
	if (area == MAP_FAILED) die(_("mmap failed"));

	getgrbm = getgrbm_pci;
}

static void cleanup_pci() {
	munmap((void *) area, MMAP_SIZE);
}

static void init_drm(int drm_fd) {
	char drm_name[10] = ""; // should be radeon or amdgpu
	drmVersionPtr ver = drmGetVersion(drm_fd);
	if (strcmp(ver->name, "radeon") != 0 && strcmp(ver->name, "amdgpu") != 0) {
		close(drm_fd);
		return;
	}
	strcpy(drm_name, ver->name);
	drmFreeVersion(ver);
	authenticate_drm(drm_fd);

	if (strcmp(drm_name, "radeon") == 0)
		init_radeon(drm_fd);
	else if (strcmp(drm_name, "amdgpu") == 0)
#ifdef ENABLE_AMDGPU
		init_amdgpu(drm_fd);
#else
		fprintf(stderr, _("amdgpu support is not compiled in (libdrm 2.4.63 required)\n"));
#endif

	drmDropMaster(drm_fd);

/*	printf("Version %u.%u.%u, name %s\n",
		ver->version_major,
		ver->version_minor,
		ver->version_patchlevel,
		ver->name);*/
}

static void open_drm_bus(const struct pci_device *dev) {
	char busid[32];
	snprintf(busid, sizeof(busid), "pci:%04x:%02x:%02x.%u",
			 dev->domain, dev->bus, dev->dev, dev->func);

	int fd = drmOpen(NULL, busid);

	if (fd >= 0)
		init_drm(fd);
	else
		printf(_("Failed to open DRM node, no VRAM support.\n"));
}

// do-nothing backend used as fallback
#define UNUSED(v)	(void) v
static int getuint32_null(uint32_t *out) { UNUSED(out); return -1; }
static int getuint64_null(uint64_t *out) { UNUSED(out); return -1; }

void init_pci(short *bus, unsigned int *device_id, const unsigned char forcemem) {
	int use_ioctl = 0;

	getgrbm = getsclk = getmclk = getuint32_null;
	getvram = getgtt = getuint64_null;

	struct pci_device pci_dev, *gpu_device = &pci_dev;
	memset(&pci_dev, 0, sizeof(struct pci_device));
	find_pci(*bus, &pci_dev);

	// DRM support for VRAM
	open_drm_bus(&pci_dev);
	use_ioctl = (getgrbm != getuint32_null);

	if (forcemem) {
		printf(_("Forcing the /dev/mem path.\n"));
		use_ioctl = 0;
	}

	if (!use_ioctl) {
		open_pci(&pci_dev);
	}

	bits.vram = (getvram != getuint64_null);
	bits.gtt = (getgtt != getuint64_null);

	*bus = gpu_device->bus;
	*device_id = gpu_device->device_id;
}

void cleanup() {
	cleanup_pci();

#ifdef ENABLE_AMDGPU
	cleanup_amdgpu();
#endif
}

int getfamily(unsigned int id) {

	switch(id) {
		#define CHIPSET(a,b,c) case a: return c;
		#include "r600_pci_ids.h"
		#undef CHIPSET
	}

	return 0;
}

void initbits(int fam) {

	// The majority of these is the same from R600 to Southern Islands.

	bits.ee = (1U << 10);
	bits.vgt = (1U << 16) | (1U << 17);
	bits.ta = (1U << 14);
	bits.tc = (1U << 19);
	bits.sx = (1U << 20);
	bits.sh = (1U << 21);
	bits.spi = (1U << 22);
	bits.smx = (1U << 23);
	bits.sc = (1U << 24);
	bits.pa = (1U << 25);
	bits.db = (1U << 26);
	bits.cr = (1U << 27);
	bits.cb = (1U << 30);
	bits.gui = (1U << 31);

	// R600 has a different texture bit, and only R600 has the TC, CR, SMX bits
	if (fam < RV770) {
		bits.ta = (1U << 18);
	} else {
		bits.tc = 0;
		bits.cr = 0;
		bits.smx = 0;
	}
}
