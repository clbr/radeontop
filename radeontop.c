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

#include "radeontop.h"
#include <getopt.h>

const void *area;
int use_ioctl;

void die(const char * const why) {
	puts(why);
	exit(1);
}

static void version() {
	printf("RadeonTop %s\n", VERSION);
	exit(1);
}

static void help(const char * const me, const unsigned int ticks, const unsigned int dumpinterval) {
	printf(_("\n\tRadeonTop for R600 and above.\n\n"
		"\tUsage: %s [-ch] [-b bus] [-d file] [-i seconds] [-l limit] [-t ticks]\n\n"
		"-b --bus 3		Pick card from this PCI bus (hexadecimal)\n"
		"-c --color		Enable colors\n"
		"-d --dump file		Dump data to this file, - for stdout\n"
		"-i --dump-interval 1	Number of seconds between dumps (default %u)\n"
		"-l --limit 3		Quit after dumping N lines, default forever\n"
		"-m --mem		Force the /dev/mem path, for the proprietary driver\n"
		"-t --ticks 50		Samples per second (default %u)\n"
		"\n"
		"-h --help		Show this help\n"
		"-v --version		Show the version\n"),
		me, dumpinterval, ticks);
	die("");
}

int get_drm_value(int fd, unsigned request, uint32_t *out) {
	struct drm_radeon_info info;
	int retval;

	memset(&info, 0, sizeof(info));

	info.value = (unsigned long)out;
	info.request = request;

	retval = drmCommandWriteRead(fd, DRM_RADEON_INFO, &info, sizeof(info));
	return !retval;
}

unsigned int readgrbm() {

	if (use_ioctl) {
		uint32_t reg = 0x8010;
		get_drm_value(drm_fd, RADEON_INFO_READ_REG, &reg);
		return reg;
	} else {
		const void *ptr = (const char *) area + 0x10;
		const unsigned int *inta = ptr;
		return *inta;
	}
}

int main(int argc, char **argv) {
	// Temporarily drop privileges to do option parsing, etc.
	seteuid(getuid());

	const unsigned int default_ticks = 120;
	const unsigned int default_dumpinterval = 1;

	unsigned int ticks = default_ticks;
	unsigned char color = 0;
	unsigned char bus = 0, forcemem = 0;
	unsigned int limit = 0;
	char *dump = NULL;
	unsigned int dumpinterval = default_dumpinterval;

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
		{"dump-interval", 1, 0, 'i'},
		{"help", 0, 0, 'h'},
		{"limit", 1, 0, 'l'},
		{"mem", 0, 0, 'm'},
		{"ticks", 1, 0, 't'},
		{"version", 0, 0, 'v'},
		{0, 0, 0, 0}
	};

	while (1) {
		int c = getopt_long(argc, argv, "b:cd:hi:l:mt:v", opts, NULL);
		if (c == -1) break;

		switch(c) {
			case 'h':
			case '?':
				help(argv[0], default_ticks, default_dumpinterval);
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
				bus = strtoul(optarg, NULL, 16);
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
			case 'i':
				dumpinterval = atoi(optarg);
				if(dumpinterval < 1)
					dumpinterval = 1;
			break;
		}
	}

	// init (regain privileges for bus initialization and ultimately drop them afterwards)
	seteuid(0);
	const unsigned int pciaddr = init_pci(bus, forcemem);
	// after init_pci we can assume that bus exists (otherwise it would die())

	setuid(getuid());

	const int family = getfamily(pciaddr);
	if (!family)
		puts(_("Unknown Radeon card. <= R500 won't work, new cards might."));

	const char * const cardname = family_str[family];

	initbits(family);

	// runtime
	collect(ticks, dumpinterval);

	if (dump)
		dumpdata(ticks, dump, limit, bus, dumpinterval);
	else
		present(ticks, cardname, color, bus, dumpinterval);

	munmap((void *) area, MMAP_SIZE);
	return 0;
}
