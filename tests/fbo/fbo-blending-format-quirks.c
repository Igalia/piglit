/*
 * Copyright © 2018 Collabora Ltd
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
 *    Erik Faye-Lund <erik.faye-lund@collabora.com>
 *
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	/* Drivers that do not support GL_ARB_texture_non_power_of_two require
	 * window dimensions that are powers of two for this test.
	 */
	config.window_width = next_power_of_two(config.window_width);
	config.window_height = next_power_of_two(config.window_height);

	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;
	config.khr_no_error_support = PIGLIT_NO_ERRORS;

PIGLIT_GL_TEST_CONFIG_END

static enum piglit_result test_formats(const char *name, GLenum formats[2],
				       float expect[2][4], GLenum factors[2])
{
	GLboolean pass = GL_TRUE;
	GLuint tex[2], fb;
	GLenum draw_bufs[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
	GLenum status;
	int i;

	glGenFramebuffersEXT(1, &fb);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fb);
	glViewport(0, 0, piglit_width, piglit_height);

	glGenTextures(2, tex);
	for (i = 0; i < 2; ++i) {
		glBindTexture(GL_TEXTURE_2D, tex[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, formats[i],
			     piglit_width, piglit_height, 0,
			     GL_RGBA, GL_FLOAT, NULL);

		glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
					  GL_COLOR_ATTACHMENT0_EXT + i,
					  GL_TEXTURE_2D,
					  tex[i],
					  0);
	}
	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
	if (status != GL_FRAMEBUFFER_COMPLETE_EXT) {
		printf(" - fbo incomplete (status = %s)\n",
		       piglit_get_gl_enum_name(status));
		return PIGLIT_SKIP;
	}

	printf("Testing %s\n", name);

	glClearColor(0.0, 0.0, 0.0, 0.5);
	glClear(GL_COLOR_BUFFER_BIT);

	glEnable(GL_BLEND);
	glBlendFunc(factors[0], factors[1]);
	glBlendColor(1.0, 0.5, 0.25, 0.125);
	glColor4f(1.0, 1.0, 1.0, 1.0);

	glDrawBuffers(2, draw_bufs);
	piglit_draw_rect(-1.0, -1.0, 2.0, 2.0);

	glReadBuffer(GL_COLOR_ATTACHMENT0);
	if (!piglit_probe_pixel_rgba(piglit_width / 2, piglit_height / 2,
				     expect[0])) {
		printf("  when testing GL_COLOR_ATTACHMENT0.\n");
		pass = GL_FALSE;
	}

	glReadBuffer(GL_COLOR_ATTACHMENT1);
	if (!piglit_probe_pixel_rgba(piglit_width / 2, piglit_height / 2,
				     expect[1])) {
		printf("  when testing GL_COLOR_ATTACHMENT1.\n");
		pass = GL_FALSE;
	}

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

enum piglit_result piglit_display(void)
{
	int i;
        enum piglit_result result, end_result = PIGLIT_PASS;
        bool all_skip = true;

	static const struct {
		const char *name;
		GLenum formats[2];
		GLenum factors[2];
		float expect[2][4];
	} cases[] = {
		{ "alpha expand",
			{ GL_RGBA, GL_RGB },
			{ GL_DST_ALPHA, GL_ZERO }, {
				{ 0.5, 0.5, 0.5, 0.5 },
				{ 1.0, 1.0, 1.0, 1.0 }
			}
		},

		{ "alpha swizzle, variant 1",
			{ GL_RGBA, GL_ALPHA },
			{ GL_DST_ALPHA, GL_ZERO }, {
				{ 0.5, 0.5, 0.5, 0.5 },
				{ 0.0, 0.0, 0.0, 0.0 }
			}
		},

		{ "alpha swizzle, variant 2",
			{ GL_RGBA, GL_ALPHA },
			{ GL_CONSTANT_COLOR, GL_ZERO }, {
				{ 1.0, 0.5, 0.25, 0.125 },
				{ 0.0, 0.0, 0.0, 0.125 }
			}
		},

		{ "alpha swizzle, variant 3",
			{ GL_ALPHA, GL_RGBA },
			{ GL_CONSTANT_COLOR, GL_ZERO }, {
				{ 0.0, 0.0, 0.0, 0.125 },
				{ 1.0, 0.5, 0.25, 0.125 }
			}
		}
	};

	for (i = 0; i < ARRAY_SIZE(cases); ++i) {
		result = test_formats(cases[i].name, cases[i].formats,
				      cases[i].expect, cases[i].factors);

		if (result != PIGLIT_SKIP)
			all_skip = false;

		if (result == PIGLIT_FAIL)
			end_result = result;
	}

	if (all_skip)
		return PIGLIT_SKIP;
	return end_result;
}

void piglit_init(int argc, char **argv)
{
	piglit_require_extension("GL_EXT_framebuffer_object");
	glDisable(GL_DITHER);
}
