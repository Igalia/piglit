/*
 * Copyright © 2012 Intel Corporation
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
 * \file glx-dont-care-mask.c
 * Verify that GLX_DONT_CARE can be used with bitmask attributes
 *
 * Page 17 (page 23 of the PDF) of the GLX 1.4 spec says:
 *
 *    "If GLX_DONT_CARE is specified as an attribute value, then the
 *    attribute will not be checked. GLX_DONT_CARE may be specified
 *    for all attributes except GLX_LEVEL."
 *
 * This test verifies that \c GLX_DONT_CARE can be supplied for
 * \c GLX_RENDER_TYPE and \c GLX_DRAWABLE_TYPE.
 *
 * \sa https://bugs.freedesktop.org/show_bug.cgi?id=47478
 */

#include "piglit-util-gl.h"
#include "piglit-glx-util.h"

int piglit_width = 10;
int piglit_height = 10;

static PFNGLXCHOOSEFBCONFIGPROC ChooseFBConfig = NULL;

int
main(int argc, char **argv)
{
	Display *dpy;
	int result = PIGLIT_PASS;
	GLXFBConfig *configs;
	int num_configs;
	static const int attrib_list[] = {
		GLX_DRAWABLE_TYPE, GLX_DONT_CARE,
		GLX_RENDER_TYPE, GLX_DONT_CARE,
		None
	};

	dpy = XOpenDisplay(NULL);
	if (dpy == NULL) {
		fprintf(stderr, "couldn't open display\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	/* Test requires at least GLX version 1.3.  Otherwise there is no
	 * glXChooseFBConfig function.
	 */
	piglit_require_glx_version(dpy, 1, 3);
	piglit_require_glx_extension(dpy, "GLX_ARB_get_proc_address");

	ChooseFBConfig = (PFNGLXCHOOSEFBCONFIGPROC)
		glXGetProcAddressARB((GLubyte *) "glXChooseFBConfig");

	configs = ChooseFBConfig(dpy, DefaultScreen(dpy), attrib_list,
				 &num_configs);

	result = (num_configs > 0 && configs != NULL)
		? PIGLIT_PASS : PIGLIT_FAIL;

	XFree(configs);

	piglit_report_result(result);
	return 0;
}
