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
int drm_fd = -1;
char drm_name[10] = ""; // should be radeon or amdgpu
const void *area;

// function pointers to the right backend
int (*getgrbm)(uint32_t *out);
int (*getvram)(uint64_t *out);
int (*getgtt)(uint64_t *out);
int (*getsclk)(uint32_t *out);
int (*getmclk)(uint32_t *out);

#ifdef ENABLE_AMDGPU
#include <libdrm/amdgpu_drm.h>
#include <libdrm/amdgpu.h>
amdgpu_device_handle amdgpu_dev;
#endif

struct pci_device * findGPUDevice(const unsigned char bus) {
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
		if (!bus || bus == dev->bus)
			break;
	}

	pci_iterator_destroy(iter);

	if (!dev)
		die(_("Can't find Radeon cards"));

	return dev;
}

static int radeon_get_drm_value(int fd, unsigned request, uint32_t *out) {
	struct drm_radeon_info info;

	memset(&info, 0, sizeof(info));
	info.value = (unsigned long) out;
	info.request = request;

	return drmCommandWriteRead(fd, DRM_RADEON_INFO, &info, sizeof(info));
}

static int getgrbm_radeon(uint32_t *out) {
	*out = GRBM_STATUS;
	return radeon_get_drm_value(drm_fd, RADEON_INFO_READ_REG, out);
}

static int getvram_radeon(uint64_t *out) {
	return radeon_get_drm_value(drm_fd, RADEON_INFO_VRAM_USAGE,
				(uint32_t *) out);
}

static int getgtt_radeon(uint64_t *out) {
	return radeon_get_drm_value(drm_fd, RADEON_INFO_GTT_USAGE,
				(uint32_t *) out);
}

void init_radeon(int fd) {
	int drm_major, drm_minor, ret;
	drmVersionPtr ver = drmGetVersion(fd);

	drm_major = ver->version_major;
	drm_minor = ver->version_minor;
	drmFreeVersion(ver);
	getgrbm = getgrbm_radeon;

	if (drm_major > 2 || (drm_major == 2 && drm_minor >= 36)) {
		struct drm_radeon_gem_info gem;

		if ((ret = drmCommandWriteRead(fd, DRM_RADEON_GEM_INFO,
					 &gem, sizeof(gem)))) {
			printf(_("Failed to get VRAM size, error %d\n"), ret);
			return;
		}

		vramsize = gem.vram_size;
		gttsize = gem.gart_size;
		getvram = getvram_radeon;
		getgtt = getgtt_radeon;
	} else
		printf(_("Kernel too old for VRAM reporting.\n"));
}

#ifdef ENABLE_AMDGPU
static int getgrbm_amdgpu(uint32_t *out) {
	return amdgpu_read_mm_registers(amdgpu_dev, GRBM_STATUS / 4, 1,
					0xffffffff, 0, out);
}

static int getvram_amdgpu(uint64_t *out) {
	return amdgpu_query_info(amdgpu_dev, AMDGPU_INFO_VRAM_USAGE,
				sizeof(uint64_t), out);
}

static int getgtt_amdgpu(uint64_t *out) {
	return amdgpu_query_info(amdgpu_dev, AMDGPU_INFO_GTT_USAGE,
				sizeof(uint64_t), out);
}

#ifdef HAS_AMDGPU_QUERY_SENSOR_INFO
static int getsclk_amdgpu(uint32_t *out) {
	return amdgpu_query_sensor_info(amdgpu_dev, AMDGPU_INFO_SENSOR_GFX_SCLK,
		sizeof(uint32_t), out);
}

static int getmclk_amdgpu(uint32_t *out) {
	return amdgpu_query_sensor_info(amdgpu_dev, AMDGPU_INFO_SENSOR_GFX_MCLK,
		sizeof(uint32_t), out);
}
#endif

void init_amdgpu(int fd) {
	uint32_t drm_major, drm_minor;
	int ret;

	if (amdgpu_device_initialize(fd, &drm_major, &drm_minor, &amdgpu_dev))
		die(_("Can't initialize amdgpu driver"));

	getgrbm = getgrbm_amdgpu;

#ifdef HAS_AMDGPU_QUERY_SENSOR_INFO
	struct amdgpu_gpu_info gpu;

	amdgpu_query_gpu_info(amdgpu_dev, &gpu);
	sclk_max = gpu.max_engine_clk;
	mclk_max = gpu.max_memory_clk;
	getsclk = getsclk_amdgpu;
	getmclk = getmclk_amdgpu;
#else
	printf(_("amdgpu DRM driver is used, but clock reporting is not enabled\n"));
#endif

	struct drm_amdgpu_info_vram_gtt vram_gtt;

	if ((ret = amdgpu_query_info(amdgpu_dev, AMDGPU_INFO_VRAM_GTT,
				sizeof(vram_gtt), &vram_gtt))) {
		printf(_("Failed to get VRAM size, error %d\n"), ret);
		return;
	}

	vramsize = vram_gtt.vram_size;
	gttsize = vram_gtt.gtt_size;
	getvram = getvram_amdgpu;
	getgtt = getgtt_amdgpu;
}

void cleanup_amdgpu() {
	if (amdgpu_dev)
		amdgpu_device_deinitialize(amdgpu_dev);
}
#endif

static int getgrbm_mem(uint32_t *out) {
	*out = *(uint32_t *)((const char *) area + 0x10);
	return 0;
}

// do-nothing backend used as fallback
#define UNUSED(v)	(void) v
static int getuint32_null(uint32_t *out) { UNUSED(out); return -1; }
static int getuint64_null(uint64_t *out) { UNUSED(out); return -1; }

void init_pci(unsigned char *bus, unsigned int *device_id, const unsigned char forcemem) {
	int use_ioctl = 0;

	getgrbm = getsclk = getmclk = getuint32_null;
	getvram = getgtt = getuint64_null;

	int ret = pci_system_init();
	if (ret)
		die(_("Failed to init pciaccess"));

	const struct pci_device * const gpu_device = findGPUDevice(*bus);

	char busid[32];
	snprintf(busid, sizeof(busid), "pci:%04x:%02x:%02x.%u",
			 gpu_device->domain, gpu_device->bus, gpu_device->dev, gpu_device->func);

	int reg = 2;
	if (getfamily(gpu_device->device_id) >= BONAIRE)
		reg = 5;

	if (!gpu_device->regions[reg].size) die(_("Can't get the register area size"));

//	printf("Found area %p, size %lu\n", area, dev->regions[reg].size);

	// DRM support for VRAM
	drm_fd = drmOpen(NULL, busid);
	if (drm_fd >= 0) {
		drmVersionPtr ver = drmGetVersion(drm_fd);
		if (strcmp(ver->name, "radeon") != 0 && strcmp(ver->name, "amdgpu") != 0) {
			close(drm_fd);
			drm_fd = -1;
		}
		strcpy(drm_name, ver->name);
		drmFreeVersion(ver);
	}
	if (drm_fd < 0 && access("/dev/ati/card0", F_OK) == 0) // fglrx path
		drm_fd = open("/dev/ati/card0", O_RDWR);

	if (drm_fd >= 0) {
		authenticate_drm(drm_fd);

		if (strcmp(drm_name, "radeon") == 0)
			init_radeon(drm_fd);
		else if (strcmp(drm_name, "amdgpu") == 0) {
#ifdef ENABLE_AMDGPU
			init_amdgpu(drm_fd);
#else
			printf(_("amdgpu DRM driver is used, but support is not compiled in\n"));
#endif
		}

		uint32_t rreg;
		use_ioctl = !getgrbm(&rreg);
	}

	if (forcemem) {
		printf(_("Forcing the /dev/mem path.\n"));
		use_ioctl = 0;
	}

	if (!use_ioctl) {
		int mem = open("/dev/mem", O_RDONLY);
		if (mem < 0) die(_("Cannot access GPU registers, are you root?"));

		area = mmap(NULL, MMAP_SIZE, PROT_READ, MAP_PRIVATE, mem,
				gpu_device->regions[reg].base_addr + 0x8000);
		if (area == MAP_FAILED) die(_("mmap failed"));

		getgrbm = getgrbm_mem;
	}

	bits.vram = 0;
	bits.gtt = 0;
	if (drm_fd < 0) {
		printf(_("Failed to open DRM node, no VRAM support.\n"));
	} else {
		drmDropMaster(drm_fd);

/*		printf("Version %u.%u.%u, name %s\n",
			ver->version_major,
			ver->version_minor,
			ver->version_patchlevel,
			ver->name);*/

		// No version indicator, so we need to test once
		// We use different codepaths for radeon and amdgpu
		// We store vram_size and check below if the ret value is sane
		uint64_t out64;

		ret = getvram(&out64);
		if (ret) {
			printf(_("Failed to get VRAM usage, kernel likely too old\n"));
			goto out;
		}

		bits.vram = 1;

		ret = getgtt(&out64);
		if (ret) {
			printf(_("Failed to get GTT usage, kernel likely too old\n"));
			goto out;
		}

		bits.gtt = 1;
	}

	out:

	*bus = gpu_device->bus;
	*device_id = gpu_device->device_id;
	pci_system_cleanup();
}

void cleanup() {
	munmap((void *) area, MMAP_SIZE);

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
