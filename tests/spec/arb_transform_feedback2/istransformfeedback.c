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

#include "piglit-util-gl.h"

/**
 * @file istransformfeedback.c
 *
 * Tests basic API support for glIsTransformFeedback().
 *
 * From the ARB_transform_feedback2 spec:
 *
 *     "The command
 *
 *         void GenTransformFeedbacks(sizei n, uint *ids)
 *
 *      returns <n> previously unused transform feedback object names in <ids>.
 *      These names are marked as used, for the purposes of
 *      GenTransformFeedbacks only, but they acquire transform feedback state
 *      only when they are first bound.
 *
 *      [...]
 *
 *      A transform feedback object is created by binding a name
 *      returned by GenTransformFeedbacks with the command
 *
 *         void BindTransformFeedback(enum target, uint id)"
 */

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

PIGLIT_GL_TEST_CONFIG_END

enum piglit_result
piglit_display(void)
{
	/* UNREACHED */
	return PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	GLuint id;

	piglit_require_gl_version(15);
	piglit_require_GLSL();
	piglit_require_extension("GL_EXT_transform_feedback");
	piglit_require_extension("GL_ARB_transform_feedback2");

	glGenTransformFeedbacks(1, &id);

	if (glIsTransformFeedback(id)) {
		fprintf(stderr, "id recognized incorrectly as a transform feedback object.\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, id);

	if (!glIsTransformFeedback(id)) {
		fprintf(stderr, "id not recognized correctly as a transform feedback object.\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	piglit_report_result(PIGLIT_PASS);
}
