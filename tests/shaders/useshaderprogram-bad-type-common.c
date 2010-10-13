/*
 * Copyright © 2010 Intel Corporation
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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

/**
 * \file useshaderprogram-bad-type-common.c
 * Common code for useshaderprogram-bad-type tests.
 *
 * \author Ian Romanick <ian.d.romanick@intel.com>
 */
#include "useshaderprogram-bad-type-common.h"

int piglit_width = 100, piglit_height = 100;
int piglit_window_mode = GLUT_RGB | GLUT_DOUBLE;

enum piglit_result
piglit_display(void)
{
	return PIGLIT_FAILURE;
}

void
try_UseShaderProgram(GLenum type)
{
	enum piglit_result result = PIGLIT_SUCCESS;
	GLenum err;

	/* There shouldn't be any GL errors, but clear them all just to be
	 * sure.
	 */
	while (glGetError() != 0)
		/* empty */ ;

	/* Type is not one of the known shader types.  This should generate
	 * the error GL_INVALID_ENUM.
	 */
	glUseShaderProgramEXT(type, 0);

	err = glGetError();
	if (err != GL_INVALID_ENUM) {
		printf("Unexpected OpenGL error state 0x%04x for "
		       "glUseShaderProgramEXT called with\n"
		       "an invalid shader target 0x%04x (expected 0x%04x).\n",
		       err, type, GL_INVALID_ENUM);
		result = PIGLIT_FAILURE;
	}

	piglit_report_result(result);
}
