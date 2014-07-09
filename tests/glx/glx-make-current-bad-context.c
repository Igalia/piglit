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

/** @file glx-make-current-bad-context.c
 *
 * Test that glXMakeCurrent() with a bad context correctly returns
 * GLXBadContext.
 */

#include "piglit-util-gl.h"
#include "piglit-glx-util.h"

int piglit_width = 50, piglit_height = 50;

bool found_badvalue = false;
int expect_badvalue(Display *dpy, XErrorEvent *e)
{
	if (e->error_code == BadValue)
		found_badvalue = true;

	return 0;
}

int
main(int argc, char **argv)
{
	Display *dpy;
	Window win;
	XVisualInfo *visinfo;
	int i;
	int (*old_handler)(Display *, XErrorEvent *);
	GLXContext ctx;
	int attrib[] = {
		GLX_RGBA,
		GLX_RED_SIZE, 1,
		GLX_GREEN_SIZE, 1,
		GLX_BLUE_SIZE, 1,
		GLX_DOUBLEBUFFER,
		None
	};
	GLXFBConfig config, *configs;
	int nconfigs;

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
	visinfo = piglit_get_glx_visual(dpy);
	win = piglit_get_glx_window(dpy, visinfo);

	configs = glXChooseFBConfig(dpy, DefaultScreen(dpy),
				    attrib, &nconfigs);
	assert(nconfigs > 0);
	config = configs[0];
	XFree(configs);

	old_handler = XSetErrorHandler(expect_badvalue);
	ctx = glXCreateNewContext(dpy, config, 0x1010, NULL, True);
	XSync(dpy, 0);
	XSetErrorHandler(old_handler);

	if (!found_badvalue) {
		printf("Failed to get BadValue from glXCreateContext().\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	if (ctx != NULL) {
		glXMakeCurrent(dpy, win, ctx);
		piglit_report_result(PIGLIT_PASS);
	} else {
		piglit_report_result(PIGLIT_SKIP);
	}

	return 0;
}
