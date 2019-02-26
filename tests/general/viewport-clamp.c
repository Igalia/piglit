/*
 * Copyright © 2019 Intel Corporation
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
 *
 * Author:
 *    Eleni Maria Stea <estea@igalia.com>
 */

/* The purpose of this test is to validate the viewport clamping
 * when the Y is flipped (0 on top). It can be used to reproduce this bug:
 * https://bugs.freedesktop.org/show_bug.cgi?id=108999
 * and test the fix: https://patchwork.freedesktop.org/series/53830/
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

PIGLIT_STRIP_ARG("-fbo");
config.supports_gl_compat_version = 30;
config.window_width = 800;
config.window_height = 600;
config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGB;
config.khr_no_error_support = PIGLIT_NO_ERRORS;

PIGLIT_GL_TEST_CONFIG_END

static int piglit_error;
static unsigned int vao, vbo;
static unsigned int sdrprog;
static const float varr[] = { -1, -1, 1, -1, 1, 1, -1, -1, 1, 1, -1, 1 };

static const char *vsdr_src =
	"attribute vec4 vertex;\n"
	"void main()\n" "{\n" "gl_Position = vertex;\n" "}\n";

static const char *fsdr_src =
	"void main()\n"
	"{\n" "gl_FragColor = vec4(0.0, 1.0, 0.0, 1.0);\n" "}\n";

void
piglit_init(int argc, char **argv)
{
	int status;

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof varr, varr, GL_STATIC_DRAW);

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	sdrprog = piglit_build_simple_program_unlinked(vsdr_src, fsdr_src);

	glBindAttribLocation(sdrprog, 0, "vertex");
	glLinkProgram(sdrprog);

	glGetProgramiv(sdrprog, GL_LINK_STATUS, &status);
	if (!status) {
		fprintf(stderr, "failed to link program\n");
		piglit_error = 1;
		return;
	}

	glUseProgram(sdrprog);

	if (glGetError() != GL_NO_ERROR)
		piglit_error = 1;
}

enum piglit_result
piglit_display(void)
{
	if (piglit_error)
		return PIGLIT_FAIL;

	glBindVertexArray(vao);

	glViewport(2, 602, 262, 296);

	glDrawArrays(GL_TRIANGLES, 0, 6);

	glViewport(0, 0, 800, 600);

	glDrawArrays(GL_TRIANGLES, 0, 6);

	glBindVertexArray(0);

	if (glGetError() != GL_NO_ERROR)
		return PIGLIT_FAIL;

	piglit_swap_buffers();

	return PIGLIT_PASS;
}
