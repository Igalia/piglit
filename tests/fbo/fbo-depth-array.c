/*
 * Copyright © 2009 Intel Corporation
 * Copyright (c) 2010 VMware, Inc.
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
 *     Christoph Bumiller
 *
 */

/** @file fbo-depth-array.c
 *
 * Tests that drawing to each level of a depth-only array texture FBO and then
 * drawing views of those individual depths to the window system framebuffer
 * succeeds.
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

#define NUM_LAYERS	6

float layer_depth[NUM_LAYERS][4] = {
	{0.1, 0.1, 0.1, 0.0},
	{0.2, 0.2, 0.2, 0.0},
	{0.4, 0.4, 0.4, 0.0},
	{0.6, 0.6, 0.6, 0.0},
	{0.8, 0.8, 0.8, 0.0},
	{1.0, 1.0, 1.0, 0.0},
};

static int num_layers = NUM_LAYERS;

static const char *prog = "fbo-depth-array";
/* debug aid */
static void
check_error(int line)
{
   GLenum err = glGetError();
   if (err) {
      printf("%s: GL error 0x%x at line %d\n", prog, err, line);
   }
}

static const char *frag_shader_depth_output_text =
   "void main() \n"
   "{ \n"
   "   gl_FragDepth = gl_Color.r;\n"
   "} \n";

static GLuint frag_shader_depth_output;
static GLuint program_depth_output;

static const char *frag_shader_2d_array_text =
   "#extension GL_EXT_texture_array : enable \n"
   "uniform sampler2DArray tex; \n"
   "void main() \n"
   "{ \n"
   "   gl_FragColor = texture2DArray(tex, gl_TexCoord[0].xyz).rrrr; \n"
   "} \n";

static GLuint frag_shader_2d_array;
static GLuint program_2d_array;


static int
create_array_fbo(void)
{
	GLuint tex, fb;
	GLenum status;
	int layer;

	glUseProgram(program_depth_output);

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D_ARRAY_EXT, tex);
	assert(glGetError() == 0);

	/* allocate empty array texture */
	glTexImage3D(GL_TEXTURE_2D_ARRAY_EXT, 0, GL_DEPTH_COMPONENT24,
		     BUF_WIDTH, BUF_HEIGHT, num_layers,
		     0,
		     GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL);
	assert(glGetError() == 0);

	glGenFramebuffersEXT(1, &fb);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fb);

	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	/* draw something into each layer of the array texture */
	for (layer = 0; layer < NUM_LAYERS; layer++) {
		glFramebufferTextureLayer(GL_FRAMEBUFFER_EXT,
					  GL_DEPTH_ATTACHMENT,
					  tex,
					  0,
					  layer);

		assert(glGetError() == 0);

		status = glCheckFramebufferStatusEXT (GL_FRAMEBUFFER_EXT);
		if (status != GL_FRAMEBUFFER_COMPLETE_EXT) {
			fprintf(stderr, "FBO incomplete\n");
			goto done;
		}

		glViewport(0, 0, BUF_WIDTH, BUF_HEIGHT);
		piglit_ortho_projection(BUF_WIDTH, BUF_HEIGHT, GL_FALSE);

		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_ALWAYS);

		/* solid color quad */
		glColor4fv(layer_depth[layer]);
		piglit_draw_rect(-2, -2, BUF_WIDTH + 2, BUF_HEIGHT + 2);

		glDisable(GL_DEPTH_TEST);
	}

	glUseProgram(0);

done:
	glDeleteFramebuffersEXT(1, &fb);
	return tex;
}

/* Draw a textured quad, sampling only the given layer of the
 * array texture.
 */
static void
draw_layer(int x, int y, int depth)
{
	GLfloat depth_coord = (GLfloat)depth;
	GLint loc;

	glUseProgram(program_2d_array);
	loc = glGetUniformLocation(program_2d_array, "tex");
	glUniform1i(loc, 0); /* texture unit p */

	glViewport(0, 0, piglit_width, piglit_height);
	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);

	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, piglit_winsys_fbo);

	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glTexParameteri(GL_TEXTURE_2D_ARRAY_EXT, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D_ARRAY_EXT, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D_ARRAY_EXT, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D_ARRAY_EXT, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D_ARRAY_EXT, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glBegin(GL_QUADS);

	glTexCoord3f(0, 0, depth_coord);
	glVertex2f(x, y);

	glTexCoord3f(1, 0, depth_coord);
	glVertex2f(x + BUF_WIDTH, y);

	glTexCoord3f(1, 1, depth_coord);
	glVertex2f(x + BUF_WIDTH, y + BUF_HEIGHT);

	glTexCoord3f(0, 1, depth_coord);
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

	glClearColor(1.0, 1.0, 1.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	tex = create_array_fbo();

	for (layer = 0; layer < NUM_LAYERS; layer++) {
		int x = 1 + layer * (BUF_WIDTH + 1);
		int y = 1;
		draw_layer(x, y, layer);
	}

	for (layer = 0; layer < NUM_LAYERS; layer++) {
		int x = 1 + layer * (BUF_WIDTH + 1);
		int y = 1;
		pass &= test_layer_drawing(x, y, layer_depth[layer]);
	}

	glDeleteTextures(1, &tex);

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

	frag_shader_depth_output =
		piglit_compile_shader_text(GL_FRAGMENT_SHADER,
					   frag_shader_depth_output_text);
	check_error(__LINE__);

	program_2d_array = piglit_link_simple_program(0, frag_shader_2d_array);
	check_error(__LINE__);

	program_depth_output = piglit_link_simple_program(0,
						  frag_shader_depth_output);
	check_error(__LINE__);
}
