/*
 * Copyright 2013 Red Hat, Inc.
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

int main(int argc, char **argv)
{
	static const int attribs[] = {
		GLX_CONTEXT_MAJOR_VERSION_ARB, 3,
		GLX_CONTEXT_MINOR_VERSION_ARB, 0,
		None
	};
	GLXContext ctx;
	int result = PIGLIT_PASS;

	GLX_ARB_create_context_setup();

	/* 
	 * The GLX_ARB_create_context spec says:
	 *
	 *  In the description of glXMakeContextCurrent, replace the text
	 *	"If either <draw> or <read> are not a valid GLX drawable, a
	 *	GLXBadDrawable error is generated."
	 *
	 *  with
	 *
	 *	"If either <draw> or <read> are not a valid GLX drawable, a
	 *	GLXBadDrawable error is generated, unless <draw> and <read> are
	 *	both None and the OpenGL version supported by <ctx> is 3.0 or
	 *	greater. In this case the context is made current without a
	 *	default framebuffer, as defined in chapter 4 of the OpenGL 3.0
	 *	Specification."
	 *
	 * Request an OpenGL 3.0 context, and then make it current with None
	 * for both the drawable and readable.
	 */
	ctx = glXCreateContextAttribsARB(dpy, fbconfig, NULL, True, attribs);

	if (!ctx) {
		/*
		 * Well, is 3.0 supported at all?  The spec says:
		 *
		 *  * If <config> does not support compatible OpenGL contexts
		 *    providing the requested API major and minor version,
		 *    forward-compatible flag, and debug context flag,
		 *    GLXBadFBConfig is generated.
		 */
		if (validate_glx_error_code(Success, GLXBadFBConfig)) {
			fprintf(stderr, "GL 3.0 not supported\n");
			result = PIGLIT_SKIP;
		} else {
			fprintf(stderr, "Failed to create a 3.0 context\n");
			result = PIGLIT_WARN;
		}
	} else {
		if (!glXMakeContextCurrent(dpy, None, None, ctx))
			result = PIGLIT_FAIL;
		else if (!validate_glx_error_code(Success, -1))
			result = PIGLIT_FAIL;

		glXDestroyContext(dpy, ctx);
	}
	GLX_ARB_create_context_teardown();

	piglit_report_result(result);
	return 0;
}
