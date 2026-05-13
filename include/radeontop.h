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

#ifndef RADEONTOP_H
#define RADEONTOP_H

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include "version.h"
#include "gettext.h"

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <locale.h>
#include <stdint.h>

enum {
	GRBM_STATUS = 0x8010,
	SRBM_STATUS = 0xe50,
	SRBM_STATUS2 = 0xe4c,
	MMAP_SIZE = 0x14,
	SRBM_MMAP_SIZE = 0xe54,
	VENDOR_AMD = 0x1002
};

// auth.c
void authenticate_drm(int fd);

// radeontop.c
void die(const char *why);

// detect.c
void init_pci(const char *path, short *bus, unsigned int *device_id, const unsigned char forcemem);
int getfamily(unsigned int id);
int getfamily_gfx(unsigned int gfx_ver);
void initbits(int fam);
void cleanup();

extern int (*getgrbm)(uint32_t *out);
extern int (*getsrbm)(uint32_t *out);
extern int (*getsrbm2)(uint32_t *out);
extern int (*getvram)(uint64_t *out);
extern int (*getgtt)(uint64_t *out);
extern int (*getsclk)(uint32_t *out);
extern int (*getmclk)(uint32_t *out);

// ticks.c
void collect(unsigned int ticks, unsigned int dumpinterval);

extern struct bits_t *results;

// ui.c
void present(const unsigned int ticks, const char card[], unsigned int color, unsigned int transparency, const unsigned char bus, const unsigned int dumpinterval);

// dump.c
void dumpdata(const unsigned int ticks, const char file[], const unsigned int limit, const unsigned char bus, const unsigned int dumpinterval);

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
	VEGAM,
	VEGA10,
	VEGA12,
	VEGA20,
	RAVEN,
	ARCTURUS,
	NAVI10,
	NAVI14,
	RENOIR,
	NAVI12,
	SIENNA_CICHLID,
	VANGOGH,
	YELLOW_CARP,
	NAVY_FLOUNDER,
	DIMGREY_CAVEFISH,
	ALDEBARAN,
	CYAN_SKILLFISH,
	BEIGE_GOBY,
	// RDNA 2 (gfx10.x)
	GFX1030,		// gfx1030, RX 6800/6900 XT, Pro W6800/V620
	GFX1031,		// gfx1031, RX 6700/6750/6800M/6850M
	GFX1032,		// gfx1032, RX 6600 series, Pro W6600
	GFX1034,		// gfx1034, RX 6300/6400/6500 XT, Pro W6300/W6400
	// RDNA 3 APUs (gfx1103-1151)
	GFX1035,		// gfx1035, Radeon 660M/680M
	MENDOCINO,		// gfx1036, Radeon Graphics 128SP
	GFX1033,		// gfx1033, Steam Deck GPU
	RADEON_780M,	// gfx1103, Radeon 740M/760M/780M/ROG Ally
	STRIX_POINT,	// gfx1150, Radeon 890M (Strix Point)
	RADEON_880M,	// gfx1151, Radeon 880M (Strix Halo)
	// RDNA 4 APUs (gfx115x - Krackan)
	KRACKAN_POINT,	// gfx1152, Radeon 820M/840M/860M (Krackan Point)
	// RDNA 4 APUs (gfx117x - Medusa)
	MEDUSA_POINT,	// gfx1170, Medusa Point
	MEDUSA_POINT_2,	// gfx1171, Medusa Point (variant mapping TBD)
	MEDUSA_POINT_3,	// gfx1172, Medusa Point (variant mapping TBD)
	// RDNA 4 dGPU (gfx12xx)
	RADEON_9000,	// gfx1200, Radeon 9000 series
	GFX1201,		// gfx1201
	// RDNA 5 (gfx13xx)
	GFX1300,		// gfx1300, RDNA 5
	GFX1310,		// gfx1310, RDNA 5
	// Instinct MI (gfx90x)
	MI8,			// gfx803, Instinct MI8/MI6
	MI25,			// gfx900, Instinct MI25
	MI50,			// gfx906, Instinct MI50/MI60
	MI100,			// gfx908, Instinct MI100
	MI210,			// gfx90a, Instinct MI210/MI250/MI250X
	MI300,			// gfx940, Instinct MI300/MI300X/MI325X
	NAVI31,			// gfx1100, RX 7900 series
	NAVI32,			// gfx1101, RX 7800/7700 series
	NAVI33,			// gfx1102, RX 7600/7500 series
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
	uint64_t vram;
	uint64_t gtt;
	unsigned int sclk;
	unsigned int mclk;
};

extern struct bits_t bits;
extern uint64_t vramsize;
extern uint64_t gttsize;
extern unsigned int sclk_max;
extern unsigned int mclk_max;

// radeon.c
void init_radeon(int fd, int drm_major, int drm_minor);

// amdgpu.c
void init_amdgpu(int fd);
void cleanup_amdgpu();
extern unsigned int gfx_version;

#endif
