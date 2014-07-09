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

static bool try_flag(int flag)
{
	const int attribs[] = {
		GLX_CONTEXT_MAJOR_VERSION_ARB, 3,
		GLX_CONTEXT_FLAGS_ARB, flag,
		None
	};
	GLXContext ctx;

	ctx = glXCreateContextAttribsARB(dpy, fbconfig, NULL, True, attribs);
	XSync(dpy, 0);

	if (ctx != NULL) {
		glXDestroyContext(dpy, ctx);
	} else {
		/* The GLX_ARB_create_context spec says:
		 *
		 *     "* If <config> does not support compatible OpenGL
		 *        contexts providing the requested API major and minor
		 *        version, forward-compatible flag, and debug context
		 *        flag, GLXBadFBConfig is generated."
		 */
		if (!validate_glx_error_code(0, GLXBadFBConfig)) {
			fprintf(stderr, "flag = 0x%08x\n", flag);
			return false;
		}
	}

	return true;
}

int main(int argc, char **argv)
{
	bool pass = true;

	GLX_ARB_create_context_setup();

	pass = try_flag(0) && pass;
	pass = try_flag(GLX_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB) && pass;

	GLX_ARB_create_context_teardown();

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
	return 0;
}
