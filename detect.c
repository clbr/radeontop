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
char drm_name[10] = ""; // should be radeon or amdgpu

unsigned int init_pci(unsigned char bus, const unsigned char forcemem) {

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
	char busid[32];

	while ((dev = pci_device_next(iter))) {
		pci_device_probe(dev);
		if ((dev->device_class & 0x00ffff00) != 0x00030000 &&
			(dev->device_class & 0x00ffff00) != 0x00038000)
			continue;
		snprintf(busid, sizeof(busid), "pci:%04x:%02x:%02x.%u",
				dev->domain, dev->bus, dev->dev, dev->func);
		if (!bus || bus == dev->bus)
			break;
	}

	pci_iterator_destroy(iter);

	if (!dev)
		die(_("Can't find Radeon cards"));

	const unsigned int device_id = dev->device_id;
	int reg = 2;
	if (getfamily(device_id) >= BONAIRE)
		reg = 5;

	if (!dev->regions[reg].size) die(_("Can't get the register area size"));

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

	use_ioctl = 0;
	if (drm_fd >= 0) {
		authenticate_drm(drm_fd);
		uint32_t rreg = GRBM_STATUS;
		if (strcmp(drm_name, "radeon") == 0) {
			use_ioctl = get_drm_value(drm_fd, rreg, RADEON_INFO_READ_REG, &rreg);
		} else if (strcmp(drm_name, "amdgpu") == 0) {
#ifdef ENABLE_AMDGPU
			use_ioctl = get_drm_value(drm_fd, rreg, AMDGPU_INFO_READ_MMR_REG, &rreg);
#else
			printf(_("amdgpu DRM driver is used, but amdgpu ioctl query is not enabled\n"));
#endif
		}
	}

	if (forcemem) {
		printf(_("Forcing the /dev/mem path.\n"));
		use_ioctl = 0;
	}

	if (!use_ioctl) {
		int mem = open("/dev/mem", O_RDONLY);
		if (mem < 0) die(_("Cannot access GPU registers, are you root?"));

		grbm_area = mmap(NULL, GRBM_MMAP_SIZE, PROT_READ, MAP_PRIVATE, mem,
				dev->regions[reg].base_addr + 0x8000);
		if (grbm_area == MAP_FAILED) die(_("mmap failed"));

		srbm_area = mmap(NULL, SRBM_MMAP_SIZE, PROT_READ, MAP_PRIVATE, mem,
				dev->regions[reg].base_addr);
		if (srbm_area == MAP_FAILED) die(_("mmap failed"));
	}

	bits.vram = 0;
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

		// No version indicator, so we need to test once
		// We use different codepaths for radeon and amdgpu
		// We store vram_size and check below if the ret value is sane
		if (strcmp(drm_name, "radeon") == 0) {
			struct drm_radeon_gem_info gem;

			ret = drmCommandWriteRead(drm_fd, DRM_RADEON_GEM_INFO,
							&gem, sizeof(gem));
			vramsize = gem.vram_size;
		} else if (strcmp(drm_name, "amdgpu") == 0) {
#ifdef ENABLE_AMDGPU
			struct drm_amdgpu_info_vram_gtt vram_gtt = {};

			struct drm_amdgpu_info request;
			memset(&request, 0, sizeof(request));
			request.return_pointer = (unsigned long) &vram_gtt;
			request.return_size = sizeof(vram_gtt);
			request.query = AMDGPU_INFO_VRAM_GTT;

			ret = drmCommandWrite(drm_fd, DRM_AMDGPU_INFO,
						&request, sizeof(request));
			vramsize = vram_gtt.vram_size;
#else
			printf(_("amdgpu DRM driver is used, but amdgpu VRAM size reporting is not enabled\n"));
#endif
		}
		if (ret) {
			printf(_("Failed to get VRAM size, error %d\n"),
				ret);
			goto out;
		}

		ret = getvram();
		if (ret == 0) {
			if (strcmp(drm_name, "amdgpu") == 0) {
#ifndef ENABLE_AMDGPU
				printf(_("amdgpu DRM driver is used, but amdgpu VRAM usage reporting is not enabled\n"));
#endif
			}
			printf(_("Failed to get VRAM usage, kernel likely too old\n"));
			goto out;
		}

		bits.vram = 1;
	}

	out:

	pci_system_cleanup();

	return device_id;
}

unsigned long long getvram() {

	int ret = -1;
	unsigned long long val = 0;

	if (strcmp(drm_name, "radeon") == 0) {
		struct drm_radeon_info info;
		memset(&info, 0, sizeof(info));
		info.value = (unsigned long) &val;
		info.request = RADEON_INFO_VRAM_USAGE;

		ret = drmCommandWriteRead(drm_fd, DRM_RADEON_INFO, &info, sizeof(info));
	} else if (strcmp(drm_name, "amdgpu") == 0) {
#ifdef ENABLE_AMDGPU
		struct drm_amdgpu_info request;
		memset(&request, 0, sizeof(request));
		request.return_pointer = (unsigned long) &val;
		request.return_size = sizeof(val);
		request.query = AMDGPU_INFO_VRAM_USAGE;

		ret = drmCommandWrite(drm_fd, DRM_AMDGPU_INFO, &request, sizeof(request));
#endif
	}
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
	bits.uvd = (1U << 19);
	bits.vce0 = (1U << 7);

	// R600 has a different texture bit, and only R600 has the TC, CR, SMX bits
	if (fam < RV770) {
		bits.ta = (1U << 18);
	} else {
		bits.tc = 0;
		bits.cr = 0;
		bits.smx = 0;
	}
}
