/*
 * Copyright 2011 Red Hat, Inc.
 * Copyright 2011 Intel Corporation.
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

/** @file glx-swap-pixmap-bad.c
 *
 * Test that glXSwapbuffer() on a non-GLXDrawable throws GLXBadDrawable.
 *
 * From the GLX 1.4 specification page 34 (page 40 of the PDF):
 *
 *	If draw is not a valid GLX drawable, glXSwapBuffers generates a
 *	GLXBadDrawable error.
 */

#include "piglit-util-gl.h"
#include "piglit-glx-util.h"

int piglit_width = 50, piglit_height = 50;
static Display *dpy;
static XVisualInfo *visinfo;

int pass = 1;

static int
handler(Display *dpy, XErrorEvent *err)
{
    if (piglit_glx_get_error(dpy, err) != GLXBadDrawable)
	pass = 0;
    return 0;
}

int
main(int argc, char **argv)
{
	Pixmap p;
	GLXPixmap g;
	GLXContext ctx;

	dpy = XOpenDisplay(NULL);
	if (dpy == NULL) {
		fprintf(stderr, "couldn't open display\n");
		piglit_report_result(PIGLIT_FAIL);
	}

        piglit_glx_get_error(dpy, NULL);
	piglit_require_glx_version(dpy, 1, 3);

	visinfo = piglit_get_glx_visual(dpy);
	p = XCreatePixmap(dpy, DefaultRootWindow(dpy), piglit_width,
			  piglit_height, visinfo->depth);

	g = glXCreateGLXPixmap(dpy, visinfo, p);

	ctx = piglit_get_glx_context(dpy, visinfo);
	glXMakeCurrent(dpy, g, ctx);
	piglit_dispatch_default_init(PIGLIT_DISPATCH_GL);

	/* Clear to green */
	glClearColor(0.0, 1.0, 0.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT);

	/* Noop */
	XSetErrorHandler(handler);
	glXSwapBuffers(dpy, p);

	/* We want to actually catch any X error that leaks through as
	 * a result of glXSwapBuffers() before we go saying "pass" or
	 * "fail".
	 */
	XSync(dpy, False);

	glXDestroyPixmap(dpy, g);

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);

	return 0;
}
