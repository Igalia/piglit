/*
 * Copyright (c) 2014 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * on the rights to use, copy, modify, merge, publish, distribute, sub
 * license, and/or sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following conditions:
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
 * @file default-vao.c
 * Verify that GL_INVALID_OPERATION is generated when the default VAO is used
 * for rendering.
 */

#include "piglit-util-gl.h"
#include "piglit-matrix.h"

PIGLIT_GL_TEST_CONFIG_BEGIN
	config.supports_gl_core_version = 31;
	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;
PIGLIT_GL_TEST_CONFIG_END

void
piglit_init(int argc, char **argv)
{
	GLuint prog = piglit_build_simple_program(
			"#version 130\n void main() { gl_Position = vec4(0); }",
			"#version 130\n void main() { gl_FragColor = vec4(0); }");
	glUseProgram(prog);

	glDrawArrays(GL_TRIANGLES, 0, 3);
	piglit_report_result(piglit_check_gl_error(GL_INVALID_OPERATION)
			     ? PIGLIT_PASS : PIGLIT_FAIL);
}

enum piglit_result
piglit_display(void)
{
	/* unreached */
	return PIGLIT_FAIL;
}
