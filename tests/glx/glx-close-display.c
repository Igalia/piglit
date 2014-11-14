/*
 * Copyright 2011 VMware, Inc.
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
 *    Jose Fonseca <jfonseca@vmware.com>
 *
 */


/** @file glx-closedisplay.c
 *
 * Test that calling XCloseDisplay after using GLX context works correctly.
 */

#include "piglit-util-gl.h"
#include "piglit-glx-util.h"

/**
 * Straightforward function to check whether GLX direct rendering is supported
 * or not.
 */
static Bool
isDirectRendering(void)
{
	Display *dpy;
	GLXContext ctx;
	GLXFBConfig *configs;
	GLXFBConfig config;
	int attribList[] = {
		GLX_DRAWABLE_TYPE, GLX_USE_GL,
		GLX_RENDER_TYPE, GLX_USE_GL,
		GLX_DOUBLEBUFFER, True,
		0
	};
	int nitems = 0;
	Bool result;

	dpy = XOpenDisplay(NULL);
	if (dpy == NULL) {
		fprintf(stderr, "couldn't open display\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	configs = glXChooseFBConfig(dpy, 0, attribList, &nitems);
	if (configs == NULL) {
		fprintf(stderr, "couldn't find a matching framebuffer configuration\n");
		piglit_report_result(PIGLIT_FAIL);
	}
	config = configs[0];

	ctx = glXCreateNewContext(dpy, config, GLX_RGBA_TYPE, NULL, True);
	result = glXIsDirect(dpy, ctx);
	glXDestroyContext(dpy, ctx);

	/* This call will cause *_dri.so to be dlclosed and unloaded. */
	XCloseDisplay(dpy);

	return result;
}

int
main(int argc, char **argv)
{
	isDirectRendering();

	/* Run a second to exercise reloading the *_dri.so driver. */
	isDirectRendering();

	piglit_report_result(PIGLIT_PASS);

	return 0;
}
