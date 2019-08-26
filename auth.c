/*
    Copyright (C) 2016 Peter Wu <peter@lekensteyn.nl>

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
#include <xf86drm.h>
#ifdef ENABLE_XCB
#include <dlfcn.h>

typedef void (*auth_magic_func)(drm_magic_t magic);

static void call_authenticate_drm_xcb(drm_magic_t magic) {
	void *handle = dlopen("libradeontop_xcb.so", RTLD_NOW);
	if (!handle) {
		return;
	}

	auth_magic_func func = (auth_magic_func) dlsym(handle,
			"authenticate_drm_xcb");
	if (func) {
		func(magic);
	}
	dlclose(handle);
}
#endif

void authenticate_drm(int fd) {
	drm_magic_t magic;

	/* Obtain magic for our DRM client. */
	if (drmGetMagic(fd, &magic) < 0) {
		return;
	}

	/* Try self-authenticate (if we are somehow the master). */
	if (drmAuthMagic(fd, magic) == 0) {
		if (drmDropMaster(fd)) {
			perror(_("Failed to drop DRM master"));
			fprintf(stderr, _("\nWARNING: other DRM clients will crash on VT switch while radeontop is running!\npress ENTER to continue\n"));
			fgetc(stdin);
		}
		return;
	}

#ifdef ENABLE_XCB
	call_authenticate_drm_xcb(magic);
#endif
}
