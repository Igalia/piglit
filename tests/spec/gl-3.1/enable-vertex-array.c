/*
 * Copyright 2016 VMware, Inc.
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
 * Exercise a failed assertion bug in Mesa when enabling legacy vertex arrays
 * with a core profile GL context.
 *
 * Brian Paul
 * 23 May 2016
 */


#include "piglit-util-gl.h"
#include "piglit-matrix.h"

PIGLIT_GL_TEST_CONFIG_BEGIN
	config.supports_gl_core_version = 31;
	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;
	config.khr_no_error_support = PIGLIT_NO_ERRORS;
PIGLIT_GL_TEST_CONFIG_END


#define VERTEX_SIZE (2 * sizeof(GLfloat))

static const float white[4] = { 1.0, 1.0, 1.0, 1.0 };

static const float triangle_fan_verts[][2] = {
	{-1, -.75},
	{-0.5, 0.75},
	{ 0.5, 0.75},
	{ 1.0, -.75},
};

#define NUM_VERTS(ARRAY)  (sizeof(ARRAY) / VERTEX_SIZE)

static GLuint triangle_fan_vao;
static GLuint program;
static GLint colorUniform, modelViewProjUniform;



static GLuint
make_program(void)
{
	static const char *vs_text =
		"#version 130 \n"
		"in vec4 vertex; \n"
		"uniform vec4 color; \n"
		"uniform mat4 modelViewProj; \n"
		"out vec4 vs_fs_color; \n"
		"void main() \n"
		"{ \n"
		"   gl_Position = vertex * modelViewProj; \n"
		"   vs_fs_color = color; \n"
		"} \n";

	static const char *fs_text =
		"#version 130 \n"
		"in vec4 vs_fs_color; \n"
		"void main() \n"
		"{ \n"
		"   gl_FragColor = vs_fs_color; \n"
		"} \n";

	GLuint program = piglit_build_simple_program(vs_text, fs_text);

	return program;
}


static GLuint
create_vao(const GLfloat (*verts)[2], GLuint numVerts)
{
	GLuint vao, vbo;

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, numVerts * VERTEX_SIZE,
		     verts, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, VERTEX_SIZE, NULL);

	return vao;
}


void
piglit_init(int argc, char **argv)
{
	triangle_fan_vao = create_vao(triangle_fan_verts,
				      NUM_VERTS(triangle_fan_verts));
	program = make_program();
}


enum piglit_result
piglit_display(void)
{
	GLfloat ortho[16];
	bool pass = true;

	glViewport(0, 0, piglit_width, piglit_height);
	glClear(GL_COLOR_BUFFER_BIT);

	glUseProgram(program);

	modelViewProjUniform = glGetUniformLocation(program, "modelViewProj");
	piglit_ortho_matrix(ortho, -2, 2, -2, 2, -1, 1);
	glUniformMatrix4fv(modelViewProjUniform, 1, GL_FALSE, ortho);

	colorUniform = glGetUniformLocation(program, "color");
	glUniform4fv(colorUniform, 1, white);

	glBindVertexArray(triangle_fan_vao);

	if (!piglit_khr_no_error) {
		// This call should be illegal and raise an error with core
		// profile.  If it actually works, it may trigger a failed
		// assertion in Mesa.
		glEnable(GL_VERTEX_ARRAY);

		if (!piglit_check_gl_error(GL_INVALID_ENUM)) {
			printf("Failed to detect invalid glEnable(GL_VERTEX_ARRAY)\n");
			pass = false;
		}
	}

	// This is the correct call to use:
	glEnableVertexAttribArray(0);

	glDrawArrays(GL_TRIANGLE_FAN, 0, NUM_VERTS(triangle_fan_verts));

	piglit_present_results();

	if (!piglit_probe_pixel_rgba(piglit_width/2, piglit_height/2, white))
		pass = false;

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}
