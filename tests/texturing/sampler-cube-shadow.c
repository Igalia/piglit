/*
 * Copyright © 2011 Intel Corporation
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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

/* Author: Anuj Phogat */
/*
 * Test to verify samplerCubeShadow support
 * It is valid for GL version >= 3.0 and GLSL version >= 1.30
 *
 * This test works by drawing 6 polygons using each cubemap face as a depth
 * texture for shadow comparisons. Color of pixels inside the polygon is
 * decided by shadow comparison between texture's depth value and provided
 * reference value ('q'texture coordinate)
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_width = 400;
	config.window_height = 300;
	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

static GLuint tex;
static GLint prog;
static GLint fs;
static GLint vs;

static const GLint stride = 8 * sizeof(GLfloat);
/* These are interlaced vertex coordinates and texture coordinates.
 * The vertex coordinates specify 6 quads set in a 3x2 grid with some space
 * in between.
 * The texture coordinates should have 1 or -1 in the major axis
 * ('r' coordinate) selecting the face, a nearly-1-or-negative-1 value
 * in the other two coordinates (s,t) and a reference value ('q' coordinate)
 * used for shadow comparisons
 */
static GLfloat cube_shadow_attributes[6][8][4] = {
	{ /* GL_TEXTURE_CUBE_MAP_POSITIVE_X */
		{100,   125,     0,     1},
		{1.0,  0.99,  0.99, -0.50},
		{150,   125,     0,     1},
		{1.0,  0.99, -0.99,  0.00},
		{150,   175,     0,     1},
		{1.0, -0.99, -0.99,  0.50},
		{100,   175,     0,     1},
		{1.0, -0.99,  0.99,  0.00},
	},
	{ /* GL_TEXTURE_CUBE_MAP_NEGATIVE_X */
		{175,    125,     0,     1},
		{-1.0,  0.99, -0.99,  0.90},
		{225,    125,     0,     1},
		{-1.0,  0.99,  0.99,  0.20},
		{225,    175,     0,     1},
		{-1.0, -0.99,  0.99, -0.50},
		{175,    175,     0,     1},
		{-1.0, -0.99, -0.99,  0.20},
	},
	{ /* GL_TEXTURE_CUBE_MAP_POSITIVE_Y */
		{250,    125,     0,    1},
		{-0.99, 1.0, -0.99,  0.35},
		{300,    125,     0,    1},
		{ 0.99, 1.0, -0.99,  1.20},
		{300,    175,     0,    1},
		{ 0.99, 1.0,  0.99,  0.35},
		{250,    175,     0,    1},
		{-0.99, 1.0,  0.99, -0.50},
	},
	{ /* GL_TEXTURE_CUBE_MAP_NEGATIVE_Y */
		{100,    200,     0,     1},
		{-0.99, -1.0,  0.99,  0.50},
		{150,    200,     0,     1},
		{-0.99, -1.0, -0.99, -0.50},
		{150,    250,     0,     1},
		{ 0.99, -1.0, -0.99,  0.50},
		{100,    250,     0,     1},
		{ 0.99, -1.0,  0.99,  1.50},
	},
	{ /* GL_TEXTURE_CUBE_MAP_POSITIVE_Z */
		{175,    200,     0,   1},
		{-0.99,  0.99, 1.0, 0.85},
		{225,    200,     0,   1},
		{-0.99, -0.99, 1.0, 0.85},
		{225,    250,     0,   1},
		{ 0.99, -0.99, 1.0, 0.85},
		{175,    250,     0,   1},
		{ 0.99,  0.99, 1.0, 0.85},
	},
	{ /* GL_TEXTURE_CUBE_MAP_NEGATIVE_Z */
		{250,    200,     0,    1},
		{ 0.99,  0.99, -1.0, 0.90},
		{300,    200,     0,    1},
		{-0.99,  0.99, -1.0, 0.90},
		{300,    250,     0,    1},
		{-0.99, -0.99, -1.0, 0.90},
		{250,    250,     0,    1},
		{ 0.99, -0.99, -1.0, 0.90},
	},
};

static const char *vertShaderText =
	"#version 130\n"
	"in vec4 vertex;\n"
	"in vec4 texCoord;\n"
	"void main()\n"
	"{\n"
	"	gl_Position = gl_ModelViewProjectionMatrix * vertex;\n"
	"	gl_TexCoord[0] = texCoord;\n"
	"}\n";

static const char *fragShaderText =
	"#version 130\n"
	"uniform samplerCubeShadow cubeShadow;\n"
	"void main()\n"
	"{\n"
	"	float shadow  = texture(cubeShadow, gl_TexCoord[0]);\n"
	"	gl_FragColor = vec4(shadow, shadow, shadow, 1.0);\n"
	"}\n";

static void
shaderSetup(void)
{
	vs = piglit_compile_shader_text(GL_VERTEX_SHADER, vertShaderText);
	fs = piglit_compile_shader_text(GL_FRAGMENT_SHADER, fragShaderText);
	prog = piglit_link_simple_program(vs, fs);
	glUseProgram(prog);
}


static void
loadTex(void)
{
	#define height 2
	#define width 2
	int i, j;

	GLfloat texDepthDataPosX[width][height];
	GLfloat texDepthDataNegX[width][height];
	GLfloat texDepthDataPosY[width][height];
	GLfloat texDepthDataNegY[width][height];
	GLfloat texDepthDataPosZ[width][height];
	GLfloat texDepthDataNegZ[width][height];

	/* Set the cubemap depth values for each face */
	for (i=0; i < height; ++i) {
		for (j=0; j < width; ++j) {
		    texDepthDataPosX[i][j] = 0.0;
		    texDepthDataNegX[i][j] = 0.2;
		    texDepthDataPosY[i][j] = 0.35;
		    texDepthDataNegY[i][j] = 0.50;
		    texDepthDataPosZ[i][j] = 0.75;
		    texDepthDataNegZ[i][j] = 1.0;
		}
	}

	/* Render the epth cube texture using LUMINANCE */
	glGenTextures(1, &tex);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, tex);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_GENERATE_MIPMAP, GL_FALSE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S,
			GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP,
			GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP,
			GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_DEPTH_TEXTURE_MODE,
			GL_LUMINANCE);
	glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_COMPARE_MODE,
			GL_COMPARE_REF_TO_TEXTURE );
	glTexParameterf(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_COMPARE_FUNC,
			GL_LEQUAL);

	/* Set a different depth value to each face of cubemap */
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_DEPTH_COMPONENT,
		     width, height, 0, GL_DEPTH_COMPONENT,
		     GL_FLOAT, texDepthDataPosX);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_DEPTH_COMPONENT,
		     width, height, 0, GL_DEPTH_COMPONENT,
		     GL_FLOAT, texDepthDataNegX);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_DEPTH_COMPONENT,
		     width, height, 0, GL_DEPTH_COMPONENT,
		     GL_FLOAT, texDepthDataPosY);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_DEPTH_COMPONENT,
		     width, height, 0, GL_DEPTH_COMPONENT,
		     GL_FLOAT, texDepthDataNegY);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_DEPTH_COMPONENT,
		     width, height, 0, GL_DEPTH_COMPONENT,
		     GL_FLOAT, texDepthDataPosZ);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_DEPTH_COMPONENT,
		     width, height, 0, GL_DEPTH_COMPONENT,
		     GL_FLOAT, texDepthDataNegZ);
	#undef height
	#undef width
}

void
piglit_init(int argc, char **argv)
{
	/* GL version must be 3.0 */
	piglit_require_gl_version(30);
	loadTex();
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0, piglit_width, 0, piglit_height, -1, 1);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glClearColor(0.1, 0.1, 0.1, 1.0);
	shaderSetup();
}

enum piglit_result
piglit_display(void)
{
	GLint cubeShadow_loc, vertex_loc, texCoord_loc;
	GLboolean pass = GL_TRUE;
	GLfloat white[4] = {1.0, 1.0, 1.0, 1.0};
	GLfloat black[4] = {0.0, 0.0, 0.0, 1.0};
	GLint i;

	cubeShadow_loc = glGetUniformLocation(prog, "cubeShadow");
	vertex_loc = glGetAttribLocation(prog, "vertex");
	texCoord_loc = glGetAttribLocation(prog, "texCoord");

	glClear(GL_COLOR_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);

	glUniform1i(cubeShadow_loc, 0);
	glEnableVertexAttribArray(vertex_loc);
	glEnableVertexAttribArray(texCoord_loc);

	/* Apply each face of cubemap as texture to a polygon */
	for (i = 0; i < 6; ++i) {
		glVertexAttribPointer(vertex_loc, 4, GL_FLOAT, GL_FALSE,
				      stride, cube_shadow_attributes[i][0]);
		glVertexAttribPointer(texCoord_loc, 4, GL_FLOAT, GL_FALSE,
				      stride, cube_shadow_attributes[i][1]);
		glDrawArrays(GL_QUADS, 0, 4);
	}

	/* Test the pixel color of polygons against the expected output */
	/* Polygon 1 */
	pass = piglit_probe_pixel_rgb(101, 170, white);
	pass = pass && piglit_probe_pixel_rgb(105, 130, white);
	pass = pass && piglit_probe_pixel_rgb(120, 145, white);
	pass = pass && piglit_probe_pixel_rgb(145, 126, white);
	pass = pass && piglit_probe_pixel_rgb(105, 174, black);
	pass = pass && piglit_probe_pixel_rgb(130, 155, black);
	pass = pass && piglit_probe_pixel_rgb(145, 170, black);
	pass = pass && piglit_probe_pixel_rgb(149, 130, black);

	/* Polygon 2 */
	pass = pass && piglit_probe_pixel_rgb(176, 170, black);
	pass = pass && piglit_probe_pixel_rgb(180, 130, black);
	pass = pass && piglit_probe_pixel_rgb(195, 145, black);
	pass = pass && piglit_probe_pixel_rgb(220, 126, black);
	pass = pass && piglit_probe_pixel_rgb(224, 130, white);
	pass = pass && piglit_probe_pixel_rgb(205, 155, white);
	pass = pass && piglit_probe_pixel_rgb(220, 170, white);
	pass = pass && piglit_probe_pixel_rgb(180, 174, white);

	/* Polygon 3 */
	pass = pass && piglit_probe_pixel_rgb(251, 130, white);
	pass = pass && piglit_probe_pixel_rgb(255, 170, white);
	pass = pass && piglit_probe_pixel_rgb(270, 155, white);
	pass = pass && piglit_probe_pixel_rgb(290, 174, white);
	pass = pass && piglit_probe_pixel_rgb(255, 126, black);
	pass = pass && piglit_probe_pixel_rgb(280, 145, black);
	pass = pass && piglit_probe_pixel_rgb(295, 130, black);
	pass = pass && piglit_probe_pixel_rgb(299, 170, black);

	/* Polygon 4 */
	pass = pass && piglit_probe_pixel_rgb(101, 205, black);
	pass = pass && piglit_probe_pixel_rgb(105, 245, black);
	pass = pass && piglit_probe_pixel_rgb(120, 230, black);
	pass = pass && piglit_probe_pixel_rgb(145, 249, black);
	pass = pass && piglit_probe_pixel_rgb(105, 201, white);
	pass = pass && piglit_probe_pixel_rgb(130, 220, white);
	pass = pass && piglit_probe_pixel_rgb(145, 205, white);
	pass = pass && piglit_probe_pixel_rgb(149, 245, white);

	/* Polygon 5 & 6 are filled with a flat color. So probe using
	 * piglit_probe_rect_rgb
	 */
	pass = pass && piglit_probe_rect_rgba(175, 200, 50, 50, black);
	pass = pass && piglit_probe_rect_rgba(250, 200, 50, 50, white);

	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;
	piglit_present_results();
	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}
