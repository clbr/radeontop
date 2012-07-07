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

struct bits_t *results = NULL;

static void collector(void *arg) {

	const unsigned int ticks = *(unsigned int *) arg;

	struct bits_t res[2];

	// Save one second's worth of history
	struct bits_t *history = calloc(ticks, sizeof(struct bits_t));
	unsigned int cur = 0;

	while (1) {
		unsigned int stat = readgrbm();
	}
}

void collect(const unsigned int ticks) {

	// Start a thread collecting data
}
