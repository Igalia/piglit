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

#include "piglit-util-gl-common.h"

PIGLIT_GL_TEST_MAIN(
    400 /*window_width*/,
    300 /*window_height*/,
    GLUT_RGB | GLUT_DOUBLE)

static GLuint tex;
static GLint prog;
static GLint fs;
static GLint vs;

/* These texture coordinates should have 1 or -1 in the major axis
 * ('r' coordinate) selecting the face, a nearly-1-or-negative-1 value
 * in the other two coordinates (s,t) and a reference value ('q' coordinate)
 * used for shadow comparisons
 */
static GLfloat cube_shadow_texcoords[6][4][4] = {
	{ /* GL_TEXTURE_CUBE_MAP_POSITIVE_X */
		{1.0,  0.99,  0.99, -0.50},
		{1.0,  0.99, -0.99,  0.00},
		{1.0, -0.99, -0.99,  0.50},
		{1.0, -0.99,  0.99,  0.00},
	},
	{ /* GL_TEXTURE_CUBE_MAP_NEGATIVE_X */
		{-1.0,  0.99, -0.99,  0.90},
		{-1.0,  0.99,  0.99,  0.20},
		{-1.0, -0.99,  0.99, -0.50},
		{-1.0, -0.99, -0.99,  0.20},
	},
	{ /* GL_TEXTURE_CUBE_MAP_POSITIVE_Y */
		{-0.99, 1.0, -0.99,  0.35},
		{ 0.99, 1.0, -0.99,  1.20},
		{ 0.99, 1.0,  0.99,  0.35},
		{-0.99, 1.0,  0.99, -0.50},
	},
	{ /* GL_TEXTURE_CUBE_MAP_NEGATIVE_Y */
		{-0.99, -1.0,  0.99,  0.50},
		{-0.99, -1.0, -0.99, -0.50},
		{ 0.99, -1.0, -0.99,  0.50},
		{ 0.99, -1.0,  0.99,  1.50},
	},
	{ /* GL_TEXTURE_CUBE_MAP_POSITIVE_Z */
		{-0.99,  0.99, 1.0, 0.85},
		{-0.99, -0.99, 1.0, 0.85},
		{ 0.99, -0.99, 1.0, 0.85},
		{ 0.99,  0.99, 1.0, 0.85},
	},
	{ /* GL_TEXTURE_CUBE_MAP_NEGATIVE_Z */
		{ 0.99,  0.99, -1.0, 0.90},
		{-0.99,  0.99, -1.0, 0.90},
		{-0.99, -0.99, -1.0, 0.90},
		{ 0.99, -0.99, -1.0, 0.90},
	},
};

static const char *vertShaderText =
	"#version 130\n"
	"void main()\n"
	"{\n"
	"	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;\n"
	"	gl_TexCoord[0] = gl_MultiTexCoord0;\n"
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
	GLint loc1;
	GLboolean pass = GL_TRUE;
	GLfloat white[4] = {1.0, 1.0, 1.0, 1.0};
	GLfloat black[4] = {0.0, 0.0, 0.0, 1.0};

	loc1 = glGetUniformLocation(prog, "cubeShadow");
	glClear(GL_COLOR_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);

	glUniform1i(loc1, 0);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	/* Apply each face of cubemap as texture to a polygon */
	/* Polygon 1 */
	glTexCoordPointer(4, GL_FLOAT, 0, cube_shadow_texcoords[0]);
	piglit_draw_rect(100, 125, 50, 50);
	/* Polygon 2 */
	glTexCoordPointer(4, GL_FLOAT, 0, cube_shadow_texcoords[1]);
	piglit_draw_rect(175, 125, 50, 50);
	/* Polygon 3 */
	glTexCoordPointer(4, GL_FLOAT, 0, cube_shadow_texcoords[2]);
	piglit_draw_rect(250, 125, 50, 50);
	/* Polygon 4 */
	glTexCoordPointer(4, GL_FLOAT, 0, cube_shadow_texcoords[3]);
	piglit_draw_rect(100, 200, 50, 50);
	/* Polygon 5 */
	glTexCoordPointer(4, GL_FLOAT, 0, cube_shadow_texcoords[4]);
	piglit_draw_rect(175, 200, 50, 50);
	/* Polygon 6 */
	glTexCoordPointer(4, GL_FLOAT, 0, cube_shadow_texcoords[5]);
	piglit_draw_rect(250, 200, 50, 50);

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
	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
	piglit_present_results();
	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}
