/*
 * Copyright © 2010 Marek Olšák <maraeo@gmail.com>
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
 */

/** @file tex-miplevel-selection.c
 *
 * Tests the interaction between GL_TEXTURE_BASE/MAX_LEVEL
 * GL_TEXTURE_MIN/MAX_LOD, TEXTURE_LOD_BIAS, mipmap
 * filtering on/off, and variable texture scaling.
 * Also tests ARB_shader_texture_lod if requested.
 */

#include "piglit-util-gl-common.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_width = 900;
	config.window_height = 600;
	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

#define TEX_WIDTH 32
#define TEX_HEIGHT 32
#define LAST_LEVEL 5

static const float colors[][3] = {
	{1.0, 0.0, 0.0},
	{0.0, 1.0, 0.0},
	{0.0, 0.0, 1.0},
	{1.0, 1.0, 0.0},
	{0.0, 1.0, 1.0},
	{1.0, 0.0, 1.0},
};
static GLboolean in_place_probing, no_bias, no_lod, ARB_shader_texture_lod;
static GLuint loc_lod;

static const char *fscode =
	"#extension GL_ARB_shader_texture_lod : require\n"
	"uniform sampler2D tex;\n"
	"uniform float lod;\n"
	"void main() {\n"
	"  gl_FragColor = texture2DLod(tex, gl_TexCoord[0].xy, lod);"
	"}\n";

void
piglit_init(int argc, char **argv)
{
	GLuint tex, fb;
	GLenum status;
	int i, dim;
	GLuint fs, prog, loc_tex;

        for (i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-inplace") == 0)
			in_place_probing = GL_TRUE;
		else if (strcmp(argv[i], "-nobias") == 0)
			no_bias = GL_TRUE;
		else if (strcmp(argv[i], "-nolod") == 0)
			no_lod = GL_TRUE;
		else if (strcmp(argv[i], "-GL_ARB_shader_texture_lod") == 0)
			ARB_shader_texture_lod = GL_TRUE;
        }

	piglit_require_extension("GL_EXT_framebuffer_object");
	if (piglit_get_gl_version() < 14)
		piglit_report_result(PIGLIT_SKIP);

	if (ARB_shader_texture_lod) {
		piglit_require_GLSL();
		piglit_require_extension("GL_ARB_shader_texture_lod");

		fs = piglit_compile_shader_text(GL_FRAGMENT_SHADER, fscode);
		prog = piglit_link_simple_program(0, fs);
		glUseProgram(prog);
		loc_tex = glGetUniformLocation(prog, "tex");
		loc_lod = glGetUniformLocation(prog, "lod");
		glUniform1i(loc_tex, 0);

		puts("Testing GL_ARB_shader_texture_lod.");
	}

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);

	for (i = 0, dim = TEX_WIDTH; dim >0; i++, dim /= 2) {
		glTexImage2D(GL_TEXTURE_2D, i, GL_RGBA,
			     dim, dim,
			     0,
			     GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	}
	assert(glGetError() == 0);

	glBindTexture(GL_TEXTURE_2D, 0);
	glDisable(GL_TEXTURE_2D);

	glGenFramebuffersEXT(1, &fb);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fb);

	for (i = 0, dim = TEX_WIDTH; dim >0; i++, dim /= 2) {
		glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
					  GL_COLOR_ATTACHMENT0_EXT,
					  GL_TEXTURE_2D,
					  tex,
					  i);


		status = glCheckFramebufferStatusEXT (GL_FRAMEBUFFER_EXT);
		if (status != GL_FRAMEBUFFER_COMPLETE_EXT) {
			fprintf(stderr, "FBO incomplete\n");
			piglit_report_result(PIGLIT_SKIP);
		}

		glClearColor(colors[i][0],
			     colors[i][1],
			     colors[i][2],
			     0.0);
		glClear(GL_COLOR_BUFFER_BIT);

		assert(glGetError() == 0);
	}

	glDeleteFramebuffersEXT(1, &fb);
	glBindTexture(GL_TEXTURE_2D, tex);

	glViewport(0, 0, piglit_width, piglit_height);

	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);

	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, piglit_winsys_fbo);

	glEnable(GL_TEXTURE_2D);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}

static void
draw_quad(int x, int y, int w, int h, int level)
{
	float s, t;

	if (ARB_shader_texture_lod) {
		float lod = level;
		glUniform1fv(loc_lod, 1, &lod);

		s = w/(float)TEX_WIDTH;
		t = h/(float)TEX_HEIGHT;
	} else {
		s = (w/(float)TEX_WIDTH) * (1<<level);
		t = (h/(float)TEX_HEIGHT) * (1<<level);
	}

	glBegin(GL_QUADS);

	glTexCoord2f(0, 0);
	glVertex2f(x, y);
	glTexCoord2f(s, 0);
	glVertex2f(x + w, y);
	glTexCoord2f(s, t);
	glVertex2f(x + w, y + h);
	glTexCoord2f(0, t);
	glVertex2f(x, y + h);

	glEnd();
}

enum piglit_result
piglit_display(void)
{
	int scale_to_level, baselevel, maxlevel, minlod, maxlod, bias, mipfilter;
	int expected_level, x, y, total, failed, c;
	int start_bias, end_bias;
	int start_min_lod, end_min_lod, end_max_lod;
	unsigned char *pix, *p;

	if (no_bias) {
		start_bias = 0;
		end_bias = 0;
	} else {
		start_bias = -LAST_LEVEL;
		end_bias = LAST_LEVEL;
	}

	if (no_lod) {
		start_min_lod = 0;
		end_min_lod = 0;
		end_max_lod = 0;
	} else {
		start_min_lod = 0;
		end_min_lod = LAST_LEVEL;
		end_max_lod = LAST_LEVEL;
	}

	glClearColor(0.5, 0.5, 0.5, 0.5);
	glClear(GL_COLOR_BUFFER_BIT);

	total = 0;
	failed = 0;
	for (scale_to_level = 0; scale_to_level <= LAST_LEVEL; scale_to_level++)
		for (baselevel = 0; baselevel <= LAST_LEVEL; baselevel++)
			for (maxlevel = baselevel; maxlevel <= LAST_LEVEL; maxlevel++)
				for (minlod = start_min_lod; minlod <= end_min_lod; minlod++)
					for (maxlod = minlod; maxlod <= end_max_lod; maxlod++)
						for (bias = start_bias; bias <= end_bias; bias++)
							for (mipfilter = 0; mipfilter < 2; mipfilter++) {
								glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, baselevel);
								glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, maxlevel);
								if (!no_lod) {
									glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_LOD, minlod);
									glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LOD, maxlod);
								}
								if (!no_bias)
									glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_LOD_BIAS, bias);
								glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
										mipfilter ? GL_NEAREST_MIPMAP_NEAREST : GL_NEAREST);

								x = (total % (piglit_width/3)) * 3;
								y = (total / (piglit_width/3)) * 3;

								if (ARB_shader_texture_lod)
									draw_quad(x, y, 3, 3, scale_to_level - baselevel);
								else
									draw_quad(x, y, 3, 3, scale_to_level);

								if (mipfilter) {
									if (no_lod) {
										expected_level = CLAMP(scale_to_level + bias,
												       baselevel,
												       maxlevel);
									} else {
										expected_level = CLAMP(scale_to_level + bias,
												       MIN2(baselevel + minlod, maxlevel),
												       MIN2(baselevel + maxlod, maxlevel));
									}
								} else {
									expected_level = baselevel;
								}
								assert(expected_level <= 5);

                                                                if (in_place_probing &&
                                                                    !piglit_probe_pixel_rgb(x, y, colors[expected_level])) {
									failed++;
									printf("  Expected mipmap level: %i\n", expected_level);
									printf("  Scale to level: %i, baselevel: %i, maxlevel: %i, "
									       "minlod: %i, maxlod: %i, bias: %i, mipfilter: %s\n",
									       scale_to_level, baselevel, maxlevel, minlod,
									       no_lod ? LAST_LEVEL : maxlod, bias, mipfilter ? "yes" : "no");
                                                                }

								total++;
							}

	if (!in_place_probing) {
		pix = malloc(piglit_width * piglit_height * 4);
		glReadPixels(0, 0, piglit_width, piglit_height, GL_RGBA, GL_UNSIGNED_BYTE, pix);

		total = 0;
		for (scale_to_level = 0; scale_to_level <= LAST_LEVEL; scale_to_level++)
			for (baselevel = 0; baselevel <= LAST_LEVEL; baselevel++)
				for (maxlevel = baselevel; maxlevel <= LAST_LEVEL; maxlevel++)
					for (minlod = start_min_lod; minlod <= end_min_lod; minlod++)
						for (maxlod = minlod; maxlod <= end_max_lod; maxlod++)
							for (bias = start_bias; bias <= end_bias; bias++)
								for (mipfilter = 0; mipfilter < 2; mipfilter++) {
									if (mipfilter) {
										if (no_lod) {
											expected_level = CLAMP(scale_to_level + bias,
													       baselevel,
													       maxlevel);
										} else {
											expected_level = CLAMP(scale_to_level + bias,
													       MIN2(baselevel + minlod, maxlevel),
													       MIN2(baselevel + maxlod, maxlevel));
										}
									} else {
										expected_level = baselevel;
									}
									assert(expected_level <= 5);

									x = (total % (piglit_width/3)) * 3;
									y = (total / (piglit_width/3)) * 3;
									p = pix + (y*piglit_width + x)*4;

									for (c = 0; c < 3; c++) {
										if (fabs(colors[expected_level][c] - (p[c]/255.0)) > 0.01) {
											failed++;

											printf("Probe color at (%i,%i)\n", x, y);
											printf("  Expected: %f %f %f\n", colors[expected_level][0],
											       colors[expected_level][1], colors[expected_level][2]);
											printf("  Observed: %f %f %f\n", p[0]/255.0, p[1]/255.0, p[2]/255.0);
											printf("  Expected mipmap level: %i\n", expected_level);
											printf("  Scale to level: %i, baselevel: %i, maxlevel: %i, "
											       "minlod: %i, maxlod: %i, bias: %i, mipfilter: %s\n",
											       scale_to_level, baselevel, maxlevel, minlod,
											       no_lod ? LAST_LEVEL : maxlod, bias, mipfilter ? "yes" : "no");
											break;
										}
									}
									total++;
								}
		free(pix);
	}
	printf("Summary: %i/%i passed\n", total-failed, total);

	piglit_present_results();

	return !failed ? PIGLIT_PASS : PIGLIT_FAIL;
}
