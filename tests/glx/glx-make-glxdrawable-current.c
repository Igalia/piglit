/*
 * Copyright © Christopher James Halse Rogers <christopher.halse.rogers@canonical.com>
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
 *    Christopher James Halse Rogers <christopher.halse.rogers@canonical.com>
 *
 */

/** @file glx-make-glxdrawable-current.c
 *
 * Test that MakeCurrent can successfully switch a single context between
 * different GLXDrawables and back.
 *
 * https://bugs.freedesktop.org/show_bug.cgi?id=30457
 */

#include "piglit-util-gl.h"
#include "piglit-glx-util.h"

int piglit_width = 50, piglit_height = 50;
static Display *dpy;
static Window win_one, win_two;
static XVisualInfo *visinfo;

enum piglit_result
draw(Display *dpy)
{
	GLXContext ctx;
	float green[] = {0.0, 1.0, 0.0, 0.0};
	GLboolean pass = GL_TRUE;
	GLXWindow glxwin_one, glxwin_two;
	GLXFBConfig *configs;
	int nconfigs;
	static const int attributes[] = {
	  GLX_DRAWABLE_TYPE,    GLX_WINDOW_BIT,
	  GLX_DOUBLEBUFFER,     GL_TRUE,
	  GLX_RED_SIZE,         1,
	  GLX_GREEN_SIZE,       1,
	  GLX_BLUE_SIZE,        1,
	  GLX_ALPHA_SIZE,       1,
	  None
	};

	configs = glXChooseFBConfig(dpy, DefaultScreen(dpy), attributes,
				    &nconfigs);

	if (nconfigs == 0 || !configs) {
		fprintf(stderr,
			"Couldn't get an RGBA, double-buffered FBConfig\n");
		piglit_report_result(PIGLIT_FAIL);
		return PIGLIT_FAIL;
	}

	glxwin_one = glXCreateWindow(dpy, configs[0], win_one, NULL);
	glxwin_two = glXCreateWindow(dpy, configs[0], win_two, NULL);

	ctx = piglit_get_glx_context(dpy, visinfo);

	glXMakeCurrent(dpy, glxwin_one, ctx);
	piglit_dispatch_default_init(PIGLIT_DISPATCH_GL);

	glClearColor(0.0, 1.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	glXMakeCurrent(dpy, glxwin_two, ctx);

	glClear(GL_COLOR_BUFFER_BIT);


	glXMakeCurrent(dpy, glxwin_one, ctx);
	pass &= piglit_probe_pixel_rgb(1, 1, green);

	glXMakeCurrent(dpy, glxwin_two, ctx);
	pass &= piglit_probe_pixel_rgb(1, 1, green);

	/* Free our resources when we're done. */
	glXDestroyWindow(dpy, glxwin_one);
	glXDestroyWindow(dpy, glxwin_two);

	free(configs);

	glXMakeCurrent(dpy, None, NULL);
	glXDestroyContext(dpy, ctx);

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
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
	visinfo = piglit_get_glx_visual(dpy);
	win_one = piglit_get_glx_window(dpy, visinfo);
	win_two = piglit_get_glx_window(dpy, visinfo);

	XMapWindow(dpy, win_one);
	XMapWindow(dpy, win_two);

	piglit_glx_event_loop(dpy, draw);

	return 0;
}
