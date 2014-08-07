/*
 * Copyright © 2010 Intel Corporation
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
 *
 * Author: Kristian Høgsberg <krh@bitplanet.net>
 */

/**
 * \file egl-util.c
 * Common framework for EGL tests.
 *
 * \author Kristian Høgsberg <krh@bitplanet.net>
 */

#include <X11/XKBlib.h>
#include "piglit-util-gl.h"
#include "egl-util.h"

static int automatic;

int depth;

int
egl_probe_front_pixel_rgb(struct egl_state *state,
			  int x, int y, const float *expected)
{
	XImage *ximage = XGetImage(state->dpy, state->win,
				   x, state->height - y - 1, 1, 1, AllPlanes, ZPixmap);
	unsigned long pixel = XGetPixel(ximage, 0, 0);
	uint8_t *probe = (uint8_t *)&pixel;
	int pass = 1;
	int i;

	XDestroyImage(ximage);

	/* NB: XGetPixel returns a normalized BGRA, byte per
	 * component, pixel format */
	for(i = 0; i < 3; ++i) {
		if (fabs(probe[2 - i]/255.0 - expected[i]) > piglit_tolerance[i]) {
			pass = 0;
		}
	}

	if (pass)
		return 1;

	printf("Front Buffer Probe at (%i,%i)\n", x, y);
	printf("  Expected: %f %f %f %f\n", expected[0], expected[1], expected[2], expected[3]);
	printf("  Observed: %f %f %f %f\n", probe[0]/255.0, probe[1]/255.0, probe[2]/255.0, probe[3]/255.0);

	return 0;
}

void
egl_init_test(struct egl_test *test)
{
	static const char *no_extensions[] = { NULL };

	test->config_attribs = egl_default_attribs;
	test->draw = NULL;
	test->extensions = no_extensions;
	test->window_width = egl_default_window_width;
	test->window_height = egl_default_window_height;
	test->stop_on_failure = true;
}

EGLSurface
egl_util_create_pixmap(struct egl_state *state,
		       int width, int height, const EGLint *attribs)
{
	Pixmap pixmap;
	EGLSurface surf;

	pixmap = XCreatePixmap(state->dpy, state->win,
			       width, height, state->depth);

	surf = eglCreatePixmapSurface(state->egl_dpy, state->cfg,
				      pixmap, attribs);

	return surf;
}

static enum piglit_result
create_window(struct egl_state *state)
{
	XSetWindowAttributes window_attr;
	XVisualInfo template, *vinfo;
	EGLint id;
	unsigned long mask;
	int screen = DefaultScreen(state->dpy);
	Window root_win = RootWindow(state->dpy, screen);
	int count;

	if (!eglGetConfigAttrib(state->egl_dpy,
				state->cfg, EGL_NATIVE_VISUAL_ID, &id)) {
		fprintf(stderr, "eglGetConfigAttrib() failed\n");
		return PIGLIT_FAIL;
	}

	template.visualid = id;
	vinfo = XGetVisualInfo(state->dpy, VisualIDMask, &template, &count);
	if (count != 1) {
		fprintf(stderr, "XGetVisualInfo() failed\n");
		if (vinfo)
			XFree(vinfo);
		return PIGLIT_FAIL;
	}

	state->depth = vinfo->depth;
	window_attr.background_pixel = 0;
	window_attr.border_pixel = 0;
	window_attr.colormap = XCreateColormap(state->dpy, root_win,
					       vinfo->visual, AllocNone);
	window_attr.event_mask =
		StructureNotifyMask | ExposureMask | KeyPressMask;
	mask = CWBackPixel | CWBorderPixel | CWColormap | CWEventMask;
	state->win = XCreateWindow(state->dpy, root_win, 0, 0,
				   state->width, state->height,
				   0, vinfo->depth, InputOutput,
				   vinfo->visual, mask, &window_attr);

	XMapWindow(state->dpy, state->win);

	XFree(vinfo);
	return PIGLIT_PASS;
}

static enum piglit_result
event_loop(struct egl_state *state, const struct egl_test *test)
{
	XEvent event;
	enum piglit_result result = PIGLIT_FAIL;

	while (1) {
		XNextEvent (state->dpy, &event);

		if (event.type == Expose) {
			result = test->draw(state);
			if (automatic || piglit_automatic)
				break;
		}

		if (event.type == KeyPress) {
			KeySym sym = XkbKeycodeToKeysym (state->dpy,
							 event.xkey.keycode,
							 0, 0);

			if (sym == XK_Escape || sym == XK_q || sym == XK_Q)
				break;
		}
	}

	return result;
}

static void
check_extensions(struct egl_state *state, const struct egl_test *test)
{
	const char *extensions;
	int i;

	extensions = eglQueryString(state->egl_dpy, EGL_EXTENSIONS);
	for (i = 0; test->extensions[i]; i++) {
		if (!strstr(extensions, test->extensions[i])) {
			fprintf(stderr, "missing extension %s\n",
				test->extensions[i]);
			piglit_report_result(PIGLIT_SKIP);
		}
	}
}

enum piglit_result
egl_util_run(const struct egl_test *test, int argc, char *argv[])
{
	struct egl_state state = { 0 };
	EGLint count;
	enum piglit_result result = PIGLIT_PASS;
	int i, dispatch_api, api_bit = EGL_OPENGL_BIT;

	EGLint ctxAttribsES[] = {
		EGL_CONTEXT_CLIENT_VERSION, 0,
		EGL_NONE
	};
	EGLint *ctxAttribs = NULL;

	for (i = 1; i < argc; ++i) {
		if (!strcmp(argv[i], "-auto"))
			automatic = 1;
		else
			fprintf(stderr, "Unknown option: %s\n", argv[i]);
	}

	state.dpy = XOpenDisplay(NULL);
	if (state.dpy == NULL) {
		fprintf(stderr, "couldn't open display\n");
		result = PIGLIT_FAIL;
		goto fail;
	}

	/* read api_bit if EGL_RENDERABLE_TYPE set in the attribs */
	for (count = 0; test->config_attribs[count] != EGL_NONE; count += 2) {
		if (test->config_attribs[count] == EGL_RENDERABLE_TYPE) {
			api_bit = test->config_attribs[count+1];
		}
	}

	/* bind chosen API and set ctxattribs if using ES */
	if (api_bit == EGL_OPENGL_BIT)
		eglBindAPI(EGL_OPENGL_API);
	else {
		eglBindAPI(EGL_OPENGL_ES_API);
		ctxAttribs = ctxAttribsES;
	}

	/* choose dispatch_api and set ctx version to ctxAttribs if using ES */
	switch (api_bit) {
	case EGL_OPENGL_ES_BIT:
		dispatch_api = PIGLIT_DISPATCH_ES1;
		ctxAttribsES[1] = 1;
		break;
	case EGL_OPENGL_ES2_BIT:
		dispatch_api = PIGLIT_DISPATCH_ES2;
		ctxAttribsES[1] = 2;
		break;
	default:
		dispatch_api = PIGLIT_DISPATCH_GL;
	}


	state.egl_dpy = eglGetDisplay(state.dpy);
	if (state.egl_dpy == EGL_NO_DISPLAY) {
		fprintf(stderr, "eglGetDisplay() failed\n");
		result = PIGLIT_FAIL;
		goto fail;
	}

	if (!eglInitialize(state.egl_dpy, &state.major, &state.minor)) {
		fprintf(stderr, "eglInitialize() failed\n");
		result = PIGLIT_FAIL;
		goto fail;
	}

	check_extensions(&state, test);

	if (!eglChooseConfig(state.egl_dpy, test->config_attribs, &state.cfg, 1, &count) ||
	    count == 0) {
		fprintf(stderr, "eglChooseConfig() failed\n");
		result = PIGLIT_FAIL;
		goto fail;
	}

	state.ctx = eglCreateContext(state.egl_dpy, state.cfg,
				     EGL_NO_CONTEXT, ctxAttribs);
	if (state.ctx == EGL_NO_CONTEXT) {
		fprintf(stderr, "eglCreateContext() failed\n");
		result = PIGLIT_FAIL;
		goto fail;
	}

	state.width = test->window_width;
	state.height = test->window_height;
	result = create_window(&state);
	if (result != PIGLIT_PASS)
		goto fail;

	state.surf = eglCreateWindowSurface(state.egl_dpy,
					    state.cfg, state.win, NULL);
	if (state.surf == EGL_NO_SURFACE) {
		fprintf(stderr, "eglCreateWindowSurface() failed\n");
		result = PIGLIT_FAIL;
		goto fail;
	}

	if (!eglMakeCurrent(state.egl_dpy,
			    state.surf, state.surf, state.ctx)) {
		fprintf(stderr, "eglMakeCurrent() failed\n");
		result = PIGLIT_FAIL;
		goto fail;
	}

	piglit_dispatch_default_init(dispatch_api);

	result = event_loop(&state, test);

fail:
	if (state.egl_dpy) {
		eglMakeCurrent(state.egl_dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
		eglDestroyContext(state.egl_dpy, state.ctx);
	}
	if (state.win)
		XDestroyWindow(state.dpy, state.win);
	if (state.egl_dpy)
		eglTerminate(state.egl_dpy);
	if (test->stop_on_failure)
		piglit_report_result(result);
	return result;
}
