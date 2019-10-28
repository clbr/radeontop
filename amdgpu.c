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
#include <xf86drm.h>
#include <libdrm/amdgpu_drm.h>
#include <libdrm/amdgpu.h>

static amdgpu_device_handle amdgpu_dev;

static int getgrbm_amdgpu(uint32_t *out) {
	return amdgpu_read_mm_registers(amdgpu_dev, GRBM_STATUS / 4, 1,
					0xffffffff, 0, out);
}

static int getsrbm_amdgpu(uint32_t *out) {
	return amdgpu_read_mm_registers(amdgpu_dev, SRBM_STATUS / 4, 1,
					0xffffffff, 0, out);
}

static int getsrbm2_amdgpu(uint32_t *out) {
	return amdgpu_read_mm_registers(amdgpu_dev, SRBM_STATUS2 / 4, 1,
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

#define DRM_ATLEAST_VERSION(maj, min) \
	(drm_major > maj || (drm_major == maj && drm_minor >= min))

void init_amdgpu(int fd) {
	uint32_t drm_major, drm_minor, out32;
	uint64_t out64;
	struct amdgpu_gpu_info gpu;
	int ret;

	if (amdgpu_device_initialize(fd, &drm_major, &drm_minor, &amdgpu_dev))
		return;

	amdgpu_query_gpu_info(amdgpu_dev, &gpu);

	if (!(ret = getgrbm_amdgpu(&out32)))
		getgrbm = getgrbm_amdgpu;
	else
		drmError(ret, _("Failed to get GPU usage"));

	if (DRM_ATLEAST_VERSION(3, 1) && gpu.family_id >= AMDGPU_FAMILY_VI) {
		if (!(ret = getsrbm_amdgpu(&out32)))
			getsrbm = getsrbm_amdgpu;
		else
			drmError(ret, _("Failed to get UVD usage"));

		if (!(ret = getsrbm2_amdgpu(&out32)))
			getsrbm2 = getsrbm2_amdgpu;
		else
			drmError(ret, _("Failed to get VCE usage"));
	} else if (gpu.family_id >= AMDGPU_FAMILY_VI)
		fprintf(stderr, _("UVD/VCE reporting is disabled (amdgpu kernel driver 3.1.0 required)\n"));

#ifdef HAS_AMDGPU_QUERY_SENSOR_INFO
	if (DRM_ATLEAST_VERSION(3, 11)) {
		sclk_max = gpu.max_engine_clk;
		mclk_max = gpu.max_memory_clk;

		if (!(ret = getsclk_amdgpu(&out32)))
			getsclk = getsclk_amdgpu;
		else
			drmError(ret, _("Failed to get shader clock"));

		if (!(ret = getmclk_amdgpu(&out32)))
			getmclk = getmclk_amdgpu;
		else	// memory clock reporting not available on APUs
			if (!(gpu.ids_flags & AMDGPU_IDS_FLAGS_FUSION))
				drmError(ret, _("Failed to get memory clock"));
	} else
		fprintf(stderr, _("Clock frenquency reporting is disabled (amdgpu kernel driver 3.11.0 required)\n"));
#else
	fprintf(stderr, _("Clock frequency reporting is not compiled in (libdrm 2.4.79 required)\n"));
#endif

	struct drm_amdgpu_info_vram_gtt vram_gtt;

	if ((ret = amdgpu_query_info(amdgpu_dev, AMDGPU_INFO_VRAM_GTT,
				sizeof(vram_gtt), &vram_gtt))) {
		drmError(ret, _("Failed to get VRAM size"));
		return;
	}

	vramsize = vram_gtt.vram_size;
	gttsize = vram_gtt.gtt_size;

	if (!(ret = getvram_amdgpu(&out64)))
		getvram = getvram_amdgpu;
	else
		drmError(ret, _("Failed to get VRAM usage"));

	if (!(ret = getgtt_amdgpu(&out64)))
		getgtt = getgtt_amdgpu;
	else
		drmError(ret, _("Failed to get GTT usage"));
}

void cleanup_amdgpu() {
	if (amdgpu_dev)
		amdgpu_device_deinitialize(amdgpu_dev);
}
