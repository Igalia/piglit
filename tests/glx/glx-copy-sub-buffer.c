/*
 * Copyright Christopher James Halse Rogers <christopher.halse.rogers at canonical.com>
 * Copyright 2010 Red Hat, Inc.
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
 *
 * Derived from glx-make-current.c
 */

/** @file glx-copy-sub-buffer.c
 *
 * Test that GLX_MESA_copy_sub_buffer works as advertised
 */

#include "piglit-util-gl.h"
#include "piglit-glx-util.h"

int piglit_width = 100, piglit_height = 100;
static Display *dpy;
static Window win_one;
static XVisualInfo *visinfo;
static PFNGLXCOPYSUBBUFFERMESAPROC CopySubBuffer;

enum piglit_result
draw(Display *dpy)
{
	GLXContext ctx;
	GLboolean pass = GL_TRUE;
	static float red[]   = {1.0, 0.0, 0.0, 0.0};
	static float green[] = {0.0, 1.0, 0.0, 0.0};

	ctx = piglit_get_glx_context(dpy, visinfo);
	glXMakeCurrent(dpy, win_one, ctx);
	piglit_dispatch_default_init(PIGLIT_DISPATCH_GL);

	glClearColor(1.0, 0.0, 0.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT);
	glXSwapBuffers(dpy, win_one);

	glClearColor(0.0, 1.0, 0.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT);
	CopySubBuffer(dpy, win_one,
		      piglit_width / 4,
		      piglit_height / 4,
		      piglit_width / 2,
		      piglit_height / 2);

	glReadBuffer(GL_FRONT);

	pass &= piglit_probe_rect_rgb(0, 0, piglit_width / 4, piglit_height / 4,
				      red);
	pass &= piglit_probe_rect_rgb(piglit_width / 4, piglit_width / 4,
				      piglit_width / 2, piglit_height / 2,
				      green);

	glXMakeCurrent(dpy, None, NULL);
	glXDestroyContext(dpy, ctx);

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

static XVisualInfo *
get_glx_visual(Display *dpy, int samples)
{
	XVisualInfo *visinfo;
	int attrib[] = {
		GLX_RGBA,
		GLX_RED_SIZE, 1,
		GLX_GREEN_SIZE, 1,
		GLX_BLUE_SIZE, 1,
		GLX_DOUBLEBUFFER,
		samples <= 1 ? None : GLX_SAMPLE_BUFFERS, 1,
		GLX_SAMPLES, samples,
		None
	};
	int screen = DefaultScreen(dpy);

	visinfo = glXChooseVisual(dpy, screen, attrib);
	if (visinfo == NULL) {
		fprintf(stderr,
			"Couldn't get an RGBA, double-buffered visual "
			"with samples=%i\n", samples);
		piglit_report_result(PIGLIT_SKIP);
		exit(1);
	}

	return visinfo;
}

int
main(int argc, char **argv)
{
	int i, samples = 0;

	for(i = 1; i < argc; ++i) {
		if (!strcmp(argv[i], "-auto"))
			piglit_automatic = 1;
		else if (!strncmp(argv[i], "-samples=", 9)) {
			samples = atoi(argv[i]+9);
		}
		else
			fprintf(stderr, "Unknown option: %s\n", argv[i]);
	}

	dpy = XOpenDisplay(NULL);
	if (dpy == NULL) {
		fprintf(stderr, "couldn't open display\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	piglit_require_glx_extension(dpy, "GLX_MESA_copy_sub_buffer");
	CopySubBuffer = (PFNGLXCOPYSUBBUFFERMESAPROC)
	    glXGetProcAddressARB((GLubyte *)"glXCopySubBufferMESA");

	visinfo = get_glx_visual(dpy, samples);
	win_one = piglit_get_glx_window(dpy, visinfo);

	XMapWindow(dpy, win_one);

	piglit_glx_event_loop(dpy, draw);

	return 0;
}
