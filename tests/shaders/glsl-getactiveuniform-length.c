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

/** @file glsl-getactiveuniform-count.c
 *
 * Tests that glGetActiveUniform() has the uniform's string length correctly
 * reflected in GL_ACTIVE_UNIFORM_MAX_LENGTH and the *length outvalue.
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
	GLint vs, fs, len, ret_len;
	GLenum type;
	char *name;
	GLsizei size;

	/* OpenGL ES 3.0 and OpenGL 4.2 require that the "[0]" be appended to
	 * the name.  Earlier versions of the spec are ambiguous.  Accept
	 * either name.
	 */
	const size_t scalar_length = strlen("color");
	const size_t array_length = strlen("color[0]");

	piglit_require_gl_version(20);

	vs = piglit_compile_shader(GL_VERTEX_SHADER,
				   "shaders/glsl-getactiveuniform-length.vert");
	fs = piglit_compile_shader(GL_FRAGMENT_SHADER,
				   "shaders/glsl-color.frag");

	prog = piglit_link_simple_program(vs, fs);

	/* From page 261 (page 275 of the PDF) of the OpenGL 2.1 specification:
	 *
	 *     If pname is ACTIVE UNIFORM MAX LENGTH, the length of
	 *     the longest active uniform name, including a null
	 *     terminator, is returned.
	 */

	glGetProgramiv(prog, GL_ACTIVE_UNIFORM_MAX_LENGTH, &len);
	if (len != (scalar_length + 1) && len != (array_length + 1)) {
		printf("Unexpected max active uniform length "
		       "(saw %d, expected %lu or %lu)\n", len,
		       (unsigned long) scalar_length,
		       (unsigned long) array_length);
		pass = GL_FALSE;
	}

	/* From page 80 (page 88 of the PDF) of the OpenGL 2.1 specification:
	 *
	 *     The actual number of characters written into name,
	 *     excluding the null terminator, is returned in length.
	 */
	name = malloc(len);
	glGetActiveUniform(prog, 0, len + 20, &ret_len, &size, &type, name);

	if (ret_len != scalar_length && ret_len != array_length) {
		printf("Unexpected active uniform length "
		       "(saw %d, expected %lu or %lu) for \"%s\"\n",
		       ret_len,
		       (unsigned long) scalar_length,
		       (unsigned long) array_length,
		       name);
		pass = GL_FALSE;
	}


	if (pass)
		piglit_report_result(PIGLIT_PASS);
	else
		piglit_report_result(PIGLIT_FAIL);
}
