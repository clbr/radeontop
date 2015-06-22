/*
    Copyright (C) 2012 Lauri Kasanen

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
unsigned long long vramsize;
int drm_fd = -1;

static int drmfilter(const struct dirent *ent) {

	if (ent->d_name[0] == '.')
		return 0;
	if (strncmp(ent->d_name, "card", 4))
		return 0;

	return 1;
}

static void finddrm(const unsigned char bus) {

	int fd, i;
	struct dirent **namelist;
	const int entries = scandir("/dev/dri", &namelist, drmfilter, alphasort);
	char tmp[160];

	if (entries < 0) {
		perror("scandir");
		return;
	}

	for (i = 0; i < entries; i++) {
		snprintf(tmp, 160, "/dev/dri/%s", namelist[i]->d_name);

		fd = open(tmp, O_RDWR);
		if (fd < 0) continue;

		const char *busid = drmGetBusid(fd);

		if (strncmp(busid, "pci:", 4))
			goto fail;

		busid = strchr(busid, ':');
		if (!busid) goto fail;
		busid++;

		busid = strchr(busid, ':');
		if (!busid) goto fail;
		busid++;

		unsigned parsed;
		if (sscanf(busid, "%x", &parsed) != 1)
			goto fail;

		if (parsed == bus) {
			drm_fd = fd;
			break;
		}

		fail:
		close(fd);
	}

	for (i = 0; i < entries; i++) {
		free(namelist[i]);
	}

	free(namelist);
}

unsigned int init_pci(unsigned char bus) {

	int ret = pci_system_init();
	if (ret)
		die(_("Failed to init pciaccess"));

	struct pci_id_match match;

	match.vendor_id = 0x1002;
	match.device_id = PCI_MATCH_ANY;
	match.subvendor_id = PCI_MATCH_ANY;
	match.subdevice_id = PCI_MATCH_ANY;
	match.device_class = 0;
	match.device_class_mask = 0;
	match.match_data = 0;

	struct pci_device_iterator *iter = pci_id_match_iterator_create(&match);
	struct pci_device *dev = NULL;

	while ((dev = pci_device_next(iter))) {
		pci_device_probe(dev);
		if ((dev->device_class & 0x00ffff00) != 0x00030000 &&
			(dev->device_class & 0x00ffff00) != 0x00038000)
			continue;
		if (!bus || bus == dev->bus)
			break;
	}

	pci_iterator_destroy(iter);

	if (!dev)
		die(_("Can't find Radeon cards"));

	int reg = 2;
	if (getfamily(dev->device_id) >= BONAIRE)
		reg = 5;

	if (!dev->regions[reg].size) die(_("Can't get the register area size"));

//	printf("Found area %p, size %lu\n", area, dev->regions[reg].size);

	// DRM support for VRAM
	if (bus)
		finddrm(bus);
	else if (access("/dev/dri/card0", F_OK) == 0)
		drm_fd = open("/dev/dri/card0", O_RDWR);
	else if (access("/dev/ati/card0", F_OK) == 0) // fglrx path
		drm_fd = open("/dev/ati/card0", O_RDWR);

	use_ioctl = 0;
	if (drm_fd >= 0) {
		uint32_t rreg = 0x8010;
		use_ioctl = get_drm_value(drm_fd, RADEON_INFO_READ_REG, &rreg);
	}

	if (!use_ioctl) {
		int mem = open("/dev/mem", O_RDONLY);
		if (mem < 0) die(_("Cannot access GPU registers, are you root?"));

		area = mmap(NULL, MMAP_SIZE, PROT_READ, MAP_PRIVATE, mem,
				dev->regions[reg].base_addr + 0x8000);
		if (area == MAP_FAILED) die(_("mmap failed"));
	}

	bits.vram = 0;
	if (drm_fd < 0) {
		printf(_("Failed to open DRM node, no VRAM support.\n"));
	} else {
		drmDropMaster(drm_fd);
		drmVersion * const ver = drmGetVersion(drm_fd);

/*		printf("Version %u.%u.%u, name %s\n",
			ver->version_major,
			ver->version_minor,
			ver->version_patchlevel,
			ver->name);*/

		const int outdatedKernel = ver->version_major <= 2 && ver->version_minor <= 36;

		// Free the allocated storage
		drmFreeVersion(ver);

		if (outdatedKernel) {
			printf(_("Kernel too old for VRAM reporting.\n"));
			goto out;
		}

		// No version indicator, so we need to test once

		struct drm_radeon_gem_info gem;

		ret = drmCommandWriteRead(drm_fd, DRM_RADEON_GEM_INFO,
						&gem, sizeof(gem));
		if (ret) {
			printf(_("Failed to get VRAM size, error %d\n"),
				ret);
			goto out;
		}
		vramsize = gem.vram_size;

		ret = getvram();
		if (ret == 0) {
			printf(_("Failed to get VRAM usage, kernel likely too old\n"));
			goto out;
		}

		bits.vram = 1;
	}

	out:

	pci_system_cleanup();

	return dev->device_id;
}

unsigned long long getvram() {

	int ret;
	unsigned long long val = 0;

	struct drm_radeon_info info;
	memset(&info, 0, sizeof(info));
	info.value = (unsigned long) &val;
	info.request = RADEON_INFO_VRAM_USAGE;

	ret = drmCommandWriteRead(drm_fd, DRM_RADEON_INFO, &info, sizeof(info));
	if (ret) return 0;

	return val;
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

void shutdown_pci(void) {

	// Not much to do here
	pci_system_cleanup();
}
