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
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
 * AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/*
 * Test verifies that when sampling from three adjoining faces in a cube map,
 * samples will be averaged. If they share the same value, that value must be
 * guaranteed to be the result of the average. Resulting color should not
 * include border color contamination.
 */

/*
 * ARB_seamless_cube_map Section 3.8.7 says:
 *     "If LINEAR filtering is done within a miplevel, always apply wrap mode
 *     CLAMP_TO_BORDER. Then, ...
 *
 *     If a texture sample location would lie in the texture border in
 *     both u and v (in one of the corners of the cube), there is no
 *     unique neighboring face from which to extract one texel. The
 *     recommended method is to average the values of the three
 *     available samples. However, implementations are free to
 *     construct this fourth texel in another way, so long as, when the
 *     three available samples have the same value, this texel also has
 *     that value."
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;
	config.supports_gl_core_version = 31;

	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

const char *vs_text =
{
	"#version 130\n"
	"\n"
	"in vec2 vertex;\n"
	"\n"
	"void main() {\n"
	"	gl_Position = vec4(vertex.xy, 0, 1);\n"
	"}\n"
};

const char *fs_text =
{
	"#version 130\n"
	"\n"
	"uniform samplerCube cubeTex;\n"
	"uniform vec3 cubeVec;\n"
	"\n"
	"void main() {\n"
	"	gl_FragColor = texture(cubeTex, cubeVec);\n"
	"}\n"
};

static const float red[3] = { 1., 0., 0. };
static const float blue[3] = { 0., 0., 1. };
static const float green[3] = { 0., 1.0, 0. };

static GLuint prog;
static GLuint vao;
static GLuint vbo;
static GLuint cubeMap;

static GLint cubeVec_loc;
static GLfloat cubeVecPositive[3] = { 0.5, 0.5, 0.5 };
static GLfloat cubeVecNegative[3] = { -0.5, -0.5, -0.5 };

static const GLenum targets[6] = {
   GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB,
   GL_TEXTURE_CUBE_MAP_POSITIVE_Y_ARB,
   GL_TEXTURE_CUBE_MAP_POSITIVE_Z_ARB,
   GL_TEXTURE_CUBE_MAP_NEGATIVE_X_ARB,
   GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_ARB,
   GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_ARB
};

static GLfloat quad_01[4][2] =
{
	{ -1., -1. },
	{ -1.,  0. },
	{  0.,  0. },
	{  0., -1. }
};

static GLfloat quad_02[4][2] =
{
	{ 0., 0. },
	{ 0., 1. },
	{ 1., 1. },
	{ 1., 0. }
};

void piglit_init( int argc, char **argv)
{
	GLint i;
	GLuint vertex_index;

	if(piglit_get_gl_version() < 32) {
		piglit_require_extension("GL_ARB_seamless_cube_map");
		piglit_require_GLSL_version(130);
	}

	/* create program */
	prog = piglit_build_simple_program(vs_text, fs_text);
	glUseProgram(prog);

	/* create buffers */
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quad_01) + sizeof(quad_02),
		      NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(quad_01),
			 &quad_01);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(quad_01), sizeof(quad_02),
			 &quad_02);

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	vertex_index = glGetAttribLocation(prog, "vertex");

	/* vertex attribs */
	glEnableVertexAttribArray(vertex_index);

	glVertexAttribPointer(vertex_index, 2, GL_FLOAT, GL_FALSE, 0, 0);

	glGenTextures(1, &cubeMap);
	glBindTexture(GL_TEXTURE_CUBE_MAP_ARB, cubeMap);

	/* set filter to linear, hardware should behave as if wrap modes are
	 * set to CLAMP_TO_BORDER
	 */
	glTexParameterfv(GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_BORDER_COLOR,
			  green);
	glTexParameteri(GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_MIN_FILTER,
			 GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_MAG_FILTER,
			 GL_LINEAR);

	/* texture positive axes/faces red */
	for(i = 0; i < 3; i++) {
		glTexImage2D(targets[i], 0, GL_RGBA8, 1, 1, 0, GL_RGB,
			     GL_FLOAT, red);
	}

	/* texuture negative axes/faces blue */
	for(i = 3; i < 6; i++) {
		glTexImage2D(targets[i], 0, GL_RGBA8, 1, 1, 0, GL_RGB,
			     GL_FLOAT, blue);
	}

	/* uniform texcoord input */
	cubeVec_loc = glGetUniformLocation(prog, "cubeVec");

	if(!piglit_check_gl_error(GL_NO_ERROR))
	       piglit_report_result(PIGLIT_FAIL);

}

enum piglit_result piglit_display(void)
{
	bool pass = true;
	int w = piglit_width / 2;
	int h = piglit_height / 2;

	glViewport(0, 0, piglit_width, piglit_height);

	glClear(GL_COLOR_BUFFER_BIT);

	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

	/* texcoords should target vector to upper corner */
	glUniform3fv(cubeVec_loc, 1, cubeVecPositive);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	/* expect red */
	pass = piglit_probe_rect_rgb(0, 0, w, h, red) && pass;

	/* texcoords should target vector to bottom corner */
	glUniform3fv(cubeVec_loc, 1, cubeVecNegative);
	glDrawArrays(GL_TRIANGLE_FAN, 4, 4);

	/* expect blue */
	pass = piglit_probe_rect_rgb(w, h, w, h, blue) && pass;

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}
