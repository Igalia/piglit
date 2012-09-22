/*
 * Copyright © 2009 Intel Corporation
 * Copyright © 2011 Red Hat Inc.
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

/** @file fbo-generatemipmap-array.c
 *
 * Tests that glGenerateMipmapEXT works correctly on level of a 2D array texture.
 */

#include "piglit-util-gl-common.h"

#define TEX_WIDTH 128
#define TEX_HEIGHT 128

PIGLIT_GL_TEST_MAIN(
    600 /*window_width*/,
    560 /*window_height*/,
    PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGB)

static const float red[] =   {1, 0, 0, 0};
static const float green[] = {0, 1, 0, 0};
static const float blue[] =  {0, 0, 1, 0};
static const float white[] = {1, 1, 1, 1};

static const char *prog = "fbo-array";
/* debug aid */
static void
check_error(int line)
{
   GLenum err = glGetError();
   if (err) {
      printf("%s: GL error 0x%x at line %d\n", prog, err, line);
   }
}

static const char *frag_shader_2d_array_text =
   "#extension GL_EXT_texture_array : enable \n"
   "uniform sampler2DArray tex; \n"
   "void main() \n"
   "{ \n"
   "   gl_FragColor = texture2DArray(tex, gl_TexCoord[0].xyz); \n"
   "} \n";

static GLuint frag_shader_2d_array;
static GLuint program_2d_array;

static const char *frag_shader_1d_array_text =
   "#extension GL_EXT_texture_array : enable \n"
   "uniform sampler1DArray tex; \n"
   "void main() \n"
   "{ \n"
   "   gl_FragColor = texture1DArray(tex, gl_TexCoord[0].xy); \n"
   "} \n";

static GLuint frag_shader_1d_array;
static GLuint program_1d_array;

#define NUM_LAYERS	4

float layer_color[NUM_LAYERS][4] = {
	{1.0, 0.0, 0.0, 0.0},
	{0.0, 1.0, 0.0, 0.0},
	{0.0, 0.0, 1.0, 0.0},
	{1.0, 0.0, 1.0, 0.0},
};

int num_layers = NUM_LAYERS;

static int
create_array_fbo_1d(void)
{
	GLuint tex, fb;
	GLenum status;
	int i, dim;
	int layer;

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_1D_ARRAY_EXT, tex);
	assert(glGetError() == 0);

	for (i = 0, dim = TEX_WIDTH; dim >0; i++, dim /= 2) {
		glTexImage2D(GL_TEXTURE_1D_ARRAY_EXT, i, GL_RGBA,
			     dim, num_layers, 0,
			     GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	}
	assert(glGetError() == 0);

	glGenFramebuffersEXT(1, &fb);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fb);

	for (layer = 0; layer < num_layers; layer++) {
		glFramebufferTextureLayer(GL_FRAMEBUFFER_EXT,
					  GL_COLOR_ATTACHMENT0_EXT,
					  tex,
					  0,
					  layer);
		assert(glGetError() == 0);

		status = glCheckFramebufferStatusEXT (GL_FRAMEBUFFER_EXT);
		if (status != GL_FRAMEBUFFER_COMPLETE_EXT) {
			fprintf(stderr, "FBO incomplete\n");
			goto done;
		}

		glViewport(0, 0, TEX_WIDTH, 1);
		piglit_ortho_projection(TEX_WIDTH, 1, GL_FALSE);

		glColor4fv(layer_color[layer]);
		piglit_draw_rect(0, 0, TEX_WIDTH / 2, 1);
		glColor4fv(layer_color[(layer + 1) % 4]);
		piglit_draw_rect(TEX_WIDTH / 2, 0, TEX_WIDTH, 1);
	}

	glGenerateMipmapEXT(GL_TEXTURE_1D_ARRAY_EXT);
done:
	glDeleteFramebuffersEXT(1, &fb);

	return tex;
}

static int
create_array_fbo_2d(void)
{
	GLuint tex, fb;
	GLenum status;
	int i, dim;
	int layer;

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D_ARRAY_EXT, tex);
	assert(glGetError() == 0);

	for (i = 0, dim = TEX_WIDTH; dim >0; i++, dim /= 2) {
		glTexImage3D(GL_TEXTURE_2D_ARRAY_EXT, i, GL_RGBA,
			     dim, dim, num_layers, 0,
			     GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	}
	assert(glGetError() == 0);

	glGenFramebuffersEXT(1, &fb);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fb);

	for (layer = 0; layer < num_layers; layer++) {
		glFramebufferTextureLayer(GL_FRAMEBUFFER_EXT,
					  GL_COLOR_ATTACHMENT0_EXT,
					  tex,
					  0,
					  layer);
		assert(glGetError() == 0);

		status = glCheckFramebufferStatusEXT (GL_FRAMEBUFFER_EXT);
		if (status != GL_FRAMEBUFFER_COMPLETE_EXT) {
			fprintf(stderr, "FBO incomplete\n");
			goto done;
		}

		glViewport(0, 0, TEX_WIDTH, TEX_HEIGHT);
		piglit_ortho_projection(TEX_WIDTH, TEX_HEIGHT, GL_FALSE);

		glColor4fv(layer_color[layer]);
		piglit_draw_rect(0, 0, TEX_WIDTH / 2, TEX_HEIGHT / 2);
		glColor4fv(green);
		piglit_draw_rect(TEX_WIDTH / 2, 0, TEX_WIDTH, TEX_HEIGHT / 2);
		glColor4fv(blue);
		piglit_draw_rect(0, TEX_HEIGHT / 2, TEX_WIDTH/2, TEX_HEIGHT);
		glColor4fv(white);
		piglit_draw_rect(TEX_WIDTH / 2, TEX_HEIGHT / 2, TEX_WIDTH, TEX_HEIGHT);
	}

	glGenerateMipmapEXT(GL_TEXTURE_2D_ARRAY_EXT);
done:
	glDeleteFramebuffersEXT(1, &fb);

	return tex;
}

GLvoid
piglit_draw_rect_tex3(float x, float y, float w, float h,
		      float tx, float ty, float tw, float th, float td)
{
	float verts[4][4];
	float tex[4][3];

	verts[0][0] = x;
	verts[0][1] = y;
	verts[0][2] = 0.0;
	verts[0][3] = 1.0;
	tex[0][0] = tx;
	tex[0][1] = ty;
	tex[0][2] = td;
	verts[1][0] = x + w;
	verts[1][1] = y;
	verts[1][2] = 0.0;
	verts[1][3] = 1.0;
	tex[1][0] = tx + tw;
	tex[1][1] = ty;
	tex[1][2] = td;
	verts[2][0] = x + w;
	verts[2][1] = y + h;
	verts[2][2] = 0.0;
	verts[2][3] = 1.0;
	tex[2][0] = tx + tw;
	tex[2][1] = ty + th;
	tex[2][2] = td;
	verts[3][0] = x;
	verts[3][1] = y + h;
	verts[3][2] = 0.0;
	verts[3][3] = 1.0;
	tex[3][0] = tx;
	tex[3][1] = ty + th;
	tex[3][2] = td;

	glVertexPointer(4, GL_FLOAT, 0, verts);
	glTexCoordPointer(3, GL_FLOAT, 0, tex);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	glDrawArrays(GL_QUADS, 0, 4);

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
}

static void
draw_mipmap_2d(int x, int y, int dim, int layer)
{
	int loc;

	glUseProgram(program_2d_array);
	loc = glGetUniformLocation(program_2d_array, "tex");
	glUniform1i(loc, 0); /* texture unit p */

	glViewport(0, 0, piglit_width, piglit_height);
	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);

	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

	glTexParameteri(GL_TEXTURE_2D_ARRAY_EXT, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D_ARRAY_EXT, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	piglit_draw_rect_tex3(x, y, dim, dim,
			      0, 0, 1, 1, layer);
	glUseProgram(0);
}

static void
draw_mipmap_1d(int x, int y, int dim, int layer)
{
	int loc;

	glUseProgram(program_1d_array);
	loc = glGetUniformLocation(program_1d_array, "tex");
	glUniform1i(loc, 0); /* texture unit p */

	glViewport(0, 0, piglit_width, piglit_height);
	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);

	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

	glTexParameteri(GL_TEXTURE_1D_ARRAY_EXT, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_1D_ARRAY_EXT, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	piglit_draw_rect_tex3(x, y, dim, dim,
			      0, layer, 1, 0, 0);
	glUseProgram(0);
}

static GLboolean
test_mipmap_drawing_2d(int start_x, int start_y, int dim, int layer)
{
	GLboolean pass = GL_TRUE;
	pass = pass && piglit_probe_rect_rgb(
			start_x, start_y, dim/2, dim/2, layer_color[layer]);
	pass = pass && piglit_probe_rect_rgb(
			start_x + dim/2, start_y, dim/2, dim/2, green);
	pass = pass && piglit_probe_rect_rgb(
			start_x, start_y + dim/2, dim/2, dim/2, blue);
	pass = pass && piglit_probe_rect_rgb(
			start_x + dim/2, start_y + dim/2, dim/2, dim/2, white);

	return pass;
}

static GLboolean
test_mipmap_drawing_1d(int start_x, int start_y, int dim, int layer)
{
	GLboolean pass = GL_TRUE;
	pass = pass && piglit_probe_rect_rgb(
			start_x, start_y, dim/2, dim/2, layer_color[layer]);
	pass = pass && piglit_probe_rect_rgb(
					     start_x + dim/2, start_y, dim/2, dim/2, layer_color[(layer + 1) % 4]);

	return pass;
}


enum piglit_result
piglit_display(void)
{
	GLboolean pass = GL_TRUE;
	int dim;
	GLuint tex1d, tex2d;
	int x, y;
	int layer;

	glClearColor(0.5, 0.5, 0.5, 0.5);
	glClear(GL_COLOR_BUFFER_BIT);

	tex1d = create_array_fbo_1d();
	tex2d = create_array_fbo_2d();

	x = 1;
	y = 1;
	for (layer = 0; layer < num_layers; layer++) {
		for (dim = TEX_WIDTH; dim > 1; dim /= 2) {
			draw_mipmap_2d(x, y, dim, layer);
			x += dim + 1;
		}
		y += TEX_HEIGHT + 5;
		x = 1;
	}

	x = 270;
	y = 1;
	for (layer = 0; layer < num_layers; layer++) {
		for (dim = TEX_WIDTH; dim > 1; dim /= 2) {
			draw_mipmap_1d(x, y, dim, layer);
			x += dim + 1;
		}
		y += TEX_HEIGHT + 5;
		x = 270;
	}


	x = 1;
	y = 1;
	for (layer = 0; layer < num_layers; layer++) {
		for (dim = TEX_WIDTH; dim > 1; dim /= 2) {
			pass &= test_mipmap_drawing_2d(x, y, dim, layer);
			x += dim + 1;
		}
		y += TEX_HEIGHT + 5;
		x = 1;
	}

	x = 270;
	y = 1;
	for (layer = 0; layer < num_layers; layer++) {
		for (dim = TEX_WIDTH; dim > 1; dim /= 2) {
			pass &= test_mipmap_drawing_1d(x, y, dim, layer);
			x += dim + 1;
		}
		y += TEX_HEIGHT + 5;
		x = 270;
	}


	glDeleteTextures(1, &tex1d);
	glDeleteTextures(1, &tex2d);

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void piglit_init(int argc, char **argv)
{
	piglit_require_extension("GL_EXT_framebuffer_object");
	piglit_require_extension("GL_EXT_texture_array");

	/* Make shader programs */
	frag_shader_2d_array =
		piglit_compile_shader_text(GL_FRAGMENT_SHADER,
					   frag_shader_2d_array_text);
	check_error(__LINE__);

	program_2d_array = piglit_link_simple_program(0, frag_shader_2d_array);
	check_error(__LINE__);

	frag_shader_1d_array =
		piglit_compile_shader_text(GL_FRAGMENT_SHADER,
					   frag_shader_1d_array_text);
	check_error(__LINE__);

	program_1d_array = piglit_link_simple_program(0, frag_shader_1d_array);
	check_error(__LINE__);

}
