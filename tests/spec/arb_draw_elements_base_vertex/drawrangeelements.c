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
 */

/** @file draw-range-elements-base-vertex.c
 *
 * Section 2.8.2(Vertex Arrays) From GL spec 3.2 core:
 * glDrawRangeElementsBaseVertex was added.
 *
 * For DrawRangeElementsBaseVertex, the index values must lie between start
 * and end inclusive, prior to adding the basevertex offset. Index values
 * lying outside the range [start, end] are treated in the same way as
 * DrawRangeElements.
 *
 * It is an error for index values other than the primitive restart index
 * to lie outside the range [start, end], but implementations are not
 * required to check for this. Such indices will cause implementation-
 * dependent behavior.
 *
 *    (0)-------(1)    Set up indices for quad 1 and 3.
 *     |    1    |
 *    (2)-------(3)    Use a basevertex of 2 on quad 3
 *     |    2    |     draw call to shift quad 3 to quad 4
 *    (4)-------(5)
 *     |    3    |    End result quad 1 will be green and quad 2 blue.
 *    (6)-------(7)   If index values are compared to start and end values
 *     |    4    |    prior to adding basevertex quad 3 will be blue,
 *    (8)-------(9)   while 4 will be green.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;
	config.supports_gl_core_version = 31;

	config.window_width  = 200;
	config.window_height = 200;
	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

const char *vs_source = {
	"#version 130\n"
	"in vec2 vertex;\n"
	"void main() {\n"
	"	gl_Position = vec4(vertex.xy, 0, 1);\n"
	"}\n"
};

const char *fs_source = {
	"#version 130\n"
	"void main() {\n"
	"	gl_FragColor = vec4(0, 1, 0, 1);\n"
	"}\n"
};

static GLuint vao;
static GLuint vertexBuffer;
static GLuint indexBuffer;

/* sets of two (x,y) */
static GLfloat vertices[] = {
	-1.0, 1.0,
	 1.0, 1.0,
	-1.0, 0.5,
	 1.0, 0.5,
	-1.0, 0.0,
	 1.0, 0.0,
	-1.0,-0.5,
	 1.0,-0.5,
	-1.0,-1.0,
	 1.0,-1.0
};
static GLsizei vertices_size = sizeof(vertices);

static GLuint indices[] = {
	0, 1, 2, 1, 2, 3, /* Top Quad */
	4, 5, 6, 5, 6, 7, /* Bot Quad */
};
static GLsizei indices_size = sizeof(indices);

void
piglit_init(int argc, char **argv)
{
	GLuint program;
	GLuint vertex_index;

	if (piglit_get_gl_version() < 32) {
		piglit_require_extension("GL_ARB_draw_elements_base_vertex");
		piglit_require_GLSL_version(130);
	}

	/* Create program */
	program = piglit_build_simple_program(vs_source, fs_source);
	glUseProgram(program);

	/* Gen vertex array buffer */
	glGenBuffers(1, &vertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, vertices_size, vertices, GL_STATIC_DRAW);

	/* Gen indices array buffer */
	glGenBuffers(1, &indexBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices_size,
		     indices, GL_STATIC_DRAW);

	/* Gen VAO */
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	/* Retrieve indices from vs */
	vertex_index = glGetAttribLocation(program, "vertex");

	/* Enable vertex attrib array */
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	glEnableVertexAttribArray(vertex_index);
	glVertexAttribPointer(vertex_index, 2, GL_FLOAT, GL_FALSE, 0, 0);
}

enum piglit_result
piglit_display(void)
{
	bool pass = true;

	float green[] = {0, 1, 0};
	float blue[] = {0, 0, 1};

	glClearColor(blue[0], blue[1], blue[2], 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	glBindVertexArray(vao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);

	 /* Top Quad */
	glDrawRangeElementsBaseVertex(GL_TRIANGLES, 0, 3, 6, GL_UNSIGNED_INT,
				      NULL, 0);

	/* Bot Quad */
	glDrawRangeElementsBaseVertex(GL_TRIANGLES, 4, 7, 6, GL_UNSIGNED_INT,
				      (GLvoid *)(sizeof(GLuint)*6), 2);

	/* Check for test pass */
	pass = piglit_probe_pixel_rgb(100, 175, green) && pass;
	pass = piglit_probe_pixel_rgb(100, 125, blue) && pass;
	pass = piglit_probe_pixel_rgb(100,  75, blue) && pass;
	pass = piglit_probe_pixel_rgb(100,  25, green) && pass;

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}
