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
#include <ncurses.h>
#include <stdarg.h>

static void printcenter(const unsigned int y, const unsigned int width,
				const char * const fmt, ...) {

	char *ptr;
	va_list ap;
	va_start(ap, fmt);

	const unsigned int len = vasprintf(&ptr, fmt, ap);

	unsigned x = (width - len)/2;
	if (len > width) x = 0;

	mvprintw(y, x, "%s", ptr);

	va_end(ap);
	free(ptr);
}

static void printright(const unsigned int y, const unsigned int width,
				const char * const fmt, ...) {

	char *ptr;
	va_list ap;
	va_start(ap, fmt);

	const unsigned int len = vasprintf(&ptr, fmt, ap);

	unsigned x = (width - len);
	if (len > width) x = 0;

	mvprintw(y, x, "%s", ptr);

	va_end(ap);
	free(ptr);
}

void present(const unsigned int ticks, const char card[], const unsigned int color) {

	// This does not need to be atomic. A delay here is acceptable.
	while(!results)
		usleep(16000);

	initscr();
	noecho();
	halfdelay(10);
	curs_set(0);
	clear();


	unsigned int w, h;
	getmaxyx(stdscr, h, w);

	const unsigned int hw = w/2;

	attron(A_REVERSE);
	mvhline(0, 0, ' ', w);
	printcenter(0, w, _("radeontop %s, running on %s, %u samples/sec"),
			VERSION, card, 	ticks);
	attroff(A_REVERSE);

	while(1) {

		move(2, 0);
		clrtobot();

		// Again, no need to protect these. Worst that happens is a slightly
		// wrong number.
		float ee = 100.0 * (float) results->ee / ticks;
		float vc = 100.0 * (float) results->vc / ticks;
		float vgt = 100.0 * (float) results->vgt / ticks;
		float gui = 100.0 * (float) results->gui / ticks;
		float ta = 100.0 * (float) results->ta / ticks;
		float tc = 100.0 * (float) results->tc / ticks;
		float sx = 100.0 * (float) results->sx / ticks;
		float sh = 100.0 * (float) results->sh / ticks;
		float spi = 100.0 * (float) results->spi / ticks;
		float smx = 100.0 * (float) results->smx / ticks;
		float sc = 100.0 * (float) results->sc / ticks;
		float pa = 100.0 * (float) results->pa / ticks;
		float db = 100.0 * (float) results->db / ticks;
		float cr = 100.0 * (float) results->cr / ticks;
		float cb = 100.0 * (float) results->cb / ticks;

		mvhline(3, 0, ACS_HLINE, w);
		mvvline(1, (w/2) + 1, ACS_VLINE, h);
		mvaddch(3, (w/2) + 1, ACS_PLUS);

		printright(2, hw, _("Graphics pipe %.2f%%"), gui);

		unsigned int start = 4;

		printright(start++, hw, _("Event Engine %.2f%%"), ee);
		printright(start++, hw, _("Vertex Cache %.2f%%"), vc);
		printright(start++, hw, _("Vertex Grouper + Tesselator %.2f%%"), vgt);
		printright(start++, hw, _("Texture Addresser %.2f%%"), ta);
		printright(start++, hw, _("Texture Cache %.2f%%"), tc);
		printright(start++, hw, _("Shader Export %.2f%%"), sx);
		printright(start++, hw, _("Sequencer Instruction Cache %.2f%%"), sh);
		printright(start++, hw, _("Shader Interpolator %.2f%%"), spi);
		printright(start++, hw, _("Shader Memory Exchange %.2f%%"), smx);
		printright(start++, hw, _("Scan Converter %.2f%%"), sc);
		printright(start++, hw, _("Primitive Assembly %.2f%%"), pa);
		printright(start++, hw, _("Depth Block %.2f%%"), db);
		printright(start++, hw, _("Color Block %.2f%%"), cb);
		printright(start++, hw, _("Clip Rectangle %.2f%%"), cr);

		refresh();

		int c = getch();
		if (c == 'q') break;
	}

	endwin();
}
