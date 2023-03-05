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
	str(GFX1100),
	str(GFX1101),
	str(GFX1102),
	str(GFX1103_R1),
	str(GFX1103_R2),
	str(GFX1036),
	str(GFX1037),
};
