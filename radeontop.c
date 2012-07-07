/*
    Copyright (C) 2012 Lauri Kasanen

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "radeontop.h"

const void *area;

void die(const char *why) {
	puts(why);
	exit(1);
}

unsigned int readgrbm() {

	const void *ptr = area + 0x10;

	const unsigned int *inta = ptr;

	return *inta;
}

int main() {

	const unsigned int pciaddr = init_pci();

	unsigned int grbm_status = readgrbm();

	const int family = getfamily(pciaddr);
	const char * const cardname = family_str[family];

	initbits(family);

	if (grbm_status & bits.ee) puts("Event Engine busy");
	if (grbm_status & bits.vc) puts("Vertex Cache busy");
	if (grbm_status & bits.vgt)
		puts("Vertex Grouper and Tesselator busy");
	if (grbm_status & bits.gui) puts("Graphics pipe busy");
	if (grbm_status & bits.ta) puts("Texture Addresser busy");
	if (grbm_status & bits.tc) puts("Texture Cache busy");
	if (grbm_status & bits.sx) puts("Shader Export busy");
	if (grbm_status & bits.sh) puts("Sequencer Instruction Cache busy");
	if (grbm_status & bits.spi) puts("Shader Interpolator busy");
	if (grbm_status & bits.smx) puts("Shader Memory Exchange busy");
	if (grbm_status & bits.sc) puts("Scan Converter busy");
	if (grbm_status & bits.pa) puts("Primitive Assembly busy");
	if (grbm_status & bits.db) puts("Depth Block busy");
	if (grbm_status & bits.cr) puts("Clip Rectangle busy");
	if (grbm_status & bits.cb) puts("Color Block busy");


	munmap((void *) area, 4);
	return 0;
}
