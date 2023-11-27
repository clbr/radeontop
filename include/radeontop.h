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
        NAVI31,
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

#endif
