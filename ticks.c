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

	const unsigned int ticks = * ((unsigned int *) arg);

	struct bits_t res[2];

	// Save one second's worth of history
	struct bits_t *history = calloc(ticks, sizeof(struct bits_t));
	unsigned int cur = 0;

	const useconds_t sleeptime = 1e6 / ticks;

	while (1) {
		unsigned int stat = readgrbm();

		if (stat & bits.ee) puts("Event Engine busy");
		if (stat & bits.vc) puts("Vertex Cache busy");
		if (stat & bits.vgt)
			puts("Vertex Grouper and Tesselator busy");
		if (stat & bits.gui) puts("Graphics pipe busy");
		if (stat & bits.ta) puts("Texture Addresser busy");
		if (stat & bits.tc) puts("Texture Cache busy");
		if (stat & bits.sx) puts("Shader Export busy");
		if (stat & bits.sh) puts("Sequencer Instruction Cache busy");
		if (stat & bits.spi) puts("Shader Interpolator busy");
		if (stat & bits.smx) puts("Shader Memory Exchange busy");
		if (stat & bits.sc) puts("Scan Converter busy");
		if (stat & bits.pa) puts("Primitive Assembly busy");
		if (stat & bits.db) puts("Depth Block busy");
		if (stat & bits.cr) puts("Clip Rectangle busy");
		if (stat & bits.cb) puts("Color Block busy");


		usleep(sleeptime);
		cur++;
		cur %= ticks;
	}

	return NULL;
}

void collect(unsigned int *ticks) {

	// Start a thread collecting data
	pthread_t tid;
	pthread_attr_t attr;

	// We don't care to join this thread
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

	pthread_create(&tid, &attr, collector, ticks);
}
