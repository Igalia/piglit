/*
 * Copyright (c) 2011 VMware, Inc.
 * Copyright (c) 2017 Józef Kucia <joseph.kucia@gmail.com>
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

/**
 * Test clearing GL_FRONT across glXMakeCurrent calls.
 *
 * Reproduces bug in st/mesa front buffer allocation logic.
 */

#include "piglit-util-gl.h"
#include "piglit-glx-util.h"

static const char *test_name = "glx-multi-context-front";
static const float green[4] = { 0.0f, 1.0f, 0.0f, 0.0f };

static Window Windows[2];
static GLXContext ctx;


enum piglit_result
draw(Display *dpy)
{
	bool pass = true;
	GLint buffer;

	glXMakeCurrent(dpy, Windows[0], ctx);

	glXMakeCurrent(dpy, Windows[1], ctx);
	glDrawBuffer(GL_FRONT);

	glXMakeCurrent(dpy, Windows[0], ctx);
	glGetIntegerv(GL_DRAW_BUFFER, &buffer);
	if (buffer != GL_FRONT) {
		printf("%s: Got unexpected draw buffer %s\n",
		       test_name, piglit_get_gl_enum_name(buffer));
		pass = false;
	}
	glXMakeCurrent(dpy, Windows[1], ctx);
	glGetIntegerv(GL_DRAW_BUFFER, &buffer);
	if (buffer != GL_FRONT) {
		printf("%s: Got unexpected draw buffer %s\n",
		       test_name, piglit_get_gl_enum_name(buffer));
		pass = false;
	}

	glClearColor(green[0], green[1], green[2], green[3]);
	glClear(GL_COLOR_BUFFER_BIT);

	glReadBuffer(GL_FRONT);
	pass &= piglit_probe_rect_rgb(0, 0, piglit_width, piglit_height,
				      green);

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}


int
main(int argc, char **argv)
{
	Display *dpy;
	XVisualInfo *visinfo;
	int i;

	for (i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-auto") == 0) {
			piglit_automatic = 1;
			break;
		}
	}

	dpy = XOpenDisplay(NULL);
	if (!dpy) {
		fprintf(stderr, "Failed to open display\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	visinfo = piglit_get_glx_visual(dpy);
	Windows[0] = piglit_get_glx_window(dpy, visinfo);
	Windows[1] = piglit_get_glx_window(dpy, visinfo);

	XMapWindow(dpy, Windows[0]);
	XMapWindow(dpy, Windows[1]);

	ctx = piglit_get_glx_context(dpy, visinfo);

	glXMakeCurrent(dpy, Windows[0], ctx);
	piglit_dispatch_default_init(PIGLIT_DISPATCH_GL);

	piglit_glx_event_loop(dpy, draw);

	return 0;
}
