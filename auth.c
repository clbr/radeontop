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

#ifdef ENABLE_XCB_DL
#include <dlfcn.h>

#define xcb_connect			DL_xcb_connect
#define xcb_connection_has_error	DL_xcb_connection_has_error
#define xcb_disconnect			DL_xcb_disconnect
#define xcb_get_setup			DL_xcb_get_setup
#define xcb_setup_roots_iterator	DL_xcb_setup_roots_iterator
#define xcb_dri2_authenticate		DL_xcb_dri2_authenticate
#define xcb_dri2_authenticate_reply	DL_xcb_dri2_authenticate_reply

static xcb_connection_t *(*DL_xcb_connect)(const char *, int *);
static int (*DL_xcb_connection_has_error)(xcb_connection_t *);
static void (*DL_xcb_disconnect)(xcb_connection_t *);
static const struct xcb_setup_t *(*DL_xcb_get_setup)(xcb_connection_t *);
static xcb_screen_iterator_t (*DL_xcb_setup_roots_iterator)(
	const xcb_setup_t *);
static xcb_dri2_authenticate_cookie_t (*DL_xcb_dri2_authenticate)(
	xcb_connection_t *, xcb_window_t, uint32_t);
static xcb_dri2_authenticate_reply_t *(*DL_xcb_dri2_authenticate_reply)(
	xcb_connection_t *, xcb_dri2_authenticate_cookie_t,
	xcb_generic_error_t **);

static void *dl_handle;

static int dl_load(void) {
	if (!(dl_handle = dlopen("libxcb-dri2.so.0", RTLD_NOW))) {
		fprintf(stderr, "%s\n", dlerror());
		return 1;
	}

#define DLSYM(sym) \
	if (!(DL_##sym = dlsym(dl_handle, #sym))) { \
		fprintf(stderr, "%s\n", dlerror()); \
		return 1; \
	}

	DLSYM(xcb_connect);
	DLSYM(xcb_connection_has_error);
	DLSYM(xcb_disconnect);
	DLSYM(xcb_get_setup);
	DLSYM(xcb_setup_roots_iterator);
	DLSYM(xcb_dri2_authenticate);
	DLSYM(xcb_dri2_authenticate_reply);
#undef DLSYM

	return 0;
}

static void dl_unload(void) {
	dlclose(dl_handle);
}
#endif

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
#ifdef ENABLE_XCB_DL
	if (dl_load()) return;
#endif

	authenticate_drm_xcb(magic);

#ifdef ENABLE_XCB_DL
	dl_unload();
#endif
#endif
}
