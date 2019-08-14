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
#include <libdrm/amdgpu_drm.h>
#include <libdrm/amdgpu.h>

static amdgpu_device_handle amdgpu_dev;

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
