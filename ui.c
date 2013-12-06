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
#include <ncurses.h>
#include <stdarg.h>

static void printcenter(const unsigned int y, const unsigned int width,
				const char * const fmt, ...) {

	char *ptr;
	va_list ap;
	va_start(ap, fmt);

#ifdef ENABLE_NLS
	vasprintf(&ptr, fmt, ap);
	const unsigned int len = mbstowcs(NULL, ptr, 0);
#else
	const unsigned int len = vasprintf(&ptr, fmt, ap);
#endif

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

#ifdef ENABLE_NLS
	vasprintf(&ptr, fmt, ap);
	const unsigned int len = mbstowcs(NULL, ptr, 0);
#else
	const unsigned int len = vasprintf(&ptr, fmt, ap);
#endif

	unsigned x = (width - len);
	if (len > width) x = 0;

	mvprintw(y, x, "%s", ptr);

	va_end(ap);
	free(ptr);
}

static void percentage(const unsigned int y, const unsigned int w, const float p) {

	const unsigned int x = (w/2) + 2;
	unsigned int len = w - x - 1;

	len = len * (p / 100.0);

	attron(A_REVERSE);
	mvhline(y, x, ' ', len);
	attroff(A_REVERSE);
}

void present(const unsigned int ticks, const char card[], const unsigned int color) {

	printf(_("Collecting data, please wait....\n"));

	// This does not need to be atomic. A delay here is acceptable.
	while(!results)
		usleep(16000);

	initscr();
	noecho();
	halfdelay(10);
	curs_set(0);
	clear();
	if(color) {
		start_color();

		init_pair(1, COLOR_GREEN, COLOR_BLACK);
		init_pair(2, COLOR_RED, COLOR_BLACK);
		init_pair(3, COLOR_CYAN, COLOR_BLACK);
		init_pair(4, COLOR_MAGENTA, COLOR_BLACK);
		init_pair(5, COLOR_YELLOW, COLOR_BLACK);
	}

	const unsigned int bigh = 23;

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

		if (color) attron(COLOR_PAIR(1));
		percentage(2, w, gui);
		printright(2, hw, _("Graphics pipe %6.2f%%"), gui);
		if (color) attroff(COLOR_PAIR(1));

		unsigned int start = 4;

		percentage(start, w, ee);
		printright(start++, hw, _("Event Engine %6.2f%%"), ee);

		// Enough height?
		if (h > bigh) start++;

		if (color) attron(COLOR_PAIR(2));
		percentage(start, w, vgt);
		printright(start++, hw, _("Vertex Grouper + Tesselator %6.2f%%"), vgt);
		if (color) attroff(COLOR_PAIR(2));

		// Enough height?
		if (h > bigh) start++;

		if (color) attron(COLOR_PAIR(3));
		percentage(start, w, ta);
		printright(start++, hw, _("Texture Addresser %6.2f%%"), ta);

		// This is only present on R600
		if (bits.tc) {
			percentage(start, w, tc);
			printright(start++, hw, _("Texture Cache %6.2f%%"), tc);
		}
		if (color) attroff(COLOR_PAIR(3));

		// Enough height?
		if (h > bigh) start++;

		if (color) attron(COLOR_PAIR(4));
		percentage(start, w, sx);
		printright(start++, hw, _("Shader Export %6.2f%%"), sx);

		percentage(start, w, sh);
		printright(start++, hw, _("Sequencer Instruction Cache %6.2f%%"), sh);

		percentage(start, w, spi);
		printright(start++, hw, _("Shader Interpolator %6.2f%%"), spi);

		// only on R600
		if (bits.smx) {
			percentage(start, w, smx);
			printright(start++, hw, _("Shader Memory Exchange %6.2f%%"), smx);
		}
		if (color) attroff(COLOR_PAIR(4));

		// Enough height?
		if (h > bigh) start++;

		percentage(start, w, sc);
		printright(start++, hw, _("Scan Converter %6.2f%%"), sc);

		percentage(start, w, pa);
		printright(start++, hw, _("Primitive Assembly %6.2f%%"), pa);

		// Enough height?
		if (h > bigh) start++;

		if (color) attron(COLOR_PAIR(5));
		percentage(start, w, db);
		printright(start++, hw, _("Depth Block %6.2f%%"), db);

		percentage(start, w, cb);
		printright(start++, hw, _("Color Block %6.2f%%"), cb);

		// Only present on R600
		if (bits.cr) {
			percentage(start, w, cr);
			printright(start++, hw, _("Clip Rectangle %6.2f%%"), cr);
		}
		if (color) attroff(COLOR_PAIR(5));

		refresh();

		int c = getch();
		if (c == 'q') break;
	}

	endwin();
}
