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

/** @file multi-draw-elements-base-vertex.c
 *
 * Section 2.8.2(Vertex Arrays) From GL spec 3.2 core:
 * glMultiDrawElementsBaseVertex was added.
 *
 *
 *    (0)-------(1)    Set up indices for quad 1 and 3.
 *     |    1    |
 *    (2)-------(3)    Use a basevertex of 2 to shift
 *     |    2    |     indices from quad 1 to 2 and
 *    (4)-------(5)    from quad 3 to 4
 *     |    3    |
 *    (6)-------(7)    End result 1 and 3 should be
 *     |    4    |     blue while 2 and 4 are green.
 *    (8)-------(9)
 *
 *
 * MultiDrawElementsBaseVertex behaves identically to
 * DrawElementsBaseVertex, except that primcount separate
 * lists of elements are specified instead. It has the
 * same effect as:
 *
 * for (int i = 0; i < primcount ; i++)
 *     if (count[i] > 0)
 *         DrawElementsBaseVertex(mode, count[i], type,
 *                               indices[i], basevertex[i]);
 *
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

/*
 * sets of two (x,y)
 */
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
	0, 1, 2, 1, 2, 3, /* top square */
	4, 5, 6, 5, 6, 7, /* bot square */
};
static GLsizei indices_size = sizeof(indices);

static const GLvoid * const indices_offset[] = {
	(GLvoid*) 0, (GLvoid*)(6 * sizeof(GLuint))
};
static GLsizei indices_count[] = {
	6, 6
};

static GLint basevertex[] = { 2, 2 };

void
piglit_init(int argc, char **argv)
{
	GLuint program;
	GLuint vertex_index;

	piglit_require_GLSL_version(130);

	if (piglit_get_gl_version() < 32) {
		piglit_require_extension("GL_ARB_draw_elements_base_vertex");
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

	/* Retrieve index from vs */
	vertex_index = glGetAttribLocation(program, "vertex");

	/* Enabel vertexAttribPointer */
	glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
	glEnableVertexAttribArray(vertex_index);
	glVertexAttribPointer(vertex_index, 2, GL_FLOAT, GL_FALSE, 0, 0);

	if(!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);
}

enum piglit_result
piglit_display(void)
{
	bool pass = true;
	float green[] = {0, 1, 0};
	float blue[]  = {0, 0, 1};

	glClearColor(blue[0], blue[1], blue[2], 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	glBindVertexArray(vao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuffer);

	glMultiDrawElementsBaseVertex(GL_TRIANGLES, indices_count,
				      GL_UNSIGNED_INT, (GLvoid*)indices_offset,
				      2, basevertex);

	/* Check for test pass */
	pass = piglit_probe_pixel_rgb(100, 175, blue) && pass;
	pass = piglit_probe_pixel_rgb(100, 125, green) && pass;
	pass = piglit_probe_pixel_rgb(100,  75, blue) && pass;
	pass = piglit_probe_pixel_rgb(100,  25, green) && pass;

	if(!piglit_check_gl_error(GL_NO_ERROR))
		pass = false;

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}
