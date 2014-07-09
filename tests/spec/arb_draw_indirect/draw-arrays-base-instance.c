/*
 * Copyright © 2013 Intel Corporation
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
 */

/* Basic test of glDrawArraysIndirect interaction with base instance */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_core_version = 31;

	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGB;

PIGLIT_GL_TEST_CONFIG_END

GLuint vao;
GLint prog;

float green[] = {0,1,0};
float blue[] = {0,0,1};

enum piglit_result
piglit_display(void)
{
	bool pass = true;

	glViewport(0, 0, 128, 128);

	glClearColor(0,0,1,1);
	glClear(GL_COLOR_BUFFER_BIT);

	glBindVertexArray(vao);
	glUseProgram(prog);

	glDrawArraysIndirect(GL_TRIANGLES, (GLvoid const *)0);

	glUseProgram(0);

	piglit_present_results();

	pass = piglit_probe_pixel_rgb(32, 32, green) && pass;
	pass = piglit_probe_pixel_rgb(96, 96, blue) && pass;

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

float vertices_data[] = {
	-1, -1,
	 1, -1,
	-1,  1,
};

int instance_data[] = { 0, 0, 0, 0, 42, 0, 0 };

GLuint indirect_data[] = {
	3,		/* count */
	1,		/* primcount */
	0,		/* first vertex */
	4,		/* base instance */
};

void
piglit_init(int argc, char **argv)
{
	GLuint vertices_bo;
	GLuint instance_bo;
	GLuint indirect_bo;

	piglit_require_extension("GL_ARB_draw_indirect");
	piglit_require_extension("GL_ARB_base_instance");
	piglit_require_extension("GL_ARB_instanced_arrays");

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glGenBuffers(1, &vertices_bo);
	glBindBuffer(GL_ARRAY_BUFFER, vertices_bo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices_data), vertices_data, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);

	glGenBuffers(1, &instance_bo);
	glBindBuffer(GL_ARRAY_BUFFER, instance_bo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(instance_data), instance_data, GL_STATIC_DRAW);
	glEnableVertexAttribArray(1);
	glVertexAttribIPointer(1, 1, GL_INT, 0, 0);
	glVertexAttribDivisorARB(1, 1);

	glGenBuffers(1, &indirect_bo);
	glBindBuffer(GL_DRAW_INDIRECT_BUFFER, indirect_bo);
	glBufferData(GL_DRAW_INDIRECT_BUFFER, sizeof(indirect_data), indirect_data, GL_STATIC_DRAW);

	prog = piglit_build_simple_program(
			"#version 130\n"
			"#extension GL_ARB_explicit_attrib_location: require\n"
			"#extension GL_ARB_draw_instanced: require\n"
			"\n"
			"layout(location=0) in vec2 pos;\n"
			"layout(location=1) in int instance_in;\n"
			"\n"
			"flat out int instance;\n"
			"\n"
			"void main() {\n"
			"	gl_Position = vec4(pos, 0, 1);\n"
			"	instance = instance_in;\n"
			"}\n",

			"#version 130\n"
			"\n"
			"flat in int instance;\n"
			"\n"
			"void main() {\n"
			"	if (instance == 42) {\n"
			"		gl_FragColor = vec4(0,1,0,1);\n"
			"	} else {\n"
			"		gl_FragColor = vec4(1,0,0,1);\n"
			"	}\n"
			"}\n");

	glBindVertexArray(0);
}
