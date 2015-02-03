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
    str(TONGA),
};
