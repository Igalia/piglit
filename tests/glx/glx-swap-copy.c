/*
 * Copyright © 2010 Intel Corporation
 * Copyright © 2017 VMWare Inc.
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
 *    Thomas Hellstrom <thellstrom@vmware.com>
 *
 */

/** @file glx-swap-copy.c
 *
 * Test that GLX_SWAP_COPY_OML does in fact cause the back buffer to get
 * preserved on swap.
 */

#include "piglit-util-gl.h"
#include "piglit-glx-util.h"

int piglit_width = 50, piglit_height = 50;
static Display *dpy;
static XVisualInfo *visinfo;
static GLXFBConfig *config;
static GLXContext ctx;
static GLXWindow gwin;

enum piglit_result
draw(Display *dpy)
{
	GLboolean pass = GL_TRUE;
	static const float red[] = {1.0, 0.0, 0.0, 0.5};

	glXMakeContextCurrent(dpy, gwin, gwin, ctx);
	glClearColor(1.0, 0.0, 0.0, 0.5);
	glClear(GL_COLOR_BUFFER_BIT);
	glXMakeContextCurrent(dpy, None, None, NULL);
	glXSwapBuffers(dpy, gwin);
	glXSwapBuffers(dpy, gwin);
	glXSwapBuffers(dpy, gwin);
	glXMakeContextCurrent(dpy, gwin, gwin, ctx);
	glReadBuffer(GL_BACK);
	pass = piglit_probe_pixel_rgba(0, 0, red);
	if (pass) {
		glReadBuffer(GL_FRONT);
		pass = piglit_probe_pixel_rgb(0, 0, red);
	}

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}


static GLXFBConfig *
piglit_get_swap_copy_config(Display *dpy)
{
	GLXFBConfig *fbc;
	int nele;
	int attrib[] = {
		GLX_RENDER_TYPE, GLX_RGBA_BIT,
		GLX_RED_SIZE, 8,
		GLX_GREEN_SIZE, 8,
		GLX_BLUE_SIZE, 8,
		GLX_ALPHA_SIZE, 8,
		GLX_SWAP_METHOD_OML, GLX_SWAP_COPY_OML,
		GLX_DOUBLEBUFFER, True,
		None
	};
	int screen = DefaultScreen(dpy);

	fbc = glXChooseFBConfig(dpy, screen, attrib, &nele);
	if (fbc == NULL) {
		fprintf(stderr,
			"Couldn't get a GLX_SWAP_COPY_OML, RGBA, "
			"double-buffered fbconfig\n");
		piglit_report_result(PIGLIT_SKIP);
		exit(1);
	}

	return fbc;
}

int
main(int argc, char **argv)
{
	int i;
	const char *glx_extension_list;
	Window win;

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

	glx_extension_list = glXQueryExtensionsString(dpy, DefaultScreen(dpy));
	if (strstr(glx_extension_list, "GLX_OML_swap_method") == NULL) {
		printf("Requires GLX_OML_swap_method\n");
		piglit_report_result(PIGLIT_SKIP);
	}

	config = piglit_get_swap_copy_config(dpy);
	visinfo = glXGetVisualFromFBConfig(dpy, config[0]);
	if (!visinfo) {
		printf("Error: couldn't create a visual from fbconfig.\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	win = piglit_get_glx_window(dpy, visinfo);
	XMapWindow(dpy, win);
	gwin = glXCreateWindow(dpy, config[0], win, NULL);
	ctx = glXCreateNewContext(dpy, config[0], GLX_RGBA_TYPE, 0, GL_TRUE);
	glXMakeContextCurrent(dpy, gwin, gwin, ctx);
	piglit_dispatch_default_init(PIGLIT_DISPATCH_GL);

	piglit_glx_event_loop(dpy, draw);

	return 0;
}
