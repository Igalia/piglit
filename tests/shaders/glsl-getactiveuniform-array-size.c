/*
 * Copyright © 2009 Intel Corporation
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
 *
 * Authors:
 *    Eric Anholt <eric@anholt.net>
 *
 */

/** @file glsl-vs-sqrt-zero.c
 *
 * Tests that sqrt(0.0) in the VS produces 0.0.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

static GLint prog;

enum piglit_result
piglit_display(void)
{
	/* unreached */
	return PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	GLboolean pass = GL_TRUE;
	GLint vs, fs, len;
	char *name;
	GLsizei size;
	GLenum type;

	piglit_require_gl_version(20);

	vs = piglit_compile_shader(GL_VERTEX_SHADER,
				   "shaders/glsl-getactiveuniform-array-size.vert");
	fs = piglit_compile_shader(GL_FRAGMENT_SHADER,
				   "shaders/glsl-color.frag");

	prog = piglit_link_simple_program(vs, fs);

	glGetProgramiv(prog, GL_ACTIVE_UNIFORM_MAX_LENGTH, &len);
	name = malloc(len + 20);

	glGetActiveUniform(prog, 0, len + 20, &len, &size, &type, name);

	/* From page 81 (page 89 of the PDF) of the OpenGL 2.1 specification:
	 *
	 *     If one or more elements of an array are active,
	 *     GetActiveUniform will return the name of the array in
	 *     name, subject to the restrictions listed above. The
	 *     type of the array is returned in type. The size
	 *     parameter contains the highest array element index
	 *     used, plus one.
	 */

	if (size != 25) {
		printf("Unexpected active uniform size "
		       "(saw %d, expected %d)\n", size, 25);
		pass = GL_FALSE;
	}

	if (pass)
		piglit_report_result(PIGLIT_PASS);
	else
		piglit_report_result(PIGLIT_FAIL);
}
