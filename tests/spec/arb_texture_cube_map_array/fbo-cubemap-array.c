/*
 * Copyright © 2009 Intel Corporation
 * Copyright (c) 2010 VMware, Inc.
 * Copyright © 2012 Red Hat Inc.
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
 * Authors:
 *     Dave Airlie
 *
 */

/** @file fbo-cubemap-array.c
 *
 * Tests that drawing to each level of an cubemap array texture FBO and then
 * drawing views * of those individual layerfaces to the window system framebuffer succeeds.
 * based on fbo-array.c
 */

#include "piglit-util-gl.h"

#define BUF_WIDTH 32
#define BUF_HEIGHT 32

PIGLIT_GL_TEST_CONFIG_BEGIN

    config.supports_gl_compat_version = 10;

    config.window_width = 200;
    config.window_height = 100;
    config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGB;

PIGLIT_GL_TEST_CONFIG_END

#define NUM_FACES       6
#define NUM_LAYERS	3

float layer_color[NUM_LAYERS * NUM_FACES][4] = {
	{1.0, 0.0, 0.0, 0.0},
	{0.0, 1.0, 0.0, 0.0},
	{0.0, 0.0, 1.0, 0.0},
	{1.0, 0.0, 1.0, 0.0},
	{1.0, 1.0, 0.0, 0.0},
	{0.0, 1.0, 1.0, 0.0},

	{0.0, 1.0, 1.0, 0.0},
	{1.0, 0.0, 0.0, 0.0},
	{0.0, 1.0, 0.0, 0.0},
	{0.0, 0.0, 1.0, 0.0},
	{1.0, 0.0, 1.0, 0.0},
	{1.0, 1.0, 0.0, 0.0},

	{1.0, 1.0, 0.0, 0.0},
	{0.0, 1.0, 1.0, 0.0},
	{1.0, 0.0, 0.0, 0.0},
	{0.0, 1.0, 0.0, 0.0},
	{0.0, 0.0, 1.0, 0.0},
	{1.0, 0.0, 1.0, 0.0},
};

int num_layers = NUM_LAYERS * NUM_FACES;

static const char *prog = "fbo-cubemap-array";

static const char *frag_shader_cube_array_text =
   "#version 130\n"
   "#extension GL_ARB_texture_cube_map_array : enable \n"
   "uniform samplerCubeArray tex; \n"
   "void main() \n"
   "{ \n"
   "   gl_FragColor = texture(tex, gl_TexCoord[0]); \n"
   "} \n";

static GLuint program_cube_array;

static int
create_array_fbo(void)
{
	GLuint tex, fb;
	GLenum status;
	int layer;

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY_ARB, tex);
	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	/* allocate empty array texture */
	glTexImage3D(GL_TEXTURE_CUBE_MAP_ARRAY_ARB, 0, GL_RGBA,
		     BUF_WIDTH, BUF_HEIGHT, num_layers,
		     0,
		     GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	glGenFramebuffersEXT(1, &fb);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fb);

	/* draw something into each layer of the array texture */
	for (layer = 0; layer < num_layers; layer++) {
		glFramebufferTextureLayer(GL_FRAMEBUFFER_EXT,
					     GL_COLOR_ATTACHMENT0_EXT,
					     tex,
					     0,
					     layer);

		if (!piglit_check_gl_error(GL_NO_ERROR))
		        piglit_report_result(PIGLIT_FAIL);

		status = glCheckFramebufferStatusEXT (GL_FRAMEBUFFER_EXT);
		if (status != GL_FRAMEBUFFER_COMPLETE_EXT) {
			fprintf(stderr, "%s: FBO incomplete\n", prog);
			piglit_report_result(PIGLIT_SKIP);
		}

		glViewport(0, 0, BUF_WIDTH, BUF_HEIGHT);
		piglit_ortho_projection(BUF_WIDTH, BUF_HEIGHT, GL_FALSE);

		/* solid color quad */
		glColor4fv(layer_color[layer]);
		piglit_draw_rect(-2, -2, BUF_WIDTH + 2, BUF_HEIGHT + 2);
	}

	glDeleteFramebuffersEXT(1, &fb);
	return tex;
}

GLfloat a_cube_face_texcoords[6][4][4];

void setup_texcoords(void)
{
	int i, j;
	for (i = 0; i < 6; i++) {
		for (j = 0; j < 4; j++) {
			memcpy(a_cube_face_texcoords[i][j], cube_face_texcoords[i][j], 3 * sizeof(GLfloat));
		}
	}
}

/* Draw a textured quad, sampling only the given layerface of the
 * array texture.
 */
static void
draw_layer(int x, int y, int layerface)
{
	GLint face = layerface % 6;
	GLint layer = layerface / 6;
	GLint loc;

	glUseProgram(program_cube_array);
	loc = glGetUniformLocation(program_cube_array, "tex");
	glUniform1i(loc, 0); /* texture unit p */

	glViewport(0, 0, piglit_width, piglit_height);
	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);

	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, piglit_winsys_fbo);

	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY_ARB, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY_ARB, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY_ARB, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY_ARB, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY_ARB, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glBegin(GL_QUADS);

	a_cube_face_texcoords[face][0][3] = layer;
	a_cube_face_texcoords[face][1][3] = layer;
	a_cube_face_texcoords[face][2][3] = layer;
	a_cube_face_texcoords[face][3][3] = layer;

	glTexCoord4fv(a_cube_face_texcoords[face][0]);
	glVertex2f(x, y);

	glTexCoord4fv(a_cube_face_texcoords[face][1]);
	glVertex2f(x + BUF_WIDTH, y);

	glTexCoord4fv(a_cube_face_texcoords[face][2]);
	glVertex2f(x + BUF_WIDTH, y + BUF_HEIGHT);

	glTexCoord4fv(a_cube_face_texcoords[face][3]);
	glVertex2f(x, y + BUF_HEIGHT);

	glEnd();

	glUseProgram(0);
}

static GLboolean test_layer_drawing(int start_x, int start_y, float *expected)
{
	return piglit_probe_rect_rgb(start_x, start_y, BUF_WIDTH, BUF_HEIGHT,
				     expected);
}

enum piglit_result
piglit_display(void)
{
	GLboolean pass = GL_TRUE;
	int layer;
	GLuint tex;
	int y;

	glClearColor(1.0, 1.0, 1.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	tex = create_array_fbo();

	y = -1;
	for (layer = 0; layer < num_layers; layer++) {
		int x = 1 + (layer % 6) * (BUF_WIDTH + 1);
		if (layer % 6 == 0)
			y++;
		draw_layer(x, y * BUF_HEIGHT, layer);

	}

	y = -1;
	for (layer = 0; layer < num_layers; layer++) {
		int x = 1 + (layer % 6) * (BUF_WIDTH + 1);
		if (layer % 6 == 0)
			y++;
		pass &= test_layer_drawing(x, y * BUF_HEIGHT, layer_color[layer]);
	}

	glDeleteTextures(1, &tex);

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void piglit_init(int argc, char **argv)
{
	piglit_require_extension("GL_ARB_texture_cube_map_array");

	program_cube_array =
		piglit_build_simple_program(NULL, frag_shader_cube_array_text);

	setup_texcoords();
}
