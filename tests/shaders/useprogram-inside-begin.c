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
 * \file useprogram-inside-begin.c
 * Verify that calling glUseProgram inside a glBegin/glEnd pair causes an error
 *
 * \author Ian Romanick <ian.d.romanick@intel.com>
 */
#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

static const char vs_text[] =
	"void main() { gl_Position = gl_Vertex; }";

static const char fs_text[] =
	"void main() { gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0); }";

enum piglit_result
piglit_display(void)
{
	return PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	enum piglit_result result = PIGLIT_PASS;
	GLint prog;
	GLint fs;
	GLint vs;
	GLenum err;

	piglit_require_gl_version(20);

	vs = piglit_compile_shader_text(GL_VERTEX_SHADER, vs_text);
	fs = piglit_compile_shader_text(GL_FRAGMENT_SHADER, fs_text);

	prog = piglit_link_simple_program(vs, fs);
	glUseProgram(prog);

	/* There shouldn't be any GL errors, but clear them all just to be
	 * sure.
	 */
	while (glGetError() != 0)
		/* empty */ ;

	glBegin(GL_TRIANGLE_STRIP);
	glUseProgram(0);
	glEnd();

	err = glGetError();
	if (err != GL_INVALID_OPERATION) {
		printf("Unexpected OpenGL error state 0x%04x for "
		       "glUseProgram(0) "
		       "inside glBegin/glEnd pair (expected 0x%04x).\n",
		       err, GL_INVALID_OPERATION);
		result = PIGLIT_FAIL;
	}

	while (glGetError() != 0)
		/* empty */ ;

	/* Try again, but re-use the same program.  This should still generate
	 * an error even though it's a no-op.
	 */
	glUseProgram(prog);

	glBegin(GL_TRIANGLE_STRIP);
	glUseProgram(prog);
	glEnd();

	err = glGetError();
	if (err != GL_INVALID_OPERATION) {
		printf("Unexpected OpenGL error state 0x%04x for "
		       "glUseProgram(prog) "
		       "inside glBegin/glEnd pair (expected 0x%04x).\n",
		       err, GL_INVALID_OPERATION);
		result = PIGLIT_FAIL;
	}

	piglit_report_result(result);
}
