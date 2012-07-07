#include <stdio.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

const unsigned long start = 0xfebe0000;

#define GRBM_STATUS                                     0x8010
#define GRBM_STATUS2                                    0x8014

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

#define  S_008014_SPI0_BUSY                   (1 << 8)
#define  S_008014_SPI1_BUSY                   (1 << 9)
#define  S_008014_SPI2_BUSY                   (1 << 10)
#define  S_008014_SPI3_BUSY                   (1 << 11)
#define  S_008014_TA0_BUSY                    (1 << 12)
#define  S_008014_TA1_BUSY                    (1 << 13)
#define  S_008014_TA2_BUSY                    (1 << 14)
#define  S_008014_TA3_BUSY                    (1 << 15)
#define  S_008014_DB0_BUSY                    (1 << 16)
#define  S_008014_DB1_BUSY                    (1 << 17)
#define  S_008014_DB2_BUSY                    (1 << 18)
#define  S_008014_DB3_BUSY                    (1 << 19)
#define  S_008014_CB0_BUSY                    (1 << 20)
#define  S_008014_CB1_BUSY                    (1 << 21)
#define  S_008014_CB2_BUSY                    (1 << 22)
#define  S_008014_CB3_BUSY                    (1 << 23)


void *area;

void die(const char *why) {
	puts(why);
	exit(1);
}

unsigned int readint(unsigned int where) {
	void *target = area + where;

	unsigned int *inta = target;
	return *inta;
}

int main() {

	int mem = open("/dev/mem", O_RDONLY);
	if (mem < 0) die("can't open devmem");

	area = mmap(NULL, 65536, PROT_READ, MAP_PRIVATE, mem, start);
	if (area == MAP_FAILED) die("mmap");

	unsigned int grbm_status = readint(GRBM_STATUS);
	unsigned int grbm_status2 = readint(GRBM_STATUS2);

	printf("grbm_status: %u\n", grbm_status);
	printf("grbm_status2: %u\n", grbm_status2);

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
puts("\n<stat2>");
	if (grbm_status2 & S_008014_SPI0_BUSY) puts("Shader Interpolator 0 busy");
	if (grbm_status2 & S_008014_SPI1_BUSY) puts("Shader Interpolator 1 busy");
	if (grbm_status2 & S_008014_SPI2_BUSY) puts("Shader Interpolator 2 busy");
	if (grbm_status2 & S_008014_SPI3_BUSY) puts("Shader Interpolator 3 busy");
	if (grbm_status2 & S_008014_TA0_BUSY) puts("Texture Addresser 0 busy");
	if (grbm_status2 & S_008014_TA1_BUSY) puts("Texture Addresser 1 busy");
	if (grbm_status2 & S_008014_TA2_BUSY) puts("Texture Addresser 2 busy");
	if (grbm_status2 & S_008014_DB0_BUSY) puts("Depth Block 0 busy");
	if (grbm_status2 & S_008014_DB1_BUSY) puts("Depth Block 1 busy");
	if (grbm_status2 & S_008014_DB2_BUSY) puts("Depth Block 2 busy");
	if (grbm_status2 & S_008014_DB3_BUSY) puts("Depth Block 3 busy");
	if (grbm_status2 & S_008014_CB0_BUSY) puts("Color Block 0 busy");
	if (grbm_status2 & S_008014_CB1_BUSY) puts("Color Block 1 busy");
	if (grbm_status2 & S_008014_CB2_BUSY) puts("Color Block 2 busy");
	if (grbm_status2 & S_008014_CB3_BUSY) puts("Color Block 3 busy");


	munmap(area, 65536);
	return 0;
}
