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
 * \file glsl-explicit-attrib-location-02.c
 * Test GL_ARB_explicit_attrib_location with glBindAttribLocation
 *
 * Load a shader that uses the location layout qualifier on an attribute.
 * Set the attribute to a different location by calling \c glBindAttribLocation.
 * Verify that the attribute is assigned to the location specified in the
 * shader.
 *
 * \author Ian Romanick
 */

#include "piglit-util.h"

int piglit_width = 100, piglit_height = 100;
int piglit_window_mode = GLUT_RGB | GLUT_DOUBLE;

enum piglit_result
piglit_display(void)
{
	return PIGLIT_FAILURE;
}

void piglit_init(int argc, char **argv)
{
	GLint vert;
	GLint prog;
	GLint loc;
	GLboolean ok;

	if (!GLEW_VERSION_2_0) {
		printf("Requires OpenGL 2.0\n");
		piglit_report_result(PIGLIT_SKIP);
	}

	piglit_require_extension("GL_ARB_explicit_attrib_location");

	vert = piglit_compile_shader(GL_VERTEX_SHADER,
				     "shaders/glsl-explicit-location-01.vert");
	prog = glCreateProgram();
	glAttachShader(prog, vert);
	glLinkProgram(prog);

	ok = piglit_link_check_status(prog);
	if (!ok)
		piglit_report_result(PIGLIT_FAILURE);

	glBindAttribLocation(prog, 3, "vertex");
	ok = piglit_link_check_status(prog);
	if (!ok)
		piglit_report_result(PIGLIT_FAILURE);

	loc = glGetAttribLocation(prog, "vertex");
	if (loc != 0) {
		fprintf(stderr,
			"Expected location of 'vertex' to be 0, got "
			"%d instead.\n", loc);
		piglit_report_result(PIGLIT_FAILURE);
	}

	piglit_report_result(PIGLIT_SUCCESS);
}

