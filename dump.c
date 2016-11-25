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

static unsigned char quit = 0;

static void sighandler(int sig) {

	switch (sig) {
		case SIGTERM:
		case SIGINT:
			quit = 1;
		break;
	}
}

void dumpdata(const unsigned int ticks, const char file[], const unsigned int limit) {

#ifdef ENABLE_NLS
	// This is a data format, so disable decimal point localization
	setlocale(LC_NUMERIC, "C");
#endif

	// Set up signals to exit gracefully when terminated
	struct sigaction sig;

	sig.sa_handler = sighandler;

	sigaction(SIGTERM, &sig, NULL);
	sigaction(SIGINT, &sig, NULL);

	printf(_("Dumping to %s, "), file);

	if (limit)
		printf(_("line limit %u.\n"), limit);
	else
		puts(_("until termination."));

	// Check the file can be output to
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

		struct timeval t;
		gettimeofday(&t, NULL);

		fprintf(f, "%llu.%llu: ", (unsigned long long) t.tv_sec,
				(unsigned long long) t.tv_usec);

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
		float uvd = 100.0 * (float) results->uvd / ticks;
		float vce0 = 100.0 * (float) results->vce0 /ticks;
		float vram = 100.0 * (float) results->vram / vramsize;
		float vrammb = results->vram / 1024.0f / 1024.0f;
		float gtt = 100.0 * (float) results->gtt / gttsize;
		float gttmb = results->gtt / 1024.0f / 1024.0f;

		fprintf(f, "gpu %.2f%%, ", gui);
		fprintf(f, "ee %.2f%%, ", ee);
		fprintf(f, "vgt %.2f%%, ", vgt);
		fprintf(f, "ta %.2f%%, ", ta);

		if (bits.tc)
			fprintf(f, "tc %.2f%%, ", tc);

		fprintf(f, "sx %.2f%%, ", sx);
		fprintf(f, "sh %.2f%%, ", sh);
		fprintf(f, "spi %.2f%%, ", spi);

		if (bits.smx)
			fprintf(f, "smx %.2f%%, ", smx);

		if (bits.cr)
			fprintf(f, "cr %.2f%%, ", cr);

		fprintf(f, "sc %.2f%%, ", sc);
		fprintf(f, "pa %.2f%%, ", pa);
		fprintf(f, "db %.2f%%, ", db);
		fprintf(f, "cb %.2f%%", cb);
		fprintf(f, "uvd %.2f%%", uvd);
		fprintf(f, "vce0 %.2f%%", vce0);

		if (bits.vram)
			fprintf(f, ", vram %.2f%% %.2fmb\n", vram, vrammb);
		else
			fprintf(f, "\n");

		if(bits.gtt)
			fprintf(f, ", gtt %.2f%% %.2fmb\n", gtt, gttmb);
		else
			fprintf(f, "\n");

		fflush(f);

		// Did we get a termination signal?
		if (quit)
			break;

		sleep(1);
	}

	fflush(f);

	if (f != stdout) {
		fsync(fileno(f));
		fclose(f);
	}
}
