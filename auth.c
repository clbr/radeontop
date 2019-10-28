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
#include <xcb/xcb.h>
#include <xcb/dri2.h>
#include <dlfcn.h>

/* Try to authenticate the DRM client with help from the X server. */
static void authenticate_drm_xcb(drm_magic_t magic) {
	xcb_connection_t *conn = xcb_connect(NULL, NULL);
	if (!conn) {
		return;
	}
	if (xcb_connection_has_error(conn)) {
		xcb_disconnect(conn);
		return;
	}

	xcb_screen_t *screen = xcb_setup_roots_iterator(xcb_get_setup(conn)).data;
	xcb_window_t window = screen->root;

	/* Authenticate our client via the X server using the magic. */
	xcb_dri2_authenticate_cookie_t auth_cookie =
		xcb_dri2_authenticate(conn, window, magic);
	xcb_dri2_authenticate_reply_t *auth_reply =
		xcb_dri2_authenticate_reply(conn, auth_cookie, NULL);
	free(auth_reply);

	xcb_disconnect(conn);
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
	authenticate_drm_xcb(magic);
#endif
}
