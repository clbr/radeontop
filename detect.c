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
#include <pciaccess.h>

struct bits_t bits;

unsigned int init_pci(unsigned char bus) {

	int ret = pci_system_init();
	if (ret)
		die(_("Failed to init pciaccess"));

	struct pci_id_match match;

	match.vendor_id = 0x1002;
	match.device_id = PCI_MATCH_ANY;
	match.subvendor_id = PCI_MATCH_ANY;
	match.subdevice_id = PCI_MATCH_ANY;
	match.device_class = 0;
	match.device_class_mask = 0;
	match.match_data = 0;

	struct pci_device_iterator *iter = pci_id_match_iterator_create(&match);
	struct pci_device *dev = NULL;

	while ((dev = pci_device_next(iter))) {
		pci_device_probe(dev);
		if ((dev->device_class & 0x00ffff00) != 0x00030000 &&
			(dev->device_class & 0x00ffff00) != 0x00038000)
			continue;
		if (!bus || bus == dev->bus)
			break;
	}

	pci_iterator_destroy(iter);

	if (!dev)
		die(_("Can't find Radeon cards"));

	if (!dev->regions[2].size) die(_("Can't get the register area size"));

//	printf("Found area %p, size %lu\n", area, dev->regions[2].size);

	int mem = open("/dev/mem", O_RDONLY);
	if (mem < 0) die(_("Can't open /dev/mem, are you root?"));

	area = mmap(NULL, 4, PROT_READ, MAP_PRIVATE, mem,
			dev->regions[2].base_addr + 0x8000);
	if (area == MAP_FAILED) die(_("mmap failed"));

	return dev->device_id;
}

int getfamily(unsigned int id) {

	switch(id) {
		#define CHIPSET(a,b,c) case a: return c;
		#include "r600_pci_ids.h"
		#undef CHIPSET
	}

	return 0;
}

void initbits(int fam) {

	// The majority of these is the same from R600 to Southern Islands.

	bits.ee = (1 << 10);
	bits.vgt = (1 << 16) | (1 << 17);
	bits.ta = (1 << 14);
	bits.tc = (1 << 19);
	bits.sx = (1 << 20);
	bits.sh = (1 << 21);
	bits.spi = (1 << 22);
	bits.smx = (1 << 23);
	bits.sc = (1 << 24);
	bits.pa = (1 << 25);
	bits.db = (1 << 26);
	bits.cr = (1 << 27);
	bits.cb = (1 << 30);
	bits.gui = (1 << 31);

	// R600 has a different texture bit, and only R600 has the TC, CR, SMX bits
	if (fam < RV770) {
		bits.ta = (1 << 18);
	} else {
		bits.tc = 0;
		bits.cr = 0;
		bits.smx = 0;
	}
}
