/*
 * Copyright © 2012 Intel Corporation
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

#ifndef PIGLIT_HAS_X11
#error "Cannot build piglit_x11_framework.c without PIGLIT_HAS_X11"
#endif

#include <piglit/gl_wrap.h>

/* If building for a GLES API, <piglit/gl_wrap.h> may include the GLES
 * headers, such as <GLES2/gl2.h>. Below, <waffle_glx.h> transitively includes
 * <GL/gl.h>, which defines some of the same symbols found in <GLES2/gl2.h>.
 * We define the include guards below in order to prevent <GL/gl.h> and
 * related headers from being included and causing compilation failure due to
 * symbol redefinitions.
 */
#define __gl_h_
#define __gltypes_h_
#define __glext_h_

#include <assert.h>
#include <unistd.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>

#ifdef PIGLIT_HAS_GLX
#	include <waffle_glx.h>
#endif

#ifdef PIGLIT_HAS_EGL
#	include <waffle_x11_egl.h>
#endif

#include "piglit-util-gl.h"

#include "piglit_gl_framework.h"
#include "piglit_x11_framework.h"

struct piglit_x11_framework {
	struct piglit_winsys_framework winsys_fw;

	Display *display;
	Window window;
};

struct piglit_x11_framework*
piglit_x11_framework(struct piglit_gl_framework *gl_fw)
{
	return (struct piglit_x11_framework*) gl_fw;
}

static void
get_window_size(struct piglit_x11_framework *x11_fw)
{
	unsigned width, height;

	Window wjunk;
	int ijunk;
	unsigned ujunk;

	XGetGeometry(x11_fw->display, x11_fw->window,
		     &wjunk, &ijunk, &ijunk,
		     &width, &height, &ujunk, &ujunk);

	piglit_width = width;
	piglit_height = height;
}

static void
process_next_event(struct piglit_x11_framework *x11_fw)
{
	struct piglit_winsys_framework *winsys_fw = &x11_fw->winsys_fw;
	const struct piglit_gl_test_config *test_config = winsys_fw->wfl_fw.gl_fw.test_config;

	Display *dpy = x11_fw->display;
	XEvent event;

	XNextEvent(dpy, &event);

	switch (event.type) {
	case Expose:
		get_window_size(x11_fw);
		winsys_fw->need_redisplay = true;
		break;
	case ConfigureNotify:
		get_window_size(x11_fw);
		if (winsys_fw->user_reshape_func)
			winsys_fw->user_reshape_func(event.xconfigure.width,
			                             event.xconfigure.height);
		winsys_fw->need_redisplay = true;
		break;
	case KeyPress: {
		char buffer[1];
		KeySym sym;
		int n;

		n = XLookupString(&event.xkey,
				  buffer,
				  sizeof(buffer), &sym, NULL);

		if (n > 0 && winsys_fw->user_keyboard_func)
			winsys_fw->user_keyboard_func(buffer[0],
			                              event.xkey.x,
			                              event.xkey.y);
		winsys_fw->need_redisplay = true;
		break;
	}
	default:
		break;
	}

	if (winsys_fw->need_redisplay) {
		enum piglit_result result = PIGLIT_PASS;
		if (test_config->display)
			result = test_config->display();
		if (piglit_automatic)
			piglit_report_result(result);
		winsys_fw->need_redisplay = false;
	}
}

static void
enter_event_loop(struct piglit_winsys_framework *winsys_fw)
{
	struct piglit_x11_framework *x11_fw = (void*) winsys_fw;

	assert(x11_fw->display != NULL);
	assert(x11_fw->window != 0);

	while (true)
		process_next_event(x11_fw);
}

static void
get_native(struct piglit_x11_framework *x11_fw)
{
	struct piglit_wfl_framework *wfl_fw = &x11_fw->winsys_fw.wfl_fw;
	union waffle_native_window *n_window = waffle_window_get_native(wfl_fw->window);

	switch (wfl_fw->platform) {
#ifdef PIGLIT_HAS_GLX
	case WAFFLE_PLATFORM_GLX:
		x11_fw->display = n_window->glx->xlib_display;
		x11_fw->window = n_window->glx->xlib_window;
		break;
#endif
#ifdef PIGLIT_HAS_EGL
	case WAFFLE_PLATFORM_X11_EGL:
		x11_fw->display = n_window->x11_egl->display.xlib_display;
		x11_fw->window = n_window->x11_egl->xlib_window;
		break;
#endif
	default:
		assert(0);
		break;
	}

	free(n_window);
}

static void
show_window(struct piglit_winsys_framework *winsys_fw)
{
	struct piglit_x11_framework *x11_fw = piglit_x11_framework(&winsys_fw->wfl_fw.gl_fw);
	struct piglit_wfl_framework *wfl_fw = &winsys_fw->wfl_fw;
	XWMHints *wm_hints;

	get_native(x11_fw);

	if (piglit_automatic) {
		/* Prevent the window from grabbing input. */
		wm_hints = XAllocWMHints();
		wm_hints->flags |= InputHint;
		wm_hints->input = False;
		XSetWMHints(x11_fw->display, x11_fw->window, wm_hints);
		XFree(wm_hints);
	}

	waffle_window_show(wfl_fw->window);
}

static void
destroy(struct piglit_gl_framework *gl_fw)
{
	struct piglit_x11_framework *x11_fw = piglit_x11_framework(gl_fw);

	if (x11_fw == NULL)
		return;

	piglit_winsys_framework_teardown(&x11_fw->winsys_fw);
	free(x11_fw);
}

struct piglit_gl_framework*
piglit_x11_framework_create(const struct piglit_gl_test_config *test_config,
                            int32_t platform)
{
	struct piglit_x11_framework *x11_fw = NULL;
	struct piglit_winsys_framework *winsys_fw = NULL;
	struct piglit_gl_framework *gl_fw = NULL;
	bool ok = true;

	x11_fw = calloc(1, sizeof(*x11_fw));
	winsys_fw = &x11_fw->winsys_fw;
	gl_fw = &x11_fw->winsys_fw.wfl_fw.gl_fw;

	ok = piglit_winsys_framework_init(&x11_fw->winsys_fw,
	                                  test_config, platform);
	if (!ok)
		goto fail;

	winsys_fw->show_window = show_window;
	winsys_fw->enter_event_loop = enter_event_loop;
	gl_fw->destroy = destroy;

	return gl_fw;

fail:
	destroy(gl_fw);
	return NULL;
}
