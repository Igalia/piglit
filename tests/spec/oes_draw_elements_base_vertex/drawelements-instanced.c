/* Copyright © 2009 Intel Corporation
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

/* Tests OES_draw_elements_base_vertex functionality by drawing a checkerboard
 * of quads using different base vertices using the same vertex and
 * index buffers and instancing
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_es_version = 30;

	config.window_width = 300;
	config.window_height = 300;
	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

#define NUM_QUADS 10
const GLfloat inc_amount = 2.0 / NUM_QUADS;
const int window_width = 300;
const int window_height = 300;

const char *vs_source = {
	"#version 300 es\n"
	"in vec2 vertex;\n"
	"in float xOffsetPerInstance;\n"
	"void main() {\n"
	"	vec2 p = vertex;\n"
	"	p.y -= 1.0 * float(gl_InstanceID);\n"
	"	p.x += xOffsetPerInstance * float(gl_InstanceID);\n"
	"	gl_Position = vec4(p, 0, 1);\n"
	"}\n"
};

const char *fs_source = {
	"#version 300 es\n"
	"out highp vec4 ocol;\n"
	"void main() {\n"
	"	ocol = vec4(1, 1, 1, 1);\n"
	"}\n"
};

static GLushort indices[] = {
	0, 1, 2, 1, 2, 3
};
static GLsizei indices_size = sizeof(indices);

static GLuint vao;
static GLuint vertexBuffer;
static GLuint indexBuffer;

void
piglit_init(int argc, char **argv)
{
	piglit_require_extension("GL_OES_draw_elements_base_vertex");

	GLuint program;
	GLuint vertex_index;
	GLuint xoffset_index;

	GLfloat* vertices = malloc(4 * 2 * NUM_QUADS * sizeof(GLfloat));
	GLsizei vertices_size = 4 * 2 * NUM_QUADS * sizeof(GLfloat);

	// Generate a checkerboard pattern
	// |x x x x x |
	// | x x x x x|
	int i;
	for (i = 0; i < NUM_QUADS; ++i)
	{
		GLfloat xoffset, yoffset;
		GLfloat x[4], y[4];

		xoffset = inc_amount * i - 1.0;
		yoffset = 1.0;

		// Top-left
		x[0] = xoffset;
		y[0] = yoffset;

		// Top-right
		x[1] = xoffset + inc_amount / 2.0;
		y[1] = yoffset;

		// Bottom-left
		x[2] = xoffset;
		y[2] = yoffset - 1.0;

		// Bottom-right
		x[3] = xoffset + inc_amount / 2.0;
		y[3] = yoffset - 1.0;

		vertices[i * 8 + 0] = x[0];
		vertices[i * 8 + 1] = y[0];

		vertices[i * 8 + 2] = x[1];
		vertices[i * 8 + 3] = y[1];

		vertices[i * 8 + 4] = x[2];
		vertices[i * 8 + 5] = y[2];

		vertices[i * 8 + 6] = x[3];
		vertices[i * 8 + 7] = y[3];
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

	xoffset_index = glGetAttribLocation(program, "xOffsetPerInstance");
	glVertexAttrib1f(xoffset_index, inc_amount / 2.0);

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
	GLboolean pass = GL_TRUE;
	float white[] = {1.0, 1.0, 1.0, 1.0};
	int i;

	glClear(GL_COLOR_BUFFER_BIT);

	glBindVertexArray(vao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);

	for (i = 0; i < NUM_QUADS; ++i)
		glDrawElementsInstancedBaseVertexOES(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, NULL, 2, i * 4);

	for (i = 0; i < (NUM_QUADS * 2); ++i)
	{
		GLfloat xoffset[2], yoffset[2];

		xoffset[0] = inc_amount * i / 4.0 * window_width;
		xoffset[1] = inc_amount * (i + 1) / 4.0 * window_width;
		yoffset[0] = (i % 2 ? 0.0 : 0.5) * window_height;
		yoffset[1] = yoffset[0] + window_height / 2;

		pass = piglit_probe_rect_rgba(xoffset[0], yoffset[0],
			xoffset[1] - xoffset[0], yoffset[1] - yoffset[0], white) && pass;
	}

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}
