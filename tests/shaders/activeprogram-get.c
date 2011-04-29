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
 * \file activeprogram-get.c
 * Call glActiveProgramEXT, verify result of glGetInteger(CURRENT_PROGRAM)
 *
 * \author Ian Romanick <ian.d.romanick@intel.com>
 */
#include "piglit-util.h"

int piglit_width = 100, piglit_height = 100;
int piglit_window_mode = GLUT_RGB | GLUT_DOUBLE;

static const char vs_text[] =
	"void main() { gl_Position = gl_Vertex; }";

enum piglit_result
piglit_display(void)
{
	return PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	enum piglit_result result = PIGLIT_PASS;
	GLuint prog[2];
	GLuint got;
	GLuint vs;

	if (!GLEW_VERSION_2_0) {
		printf("Requires OpenGL 2.0\n");
		piglit_report_result(PIGLIT_SKIP);
	}

	piglit_require_extension("GL_EXT_separate_shader_objects");

	vs = piglit_compile_shader_text(GL_VERTEX_SHADER, vs_text);
	prog[0] = piglit_link_simple_program(vs, 0);
	prog[1] = piglit_link_simple_program(vs, 0);

	glUseProgram(prog[0]);
	glGetIntegerv(GL_ACTIVE_PROGRAM_EXT, (GLint *) &got);
	if (got != prog[0]) {
		printf("After calling glUseProgram, "
		       "GL_ACTIVE_PROGRAM_EXT should be %d (got %d)\n",
		       prog[0], got);
		result = PIGLIT_FAIL;
	}

	glActiveProgramEXT(prog[1]);
	glGetIntegerv(GL_ACTIVE_PROGRAM_EXT, (GLint *) &got);
	if (got != prog[1]) {
		printf("After calling glActiveProgramEXT, "
		       "GL_ACTIVE_PROGRAM_EXT should be %d (got %d)\n",
		       prog[1], got);
		result = PIGLIT_FAIL;
	}

	glUseProgram(0);
	glDeleteProgram(prog[0]);
	glDeleteProgram(prog[1]);

	piglit_report_result(result);
}
