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
#include <xcb/xcb.h>
#include <xcb/dri2.h>

/* Try to authenticate the DRM client with help from the X server. */
void authenticate_drm_xcb(drm_magic_t magic) {
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
