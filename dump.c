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

static unsigned char quit = 0;

static void sighandler(int sig) {

	switch (sig) {
		case SIGTERM:
		case SIGINT:
			quit = 1;
		break;
	}
}

void dumpdata(const unsigned int ticks, const char file[], const unsigned int limit,
		const unsigned char bus, const unsigned int dumpinterval) {

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

		fprintf(f, "bus %02x, ", bus);

		// Again, no need to protect these. Worst that happens is a slightly
		// wrong number.
		float k = 1.0f / ticks / dumpinterval;
		float ee = 100 * results->ee * k;
		float vgt = 100 * results->vgt * k;
		float gui = 100 * results->gui * k;
		float ta = 100 * results->ta * k;
		float tc = 100 * results->tc * k;
		float sx = 100 * results->sx * k;
		float sh = 100 * results->sh * k;
		float spi = 100 * results->spi * k;
		float smx = 100 * results->smx * k;
		float sc = 100 * results->sc * k;
		float pa = 100 * results->pa * k;
		float db = 100 * results->db * k;
		float cr = 100 * results->cr * k;
		float cb = 100 * results->cb * k;
		float vram = 100.0f * results->vram / vramsize;
		float vrammb = results->vram / 1024.0f / 1024.0f;
		float gtt = 100.0f * results->gtt / gttsize;
		float gttmb = results->gtt / 1024.0f / 1024.0f;
		float mclk = 100.0f * (results->mclk * k) / (mclk_max / 1e3f);
		float sclk = 100.0f * (results->sclk * k) / (sclk_max / 1e3f);
		float mclk_ghz = results->mclk * k / 1000.0f;
		float sclk_ghz = results->sclk * k / 1000.0f;

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

		if (bits.vram)
			fprintf(f, ", vram %.2f%% %.2fmb", vram, vrammb);

		if (bits.gtt)
			fprintf(f, ", gtt %.2f%% %.2fmb", gtt, gttmb);

		if (sclk_max != 0 && sclk > 0)
			fprintf(f, ", mclk %.2f%% %.3fghz, sclk %.2f%% %.3fghz",
					mclk, mclk_ghz, sclk, sclk_ghz);

		fprintf(f, "\n");
		fflush(f);

		// Did we get a termination signal?
		if (quit)
			break;

		// No sleeping on the last line.
		if (!limit || count > 1)
			sleep(dumpinterval);
	}

	fflush(f);

	if (f != stdout) {
		fsync(fileno(f));
		fclose(f);
	}
}
