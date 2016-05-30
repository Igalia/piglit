/*
 * Copyright © 2016 Intel Corporation
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
 */


/** @file glx-fbconfig-bad.c
 *
 * Tests that driver sets error correctly (GLXBadFBConfig) when calling
 * glXCreateNewContext with an invalid GLXFBConfig.
 */

#include "piglit-util-gl.h"
#include "piglit-glx-util.h"
#include <GL/glxint.h>

static bool bad_config_error = false;

static int
error_handler(Display *dpy, XErrorEvent *event)
{
	if (piglit_glx_get_error(dpy, event) == GLXBadFBConfig) {
		bad_config_error = true;
	} else {
		bad_config_error = false;
	}
	return 0;
}

int
main(int argc, char **argv)
{
	Display *dpy;
	__GLXFBConfig bad_config;
	memset(&bad_config, 0, sizeof(__GLXFBConfig));

	dpy = XOpenDisplay(NULL);
	if (dpy == NULL) {
		fprintf(stderr, "couldn't open display\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	XSetErrorHandler(error_handler);

	glXCreateNewContext(dpy, NULL, GLX_RGBA_TYPE, NULL, True);
	XFlush(dpy);
	if (!bad_config_error) {
		piglit_report_result(PIGLIT_FAIL);
	}

	glXCreateNewContext(dpy, &bad_config, GLX_RGBA_TYPE, NULL, True);
	XFlush(dpy);
	if (!bad_config_error) {
		piglit_report_result(PIGLIT_FAIL);
	}
	XCloseDisplay(dpy);
	piglit_report_result(PIGLIT_PASS);
}
