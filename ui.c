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

void present(const unsigned int ticks, const char card[]) {

	while(!results)
		usleep(16000);

	while(1) {

		system("clear");
		printf("radeontop %s, running on %s, %u samples/sec\n", VERSION, card, ticks);

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

		printf("Event Engine busy %.2f\n", ee);
		printf("Vertex Cache busy %.2f\n", vc);
		printf("Vertex Grouper and Tesselator busy %.2f\n", vgt);
		printf("Graphics pipe busy %.2f\n", gui);
		printf("Texture Addresser busy %.2f\n", ta);
		printf("Texture Cache busy %.2f\n", tc);
		printf("Shader Export busy %.2f\n", sx);
		printf("Sequencer Instruction Cache busy %.2f\n", sh);
		printf("Shader Interpolator busy %.2f\n", spi);
		printf("Shader Memory Exchange busy %.2f\n", smx);
		printf("Scan Converter busy %.2f\n", sc);
		printf("Primitive Assembly busy %.2f\n", pa);
		printf("Depth Block busy %.2f\n", db);
		printf("Clip Rectangle busy %.2f\n", cr);
		printf("Color Block busy %.2f\n", cb);

		int c = getchar();
		if (c == 'q') break;
	}
}
