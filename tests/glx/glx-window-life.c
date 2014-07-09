/*
 * Copyright 2011 Red Hat, Inc.
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
 * Authors:
 *      Adam Jackson <ajax@redhat.com>
 *
 */

/** @file glx-window-life.c
 *
 * Test the lifetime rules for GLXWindows.  Windows are not refcounted, so
 * destroying a Window should destroy the GLXWindow.
 */

#include "piglit-util-gl.h"
#include "piglit-glx-util.h"

int piglit_width = 50, piglit_height = 50;
static Display *dpy;
static XVisualInfo *visinfo;
int errbase, evbase;

int pass = 1;

static int
expect_no_error(Display *dpy, XErrorEvent *err)
{
	pass = 0;
	return 0;
}

static int
expect_glxbadwindow(Display *dpy, XErrorEvent *err)
{
	if (piglit_glx_get_error(dpy, err) != GLXBadWindow)
		pass = 0;
	return 0;
}

int
main(int argc, char **argv)
{
	Window w;
	GLXWindow g;
	GLXFBConfig fbc;
	XWindowAttributes xwa;

	dpy = XOpenDisplay(NULL);
	if (dpy == NULL) {
		fprintf(stderr, "couldn't open display\n");
		piglit_report_result(PIGLIT_FAIL);
	}

        piglit_glx_get_error(dpy, NULL);
        piglit_require_glx_version(dpy, 1, 3);

	visinfo = piglit_get_glx_visual(dpy);
        fbc = piglit_glx_get_fbconfig_for_visinfo(dpy, visinfo);
        if (fbc == None) {
		fprintf(stderr, "No fbconfig available\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	/*
	 * GLX teardown before X teardown is legal, and should not destroy
	 * the X window.
	 */
	XSetErrorHandler(expect_no_error);

        w = piglit_get_glx_window(dpy, visinfo);
	g = glXCreateWindow(dpy, fbc, w, NULL);
	glXDestroyWindow(dpy, g);
	XGetWindowAttributes(dpy, w, &xwa); /* check w still exists */
	XDestroyWindow(dpy, w);

	XSync(dpy, 0);

	/*
	 * X teardown before GLX teardown is legal, and should destroy the
	 * GLX window.
	 */
	XSetErrorHandler(expect_glxbadwindow);
	w = piglit_get_glx_window(dpy, visinfo);
	g = glXCreateWindow(dpy, fbc, w, NULL);
	XDestroyWindow(dpy, w);
	glXDestroyWindow(dpy, g); /* should throw GLXBadWindow */

	XSync(dpy, 0);

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);

	return 0;
}
