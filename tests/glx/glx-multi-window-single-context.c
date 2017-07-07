/*
 * Copyright 2017 VMware, Inc.
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

/** @file glx-multi-window-single-context.c
 *
 * Exercise rendering to multiple windows with one context.
 */

#include <unistd.h>
#include "piglit-util-gl.h"
#include "piglit-glx-util.h"

#define MAX_WINDOWS 8

static Window win[MAX_WINDOWS];
static int num_windows = MAX_WINDOWS;
static Display *dpy;
static GLXContext ctx;

static const float colors[MAX_WINDOWS][4] = {
	{1, 0, 0, 1},
	{0, 1, 0, 1},
	{0, 0, 1, 1},
	{0, 1, 1, 1},
	{1, 0, 1, 1},
	{1, 1, 0, 1},
	{1, 1, 1, 1},
	{.5, .5, .5, 1},
};

int piglit_width = 50, piglit_height = 50;


enum piglit_result
draw(Display *dpy)
{
	int i;
	bool pass = true;

	/* draw colored quad in each window */
	for (i = 0; i < num_windows; i++) {
		glXMakeCurrent(dpy, win[i], ctx);

		glClear(GL_COLOR_BUFFER_BIT);
		glColor4fv(colors[i]);
		piglit_draw_rect(-1, -1, 2, 2);
	}

	/* probe windows */
	for (i = 0; i < num_windows; i++) {
		glXMakeCurrent(dpy, win[i], ctx);

		int p = piglit_probe_rect_rgb(0, 0, piglit_width, piglit_height,
					  colors[i]);

		glXSwapBuffers(dpy, win[i]);

		if (!p) {
			printf("Failed probe in window %d\n", i);
			pass = false;
		}
	}

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}


int
main(int argc, char **argv)
{
	XVisualInfo *visinfo;
	int i;

	for (i = 1; i < argc; ++i) {
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
	visinfo = piglit_get_glx_visual(dpy);

	for (i = 0; i < num_windows; i++) {
		win[i] = piglit_get_glx_window(dpy, visinfo);
		XMoveWindow(dpy, win[i], 60*i, 10);
		XMapWindow(dpy, win[i]);
	}

	ctx = piglit_get_glx_context(dpy, visinfo);

	glXMakeCurrent(dpy, win[0], ctx);
	piglit_dispatch_default_init(PIGLIT_DISPATCH_GL);

	piglit_glx_event_loop(dpy, draw);

	XFree(visinfo);
	glXDestroyContext(dpy, ctx);
	for (i = 0; i < num_windows; i++) {
		glXDestroyWindow(dpy, win[i]);
	}

	return 0;
}
