/*
 * Copyright (c) 2020 Simon Zeni
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * on the rights to use, copy, modify, merge, publish, distribute, sub
 * license, and/or sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NON-INFRINGEMENT.  IN NO EVENT SHALL THE AUTHORS OR COYPRIGTH
 * HOLDERS AND/OR THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
 * OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/**
 * @file
 * Tests GL_EXT_draw_instanced
 */

#include "piglit-util-gl.h"
#include "piglit-matrix.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_es_version = 20;

	config.window_width = 400;
	config.window_height = 400;
	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;
	config.khr_no_error_support = PIGLIT_NO_ERRORS;

PIGLIT_GL_TEST_CONFIG_END

static const char *TestName = "ext-draw-instanced";

#define PRIMS 8

/* Ortho projection width, height */
#define W 10.0
#define H 10.0

static const char *VertShaderText =
	"#version 100 \n"
	"#extension GL_EXT_draw_instanced: enable \n"
	"uniform vec4 Colors[8]; \n"
	"uniform vec2 Positions[8]; \n"
	"uniform mat4 MVP; \n"
	"attribute vec2 Vertex; \n"
	"varying vec4 color; \n"
	"void main() \n"
	"{ \n"
	"    vec2 pos = Positions[gl_InstanceIDEXT]; \n"
	"    vec4 p = vec4(Vertex + pos, 0.0, 1.0); \n"
	"    gl_Position = MVP * p; \n"
	"    color = Colors[gl_InstanceIDEXT]; \n"
	"} \n";

static const char *FragShaderText =
	"#version 100 \n"
	"precision highp float;\n"
	"varying vec4 color; \n"
	"void main() \n"
	"{ \n"
	"    gl_FragColor = color; \n"
	"} \n";

static GLuint VertShader, FragShader, Program;
static GLint VertexAttrib;
static GLint ColorsUniform, PositionsUniform, MVPUniform;
static float modelview[16], projection[16], modelviewproj[16];

/* Instance positions in uniform array */
static const GLfloat Positions[PRIMS][2] = {
	{ -6, 6 },
	{ -4, 4 },
	{ -2, 2 },
	{  0, 0 },
	{  2, -2 },
	{  4, -4 },
	{  6, -6 },
	{  8, -8 }
};

/* Instance colors in vertex array */
static const GLfloat Colors[PRIMS][4] = {
	{1, 0, 0, 1},
	{0, 1, 0, 1},
	{0, 0, 1, 1},
	{1, 1, 0, 1},
	{0, 1, 1, 1},
	{1, 0, 1, 1},
	{1, 1, 1, 1},
	{0.5, 0.5, 0.5, 1},
};

/** Convert object position to window position */
static void
objpos_to_winpos(const float obj[2], int win[2])
{
	float winpos[4];
	float objpos[4] = { obj[0], obj[1], 0.0, 1.0 };

	piglit_project_to_window(winpos, objpos, modelview, projection,
				 0, 0, piglit_width, piglit_height);
	win[0] = (int) winpos[0];
	win[1] = (int) winpos[1];
}

enum piglit_result
piglit_display(void)
{
	static const GLfloat verts[4][2] = {
		{-1.0, -1.0}, {1.0, -1.0}, {1.0, 1.0}, {-1.0, 1.0}
	};

	glVertexAttribPointer(VertexAttrib, 2, GL_FLOAT, GL_FALSE, 0, verts);
	glEnableVertexAttribArray(VertexAttrib);

	glViewport(0, 0, piglit_width, piglit_height);
	glClear(GL_COLOR_BUFFER_BIT);

	glUseProgram(Program);

	glDrawArraysInstancedEXT(GL_TRIANGLE_FAN, 0, 4, PRIMS);

	/* check rendering */
	GLint i, pos[2];
	for (i = 0; i < PRIMS; i++) {
		/* compute probe location */
		objpos_to_winpos(Positions[i], pos);

		if (!piglit_probe_pixel_rgba(pos[0], pos[1], Colors[i])) {
			fprintf(stderr, "%s: instance %d failed to draw correctly\n",
				TestName, i);
			return PIGLIT_FAIL;
		}
	}

	glUseProgram(0);

	glDisableVertexAttribArray(VertexAttrib);

	piglit_present_results();

	return PIGLIT_PASS;
}

void
piglit_init(int argc, char **argv)
{
	piglit_require_extension("GL_EXT_draw_instanced");

	VertShader = piglit_compile_shader_text(GL_VERTEX_SHADER, VertShaderText);
	assert(VertShader);

	FragShader = piglit_compile_shader_text(GL_FRAGMENT_SHADER, FragShaderText);
	assert(FragShader);

	Program = piglit_link_simple_program(VertShader, FragShader);

	glUseProgram(Program);

	VertexAttrib = glGetAttribLocation(Program, "Vertex");

	ColorsUniform = glGetUniformLocation(Program, "Colors");
	PositionsUniform = glGetUniformLocation(Program, "Positions");
	MVPUniform = glGetUniformLocation(Program, "MVP");

	glUniform4fv(ColorsUniform, PRIMS, (GLfloat *)Colors);
	glUniform2fv(PositionsUniform, PRIMS, (GLfloat *)Positions);

	/* Setup coordinate transformation */
	piglit_scale_matrix(modelview, 0.5, 0.5, 1.0);
	piglit_ortho_matrix(projection, -0.5 * W, 0.5 * W,
			    -0.5 * H, 0.5 * H, -1.0, 1.0);
	piglit_matrix_mul_matrix(modelviewproj, modelview, projection);

	glUniformMatrix4fv(MVPUniform, 1, GL_FALSE, modelviewproj);

	glUseProgram(0);
}
