/**
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

/**
 * Verify that the baseinstance setting (from GL_ARB_base_instance) does
 * not affect the value of gl_InstanceID.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_core_version = 31;

	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

static const char *vstext =
	"#version 140\n"
	"in vec3 vertex;\n"
	"out vec4 passColor;\n"
	"void main() {\n"
	"	if(gl_InstanceID != 0) passColor = vec4(1, 0, 0, 1);\n"
	"	else passColor = vec4(0, 1, 0, 1);\n"
	"	gl_Position = vec4(vertex, 1.);\n"
	"}\n";

static const char *fstext =
	"#version 140\n"
	"in vec4 passColor;\n"
	"out vec4 color;\n"
	"void main() {\n"
	"	color = passColor;\n"
	"}\n";

static GLuint vao;
static GLuint vertBuff;
static GLuint indexBuf;

static GLfloat vertices[] = {
	-1.0, 1.0, 0.0,
	 1.0, 1.0, 0.0,
	 1.0,-1.0, 0.0,
	-1.0,-1.0, 0.0
};
static GLsizei vertSize = sizeof(vertices);

static GLuint indices[] = {
	0, 1, 2, 0, 2, 3
};
static GLsizei indSize = sizeof(indices);

static GLuint prog;

void
piglit_init(int argc, char **argv)
{
	GLuint vertIndex;

	piglit_require_extension("GL_ARB_base_instance");

	prog = piglit_build_simple_program(vstext, fstext);

	glUseProgram(prog);

	glGenBuffers(1, &vertBuff);
	glBindBuffer(GL_ARRAY_BUFFER, vertBuff);
	glBufferData(GL_ARRAY_BUFFER, vertSize, vertices, GL_STATIC_DRAW);

	glGenBuffers(1, &indexBuf);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuf);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indSize,
			indices, GL_STATIC_DRAW);

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	vertIndex = glGetAttribLocation(prog, "vertex");

	glBindBuffer(GL_ARRAY_BUFFER, vertBuff);
	glEnableVertexAttribArray(vertIndex);
	glVertexAttribPointer(vertIndex, 3, GL_FLOAT, GL_FALSE, 0, 0);
}

enum piglit_result
piglit_display(void)
{
	bool pass = true;
	float green[] = {0, 1, 0};

	glClearColor(.4, .4, .4, 1.);
	glClear(GL_COLOR_BUFFER_BIT);

	glBindVertexArray(vao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuf);

	/* use non zero baseinstance value to verify it doesn't affect
	 * gl_InstanceID
	 */
	glDrawElementsInstancedBaseInstance(GL_TRIANGLES, ARRAY_SIZE(indices),
			GL_UNSIGNED_INT, NULL, 1, 15);

	pass = piglit_probe_rect_rgb(0, 0, piglit_width, piglit_height, green)
		&& pass;

	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}
