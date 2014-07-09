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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

/**
 * \file glsl-link-initializer-03.c
 * Test linking a single shader into two programs.
 *
 * Each of the 3 shaders involved in this test have a global variable called
 * \c global_variable.  Two of the shaders have (differing) initializers for
 * this variable, and the other lacks an initalizer.  The test verifies that
 * the shader lacking an initializer can successfully be linked with each of
 * the shaders that have initalizers.
 *
 * \author Ian Romanick
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

enum piglit_result
piglit_display(void)
{
	return PIGLIT_FAIL;
}

void piglit_init(int argc, char **argv)
{
	GLint vert[3];
	GLint prog_a;
	GLint prog_b;
	GLint prog_c;
	GLint prog_d;

	piglit_require_gl_version(20);

	vert[0] =
		piglit_compile_shader(GL_VERTEX_SHADER,
				      "shaders/glsl-link-initializer-01a.vert");
	vert[1] =
		piglit_compile_shader(GL_VERTEX_SHADER,
				      "shaders/glsl-link-initializer-01b.vert");
	vert[2] =
		piglit_compile_shader(GL_VERTEX_SHADER,
				      "shaders/glsl-link-initializer-01c.vert");
	prog_a = piglit_link_simple_program(vert[0], vert[1]);
	prog_b = piglit_link_simple_program(vert[1], vert[0]);
	prog_c = piglit_link_simple_program(vert[0], vert[2]);
	prog_d = piglit_link_simple_program(vert[2], vert[0]);

	/* piglit_link_simple_program() returns 0 on link failure.  So
	 * verify that there was no link failure by simply checking
	 * that two programs were returned.
	 */
	piglit_report_result((prog_a && prog_b && prog_c && prog_d)
			     ? PIGLIT_PASS : PIGLIT_FAIL);
}

