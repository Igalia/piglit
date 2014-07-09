/*
 * Copyright © 2011 Intel Corporation
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

/** @file glx-swap-singlebuffer.c
 *
 * Test that glXSwapbuffer() on a single-buffered FBConfig is a noop.
 *
 * From the GLX 1.4 specification page 34 (page 40 of the PDF):
 *
 *     This operation is a no-op if draw was created with a
 *     non-double-buffered GLXFBConfig, or if draw is a GLXPixmap.
 */

#include "piglit-util-gl.h"
#include "piglit-glx-util.h"

int piglit_width = 50, piglit_height = 50;
static Display *dpy;
static Window win;
static XVisualInfo *visinfo;

enum piglit_result
draw(Display *dpy)
{
	bool pass = true;
	float green[] = {0.0, 1.0, 0.0, 0.0};
	GLXContext ctx;

	ctx = piglit_get_glx_context(dpy, visinfo);
	glXMakeCurrent(dpy, win, ctx);
	piglit_dispatch_default_init(PIGLIT_DISPATCH_GL);

	/* Clear to green */
	glClearColor(0.0, 1.0, 0.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT);

	/* Noop */
	glXSwapBuffers(dpy, win);

	/* We want to actually catch any X error that leaks through as
	 * a result of glXSwapBuffers() before we go saying "pass" or
	 * "fail".
	 */
	XSync(dpy, False);

	pass = piglit_probe_rect_rgba(0, 0, piglit_width, piglit_height, green);

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

XVisualInfo *
get_single_buffer_visual(Display *dpy)
{
	XVisualInfo *visinfo;
	int attrib[] = {
		GLX_RGBA,
		GLX_RED_SIZE, 1,
		GLX_GREEN_SIZE, 1,
		GLX_BLUE_SIZE, 1,
		GLX_ALPHA_SIZE, 1,
		None
	};
	int screen = DefaultScreen(dpy);

	visinfo = glXChooseVisual(dpy, screen, attrib);
	if (visinfo == NULL) {
		fprintf(stderr,
			"Couldn't get a single buffered, RGBA visual\n");
		piglit_report_result(PIGLIT_SKIP);
	}

	return visinfo;
}

int
main(int argc, char **argv)
{
	int i;

	for(i = 1; i < argc; ++i) {
		if (!strcmp(argv[i], "-auto"))
			piglit_automatic = 1;
		else
			fprintf(stderr, "Unknown option: %s\n", argv[i]);
	}

	dpy = XOpenDisplay(NULL);
	if (dpy == NULL) {
		fprintf(stderr, "couldn't open display\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	visinfo = get_single_buffer_visual(dpy);
	win = piglit_get_glx_window(dpy, visinfo);

	XMapWindow(dpy, win);

	piglit_glx_event_loop(dpy, draw);

	return 0;
}
