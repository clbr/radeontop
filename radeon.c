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
#include <radeon_drm.h>
#include <xf86drm.h>

static int drm_fd;

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

	drm_fd = fd;
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
