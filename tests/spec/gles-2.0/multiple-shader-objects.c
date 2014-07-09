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
 * From the OpenGL ES 2.0 spec, Section 2.10.3 Program Objects:
 *
 *      Multiple shader objects of the same type may not be attached to
 *      a single program object. [...] The error INVALID_OPERATION is
 *      generated if [...] another shader object of the same type as shader is
 *      already attached to program.
 *
 * This test checks that GL_INVALID_OPERATION is generated.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_es_version = 20;

PIGLIT_GL_TEST_CONFIG_END

enum piglit_result
piglit_display(void)
{
	return PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	bool pass = true;

	GLuint vs1 = glCreateShader(GL_VERTEX_SHADER);
	GLuint vs2 = glCreateShader(GL_VERTEX_SHADER);

	GLuint fs1 = glCreateShader(GL_FRAGMENT_SHADER);
	GLuint fs2 = glCreateShader(GL_FRAGMENT_SHADER);

	GLuint prog1 = glCreateProgram();
	GLuint prog2 = glCreateProgram();

	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	/* It's illegal to attach two vertex shaders. */
	glAttachShader(prog1, vs1);
	pass &= piglit_check_gl_error(GL_NO_ERROR);

	glAttachShader(prog1, vs2);
	pass &= piglit_check_gl_error(GL_INVALID_OPERATION);

	/* It's illegal to attach two fragment shaders. */
	glAttachShader(prog2, fs1);
	pass &= piglit_check_gl_error(GL_NO_ERROR);

	glAttachShader(prog2, fs2);
	pass &= piglit_check_gl_error(GL_INVALID_OPERATION);

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}
