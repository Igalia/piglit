/*
 * Copyright (c) 2014 Intel Corporation
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

/** @file srgb.c
 *
 * Test using glClearTexSubImage with sRGB-format textures. This is
 * interesting to test because the clear implementation should not be
 * applying the sRGB conversion but a naïve implementation using
 * glClear might accidentally do so.
 */

#include "common.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 13;

	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

	config.khr_no_error_support = PIGLIT_NO_ERRORS;

PIGLIT_GL_TEST_CONFIG_END

static const struct format
formats[] = {
	{ GL_SRGB_EXT, GL_RGB, GL_UNSIGNED_BYTE, 3 },
	{ GL_SRGB8_EXT, GL_RGB, GL_UNSIGNED_BYTE, 3 },
	{ GL_SRGB_ALPHA_EXT, GL_RGBA, GL_UNSIGNED_BYTE, 4 },
	{ GL_SRGB8_ALPHA8_EXT, GL_RGBA, GL_UNSIGNED_BYTE, 4 }
};

void
piglit_init(int argc, char **argv)
{
	bool pass;

	/* sRGB textures are supported in GL 2.1 or with the
	 * GL_EXT_texture_sRGB extension
	 */
	if (piglit_get_gl_version() < 21 &&
	    !piglit_is_extension_supported("GL_EXT_texture_sRGB")) {
		piglit_report_result(PIGLIT_SKIP);
	}

	if (piglit_get_gl_version() >= 30 ||
	    piglit_is_extension_supported("GL_EXT_framebuffer_sRGB")) {
		/* Enable SRGB. This shouldn't affect the results of
		 * the test because the clear value should be treated
		 * like as in glTexImage2D and so they shouldn't be
		 * converted. Enabling it tests that the GL
		 * successfully ignores it.
		 */
		glEnable(GL_FRAMEBUFFER_SRGB);
	}

	pass = test_formats(formats, ARRAY_SIZE(formats));

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}

enum piglit_result
piglit_display(void)
{
	/* unused */
	return PIGLIT_FAIL;
}
