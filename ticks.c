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
#include <pthread.h>

struct bits_t *results = NULL;

static void *collector(void *arg) {

	const unsigned int ticks = *(unsigned int *) arg;

	struct bits_t res[2];

	// Save one second's worth of history
	struct bits_t *history = calloc(ticks, sizeof(struct bits_t));
	unsigned int cur = 0;

	const useconds_t sleeptime = 1e6 / ticks;

	while (1) {
		unsigned int stat = readgrbm();

		usleep(sleeptime);
		cur++;
		cur %= ticks;
	}

	return NULL;
}

void collect(unsigned int ticks) {

	// Start a thread collecting data
	pthread_t tid;
	pthread_attr_t attr;

	// We don't care to join this thread
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

	pthread_create(&tid, &attr, collector, &ticks);
}
