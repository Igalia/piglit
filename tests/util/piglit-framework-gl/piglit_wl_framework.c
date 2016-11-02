/*
 * Copyright © 2012 Intel Corporation
 * Copyright © 2008 Kristian Høgsberg
 * Copyright © 2012-2013 Collabora, Ltd.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include <assert.h>
#include <stdlib.h>
#include <unistd.h>

#include "piglit-util-gl.h"
#include "piglit_wl_framework.h"

#include <wayland-client.h>
#include <waffle_wayland.h>
#include <xkbcommon/xkbcommon.h>
#include <sys/mman.h>

struct piglit_wl_framework {
        struct piglit_winsys_framework winsys_fw;

	struct wl_display *dpy;
	struct wl_registry *registry;

	struct wl_seat *seat;
	struct wl_keyboard *keyboard;

	struct {
		struct xkb_context *context;
		struct xkb_keymap *keymap;
		struct xkb_state *state;
	} xkb;
};

/* Most of the code here taken from window.c in Weston source code. */
static void keymap(void *data,
	struct wl_keyboard *wl_keyboard,
	uint32_t format,
	int32_t fd,
	uint32_t size)
{
	struct piglit_wl_framework *wl_fw =
		(struct piglit_wl_framework *) data;
	struct xkb_keymap *keymap;
	struct xkb_state *state;
	char *locale;
	char *map_str;

	if (!data || format != WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1) {
		close(fd);
		return;
	}

	map_str = mmap(NULL, size, PROT_READ, MAP_SHARED, fd, 0);
	if (map_str == MAP_FAILED) {
		close(fd);
		return;
	}

	/* Set up XKB keymap */
	keymap = xkb_keymap_new_from_string(wl_fw->xkb.context,
		map_str,
		XKB_KEYMAP_FORMAT_TEXT_V1,
		0);

	munmap(map_str, size);
	close(fd);

	if (!keymap) {
		fprintf(stderr, "failed to compile keymap\n");
		return;
	}

	/* Set up XKB state */
	state = xkb_state_new(keymap);
	if (!state) {
		fprintf(stderr, "failed to create XKB state\n");
		xkb_keymap_unref(keymap);
		return;
	}

	/* Look up the preferred locale, falling back to "C" as default */
	if (!(locale = getenv("LC_ALL")))
		if (!(locale = getenv("LC_CTYPE")))
			if (!(locale = getenv("LANG")))
				locale = "C";

	xkb_keymap_unref(wl_fw->xkb.keymap);
	xkb_state_unref(wl_fw->xkb.state);
	wl_fw->xkb.keymap = keymap;
	wl_fw->xkb.state = state;
}

static void enter(void *data,
	struct wl_keyboard *wl_keyboard,
	uint32_t serial,
	struct wl_surface *surface,
	struct wl_array *keys)
{
}

static void leave(void *data,
	struct wl_keyboard *wl_keyboard,
	uint32_t serial,
	struct wl_surface *surface)
{
}

static void key(void *data,
	struct wl_keyboard *wl_keyboard,
	uint32_t serial,
	uint32_t time,
	uint32_t key,
	uint32_t state)
{
	struct piglit_wl_framework *wl_fw =
		(struct piglit_wl_framework *) data;
	struct piglit_winsys_framework *winsys_fw = &wl_fw->winsys_fw;
	const struct piglit_gl_test_config *test_config =
		winsys_fw->wfl_fw.gl_fw.test_config;

	const xkb_keysym_t *syms;
	xkb_keysym_t sym;

	uint32_t code = key + 8;
	uint32_t num_syms;

	if (!wl_fw->xkb.state)
		return;

	num_syms = xkb_state_key_get_syms(wl_fw->xkb.state, code, &syms);
	sym = XKB_KEY_NoSymbol;
	if (num_syms == 1)
		sym = syms[0];

	winsys_fw->need_redisplay = true;

	if (state != WL_KEYBOARD_KEY_STATE_PRESSED)
		return;

	if (winsys_fw->user_keyboard_func)
		winsys_fw->user_keyboard_func(sym, 0, 0);

	if (winsys_fw->need_redisplay) {
		enum piglit_result result = PIGLIT_PASS;
		if (test_config->display)
			result = test_config->display();
		if (piglit_automatic)
			piglit_report_result(result);
		winsys_fw->need_redisplay = false;
	}
}

static void modifiers(void *data,
	struct wl_keyboard *wl_keyboard,
	uint32_t serial,
	uint32_t mods_depressed,
	uint32_t mods_latched,
	uint32_t mods_locked,
	uint32_t group)
{
}

static const struct wl_keyboard_listener keyboard_listener = {
	keymap,
	enter,
	leave,
	key,
	modifiers
};

static void
process_events(struct wl_display *dpy)
{
	while (1) {
		if (wl_display_dispatch(dpy) == -1)
			break;
	}
}

static void global(void *data,
	struct wl_registry *wl_registry,
	uint32_t name,
	const char *interface,
	uint32_t version)
{
	struct piglit_wl_framework *wl_fw =
		(struct piglit_wl_framework *) data;

	if (strcmp(interface, "wl_seat") != 0)
		return;

	wl_fw->seat = wl_registry_bind(wl_registry, name, &wl_seat_interface, 1);
	wl_fw->keyboard = wl_seat_get_keyboard(wl_fw->seat);

	if (wl_fw->keyboard)
		wl_keyboard_add_listener(wl_fw->keyboard, &keyboard_listener, data);
}

static void global_remove(void *data,
	struct wl_registry *wl_registry,
	uint32_t name)
{
}

static const struct wl_registry_listener registry_listener = {
	global,
	global_remove
};

static void
enter_event_loop(struct piglit_winsys_framework *winsys_fw)
{
	struct piglit_wl_framework *wl_fw =
		(struct piglit_wl_framework *) winsys_fw;
	const struct piglit_gl_test_config *test_config =
		winsys_fw->wfl_fw.gl_fw.test_config;
	enum piglit_result result = PIGLIT_PASS;

	/* The Wayland window fails to appear on the first call to
	 * swapBuffers (which occured in display_cb above). This is
	 * likely due to swapBuffers being called before receiving an
	 * expose event. Until piglit has proper Wayland support,
	 * redraw as a workaround.
	 */
	if (test_config->display)
		result = test_config->display();

	/* Do not proceed to event loop in case of piglit_automatic. */
	if (piglit_automatic)
		piglit_report_result(result);

	process_events(wl_fw->dpy);
}

static void
show_window(struct piglit_winsys_framework *winsys_fw)
{
	waffle_window_show(winsys_fw->wfl_fw.window);
}

static void
destroy(struct piglit_gl_framework *gl_fw)
{
	struct piglit_wl_framework *wl_fw =
		(struct piglit_wl_framework *) gl_fw;

	if (wl_fw == NULL)
		return;

	xkb_keymap_unref(wl_fw->xkb.keymap);
	xkb_state_unref(wl_fw->xkb.state);
	xkb_context_unref(wl_fw->xkb.context);

	wl_registry_destroy(wl_fw->registry);

	piglit_winsys_framework_teardown(&wl_fw->winsys_fw);
	free(wl_fw);
}

struct piglit_gl_framework*
piglit_wl_framework_create(const struct piglit_gl_test_config *test_config)
{
	struct piglit_wl_framework *wl_fw = NULL;
	struct piglit_winsys_framework *winsys_fw = NULL;
	struct piglit_gl_framework *gl_fw = NULL;
	bool ok = true;

	wl_fw = calloc(1, sizeof(*wl_fw));
	winsys_fw = &wl_fw->winsys_fw;
	gl_fw = &wl_fw->winsys_fw.wfl_fw.gl_fw;

        ok = piglit_winsys_framework_init(&wl_fw->winsys_fw,
                                          test_config, WAFFLE_PLATFORM_WAYLAND);
	if (!ok)
		goto fail;

	wl_fw->xkb.context = xkb_context_new(0);
	wl_fw->xkb.keymap = NULL;
	wl_fw->xkb.state = NULL;

	if (!wl_fw->xkb.context)
		goto fail;

	union waffle_native_window *n_window =
		waffle_window_get_native(winsys_fw->wfl_fw.window);

	wl_fw->dpy = n_window->wayland->display.wl_display;
	wl_fw->registry = wl_display_get_registry(wl_fw->dpy);

	wl_registry_add_listener(wl_fw->registry, &registry_listener, wl_fw);

	winsys_fw->show_window = show_window;
	winsys_fw->enter_event_loop = enter_event_loop;
	gl_fw->destroy = destroy;

	return gl_fw;

fail:
	destroy(gl_fw);
	return NULL;
}
