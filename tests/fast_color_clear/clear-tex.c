/* Copyright © 2020 Intel Corporation
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

/**
 * Check that drivers correctly handle the clear color when fast-clearing via
 * glClearTexImage.
 */

#include "piglit-util-gl.h"


PIGLIT_GL_TEST_CONFIG_BEGIN

config.supports_gl_compat_version = 44;

PIGLIT_GL_TEST_CONFIG_END

enum piglit_result
piglit_display(void)
{
	return PIGLIT_FAIL;
}

static void
clear_tex(GLuint internalFormat, GLuint format, GLuint type,
	  const void *clear_pix)
{
	GLuint tex;
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, 32, 32, 0, format, type,
		     NULL);
	glClearTexImage(tex, 0, format, type, clear_pix);

	if (glGetError() != GL_NO_ERROR)
		piglit_report_result(PIGLIT_FAIL);
}

static bool
test_16bpc_base(const char *fmt, GLuint format, const void *pix,
		const float *expected)
{
	printf("Testing 16bpc %s\n", fmt);
	clear_tex(format, format, GL_UNSIGNED_SHORT, pix);
	return piglit_probe_texel_rgba(GL_TEXTURE_2D, 0, 0, 0, expected);
}

void
piglit_init(int argc, char **argv)
{
	const uint16_t pix[] = {0x3fff, 0x7fff};
	const float a[] = {0.0, 0.0, 0.0, 0.5};
	const float la[] = {0.25, 0.0, 0.0, 0.5};

	bool pass = true;
	pass &= test_16bpc_base("A", GL_ALPHA, &pix[1], a);
	pass &= test_16bpc_base("LA", GL_LUMINANCE_ALPHA, &pix[0], la);

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}
