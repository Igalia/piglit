/*
 * Copyright Christopher James Halse Rogers <christopher.halse.rogers at canonical.com>
 * Copyright 2010 Red Hat, Inc.
 * Copyright 2014 Adel Gadllah <adel.gadllah@gmail.com>
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
 *    Christopher James Halse Rogers <christopher.halse.rogers at canonical.com>
 *    Adam Jackson <ajax@redhat.com>
 *    Adel Gadllah <adel.gadllah@gmail.com>
 *
 * Derived from glx-copy-sub-buffer.c
 *
 */

/** @file glx-buffer-age.c
 *
 * Test that GLX_EXT_buffer_age works as advertised
 */

#include "piglit-util-gl.h"
#include "piglit-glx-util.h"

#ifndef GLX_BACK_BUFFER_AGE_EXT
#define GLX_BACK_BUFFER_AGE_EXT 0x20F4
#endif

int piglit_width = 100, piglit_height = 100;
static Display *dpy;
static Window window;
static XVisualInfo *visinfo;

enum piglit_result
draw(Display *dpy)
{
	GLXContext ctx;
	bool pass = true;
	unsigned int age;
	int i;
	static GLfloat colors[5][4] = {
		{1.0, 0.0, 0.0, 1.0},
		{0.0, 1.0, 0.0, 1.0},
		{0.0, 0.0, 1.0, 1.0},
		{1.0, 0.0, 1.0, 1.0},
		{0.0, 1.0, 1.0, 1.0}
	};

	ctx = piglit_get_glx_context(dpy, visinfo);
	glXMakeCurrent(dpy, window, ctx);
	piglit_dispatch_default_init(PIGLIT_DISPATCH_GL);

	glXQueryDrawable(dpy, window, GLX_BACK_BUFFER_AGE_EXT, &age);
	if (age != 0) {
		fprintf(stderr, "Initial age was %d, should be 0\n", age);
		pass = false;
	}

	for (i = 0; i < 5; i++) {
		glClearColor(colors[i][0],
			     colors[i][1],
			     colors[i][2],
			     colors[i][3]);
		glClear(GL_COLOR_BUFFER_BIT);
		glXSwapBuffers(dpy, window);

		glXQueryDrawable(dpy, window, GLX_BACK_BUFFER_AGE_EXT, &age);
		printf("Frame %d: age %d\n", i + 1, age);

		if (age > 0) {
			int color_i = i - (age - 1);
			if (color_i < 0) {
				fprintf(stderr, "too old\n");
				pass = false;
			} else {
				pass = piglit_probe_rect_rgba(0, 0,
							      piglit_width,
							      piglit_height,
							      colors[color_i])
					&& pass;
			}
		}
	}

	glXMakeCurrent(dpy, None, NULL);
	glXDestroyContext(dpy, ctx);

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}


int
main(int argc, char **argv)
{
	int i;

	for(i = 1; i < argc; i++) {
		if (!strcmp(argv[i], "-auto"))
			piglit_automatic = 1;
		else
			fprintf(stderr, "Unknown option: %s\n", argv[i]);
	}

	dpy = piglit_get_glx_display();
	piglit_require_glx_extension(dpy, "GLX_EXT_buffer_age");
	visinfo = piglit_get_glx_visual(dpy);
	window = piglit_get_glx_window(dpy, visinfo);

	XMapWindow(dpy, window);

	piglit_glx_event_loop(dpy, draw);

	return 0;
}
