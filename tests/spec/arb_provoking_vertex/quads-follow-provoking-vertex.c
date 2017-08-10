/**
 * Copyright © 2013 Intel Corporation
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
 * QUADS_FOLLOW_PROVOKING_VERTEX was erringly added to GL 3.2 core spec
 *
 *  This was removed from the core spec in 3.3.
 *
 *  Table 6.45 of GL 3.2 core spec includes QUADS_FOLLOW_PROVOKING_VERTEX
 *  which can be queried with GetBooleanv() to see "Whether quads follow
 *  provoking vertex convention"
 *
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

        config.supports_gl_core_version = 32;
	config.khr_no_error_support = PIGLIT_HAS_ERRORS;

PIGLIT_GL_TEST_CONFIG_END

void
piglit_init(int argc, char **argv)
{
	bool pass = true;
	GLboolean followsProvoking = false;
	GLint major, minor, profile;
	GLenum expected_error;

	glGetIntegerv(GL_MAJOR_VERSION, &major);
	glGetIntegerv(GL_MINOR_VERSION, &minor);
	printf("GL version: %d.%d\n", major, minor);

	glGetIntegerv(GL_CONTEXT_PROFILE_MASK, &profile);
	printf("GL profile: 0x%x\n", profile);

	if ((profile & GL_CONTEXT_COMPATIBILITY_PROFILE_BIT) ||
	    major * 10 + minor == 32) {
		expected_error = GL_NO_ERROR;
	} else {
		expected_error = GL_INVALID_ENUM;
	}

	glGetBooleanv(GL_QUADS_FOLLOW_PROVOKING_VERTEX_CONVENTION,
		      &followsProvoking);

	pass = piglit_check_gl_error(expected_error) && pass;

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}

enum piglit_result
piglit_display(void)
{
	return PIGLIT_FAIL;
}
