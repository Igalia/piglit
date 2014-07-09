/* Copyright © 2013 Intel Corporation
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
 * \file
 *
 * Create a context with or without the forward-compatible bit, according to
 * a command line flag.  Then verify that GL_CONTEXT_FLAGS does or does not
 * contain GL_CONTEXT_FLAG_FORWARD_COMPATIBLE_BIT.
 */

#include "piglit-util-gl.h"

static void
usage_error(void)
{
	printf("usage error\n"
	       "usage: gl-3.0-forward-compatible-bit yes|no\n");
	piglit_report_result(PIGLIT_FAIL);
}

static bool expect_fwd_compat;

PIGLIT_GL_TEST_CONFIG_BEGIN

	if (PIGLIT_STRIP_ARG("yes")) {
		expect_fwd_compat = true;
	} else if (PIGLIT_STRIP_ARG("no")) {
		expect_fwd_compat = false;
	} else {
		usage_error();
	}

	config.supports_gl_compat_version = 30;
	config.require_forward_compatible_context = expect_fwd_compat;

PIGLIT_GL_TEST_CONFIG_END


void
piglit_init(int argc, char **argv)
{
	GLint flags;
	enum piglit_result result = PIGLIT_PASS;

	glGetIntegerv(GL_CONTEXT_FLAGS, &flags);

	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	if (expect_fwd_compat &&
	    !(flags & GL_CONTEXT_FLAG_FORWARD_COMPATIBLE_BIT)) {
		fprintf(stderr, "error: Requested creation of a "
			"forward-compatible OpenGL 3.0 context, but "
			"GL_CONTEXT_FLAGS does not contain "
			"GL_CONTEXT_FLAG_FORWARD_COMPATIBLE_BIT\n");
		fprintf(stderr, "error: GL_CONTEXT_FLAGS=0x%x\n", flags);
		result = PIGLIT_FAIL;
	} else if (!expect_fwd_compat &&
	           (flags & GL_CONTEXT_FLAG_FORWARD_COMPATIBLE_BIT)) {
		fprintf(stderr, "error: Requested creation of a "
			"non-forward-compatible OpenGL 3.0 context, but "
			"GL_CONTEXT_FLAGS contains "
			"GL_CONTEXT_FLAG_FORWARD_COMPATIBLE_BIT\n");
		fprintf(stderr, "error: GL_CONTEXT_FLAGS=0x%x\n", flags);
		result = PIGLIT_FAIL;
	}

	piglit_report_result(result);
}

enum piglit_result
piglit_display(void)
{
	return PIGLIT_FAIL;
}
