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

	while(!results)
		usleep(16000);

	initscr();
	noecho();
	halfdelay(10);
	curs_set(0);

	unsigned int w, h;
	getmaxyx(stdscr, h, w);

	const unsigned int hw = w/2;

	while(1) {

		clear();
		attron(A_REVERSE);
		hline(' ', w);
		printcenter(0, w, _("radeontop %s, running on %s, %u samples/sec"),
			VERSION, card, 	ticks);
		attroff(A_REVERSE);

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

		printright(2, hw, _("Event Engine %.2f%%\n"), ee);
		printright(3, hw, _("Vertex Cache %.2f%%\n"), vc);
		printright(4, hw, _("Vertex Grouper + Tesselator %.2f%%\n"), vgt);
		printright(5, hw, _("Graphics pipe %.2f%%\n"), gui);
		printright(6, hw, _("Texture Addresser %.2f%%\n"), ta);
		printright(7, hw, _("Texture Cache %.2f%%\n"), tc);
		printright(8, hw, _("Shader Export %.2f%%\n"), sx);
		printright(9, hw, _("Sequencer Instruction Cache %.2f%%\n"), sh);
		printright(10, hw, _("Shader Interpolator %.2f%%\n"), spi);
		printright(11, hw, _("Shader Memory Exchange %.2f%%\n"), smx);
		printright(12, hw, _("Scan Converter %.2f%%\n"), sc);
		printright(13, hw, _("Primitive Assembly %.2f%%\n"), pa);
		printright(14, hw, _("Depth Block %.2f%%\n"), db);
		printright(15, hw, _("Clip Rectangle %.2f%%\n"), cr);
		printright(16, hw, _("Color Block %.2f%%\n"), cb);

		refresh();

		int c = getch();
		if (c == 'q') break;
	}

	endwin();
}
