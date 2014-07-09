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
 * \file gen-names-only.c
 *
 * The ARB_transform_feedback2 spec says:
 *
 *     "BindTransformFeedback fails and an INVALID_OPERATION error is
 *     generated if <id> is not zero or a name returned from a previous
 *     call to GenTransformFeedbacks, or if such a name has since been
 *     deleted with DeleteTransformFeedbacks."
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;
	config.window_visual = PIGLIT_GL_VISUAL_RGB;

PIGLIT_GL_TEST_CONFIG_END

enum piglit_result
piglit_display(void)
{
	return PIGLIT_FAIL;
}

void piglit_init(int argc, char **argv)
{
	GLuint id;
	bool pass = true;

	piglit_require_transform_feedback();
	piglit_require_extension("GL_ARB_transform_feedback2");

	glGenTransformFeedbacks(1, &id);

	glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, 0);
	pass = piglit_check_gl_error(0) && pass;

	glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, id + 1);
	pass = piglit_check_gl_error(GL_INVALID_OPERATION) && pass;

	glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, id);
	pass = piglit_check_gl_error(0) && pass;

	glDeleteTransformFeedbacks(1, &id);
	glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, id);
	pass = piglit_check_gl_error(GL_INVALID_OPERATION) && pass;

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}
