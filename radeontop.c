#include <stdio.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

const unsigned long start = 0xfebe0000;

#define GRBM_STATUS                                     0x8010
#define GRBM_STATUS2                                    0x8014

#define  S_008010_GRBM_EE_BUSY                (1 << 10)
#define  S_008010_VC_BUSY                     (1 << 11)
#define  S_008010_VGT_BUSY_NO_DMA             (1 << 16)
#define  S_008010_VGT_BUSY                    (1 << 17)
#define  S_008010_TA03_BUSY                   (1 << 18)
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
	if (grbm_status & S_008010_VC_BUSY) puts("vc busy");
	if (grbm_status & S_008010_VGT_BUSY) puts("vgt busy");
	if (grbm_status & S_008010_GUI_ACTIVE) puts("gui busy");
	if (grbm_status & S_008010_TA03_BUSY) puts("ta03 busy");
	if (grbm_status & S_008010_TC_BUSY) puts("tc busy");
	if (grbm_status & S_008010_SX_BUSY) puts("sx busy");
	if (grbm_status & S_008010_SH_BUSY) puts("sh busy");
	if (grbm_status & S_008010_SPI03_BUSY) puts("spi busy");
	if (grbm_status & S_008010_SMX_BUSY) puts("smx busy");
	if (grbm_status & S_008010_SC_BUSY) puts("sc busy");
	if (grbm_status & S_008010_PA_BUSY) puts("pa busy");
	if (grbm_status & S_008010_DB03_BUSY) puts("db03 busy");
	if (grbm_status & S_008010_CR_BUSY) puts("cr busy");
	if (grbm_status & S_008010_CB03_BUSY) puts("cb03 busy");
puts("\n<stat2>");
	if (grbm_status2 & S_008014_SPI0_BUSY) puts("spi0 busy");
	if (grbm_status2 & S_008014_SPI1_BUSY) puts("spi1 busy");
	if (grbm_status2 & S_008014_SPI2_BUSY) puts("spi2 busy");
	if (grbm_status2 & S_008014_SPI3_BUSY) puts("spi3 busy");
	if (grbm_status2 & S_008014_TA0_BUSY) puts("ta0 busy");
	if (grbm_status2 & S_008014_TA1_BUSY) puts("ta1 busy");
	if (grbm_status2 & S_008014_TA2_BUSY) puts("ta2 busy");
	if (grbm_status2 & S_008014_DB0_BUSY) puts("db0 busy");
	if (grbm_status2 & S_008014_DB1_BUSY) puts("db1 busy");
	if (grbm_status2 & S_008014_DB2_BUSY) puts("db2 busy");
	if (grbm_status2 & S_008014_DB3_BUSY) puts("db3 busy");
	if (grbm_status2 & S_008014_CB0_BUSY) puts("cb0 busy");
	if (grbm_status2 & S_008014_CB1_BUSY) puts("cb1 busy");
	if (grbm_status2 & S_008014_CB2_BUSY) puts("cb2 busy");
	if (grbm_status2 & S_008014_CB3_BUSY) puts("cb3 busy");


	munmap(area, 65536);
	return 0;
}
