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
 * Authors:
 *    Eric Anholt <eric@anholt.net>
 *
 */

/** @file glx-swap-exchange.c
 *
 * Test that GLX_SWAP_EXCHANGE_OML does in fact cause the back buffer to get
 * exchanged on swap.
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
	GLboolean pass = GL_TRUE;
	float green[] = {0.0, 1.0, 0.0, 0.0};
	GLXContext ctx;

	ctx = piglit_get_glx_context(dpy, visinfo);
	glXMakeCurrent(dpy, win, ctx);

	/* Clear background to gray */
	glClearColor(0.0, 1.0, 0.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT);
	glXSwapBuffers(dpy, win);
	glClearColor(1.0, 0.0, 0.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT);
	glXSwapBuffers(dpy, win);

	pass = piglit_probe_pixel_rgba(0, 0, green);

	glXSwapBuffers(dpy, win);

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}


XVisualInfo *
piglit_get_swap_exchange_visual(Display *dpy)
{
	XVisualInfo *visinfo;
	int attrib[] = {
		GLX_RGBA,
		GLX_RED_SIZE, 1,
		GLX_GREEN_SIZE, 1,
		GLX_BLUE_SIZE, 1,
		GLX_DOUBLEBUFFER,
		GLX_SWAP_METHOD_OML, GLX_SWAP_EXCHANGE_OML,
		None
	};
	int screen = DefaultScreen(dpy);

	visinfo = glXChooseVisual(dpy, screen, attrib);
	if (visinfo == NULL) {
		fprintf(stderr,
			"Couldn't get a GLX_SWAP_EXCHANGE_OML, RGBA, "
			"double-buffered visual\n");
		piglit_report_result(PIGLIT_SKIP);
		exit(1);
	}

	return visinfo;
}

int
main(int argc, char **argv)
{
	int i;
	const char *glx_extension_list;

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

	glx_extension_list = glXQueryExtensionsString(dpy, DefaultScreen(dpy));
	if (strstr(glx_extension_list, "GLX_OML_swap_method") == NULL) {
		printf("Requires GLX_OML_swap_method\n");
		piglit_report_result(PIGLIT_SKIP);
	}

	visinfo = piglit_get_swap_exchange_visual(dpy);
	win = piglit_get_glx_window(dpy, visinfo);

	XMapWindow(dpy, win);

	piglit_glx_event_loop(dpy, draw);

	return 0;
}
