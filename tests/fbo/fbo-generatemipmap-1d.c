/*
 * Copyright © 2009 Intel Corporation
 * Copyright © 2011 Red Hat Inc.
 * Copyright © 2014 Advanced Micro Devices, Inc.
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
 *     Marek Olšák <maraeo@gmail.com>
 *
 */

#include "piglit-util-gl.h"

#define TEX_SIZE 128
#define TEX_LEVELS 8

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 20;

	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGB;

PIGLIT_GL_TEST_CONFIG_END

static const char *fs_1d =
   "uniform sampler1D tex; \n"
   "void main() \n"
   "{ \n"
   "   gl_FragColor = texture1D(tex, gl_TexCoord[0].x); \n"
   "} \n";

static GLuint prog;

static float colors[4][4] = {
	{1.0, 0.0, 0.0, 1.0},
	{0.0, 1.0, 0.0, 1.0},
	{0.0, 0.0, 1.0, 1.0},
	{1.0, 0.0, 1.0, 1.0},
};

static GLenum format;

static void
load_texture_1d(void)
{
	float p[TEX_SIZE * 4];
	int x;

	for (x = 0; x < TEX_SIZE; x++) {
		int c = x / (TEX_SIZE/4);
		memcpy(&p[x*4], colors[c], sizeof(float) * 4);
	}

	glTexSubImage1D(GL_TEXTURE_1D, 0, 0, TEX_SIZE, GL_RGBA, GL_FLOAT, p);
}

static GLuint
create_texture_1d(void)
{
	GLuint tex, fb;
	GLenum status;
	int i, dim;

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_1D, tex);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	for (i = 0, dim = TEX_SIZE; dim >0; i++, dim /= 2) {
		glTexImage1D(GL_TEXTURE_1D, i, format, dim, 0,
			     GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	}

	glGenFramebuffers(1, &fb);
	glBindFramebuffer(GL_FRAMEBUFFER, fb);

	glFramebufferTexture1D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
			       GL_TEXTURE_1D, tex, 0);

	status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE) {
		load_texture_1d();
		goto done;
	}

	glViewport(0, 0, TEX_SIZE, 1);
	piglit_ortho_projection(TEX_SIZE, 1, GL_FALSE);

	glColor4fv(colors[0]);
	piglit_draw_rect(0, 0, TEX_SIZE / 4, 1);
	glColor4fv(colors[1]);
	piglit_draw_rect(TEX_SIZE / 4, 0, TEX_SIZE / 4, 1);
	glColor4fv(colors[2]);
	piglit_draw_rect(TEX_SIZE / 2, 0, TEX_SIZE / 4, 1);
	glColor4fv(colors[3]);
	piglit_draw_rect(TEX_SIZE/4 * 3, 0, TEX_SIZE / 4, 1);

done:
	glDeleteFramebuffers(1, &fb);
	glGenerateMipmap(GL_TEXTURE_1D);
	return tex;
}

static void
draw_level(int x, int y, int level)
{
	int loc;

	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_LOD, level);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAX_LOD, level);

	glUseProgram(prog);
	loc = glGetUniformLocation(prog, "tex");
	glUniform1i(loc, 0); /* texture unit p */

	glViewport(0, 0, piglit_width, piglit_height);
	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);

	glBindFramebuffer(GL_FRAMEBUFFER, piglit_winsys_fbo);

	piglit_draw_rect_tex(x, y, TEX_SIZE, 5, 0, 0, 1, 1);
	glUseProgram(0);
}

static bool
test_level(int level)
{
	int size = TEX_SIZE >> level;
	int x,i,c;
	float e[TEX_SIZE * 4], observed[TEX_SIZE * 4];

	glGetTexImage(GL_TEXTURE_1D, level, GL_RGBA, GL_FLOAT, observed);

	if (size == 1) {
		memset(e, 0, 4 * sizeof(float));

		/* Average the colors for the last level. */
		for (i = 0; i < 4; i++)
			for (c = 0; c < 4; c++)
				e[c] += colors[i][c] * 0.25;
	}
	else if (size == 2) {
		memset(e, 0, 8 * sizeof(float));

		for (i = 0; i < 2; i++)
			for (c = 0; c < 4; c++)
				e[c] += colors[i][c] * 0.5;
		for (i = 2; i < 4; i++)
			for (c = 0; c < 4; c++)
				e[4+c] += colors[i][c] * 0.5;
	}
	else {
		for (x = 0; x < size; x++) {
			int col = x / (size/4);
			memcpy(&e[x*4], colors[col], sizeof(float) * 4);
		}
	}

	for (x = 0; x < size; x++) {
		float *probe = &observed[x*4];
		float *expected = &e[x*4];

		for (i = 0; i < 4; ++i) {
			if (fabs(probe[i] - expected[i]) >= piglit_tolerance[i]) {
				printf("Probe color at (%i)\n", x);
				printf("  Expected: %f %f %f %f\n",
				       expected[0], expected[1], expected[2], expected[3]);
				printf("  Observed: %f %f %f %f\n",
				       probe[0], probe[1], probe[2], probe[3]);
				printf("  when testing level %i\n", level);
				return false;
			}
		}
	}
	return true;
}

enum piglit_result
piglit_display(void)
{
	GLboolean pass = GL_TRUE;
	GLuint tex1d;
	int level;

	glClearColor(0.1, 0.1, 0.1, 0.1);
	glClear(GL_COLOR_BUFFER_BIT);

	tex1d = create_texture_1d();

	for (level = 0; level < TEX_LEVELS; level++) {
		draw_level(5, 5 + level * 10, level);
		pass = pass && test_level(level);
	}

	glDeleteTextures(1, &tex1d);
	piglit_present_results();
	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void piglit_init(int argc, char **argv)
{
	int i;

	piglit_require_extension("GL_EXT_framebuffer_object");

	format = GL_RGBA8;

	for (i = 1; i < argc; i++) {
		if (strcmp(argv[i], "RGB9_E5") == 0) {
			/* Test a non-renderable format. */
			piglit_require_extension("GL_EXT_texture_shared_exponent");
			format = GL_RGB9_E5;
		}
		else {
			assert(0);
		}
	}

	prog = piglit_build_simple_program(NULL, fs_1d);
}
