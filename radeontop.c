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
#include <getopt.h>

const void *grbm_area;
const void *srbm_area;
int use_ioctl;

void die(const char * const why) {
	puts(why);
	exit(1);
}

static void version() {
	printf("RadeonTop %s\n", VERSION);
	exit(1);
}

static void help(const char * const me, const unsigned int ticks) {
	printf(_("\n\tRadeonTop for R600 and above.\n\n"
		"\tUsage: %s [-ch] [-b bus] [-d file] [-l limit] [-t ticks]\n\n"
		"-b --bus 3		Pick card from this PCI bus\n"
		"-c --color		Enable colors\n"
		"-d --dump file		Dump data to this file, - for stdout\n"
		"-l --limit 3		Quit after dumping N lines, default forever\n"
		"-m --mem		Force the /dev/mem path, for the proprietary driver\n"
		"-t --ticks 50		Samples per second (default %u)\n"
		"\n"
		"-h --help		Show this help\n"
		"-v --version		Show the version\n"),
		me, ticks);
	die("");
}

int get_drm_value(int fd, unsigned reg_offset, unsigned request, uint32_t *out){

	int retval = -1;

	if (strcmp(drm_name, "radeon") == 0) {
		struct drm_radeon_info info;
		memset(&info, 0, sizeof(info));

		info.value = (unsigned long)out;
		info.request = request;

		retval = drmCommandWriteRead(fd, DRM_RADEON_INFO, &info, sizeof(info));
	} else if (strcmp(drm_name, "amdgpu") == 0) {
#ifdef ENABLE_AMDGPU
		struct drm_amdgpu_info info;
		memset(&info, 0, sizeof(info));

		info.return_pointer = (unsigned long)out;
		info.return_size = sizeof(uint32_t);
		info.query = request;
		info.read_mmr_reg.dword_offset = reg_offset;
		info.read_mmr_reg.count = 1;
		info.read_mmr_reg.instance = 0xffffffff;
		info.read_mmr_reg.flags = 0;

		retval = drmCommandWriteRead(fd, DRM_AMDGPU_INFO, &info, sizeof(info));
#endif
	}
	return !retval;
}

unsigned int readgrbm() {

	if (use_ioctl) {
		uint32_t reg = GRBM_STATUS;
		if (strcmp(drm_name, "radeon") == 0) {
			get_drm_value(drm_fd, reg, RADEON_INFO_READ_REG, &reg);
		} else if (strcmp(drm_name, "amdgpu") == 0) {
#ifdef ENABLE_AMDGPU
			get_drm_value(drm_fd, reg, AMDGPU_INFO_READ_MMR_REG, &reg);
#else
			printf(_("amdgpu DRM driver is used, but amdgpu ioctl query is not enabled\n"));
#endif
		}
		return reg;
	} else {
		const void *ptr = (const char *) grbm_area + 0x10;
		const unsigned int *inta = ptr;
		return *inta;
	}
}

unsigned int readsrbm() {

	if (use_ioctl) {
		uint32_t reg = SRBM_STATUS;
		if (strcmp(drm_name, "radeon") == 0) {
			get_drm_value(drm_fd, reg, RADEON_INFO_READ_REG, &reg);
		} else if (strcmp(drm_name, "amdgpu") == 0) {
#ifdef ENABLE_AMDGPU
			get_drm_value(drm_fd,reg, AMDGPU_INFO_READ_MMR_REG, &reg);
#else
			printf(_("amdgpu DRM driver is used, but amdgpu ioctl query is not enabled\n"));
#endif
		}
		return reg;
	} else {
		const void *ptr = (const char *) srbm_area + 0xe50;
		const unsigned int *inta = ptr;
		return *inta;
	}
}

unsigned int readsrbm2() {

	if (use_ioctl) {
		uint32_t reg = SRBM_STATUS2;
		if (strcmp(drm_name, "radeon") == 0) {
			get_drm_value(drm_fd, reg, RADEON_INFO_READ_REG, &reg);
		} else if (strcmp(drm_name, "amdgpu") == 0) {
#ifdef ENABLE_AMDGPU
			get_drm_value(drm_fd,reg, AMDGPU_INFO_READ_MMR_REG, &reg);
#else
			printf(_("amdgpu DRM driver is used, but amdgpu ioctl query is not enabled\n"));
#endif
		}
		return reg;
	} else {
		const void *ptr = (const char *) srbm_area + 0xe4c;
		const unsigned int *inta = ptr;
		return *inta;
	}
}

int main(int argc, char **argv) {

	unsigned int ticks = 120;
	unsigned char color = 0;
	unsigned char bus = 0, forcemem = 0;
	unsigned int limit = 0;
	char *dump = NULL;

	// Translations
#ifdef ENABLE_NLS
	setlocale(LC_ALL, "");
	bindtextdomain("radeontop", "/usr/share/locale");
	textdomain("radeontop");
#endif

	// opts
	const struct option opts[] = {
		{"bus", 1, 0, 'b'},
		{"color", 0, 0, 'c'},
		{"dump", 1, 0, 'd'},
		{"help", 0, 0, 'h'},
		{"limit", 1, 0, 'l'},
		{"mem", 0, 0, 'm'},
		{"ticks", 1, 0, 't'},
		{"version", 0, 0, 'v'},
		{0, 0, 0, 0}
	};

	while (1) {
		int c = getopt_long(argc, argv, "b:cd:hl:mt:v", opts, NULL);
		if (c == -1) break;

		switch(c) {
			case 'h':
			case '?':
				help(argv[0], ticks);
			break;
			case 't':
				ticks = atoi(optarg);
			break;
			case 'c':
				color = 1;
			break;
			case 'm':
				forcemem = 1;
			break;
			case 'b':
				bus = atoi(optarg);
			break;
			case 'v':
				version();
			break;
			case 'l':
				limit = atoi(optarg);
			break;
			case 'd':
				dump = optarg;
			break;
		}
	}

	// init
	const unsigned int pciaddr = init_pci(bus, forcemem);

	const int family = getfamily(pciaddr);
	if (!family)
		puts(_("Unknown Radeon card. <= R500 won't work, new cards might."));

	const char * const cardname = family_str[family];

	initbits(family);

	// runtime
	collect(&ticks);

	if (dump)
		dumpdata(ticks, dump, limit);
	else
		present(ticks, cardname, color);

	munmap((void *) grbm_area, GRBM_MMAP_SIZE);
	munmap((void *) srbm_area, SRBM_MMAP_SIZE);
	return 0;
}
