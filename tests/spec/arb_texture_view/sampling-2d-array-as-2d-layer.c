/*
 * Copyright © 2015 Glenn Kennard
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
 * Author: Glenn Kennard <glenn.kennard@gmail.com>
 */

/**
 * \file sampling-2d-array-as-2d-layer.c
 * This tests that you can cast from a 2D Array texture to a regular 2D texture
 * with layer>0 and sample from the latter.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN
	config.supports_gl_compat_version = 30;
	config.supports_gl_es_version = 31;
	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;
	config.khr_no_error_support = PIGLIT_NO_ERRORS;

PIGLIT_GL_TEST_CONFIG_END

static const GLubyte green[] = {0, 255, 0, 255};
static const float greenf[] = {0, 1.0f, 0, 1.0f};
static const GLubyte red[] = {255, 0, 0, 255};

typedef struct Params {
	int num_layers;
	int width;
	int height;
	const char * const desc;
} Params;

/* test a few size combinations that tend to require particular alignment
   requirements by the hardware */
static const Params testparams[] = {
	{ 8, 1, 1, "1x1" },
	{ 3, 2, 1, "2x1" },
	{ 3, 8, 1, "8x1" },
	{ 1, 16, 1, "16x1" },
	{ 5, 1, 16, "1x16" },
	{ 9, 32, 32, "32x32" },
	{ 2, 64, 64, "64x64" },
	{ 4, 128, 64, "128x64" },
	{ 3, 35, 67, "35x67" }
};

static GLubyte *makesolidimage(int w, int h, const GLubyte color[4])
{
	GLubyte *p = malloc(w * h * 4 * sizeof(GLubyte));
	size_t n;
	assert(p);
	for (n = 0; n < w * h; n++) {
		p[n*4 + 0] = color[0];
		p[n*4 + 1] = color[1];
		p[n*4 + 2] = color[2];
		p[n*4 + 3] = color[3];
	}
	return p;
}

static bool
test_single_layer(const Params* p, int layer)
{
	int l;
	GLuint tex_src, tex_view;
	GLboolean pass;
	GLubyte *image;

	assert(layer < p->num_layers);

	glGenTextures(1, &tex_src);
	glBindTexture(GL_TEXTURE_2D_ARRAY, tex_src);

	glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_RGBA8, p->width, p->height, p->num_layers);

	/* load each array layer with red */
	image = makesolidimage(p->width, p->height, red);
	for (l = 0; l < p->num_layers; l++) {
		glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, l,
				p->width, p->height, 1, GL_RGBA, GL_UNSIGNED_BYTE, image);
	}

	/* make layer to check red, but green for pixel at (0,0) which should be the only one sampled */
	memcpy(image, green, sizeof(green));

	glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, layer,
			p->width, p->height, 1, GL_RGBA, GL_UNSIGNED_BYTE, image);

	free(image);

	glGenTextures(1, &tex_view);
	/* checked layer is supposed to be green */
	glTextureView(tex_view, GL_TEXTURE_2D, tex_src, GL_RGBA8,
		      0, 1, layer, 1);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glBindTexture(GL_TEXTURE_2D, tex_view);

	/* draw it! */
	piglit_draw_rect(-1, -1, 2, 2);

	pass = piglit_probe_rect_rgba(0, 0, piglit_width, piglit_height, greenf);
	if (!pass) {
		printf("layer %d failed\n", layer);
	}

	glDeleteTextures(1, &tex_view);
	glDeleteTextures(1, &tex_src);

	return pass;
}

enum piglit_result
piglit_display(void)
{
	GLboolean pass = GL_TRUE;
	size_t n;
	int layer;

	glViewport(0, 0, piglit_width, piglit_height);
	glClearColor(0.0, 0.0, 1.0, 1.0);

	for (n = 0; n < ARRAY_SIZE(testparams); n++) {
		GLboolean subtest_pass = GL_TRUE;
		for (layer = 0; layer < testparams[n].num_layers; layer++) {
			glClear(GL_COLOR_BUFFER_BIT);

			subtest_pass &= test_single_layer(&testparams[n], layer);
		}
		piglit_report_subtest_result(subtest_pass ? PIGLIT_PASS : PIGLIT_FAIL,
			"%s", testparams[n].desc);
		pass &= subtest_pass;
	}

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

#ifdef PIGLIT_USE_OPENGL
#define GLSL_VERSION "130"
#else
#define GLSL_VERSION "310 es"
#endif

static const char *vs =
	"#version " GLSL_VERSION "\n"
	"in vec4 piglit_vertex;\n"
	"void main() { \n"
	"	gl_Position = piglit_vertex;\n"
	"}\n";

static const char *fs =
	"#version " GLSL_VERSION "\n"
	"#ifdef GL_ES\n"
	"precision highp float;\n"
	"precision highp sampler2D;\n"
	"#endif\n"
	"uniform sampler2D tex;\n"
	"out vec4 color;\n"
	"void main() { \n"
	"	ivec2 size = textureSize(tex, 0);\n"
	/* texel in (0,0) should be the only green texel in the entire texture */
	"	vec2 offset = vec2(0.5/float(size.x), 0.5/float(size.y));\n"
	"	color = vec4(texture(tex, offset).xyz, 1.0);\n"
	"}\n";

void
piglit_init(int argc, char **argv)
{
	int tex_loc_view, prog_view;

#ifdef PIGLIT_USE_OPENGL
	piglit_require_extension("GL_ARB_texture_view");
	piglit_require_extension("GL_ARB_texture_storage");
#else
	piglit_require_extension("GL_OES_texture_view");
#endif

	/* setup shaders and program object for texture rendering */
	prog_view = piglit_build_simple_program(vs, fs);
	tex_loc_view = glGetUniformLocation(prog_view, "tex");

	glUseProgram(prog_view);
	glDeleteProgram(prog_view);
	glUniform1i(tex_loc_view, 0);
}
