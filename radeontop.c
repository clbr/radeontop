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

#define  S_008010_EE_BUSY                     (1 << 10)
#define  S_008010_VC_BUSY                     (1 << 11)
#define  S_008010_VGT_BUSY_NO_DMA             (1 << 16)
#define  S_008010_VGT_BUSY                    (1 << 17)
#define  S_008010_TA03_BUSY                   (1 << 14)
#define  S_008010_TC_BUSY                     (1 << 19)
#define  S_008010_SX_BUSY                     (1 << 20)
#define  S_008010_SH_BUSY                     (1 << 21)
#define  S_008010_SPI03_BUSY                  (1 << 22)
#define  S_008010_SMX_BUSY                    (1 << 23)
#define  S_008010_SC_BUSY                     (1 << 24)
#define  S_008010_PA_BUSY                     (1 << 25)
#define  S_008010_DB03_BUSY                   (1 << 26)
#define  S_008010_CR_BUSY                     (1 << 27)
#define  S_008010_CP_COHERENCY_BUSY           (1 << 28)
#define  S_008010_CP_BUSY                     (1 << 29)
#define  S_008010_CB03_BUSY                   (1 << 30)
#define  S_008010_GUI_ACTIVE		      (1 << 31)

void *area;

void die(const char *why) {
	puts(why);
	exit(1);
}

unsigned int readgrbm() {

	void *ptr = area + 0x10;

	unsigned int *inta = ptr;

	return *inta;
}

int main() {

	const unsigned int pciaddr = init_pci();

	unsigned int grbm_status = readgrbm();

	const int family = getfamily(pciaddr);
	const char * const cardname = family_str[family];

	printf("grbm_status: %u\n", grbm_status);

puts("\n\n<stat>");
	if (grbm_status & S_008010_EE_BUSY) puts("Event Engine busy");
	if (grbm_status & S_008010_VC_BUSY) puts("Vertex Cache busy");
	if (grbm_status & (S_008010_VGT_BUSY|S_008010_VGT_BUSY_NO_DMA))
		puts("Vertex Grouper and Tesselator busy");
	if (grbm_status & S_008010_GUI_ACTIVE) puts("Graphics pipe busy");
	if (grbm_status & S_008010_TA03_BUSY) puts("Texture Addresser busy");
	if (grbm_status & S_008010_TC_BUSY) puts("Texture Cache busy");
	if (grbm_status & S_008010_SX_BUSY) puts("Shader Export busy");
	if (grbm_status & S_008010_SH_BUSY) puts("Sequencer Instruction Cache busy");
	if (grbm_status & S_008010_SPI03_BUSY) puts("Shader Interpolator busy");
	if (grbm_status & S_008010_SMX_BUSY) puts("Shader Memory Exchange busy");
	if (grbm_status & S_008010_SC_BUSY) puts("Scan Converter busy");
	if (grbm_status & S_008010_PA_BUSY) puts("Primitive Assembly busy");
	if (grbm_status & S_008010_DB03_BUSY) puts("Depth Block busy");
	if (grbm_status & S_008010_CR_BUSY) puts("Clip Rectangle busy");
	if (grbm_status & S_008010_CB03_BUSY) puts("Color Block busy");


	munmap(area, 4);
	return 0;
}
