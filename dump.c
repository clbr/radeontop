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

void dumpdata(const unsigned int ticks, const char file[], const unsigned int limit) {

	printf(_("Dumping to %s, "), file);

	if (limit)
		printf(_("line limit %u.\n"), limit);
	else
		puts(_("until termination."));

	FILE *f = NULL;
	if (file[0] == '-')
		f = stdout;
	else
		f = fopen(file, "a");

	if (!f)
		die(_("Can't open file for writing."));

	// This does not need to be atomic. A delay here is acceptable.
	while(!results)
		usleep(16000);

	// Action
	unsigned int count;

	for (count = limit; !limit || count; count--) {
	}
}
