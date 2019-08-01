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
unsigned long long vramsize;
unsigned long long gttsize;
unsigned long long mclk_max = 0; // kilohertz
unsigned long long sclk_max = 0; // kilohertz
int drm_fd = -1;
char drm_name[10] = ""; // should be radeon or amdgpu
const void *area;

// function pointers to the right backend
int (*getgrbm_device)(uint32_t *out);
int (*getvram_device)(unsigned long long *out);
int (*getgtt_device)(unsigned long long *out);

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

int getgrbm_device_radeon(uint32_t *out) {
	struct drm_radeon_info info;

	memset(&info, 0, sizeof(info));
	*out = GRBM_STATUS;

	info.value = (unsigned long)out;
	info.request = RADEON_INFO_READ_REG;

	return drmCommandWriteRead(drm_fd, DRM_RADEON_INFO, &info, sizeof(info));
}

int getvram_device_radeon(unsigned long long *out) {
	struct drm_radeon_info info;
	memset(&info, 0, sizeof(info));
	info.value = (unsigned long) out;
	info.request = RADEON_INFO_VRAM_USAGE;

	return drmCommandWriteRead(drm_fd, DRM_RADEON_INFO, &info, sizeof(info));
}

int getgtt_device_radeon(unsigned long long *out) {
	struct drm_radeon_info info;
	memset(&info, 0, sizeof(info));
	info.value = (unsigned long) out;
	info.request = RADEON_INFO_GTT_USAGE;

	return drmCommandWriteRead(drm_fd, DRM_RADEON_INFO, &info, sizeof(info));
}

#ifdef ENABLE_AMDGPU
int getgrbm_device_amdgpu(uint32_t *out) {
	return amdgpu_read_mm_registers(amdgpu_dev, GRBM_STATUS / 4, 1,
					0xffffffff, 0, out);
}

int getvram_device_amdgpu(unsigned long long *out) {
	return amdgpu_query_info(amdgpu_dev, AMDGPU_INFO_VRAM_USAGE,
				sizeof(unsigned long long), out);
}

int getgtt_device_amdgpu(unsigned long long *out) {
	return amdgpu_query_info(amdgpu_dev, AMDGPU_INFO_GTT_USAGE,
				sizeof(unsigned long long), out);
}
#endif

int getgrbm_device_mem(uint32_t *out) {
	*out = *(uint32_t *)((const char *) area + 0x10);
	return 0;
}

void init_pci(unsigned char *bus, unsigned int *device_id, const unsigned char forcemem) {
	int use_ioctl = 0;
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
		getgrbm_device = getgrbm_device_radeon;
		getvram_device = getvram_device_radeon;
		getgtt_device = getgtt_device_radeon;

		if (strcmp(drm_name, "amdgpu") == 0) {
#ifdef ENABLE_AMDGPU
			uint32_t maj, min;

			if (amdgpu_device_initialize(drm_fd, &maj, &min,
						&amdgpu_dev))
				die(_("Can't initialize amdgpu driver"));

			getgrbm_device = getgrbm_device_amdgpu;
			getvram_device = getvram_device_amdgpu;
			getgtt_device = getgtt_device_amdgpu;
#else
			printf(_("amdgpu DRM driver is used, but support is not compiled in\n"));
#endif
		}

		uint32_t rreg;
		use_ioctl = !getgrbm_device(&rreg);
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

		getgrbm_device = getgrbm_device_mem;
	}

	bits.vram = 0;
	bits.gtt = 0;
	if (drm_fd < 0) {
		printf(_("Failed to open DRM node, no VRAM support.\n"));
	} else {
		drmDropMaster(drm_fd);
		drmVersionPtr ver = drmGetVersion(drm_fd);

/*		printf("Version %u.%u.%u, name %s\n",
			ver->version_major,
			ver->version_minor,
			ver->version_patchlevel,
			ver->name);*/

		if (ver->version_major < 2 ||
			(ver->version_major == 2 && ver->version_minor < 36)) {
			printf(_("Kernel too old for VRAM reporting.\n"));
			drmFreeVersion(ver);
			goto out;
		}
		drmFreeVersion(ver);

		if (strcmp(drm_name, "amdgpu") == 0) {
#ifdef HAS_AMDGPU_QUERY_SENSOR_INFO
			struct amdgpu_gpu_info gpu = {};
			ret = amdgpu_query_gpu_info(amdgpu_dev, &gpu);

			if (ret == 0) {
				mclk_max = gpu.max_memory_clk;
				sclk_max = gpu.max_engine_clk;
			}
#else
			printf(_("amdgpu DRM driver is used, but clock reporting is not enabled\n"));
#endif
		}

		// No version indicator, so we need to test once
		// We use different codepaths for radeon and amdgpu
		// We store vram_size and check below if the ret value is sane
		if (strcmp(drm_name, "radeon") == 0) {
			struct drm_radeon_gem_info gem;

			ret = drmCommandWriteRead(drm_fd, DRM_RADEON_GEM_INFO,
							&gem, sizeof(gem));
			vramsize = gem.vram_size;
			gttsize = gem.gart_size;
		} else if (strcmp(drm_name, "amdgpu") == 0) {
#ifdef ENABLE_AMDGPU
			struct drm_amdgpu_info_vram_gtt vram_gtt = {};
			ret = amdgpu_query_info(amdgpu_dev,
						AMDGPU_INFO_VRAM_GTT,
						sizeof(vram_gtt), &vram_gtt);
			vramsize = vram_gtt.vram_size;
			gttsize = vram_gtt.gtt_size;
#endif
		}
		if (ret) {
			printf(_("Failed to get VRAM size, error %d\n"),
				ret);
			goto out;
		}

		ret = getvram();
		if (ret == 0) {
			printf(_("Failed to get VRAM usage, kernel likely too old\n"));
			goto out;
		}

		bits.vram = 1;

		ret = getgtt();
		if (ret == 0) {
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
	amdgpu_device_deinitialize(amdgpu_dev);
#endif
}

unsigned int readgrbm() {
	uint32_t reg;
	getgrbm_device(&reg);
	return reg;
}

unsigned long long getvram() {
	unsigned long long val = 0;

	if (getvram_device(&val))
		return 0;

	return val;
}

unsigned long long getgtt() {
	unsigned long long val = 0;

	if (getgtt_device(&val))
		return 0;

	return val;
}

#ifdef HAS_AMDGPU_QUERY_SENSOR_INFO
static unsigned long long amdgpu_read_sensor(uint32_t sensor_id) {
	int ret;
	unsigned long long val = 0;

	ret = amdgpu_query_sensor_info(amdgpu_dev, sensor_id,
				sizeof(val), &val);

	if (ret) return 0;

	return val;
}
#endif

unsigned long long getmclk() {
#ifdef HAS_AMDGPU_QUERY_SENSOR_INFO
	return amdgpu_read_sensor(AMDGPU_INFO_SENSOR_GFX_MCLK);
#endif
	return 0;
}

unsigned long long getsclk() {
#ifdef HAS_AMDGPU_QUERY_SENSOR_INFO
	return amdgpu_read_sensor(AMDGPU_INFO_SENSOR_GFX_SCLK);
#endif
	return 0;
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
