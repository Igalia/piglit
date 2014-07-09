/*
 * Copyright (c) 2010 VMware, Inc.
 * Copyright © 2011 Intel Corporation
 *
 * Permission is hereby , free of charge, to any person obtaining a
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

/** @file maxlayers.c
 *
 * Test that we can make and render from an array texture with
 * GL_MAX_TEXTURE_LAYERS layers.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;

PIGLIT_GL_TEST_CONFIG_END

int height = 100, ybase = 0;

/* We'll set each 1x1 texture slice to a different color. */
static GLfloat colors[][4] = {
	{1.0, 0.0, 0.0, 0.0},
	{0.0, 1.0, 0.0, 0.0},
	{0.0, 0.0, 1.0, 0.0},
	{0.0, 0.0, 1.0, 0.0},
	{0.0, 1.0, 0.0, 0.0},
	{1.0, 1.0, 0.0, 0.0},
	{1.0, 1.0, 1.0, 0.0}
};


static const char *fs_source =
	"#extension GL_EXT_texture_array : enable \n"
	"uniform sampler2DArray tex;\n"
	"uniform int layer;\n"
	"void main() \n"
	"{ \n"
	"   gl_FragColor = texture2DArray(tex, vec3(0.0, 0.0, layer)); \n"
	"} \n";

static GLint max_layers;
static GLint layer_loc;

static void
bind_2d_array_texture(void)
{
	float *data;
	GLuint tex;
	int i;

	data = malloc(max_layers * 4 * sizeof(float));

	for (i = 0; i < max_layers; i++) {
		data[i * 4 + 0] = colors[i % ARRAY_SIZE(colors)][0];
		data[i * 4 + 1] = colors[i % ARRAY_SIZE(colors)][1];
		data[i * 4 + 2] = colors[i % ARRAY_SIZE(colors)][2];
		data[i * 4 + 3] = colors[i % ARRAY_SIZE(colors)][3];
	}

	glGenTextures(1, &tex);

	glBindTexture(GL_TEXTURE_2D_ARRAY, tex);

	glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA,
		     1, 1, max_layers, /* w, h, d */
		     0, /* border */
		     GL_RGBA, GL_FLOAT, data);

	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER,
			GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER,
			GL_NEAREST);
}

enum piglit_result
piglit_display(void)
{
	bool pass = true;
	int layer;

	glClear(GL_COLOR_BUFFER_BIT);

	piglit_ortho_projection(piglit_width, piglit_height, false);

	for (layer = 0; layer < max_layers; layer++) {
		int x = layer % piglit_width;
		int y = layer / piglit_width;

		glUniform1i(layer_loc, layer);
		piglit_draw_rect(x, y, 1, 1);
	}


	for (layer = 0; layer < max_layers; layer++) {
		int x = layer % piglit_width;
		int y = layer / piglit_width;
		float *color = colors[layer % ARRAY_SIZE(colors)];

		pass = pass &&
			piglit_probe_rect_rgba(x, y, 1, 1, color);
	}

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}


void
piglit_init(int argc, char **argv)
{
	GLuint prog;

	piglit_require_extension("GL_EXT_texture_array");

	glGetIntegerv(GL_MAX_ARRAY_TEXTURE_LAYERS, &max_layers);

	if (max_layers >= piglit_width * piglit_height)
		max_layers = piglit_width * piglit_height;

	printf("Testing %d texture layers\n", max_layers);

	/* Make shader programs */
	prog = piglit_build_simple_program(NULL, fs_source);

	glUseProgram(prog);
	layer_loc = glGetUniformLocation(prog, "layer");
	assert(layer_loc != -1);

	bind_2d_array_texture();
}
