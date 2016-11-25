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

#ifndef RADEONTOP_H
#define RADEONTOP_H

#define _GNU_SOURCE

#include "version.h"
#include "gettext.h"

#include <stdio.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <locale.h>
#include <xf86drm.h>
#include <radeon_drm.h>
#ifdef ENABLE_AMDGPU
#include <amdgpu_drm.h>
enum {
	GRBM_STATUS = 0x2004,
	SRBM_STATUS = 0x394,
	SRBM_STATUS2 = 0x393,
	GRBM_MMAP_SIZE = 0x14,
	SRBM_MMAP_SIZE = 0xe54
};
#else
enum {
	GRBM_STATUS = 0x8010,
	SRBM_STATUS = 0xe50,
	SRBM_STATUS2 = 0xe4c,
	GRBM_MMAP_SIZE = 0x14,
	SRBM_MMAP_SIZE = 0xe54
};
#endif

#ifndef RADEON_INFO_VRAM_USAGE
#define RADEON_INFO_VRAM_USAGE 0x1e
#endif
#ifndef RADEON_INFO_READ_REG
#define RADEON_INFO_READ_REG 0x24
#endif

// auth.c
void authenticate_drm(int fd);

// radeontop.c
void die(const char *why);
int get_drm_value(int fd, unsigned regoffset, unsigned request, uint32_t *out);
unsigned int readgrbm();
unsigned int readsrbm();
unsigned int readsrbm2();

extern const void *grbm_area;
extern const void *srbm_area;
extern int use_ioctl;
extern char drm_name[10];

// detect.c
unsigned int init_pci(unsigned char bus, const unsigned char forcemem);
int getfamily(unsigned int id);
void initbits(int fam);
unsigned long long getvram();
unsigned long long getgtt();

// ticks.c
void collect(unsigned int *ticks);

extern struct bits_t *results;

// ui.c
void present(const unsigned int ticks, const char card[], unsigned int color);

// dump.c
void dumpdata(const unsigned int ticks, const char file[], const unsigned int limit);

// chips
enum radeon_family {
	UNKNOWN_CHIP,
	R600,
	RV610,
	RV630,
	RV670,
	RV620,
	RV635,
	RS780,
	RS880,
	RV770,
	RV730,
	RV710,
	RV740,
	CEDAR,
	REDWOOD,
	JUNIPER,
	CYPRESS,
	HEMLOCK,
	PALM,
	SUMO,
	SUMO2,
	BARTS,
	TURKS,
	CAICOS,
	CAYMAN,
	ARUBA,
	TAHITI,
	PITCAIRN,
	VERDE,
	OLAND,
	HAINAN,
	BONAIRE,
	KABINI,
	MULLINS,
	KAVERI,
	HAWAII,
	TOPAZ,
	TONGA,
	FIJI,
	CARRIZO,
	STONEY,
	POLARIS11,
	POLARIS10,
	POLARIS12,
	VEGA10,
};

extern const char * const family_str[];

// bits
struct bits_t {
	unsigned int ee;
	unsigned int vgt;
	unsigned int gui;
	unsigned int ta;
	unsigned int tc;
	unsigned int sx;
	unsigned int sh;
	unsigned int spi;
	unsigned int smx;
	unsigned int sc;
	unsigned int pa;
	unsigned int db;
	unsigned int cb;
	unsigned int cr;
	unsigned int uvd;
	unsigned int vce0;
	unsigned long long vram;
	unsigned long long gtt;
};

extern struct bits_t bits;
extern unsigned long long vramsize;
extern unsigned long long gttsize;
extern int drm_fd;

#endif
