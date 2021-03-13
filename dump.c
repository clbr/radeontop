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
		const unsigned char bus, const unsigned int dumpinterval, const enum DumpFormat dumpformat) {

#ifdef ENABLE_NLS
	// This is a data format, so disable decimal point localization
	setlocale(LC_NUMERIC, "C");
#endif

	// Set up signals to exit gracefully when terminated
	struct sigaction sig;

	sig.sa_handler = sighandler;

	sigaction(SIGTERM, &sig, NULL);
	sigaction(SIGINT, &sig, NULL);

	// Check the file can be output to
	FILE *f = NULL;
	if (file[0] == '-')
		f = stdout;
	else {
		// Don't print info messages if dumping to stdout, will just confuse parsers
		printf(_("Dumping to %s, "), file);

		if (limit)
			printf(_("line limit %u.\n"), limit);
		else
			puts(_("until termination."));

		f = fopen(file, "a");
	}

	if (!f) {
		printf(_("Can't open file %s for writing."), file);
		die("");
	}

	// This does not need to be atomic. A delay here is acceptable.
	while(!results)
		usleep(16000);

	// Action
	unsigned int count;

	for (count = limit; !limit || count; count--) {
		struct timeval t;
		gettimeofday(&t, NULL);
		char timestamp[17];
		sprintf(timestamp, "%llu.%llu", (unsigned long long) t.tv_sec, (unsigned long long) t.tv_usec);

		// Again, no need to protect these. Worst that happens is a slightly
		// wrong number.
		float k = 1.0f / ticks / dumpinterval;
		float ee = results->ee * k;
		float vgt = results->vgt * k;
		float gui = results->gui * k;
		float ta = results->ta * k;
		float tc = results->tc * k;
		float sx = results->sx * k;
		float sh = results->sh * k;
		float spi = results->spi * k;
		float smx = results->smx * k;
		float sc = results->sc * k;
		float pa = results->pa * k;
		float db = results->db * k;
		float cr = results->cr * k;
		float cb = results->cb * k;
		float vram_max_b = vramsize;
		float vram_b = results->vram;
		float vram = vram_b / vram_max_b;
		float gtt_max_b = gttsize;
		float gtt_b = results->gtt;
		float gtt = gtt_b / gtt_max_b;
		float mclk_max_hz = mclk_max / 1e3f;
		float mclk_hz = results->mclk * k;
		float mclk = mclk_hz / mclk_max_hz;
		float sclk_max_hz = sclk_max / 1e3f;
		float sclk_hz = results->sclk * k;
		float sclk = sclk_hz / sclk_max_hz;

		switch (dumpformat) {
			case Custom:
				fprintf(f, "%s: ", timestamp);

				fprintf(f, "bus %02x, ", bus);

				fprintf(f, "gpu %.2f%%, ", 100 * gui);
				fprintf(f, "ee %.2f%%, ", 100 * ee);
				fprintf(f, "vgt %.2f%%, ", 100 * vgt);
				fprintf(f, "ta %.2f%%, ", 100 * ta);

				if (bits.tc)
					fprintf(f, "tc %.2f%%, ", 100 * tc);

				fprintf(f, "sx %.2f%%, ", 100 * sx);
				fprintf(f, "sh %.2f%%, ", 100 * sh);
				fprintf(f, "spi %.2f%%, ", 100 * spi);

				if (bits.smx)
					fprintf(f, "smx %.2f%%, ", 100 * smx);

				if (bits.cr)
					fprintf(f, "cr %.2f%%, ", 100 * cr);

				fprintf(f, "sc %.2f%%, ", 100 * sc);
				fprintf(f, "pa %.2f%%, ", 100 * pa);
				fprintf(f, "db %.2f%%, ", 100 * db);
				fprintf(f, "cb %.2f%%", 100 * cb);

				if (bits.vram)
					fprintf(f, ", vram %.2f%% %.2fmb %.2fmb", 100.0f * vram, vram_b / 1024.0f / 1024.0f, vram_max_b / 1024.0f / 1024.0f);

				if (bits.gtt)
					fprintf(f, ", gtt %.2f%% %.2fmb %.2fmb", 100.0f * gtt, gtt_b / 1024.0f / 1024.0f, gtt_max_b / 1024.0f / 1024.0f);

				if (sclk_max != 0 && sclk > 0)
					fprintf(f, ", mclk %.2f%% %.3fghz %.3fghz, sclk %.2f%% %.3fghz %.3fghz",
							100.0f * mclk, mclk_hz / 1000.0f, mclk_max_hz / 1000.0f, 100.0f * sclk, sclk_hz / 1000.0f, sclk_max_hz / 1000.0f);
			break;
			case Json:
				fprintf(f, "{\"timestamp\": %s", timestamp);

				fprintf(f, ", \"bus\": %u", bus);

				fprintf(f, ", \"gpu\": %f", gui);
				fprintf(f, ", \"ee\": %f", ee);
				fprintf(f, ", \"vgt\": %f", vgt);
				fprintf(f, ", \"ta\": %f", ta);

				if (bits.tc)
					fprintf(f, ", \"tc\": %f", tc);

				fprintf(f, ", \"sx\": %f", sx);
				fprintf(f, ", \"sh\": %f", sh);
				fprintf(f, ", \"spi\": %f", spi);

				if (bits.smx)
					fprintf(f, ", \"smx\": %f", smx);

				if (bits.cr)
					fprintf(f, ", \"cr\": %f", cr);

				fprintf(f, ", \"sc\": %f", sc);
				fprintf(f, ", \"pa\": %f", pa);
				fprintf(f, ", \"db\": %f", db);
				fprintf(f, ", \"cb\": %f", cb);

				if (bits.vram)
					fprintf(f, ", \"vram\": {\"used_per\": %f, \"used_b\": %.0f, \"max_b\": %.0f}", vram, vram_b, vram_max_b);

				if (bits.gtt)
					fprintf(f, ", \"gtt\": {\"used_per\": %f, \"used_b\": %.0f, \"max_b\": %.0f}", gtt, gtt_b, gtt_max_b);

				if (sclk_max != 0 && sclk > 0)
					fprintf(f, ", \"mclk\": {\"used_per\": %f, \"used_hz\": %f, \"max_hz\": %f}, \"sclk\": {\"used_per\": %f, \"used_hz\": %f, \"max_hz\": %f}",
							mclk, mclk_hz, mclk_max_hz, sclk, sclk_hz, sclk_max_hz);

				fprintf(f, "}");
			break;
		}

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
