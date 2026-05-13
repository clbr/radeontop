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

#define str(a) #a

const char * const family_str[] = {
	str(UNKNOWN_CHIP),
	str(R600),
	str(RV610),
	str(RV630),
	str(RV670),
	str(RV620),
	str(RV635),
	str(RS780),
	str(RS880),
	str(RV770),
	str(RV730),
	str(RV710),
	str(RV740),
	str(CEDAR),
	str(REDWOOD),
	str(JUNIPER),
	str(CYPRESS),
	str(HEMLOCK),
	str(PALM),
	str(SUMO),
	str(SUMO2),
	str(BARTS),
	str(TURKS),
	str(CAICOS),
	str(CAYMAN),
	str(ARUBA),
	str(TAHITI),
	str(PITCAIRN),
	str(VERDE),
	str(OLAND),
	str(HAINAN),
	str(BONAIRE),
	str(KABINI),
	str(MULLINS),
	str(KAVERI),
	str(HAWAII),
	str(TOPAZ),
	str(TONGA),
	str(FIJI),
	str(CARRIZO),
	str(STONEY),
	str(POLARIS11),
	str(POLARIS10),
	str(POLARIS12),
	str(VEGAM),
	str(VEGA10),
	str(VEGA12),
	str(VEGA20),
	str(RAVEN),
	str(ARCTURUS),
	str(NAVI10),
	str(NAVI14),
	str(RENOIR),
	str(NAVI12),
	str(SIENNA_CICHLID),
	str(VANGOGH),
	str(YELLOW_CARP),
	str(NAVY_FLOUNDER),
	str(DIMGREY_CAVEFISH),
	str(ALDEBARAN),
	str(CYAN_SKILLFISH),
	str(BEIGE_GOBY),
	// RDNA 2 (gfx10.x)
	"Radeon RX 6800/6900 XT (gfx1030)",
	"Radeon RX 6700/6750/6800M/6850M (gfx1031)",
	"Radeon RX 6600 (gfx1032)",
	"Radeon RX 6500/W6300 (gfx1034)",
	// RDNA 3 APUs (gfx1103-1151)
	"Radeon 660M/680M (gfx1035)",
	"Radeon Graphics 128SP (gfx1036)",
	"Steam Deck GPU (gfx1033)",
	"Radeon 740M/760M/780M (gfx1103)",
	"Radeon 890M (Strix Point)",
	"Radeon 880M (Strix Halo)",
	// RDNA 4 APUs (gfx115x - Kraken)
	"Radeon 820M/840M/860M (Krackan Point)",
	// RDNA 4 APUs (gfx117x - Medusa Point variants)
	"Medusa Point (gfx1170)",
	"Medusa Point (gfx1171)",
	"Medusa Point (gfx1172)",
	// RDNA 4 dGPU (gfx12xx)
	"Radeon 9000 (gfx1200)",
	"Radeon 9000 XT (gfx1201)",
	// RDNA 5 (gfx13xx)
	"Radeon (gfx1300)",
	"Radeon (gfx1310)",
	// Instinct MI (gfx90x)
	"Instinct MI8/MI6 (gfx803)",
	"Instinct MI25 (gfx900)",
	"Instinct MI50/MI60 (gfx906)",
	"Instinct MI100 (gfx908)",
	"Instinct MI210/MI250/MI250X (gfx90a)",
	"Instinct MI300/MI300X/MI325X (gfx940)",
	// RDNA 3 dGPU (gfx110x)
	"Radeon RX 7900 (gfx1100)",
	"Radeon RX 7800/7700 (gfx1101)",
	"Radeon RX 7600/7500 (gfx1102)",
};
