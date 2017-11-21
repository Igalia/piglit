/*
 * Copyright © 2017 Fabian Bieler
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
 * @file uniform-matrix-transposed.c:  Test transposed matrix loading
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 20;
	config.window_visual = PIGLIT_GL_VISUAL_RGBA;
	config.khr_no_error_support = PIGLIT_NO_ERRORS;

PIGLIT_GL_TEST_CONFIG_END

static const char *fs_text =
	"uniform mat4 uniformMat4t;\n"
	"void main()\n"
	"{\n"
	"   gl_FragColor = uniformMat4t[2];\n"
	"}\n";

enum piglit_result
piglit_display(void)
{
	glClear(GL_COLOR_BUFFER_BIT);

	piglit_draw_rect(-1, -1, 2, 2);

	const float expected_color[] = {0.2, 0.0, 1.0, 0.8};
	const bool pass = piglit_probe_pixel_rgba(piglit_width / 2,
						    piglit_height / 2,
						    expected_color);

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	const int program = piglit_build_simple_program(NULL, fs_text);
	glUseProgram(program);

	static const float uniformMatrix[16] = {
		1.0, 0.1, 0.2, 0.3,
		0.0, 1.0, 0.0, 0.4,
		0.0, 1.0, 1.0, 0.5,
		0.6, 0.7, 0.8, 1.0
	};
	const int umat4t = glGetUniformLocation(program, "uniformMat4t");
	glUniformMatrix4fv(umat4t, 1, GL_TRUE, uniformMatrix);
}
