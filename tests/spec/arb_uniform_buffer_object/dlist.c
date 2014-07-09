/*
 * Copyright © 2011 Intel Corporation
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

#include "piglit-util-gl.h"

/**
 * @file dlist.c
 *
 * Tests that conditional rendering appropriately affects commands
 * inside of display lists.  From the GL_ARB_uniform_buffer_object
 * spec:
 *
 *     "(33) Which uniform buffer object commands must be excluded
 *      from display lists?
 *
 *      RESOLUTION:  Resolved
 *
 *      When used with 3.1 (where display lists have been removed
 *      altogether) obviously, this question is moot.
 *
 *      For GL 2.0/3.0, this should be resolved with the following
 *      precedents:
 *
 *      ...
 *
 *      UniformBlockBinding should follow the precedent of glUniform (for
 *      setting samplers) which *does* get included in display lists.
 *
 *      ...
 *
 *      Since we use the BindBufferOffset/BindBufferRange API
 *      introduced by OpenGL 3.0, and those routines are already
 *      excluded, there's no additions to the display list exclusion
 *      list needed."
 */

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;

PIGLIT_GL_TEST_CONFIG_END

const char *source =
	"#extension GL_ARB_uniform_buffer_object : enable\n"
	"uniform A { float a; };\n"
	"uniform B { float b; };\n"
	"void main() {\n"
	"	gl_FragColor = vec4(a + b);\n"
	"}\n";

enum piglit_result
piglit_display(void)
{
	/* UNREACHED */
	return PIGLIT_FAIL;
}

static bool
check_binding(int line, GLuint prog, int index, int binding)
{
	GLint current_binding;

	glGetActiveUniformBlockiv(prog, index, GL_UNIFORM_BLOCK_BINDING,
				  &current_binding);

	if (current_binding != binding) {
		printf("%s:%d: Binding %d should be %d, was %d\n",
		       __FILE__, line, index, binding, current_binding);
		return false;
	}

	return true;
}


void
piglit_init(int argc, char **argv)
{
	bool pass = true;
	GLuint prog;
	GLuint bo[2];
	GLint current_bo;
	GLint list;

	piglit_require_extension("GL_ARB_uniform_buffer_object");

	prog = piglit_build_simple_program(NULL, source);

	/* Test that glUniformBlockBinding() goes into display lists. */
	glUniformBlockBinding(prog, 0, 0);
	glUniformBlockBinding(prog, 0, 1);

	list = glGenLists(1);
	glNewList(list, GL_COMPILE_AND_EXECUTE);
	glUniformBlockBinding(prog, 0, 2);
	glUniformBlockBinding(prog, 1, 3);
	glEndList();

	pass = check_binding(__LINE__, prog, 0, 2) && pass;
	pass = check_binding(__LINE__, prog, 1, 3) && pass;

	glUniformBlockBinding(prog, 0, 0);
	glUniformBlockBinding(prog, 0, 1);

	glCallList(list);

	pass = check_binding(__LINE__, prog, 0, 2) && pass;
	pass = check_binding(__LINE__, prog, 1, 3) && pass;

	/* Test that glBindBufferBase()/glBindBufferRange() fail
	 * inside a display list.
	 */
	glGenBuffers(2, bo);
	glBindBuffer(GL_UNIFORM_BUFFER, bo[0]);
	glBufferData(GL_UNIFORM_BUFFER, 4, NULL, GL_STATIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, bo[1]);
	glBufferData(GL_UNIFORM_BUFFER, 4, NULL, GL_STATIC_DRAW);

	pass = piglit_check_gl_error(0) && pass;

	glBindBufferBase(GL_UNIFORM_BUFFER, 0, bo[0]);
	glBindBufferBase(GL_UNIFORM_BUFFER, 1, bo[0]);

	glNewList(list, GL_COMPILE);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, bo[1]);
	glBindBufferRange(GL_UNIFORM_BUFFER, 1, bo[1], 0, 4);
	glEndList();

	glGetIntegeri_v(GL_UNIFORM_BUFFER_BINDING, 0, &current_bo);
	if (current_bo != bo[1]) {
		fprintf(stderr, "glBindBufferBase() during display list compile "
			"set BO to %d, expected %d\n",
			current_bo, bo[1]);
	}

	glGetIntegeri_v(GL_UNIFORM_BUFFER_BINDING, 1, &current_bo);
	if (current_bo != bo[1]) {
		fprintf(stderr, "glBindBufferRange() during display list "
			"compile set BO to %d, expected %d\n",
			current_bo, bo[1]);
	}


	glBindBufferBase(GL_UNIFORM_BUFFER, 0, bo[0]);
	glBindBufferBase(GL_UNIFORM_BUFFER, 1, bo[0]);
	glCallList(list);

	glGetIntegeri_v(GL_UNIFORM_BUFFER_BINDING, 0, &current_bo);
	if (current_bo != bo[0]) {
		fprintf(stderr, "glBindBufferBase() during display list exec "
			"set BO to %d, expected %d\n",
			current_bo, bo[0]);
	}

	glGetIntegeri_v(GL_UNIFORM_BUFFER_BINDING, 1, &current_bo);
	if (current_bo != bo[0]) {
		fprintf(stderr, "glBindBufferRange() during display list exec  "
			"set BO to %d, expected %d\n",
			current_bo, bo[0]);
	}

	pass = piglit_check_gl_error(0) && pass;

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}
