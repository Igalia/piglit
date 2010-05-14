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

/** @file egl-nok-swap-region.c
 *
 * Test EGL_NOK_swap_region.
 */

#include "piglit-util.h"
#include "egl-util.h"

static int automatic;

static const EGLint attribs[] = {
	EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
	EGL_RED_SIZE, 1,
	EGL_GREEN_SIZE, 1,
	EGL_BLUE_SIZE, 1,
	EGL_DEPTH_SIZE, 1,
	EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
	EGL_NONE
};

static Window
create_window(Display *dpy, EGLDisplay egl_dpy,
	      EGLConfig cfg, int width, int height)
{
	XSetWindowAttributes window_attr;
	XVisualInfo template, *vinfo;
	EGLint id;
	unsigned long mask;
	int screen = DefaultScreen(dpy);
	Window root_win = RootWindow(dpy, screen);
	Window win;
	int count;

	if (!eglGetConfigAttrib(egl_dpy, cfg, EGL_NATIVE_VISUAL_ID, &id)) {
		fprintf(stderr, "eglGetConfigAttrib() failed\n");
		piglit_report_result(PIGLIT_FAILURE);
	}

	template.visualid = id;
	vinfo = XGetVisualInfo(dpy, VisualIDMask, &template, &count);
	if (count != 1) {
		fprintf(stderr, "XGetVisualInfo() failed\n");
		piglit_report_result(PIGLIT_FAILURE);
	}

	window_attr.background_pixel = 0;
	window_attr.border_pixel = 0;
	window_attr.colormap =
		XCreateColormap(dpy, root_win, vinfo->visual, AllocNone);
	window_attr.event_mask =
		StructureNotifyMask | ExposureMask | KeyPressMask;
	mask = CWBackPixel | CWBorderPixel | CWColormap | CWEventMask;
	win = XCreateWindow(dpy, root_win, 0, 0,
			    width, height,
			    0, vinfo->depth, InputOutput,
			    vinfo->visual, mask, &window_attr);

	XMapWindow(dpy, win);

	XFree(vinfo);

	return win;
}

static enum piglit_result
event_loop(Display *dpy, EGLDisplay egl_dpy, EGLSurface surf,
	   const struct egl_test *test)
{
	XEvent event;
	enum piglit_result result = PIGLIT_FAILURE;

	while (1) {
		XNextEvent (dpy, &event);

		if (event.type == Expose) {
			result = test->draw(egl_dpy, surf);
			if (automatic)
				break;
		}

		if (event.type == KeyPress) {
			KeySym sym = XKeycodeToKeysym (dpy,
						       event.xkey.keycode, 0);

			if (sym == XK_Escape || sym == XK_q || sym == XK_Q)
				break;
		}
	}

	return result;
}

static void
check_extensions(EGLDisplay egl_dpy, const struct egl_test *test)
{
	const char *extensions;
	int i;

	extensions = eglQueryString(egl_dpy, EGL_EXTENSIONS);
	for (i = 0; test->extensions[i]; i++) {
		if (!strstr(extensions, test->extensions[i])) {
			fprintf(stderr, "missing extension %s\n",
				test->extensions[i]);
			piglit_report_result(PIGLIT_SKIP);
		}
	}
}

int
egl_run(const struct egl_test *test, int argc, char *argv[])
{
	Display *dpy;
	Window win;
	EGLDisplay egl_dpy;
	EGLConfig cfg;
	EGLContext ctx;
	EGLSurface surf;
	EGLint major, minor, count;
	enum piglit_result result;
	int i;

	for (i = 1; i < argc; ++i) {
		if (!strcmp(argv[i], "-auto"))
			automatic = 1;
		else
			fprintf(stderr, "Unknown option: %s\n", argv[i]);
	}

	dpy = XOpenDisplay(NULL);
	if (dpy == NULL) {
		fprintf(stderr, "couldn't open display\n");
		piglit_report_result(PIGLIT_FAILURE);
	}

	eglBindAPI(EGL_OPENGL_API);

	egl_dpy = eglGetDisplay(dpy);
	if (egl_dpy == EGL_NO_DISPLAY) {
		fprintf(stderr, "eglGetDisplay() failed\n");
		piglit_report_result(PIGLIT_FAILURE);
	}

	if (!eglInitialize(egl_dpy, &major, &minor)) {
		fprintf(stderr, "eglInitialize() failed\n");
		piglit_report_result(PIGLIT_FAILURE);
	}

	check_extensions(egl_dpy, test);

	if (!eglChooseConfig(egl_dpy, attribs, &cfg, 1, &count) ||
	    count == 0) {
		fprintf(stderr, "eglChooseConfig() failed\n");
		piglit_report_result(PIGLIT_FAILURE);
	}

	ctx = eglCreateContext(egl_dpy, cfg, EGL_NO_CONTEXT, NULL);
	if (ctx == EGL_NO_CONTEXT) {
		fprintf(stderr, "eglCreateContext() failed\n");
		piglit_report_result(PIGLIT_FAILURE);
	}

	win = create_window(dpy, egl_dpy, cfg, 300, 300);

	surf = eglCreateWindowSurface(egl_dpy, cfg, win, NULL);
	if (surf == EGL_NO_SURFACE) {
		fprintf(stderr, "eglCreateWindowSurface() failed\n");
		piglit_report_result(PIGLIT_FAILURE);
	}

	if (!eglMakeCurrent(egl_dpy, surf, surf, ctx)) {
		fprintf(stderr, "eglMakeCurrent() failed\n");
		piglit_report_result(PIGLIT_FAILURE);
	}

	result = event_loop(dpy, egl_dpy, surf, test);

	eglTerminate(egl_dpy);

	piglit_report_result(result);

	return EXIT_SUCCESS;
}
