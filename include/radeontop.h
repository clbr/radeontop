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

enum {
	GRBM_STATUS = 0x8010,
	MMAP_SIZE = 0x14
};

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

// detect.c
void init_pci(unsigned char *bus, unsigned int *device_id, const unsigned char forcemem);
int getfamily(unsigned int id);
void initbits(int fam);
void cleanup();
unsigned int readgrbm();
unsigned long long getvram();
unsigned long long getgtt();
unsigned long long getmclk();
unsigned long long getsclk();

// ticks.c
void collect(unsigned int ticks, unsigned int dumpinterval);

extern struct bits_t *results;

// ui.c
void present(const unsigned int ticks, const char card[], unsigned int color, const unsigned char bus, const unsigned int dumpinterval);

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
	unsigned long long vram;
	unsigned long long gtt;
	unsigned long long mclk;
	unsigned long long sclk;
};

extern struct bits_t bits;
extern unsigned long long vramsize;
extern unsigned long long gttsize;
extern unsigned long long mclk_max;
extern unsigned long long sclk_max;
extern int drm_fd;

#endif
