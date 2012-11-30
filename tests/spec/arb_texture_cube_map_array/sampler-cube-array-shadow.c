/*
 * Copyright © 2011 Intel Corporation
 * Copyright © 2012 Red Hat
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

/* Author: Dave Airlie */
/*
 * Test to verify samplerCubeArrayShadow support
 *
 * samplerCubeArrayShadow takes the compare value in an extra vertex attrib.
 * This test works like sampler-cube-shadow except it uses the cube
 * map array interfaces.
 */

#include "piglit-util-gl-common.h"

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

/* grab the coordinates from the main definition, and grab the
   compvals from here */
static GLfloat cube_shadow_texcoords[6][4][5];

static GLfloat compvals[6][4] = { { -0.50,  0.00,  0.50,  0.00 },
				  {  0.90,  0.20, -0.50,  0.20 },
				  {  0.35,  1.20,  0.35, -0.50 },
				  {  0.50, -0.50,  0.50,  1.50 },
				  {  0.85,  0.85,  0.85,  0.85 },
				  {  0.90,  0.90,  0.90,  0.90 } };

#define STRIDE (5 * sizeof(GLfloat))
void setup_texcoords(float layer_sample)
{
	int i, j;
	for (i = 0; i < 6; i++) {
		for (j = 0; j < 4; j++) {
			memcpy(cube_shadow_texcoords[i][j], cube_face_texcoords[i][j], 3 * sizeof(GLfloat));
			cube_shadow_texcoords[i][j][3] = layer_sample;
			cube_shadow_texcoords[i][j][4] = compvals[i][j];
		}
	}
}

static const char *vertShaderText =
	"#version 130\n"
	"in float compf;\n"
	"out float compval;\n"
	"void main()\n"
	"{\n"
	"	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;\n"
	"	gl_TexCoord[0] = gl_MultiTexCoord0;\n"
	"       compval = compf;\n"
	"}\n";

static const char *fragShaderText =
	"#version 130\n"
	"#extension GL_ARB_texture_cube_map_array : enable\n"
	"uniform samplerCubeArrayShadow cubeArrayShadow;\n"
	"in float compval;\n"
	"void main()\n"
	"{\n"
	"	float shadow  = texture(cubeArrayShadow, gl_TexCoord[0], compval);\n"
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
	GLfloat tex_vals[12][width][height];

	/* Set the cubemap depth values for each face */
	for (i=0; i < height; ++i) {
		for (j=0; j < width; ++j) {
			/* fill base layer with wrong stuff - checks
			   we don't accidentally sample layer 0 */
			tex_vals[0][i][j] = 1.0;
			tex_vals[1][i][j] = 0.75;
			tex_vals[2][i][j] = 0.50;
			tex_vals[3][i][j] = 0.35;
			tex_vals[4][i][j] = 0.2;
			tex_vals[5][i][j] = 0.0;

			tex_vals[6][i][j] = 0.0;
			tex_vals[7][i][j] = 0.2;
			tex_vals[8][i][j] = 0.35;
			tex_vals[9][i][j] = 0.50;
			tex_vals[10][i][j] = 0.75;
			tex_vals[11][i][j] = 1.0;
		}
	}

	/* Render the epth cube texture using LUMINANCE */
	glGenTextures(1, &tex);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, tex);
	glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_GENERATE_MIPMAP, GL_FALSE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_WRAP_S,
			GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY,
			GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY,
			GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_DEPTH_TEXTURE_MODE,
			GL_LUMINANCE);
	glTexParameterf(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_COMPARE_MODE,
			GL_COMPARE_REF_TO_TEXTURE );
	glTexParameterf(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_COMPARE_FUNC,
			GL_LEQUAL);

	glTexImage3D(GL_TEXTURE_CUBE_MAP_ARRAY, 0, GL_DEPTH_COMPONENT,
		     width, height, 12, 0, GL_DEPTH_COMPONENT, GL_FLOAT, (void*)tex_vals);
#undef height
#undef width
}

void
piglit_init(int argc, char **argv)
{
	piglit_require_extension("GL_ARB_texture_cube_map_array");
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
	setup_texcoords(1.0);
}

enum piglit_result
piglit_display(void)
{
	GLint loc1, loc2;
	GLboolean pass = GL_TRUE;
	GLfloat white[4] = {1.0, 1.0, 1.0, 1.0};
	GLfloat black[4] = {0.0, 0.0, 0.0, 1.0};

	loc1 = glGetUniformLocation(prog, "cubeArrayShadow");
	loc2 = glGetAttribLocation(prog, "compf");
	
	glClear(GL_COLOR_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);

	glUniform1i(loc1, 0);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glEnableVertexAttribArray(loc2);

	/* Apply each face of cubemap as texture to a polygon */
	/* Polygon 1 */
	glTexCoordPointer(4, GL_FLOAT, STRIDE, cube_shadow_texcoords[0]);
	glVertexAttribPointer(loc2, 1, GL_FLOAT, GL_FALSE, STRIDE,
			      &cube_shadow_texcoords[0][0][4]);
	piglit_draw_rect(100, 125, 50, 50);
	/* Polygon 2 */
	glTexCoordPointer(4, GL_FLOAT, STRIDE, cube_shadow_texcoords[1]);
	glVertexAttribPointer(loc2, 1, GL_FLOAT, GL_FALSE, STRIDE,
			      &cube_shadow_texcoords[1][0][4]);
	piglit_draw_rect(175, 125, 50, 50);
	/* Polygon 3 */
	glTexCoordPointer(4, GL_FLOAT, STRIDE, cube_shadow_texcoords[2]);
	glVertexAttribPointer(loc2, 1, GL_FLOAT, GL_FALSE, STRIDE,
			      &cube_shadow_texcoords[2][0][4]);
	piglit_draw_rect(250, 125, 50, 50);
	/* Polygon 4 */
	glTexCoordPointer(4, GL_FLOAT, STRIDE, cube_shadow_texcoords[3]);

	glVertexAttribPointer(loc2, 1, GL_FLOAT, GL_FALSE, STRIDE,
			      &cube_shadow_texcoords[3][0][4]);
	piglit_draw_rect(100, 200, 50, 50);
	/* Polygon 5 */
	glTexCoordPointer(4, GL_FLOAT, STRIDE, cube_shadow_texcoords[4]);
	glVertexAttribPointer(loc2, 1, GL_FLOAT, GL_FALSE, STRIDE,
			      &cube_shadow_texcoords[4][0][4]);
	piglit_draw_rect(175, 200, 50, 50);
	/* Polygon 6 */
	glTexCoordPointer(4, GL_FLOAT, STRIDE, cube_shadow_texcoords[5]);
	glVertexAttribPointer(loc2, 1, GL_FLOAT, GL_FALSE, STRIDE,
			      &cube_shadow_texcoords[5][0][4]);
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
	if (piglit_automatic)
		piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
	piglit_present_results();
	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}
