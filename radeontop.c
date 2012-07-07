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
#include <getopt.h>

const void *area;
unsigned int ticks = 60;
unsigned char color = 0;

void die(const char *why) {
	puts(why);
	exit(1);
}

static void help(const char * const me) {
	printf("\n\tUsage: %s [-ch] [-t ticks]\n\n"
		"-c --color		Enable colors\n"
		"-h --help		Show this help\n"
		"-t --ticks 50		Samples per second (default %u)\n",
		me, ticks);
	die("");
}

unsigned int readgrbm() {

	const void *ptr = area + 0x10;

	const unsigned int *inta = ptr;

	return *inta;
}

int main(int argc, char **argv) {

	// opts
	const struct option opts[] = {
		{"color", 0, 0, 'c'},
		{"help", 0, 0, 'h'},
		{"ticks", 1, 0, 't'},
		{0, 0, 0, 0}
	};

	while (1) {
		int c = getopt_long(argc, argv, "cht:", opts, NULL);
		if (c == -1) break;

		switch(c) {
			case 'h':
			case '?':
				help(argv[0]);
			break;
			case 't':
				ticks = atoi(optarg);
			break;
			case 'c':
				color = 1;
			break;
		}
	}

	// init
	const unsigned int pciaddr = init_pci();

	const int family = getfamily(pciaddr);
	const char * const cardname = family_str[family];

	initbits(family);

	// runtime
	unsigned int grbm_status = readgrbm();

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
