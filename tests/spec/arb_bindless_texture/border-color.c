/*
 * Copyright (C) 2017 Valve Corporation
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
 */

/** \file
 *
 * Test cases which exercise different border color values with
 * glGetTextureHandleARB() and glGetTextureSamplerARB().
 */

/* The ARB_bindless_texture spec says:
 *
 * "The error INVALID_OPERATION is generated if the border color (taken from
 *  the embedded sampler for GetTextureHandleARB or from the <sampler> for
 *  GetTextureSamplerHandleARB) is not one of the following allowed values.
 *  If the texture's base internal format is signed or unsigned integer,
 *  allowed values are (0,0,0,0), (0,0,0,1), (1,1,1,0), and (1,1,1,1). If the
 *  base internal format is not integer, allowed values are
 *  (0.0,0.0,0.0,0.0), (0.0,0.0,0.0,1.0), (1.0,1.0,1.0,0.0), and
 *  (1.0,1.0,1.0,1.0)."
 */

#include "common.h"

static struct piglit_gl_test_config *piglit_config;

PIGLIT_GL_TEST_CONFIG_BEGIN

	piglit_config = &config;
	config.supports_gl_compat_version = 33;
	config.supports_gl_core_version = 33;
	config.khr_no_error_support = PIGLIT_NO_ERRORS;

PIGLIT_GL_TEST_CONFIG_END

static enum piglit_result
check_valid_integer_border_colors(void *data)
{
	GLint border_colors[4][4] = {
		{ 0, 0, 0, 0 },
		{ 0, 0, 0, 1 },
		{ 1, 1, 1, 0 },
		{ 1, 1, 1, 1 },
	};
	GLuint tex, i;

	for (i = 0; i < 4; i++) {
		tex = piglit_integer_texture(GL_RGBA32I, 16, 16, 0, 0);
		glTextureParameterIiv(tex, GL_TEXTURE_BORDER_COLOR,
				      border_colors[i]);
		glBindTexture(GL_TEXTURE_2D, 0);

		glGetTextureHandleARB(tex);
		if (!piglit_check_gl_error(GL_NO_ERROR))
			return PIGLIT_FAIL;
	}

	return PIGLIT_PASS;
}

static enum piglit_result
check_invalid_integer_border_colors(void *data)
{
	GLint border_colors[7][4] = {
		{ 1, 0, 0, 0 },
		{ 0, 0, 1, 0 },
		{ 0, 1, 0, 0 },
		{ 0, 1, 1, 0 },
		{ 0, 0, 1, 1 },
		{ 0, 1, 1, 1 },
		{ 42, 2, 7, 9 },
	};
	GLuint sampler, texture;
	GLint i;

	if (piglit_khr_no_error)
		return PIGLIT_SKIP;

	texture = piglit_integer_texture(GL_RGBA32I, 16, 16, 0, 0);
	sampler = new_sampler();
	glBindTexture(GL_TEXTURE_2D, 0);

	for (i = 0; i < 7; i++) {
		glSamplerParameterIiv(sampler, GL_TEXTURE_BORDER_COLOR,
				      border_colors[i]);
		glGetTextureSamplerHandleARB(texture, sampler);
		if (!piglit_check_gl_error(GL_INVALID_OPERATION))
			return PIGLIT_FAIL;
	}

	return PIGLIT_PASS;
}

static enum piglit_result
check_valid_float_border_colors(void *data)
{
	GLfloat border_colors[4][4] = {
		{ 0.0, 0.0, 0.0, 0.0 },
		{ 0.0, 0.0, 0.0, 1.0 },
		{ 1.0, 1.0, 1.0, 0.0 },
		{ 1.0, 1.0, 1.0, 1.0 },
	};
	GLuint tex, i;

	for (i = 0; i < 4; i++) {
		tex = piglit_rgbw_texture(GL_RGBA32F, 16, 16, GL_FALSE,
					  GL_FALSE, GL_UNSIGNED_NORMALIZED);
		glTextureParameterfv(tex, GL_TEXTURE_BORDER_COLOR,
				     border_colors[i]);
		glBindTexture(GL_TEXTURE_2D, 0);

		glGetTextureHandleARB(tex);
		if (!piglit_check_gl_error(GL_NO_ERROR))
			return PIGLIT_FAIL;
	}

	return PIGLIT_PASS;
}

static enum piglit_result
check_invalid_float_border_colors(void *data)
{
	GLfloat border_colors[7][4] = {
		{ 1.0, 0.0, 0.0, 0.0 },
		{ 0.0, 0.0, 1.0, 0.0 },
		{ 0.0, 1.0, 0.0, 0.0 },
		{ 0.0, 1.0, 1.0, 0.0 },
		{ 0.0, 0.0, 1.0, 1.0 },
		{ 0.0, 1.0, 1.0, 1.0 },
		{ 0.3, 0.9, 0.7, 0.5 },
	};
	GLuint sampler, texture;
	GLint i;

	if (piglit_khr_no_error)
		return PIGLIT_SKIP;

	texture = piglit_rgbw_texture(GL_RGBA32F, 16, 16, GL_FALSE, GL_FALSE, 
				      GL_UNSIGNED_NORMALIZED);
	sampler = new_sampler();
	glBindTexture(GL_TEXTURE_2D, 0);

	for (i = 0; i < 7; i++) {
		glSamplerParameterfv(sampler, GL_TEXTURE_BORDER_COLOR,
				     border_colors[i]);

		glGetTextureSamplerHandleARB(texture, sampler);
		if (!piglit_check_gl_error(GL_INVALID_OPERATION))
			return PIGLIT_FAIL;
	}

	return PIGLIT_PASS;
}

static const struct piglit_subtest subtests[] = {
	{
		"Check valid integer border color values",
		"check_valid_integer_border_colors",
		check_valid_integer_border_colors,
		NULL
	},
	{
		"Check invalid integer border color values",
		"check_invalid_integer_border_colors",
		check_invalid_integer_border_colors,
		NULL
	},
	{
		"Check valid float border color values",
		"check_valid_float_border_colors",
		check_valid_float_border_colors,
		NULL
	},
	{
		"Check invalid float border color values",
		"check_invalid_float_border_colors",
		check_invalid_float_border_colors,
		NULL
	},
	{
		NULL,
		NULL,
		NULL,
		NULL
	}
};

enum piglit_result
piglit_display(void)
{
	return PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	enum piglit_result result;

	piglit_require_extension("GL_ARB_bindless_texture");
	result = piglit_run_selected_subtests(subtests,
					      piglit_config->selected_subtests,
					      piglit_config->num_selected_subtests,
					      PIGLIT_SKIP);
	piglit_report_result(result);
}
