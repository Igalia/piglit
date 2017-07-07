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

/** @file glx-multi-context-single-window.c
 *
 * Exercise rendering to a single window with multiple contexts.
 */

#include <unistd.h>
#include "piglit-util-gl.h"
#include "piglit-glx-util.h"

#define MAX_CONTEXTS 8

static GLXContext ctx[MAX_CONTEXTS];
static int num_contexts = MAX_CONTEXTS;
static Window win;
static Display *dpy;

static const float colors[MAX_CONTEXTS][4] = {
	{1, 0, 0, 1},
	{0, 1, 0, 1},
	{0, 0, 1, 1},
	{0, 1, 1, 1},
	{1, 0, 1, 1},
	{1, 1, 0, 1},
	{1, 1, 1, 1},
	{.5, .5, .5, 1},
};

int piglit_width = 500, piglit_height = 500;

static int rect_size = 40;


static int
rect_pos(int i)
{
	return i * rect_size / 2;
}


enum piglit_result
draw(Display *dpy)
{
	int i;
	bool pass = true;

	/* draw a series of colored quads, one per context, at increasing
	 * Z distance.
	 */
	for (i = 0; i < num_contexts; i++) {
		glXMakeCurrent(dpy, win, ctx[i]);

		if (i == 0) {
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		}

		glEnable(GL_DEPTH_TEST);
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(0, piglit_width, 0, piglit_height, 0, 1);

		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		glPushMatrix();
		float p = rect_pos(i);
		float z = -i / 10.0;
		glTranslatef(p, p, z);

		glColor4fv(colors[i]);
		piglit_draw_rect(0, 0, rect_size, rect_size);

		glPopMatrix();
	}

	/* probe rendering */
	glXMakeCurrent(dpy, win, ctx[0]);
	for (i = 0; i < num_contexts; i++) {
		int x = rect_pos(i) + rect_size * 3 / 4;
		int p = piglit_probe_pixel_rgb(x, x, colors[i]);

		if (!p) {
			printf("Failed probe for rect/context %d\n", i);
			pass = false;
		}
	}

	glXSwapBuffers(dpy, win);

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

	win = piglit_get_glx_window(dpy, visinfo);
	XMapWindow(dpy, win);

	for (i = 0; i < num_contexts; i++) {
		ctx[i] = piglit_get_glx_context(dpy, visinfo);
	}

	glXMakeCurrent(dpy, win, ctx[0]);
	piglit_dispatch_default_init(PIGLIT_DISPATCH_GL);

	piglit_glx_event_loop(dpy, draw);

	XFree(visinfo);
	glXDestroyWindow(dpy, win);
	for (i = 0; i < num_contexts; i++) {
		glXDestroyContext(dpy, ctx[i]);
	}

	return 0;
}
