/* Copyright © 2011 Intel Corporation
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
#include "piglit-util-gl.h"
#include "piglit-glx-util.h"
#include "common.h"

static bool check_version(int major, int minor, int *flags)
{
	int attribs[] = {
		GLX_CONTEXT_MAJOR_VERSION_ARB, major,
		GLX_CONTEXT_MINOR_VERSION_ARB, minor,
		GLX_CONTEXT_FLAGS_ARB, 0,
		None
	};
	GLXContext ctx;

	ctx = glXCreateContextAttribsARB(dpy, fbconfig, NULL, True, attribs);
	if (ctx != NULL) {
		glXDestroyContext(dpy, ctx);
		*flags = attribs[5];
		return true;
	}

	attribs[5] = GLX_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB;
	ctx = glXCreateContextAttribsARB(dpy, fbconfig, NULL, True, attribs);
	if (ctx != NULL) {
		glXDestroyContext(dpy, ctx);
		*flags = attribs[5];
		return true;
	}

	return false;
}

static bool try_render_type(int type, int flags)
{
	const int attribs[] = {
		GLX_CONTEXT_MAJOR_VERSION_ARB, 3,
		GLX_RENDER_TYPE, type,
		GLX_CONTEXT_FLAGS_ARB, flags,
		None
	};
	GLXContext ctx;
	bool pass = true;

	ctx = glXCreateContextAttribsARB(dpy, fbconfig, NULL, True, attribs);
	XSync(dpy, 0);

	if (ctx != NULL) {
		fprintf(stderr,
			"Created OpenGL context with invalid render-type "
			"0x%08x, but this should have failed.\n",
			type);
		glXDestroyContext(dpy, ctx);
		pass = false;
	}

	/* The GLX_ARB_create_context spec says:
	 *
	 *     "OpenGL contexts supporting version 3.0 or later of the API do
	 *     not support color index rendering, even if a color index
	 *     <config> is available.
	 *
	 *     ...
	 *
	 *     If attributes GLX_CONTEXT_MAJOR_VERSION_ARB and
	 *     GLX_CONTEXT_MINOR_VERSION_ARB, when considered together with
	 *     attributes GLX_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB and
	 *     GLX_RENDER_TYPE, specify an OpenGL version and feature set that
	 *     are not defined, BadMatch is generated."
	 */
	return validate_glx_error_code(BadMatch, -1) && pass;
}

int main(int argc, char **argv)
{
	bool pass = true;
	int flags;

	GLX_ARB_create_context_setup();

	if (!check_version(3, 0, &flags)) {
		printf("Test requires OpenGL 3.0.\n");
		piglit_report_result(PIGLIT_SKIP);
	}

	pass = try_render_type(GLX_COLOR_INDEX_TYPE, flags);

	GLX_ARB_create_context_teardown();

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
	return 0;
}
