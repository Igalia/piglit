/*
 * Copyright © 2018 Advanced Micro Devices, Inc.
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

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_es_version = 30;

	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;
	config.khr_no_error_support = PIGLIT_NO_ERRORS;

PIGLIT_GL_TEST_CONFIG_END

void
piglit_init(int argc, char **argv)
{
	piglit_require_extension("GL_EXT_texture_compression_bptc");

	GLuint tex;
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_RGBA_BPTC_UNORM_EXT,
		     64, 64, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM_EXT,
		     64, 64, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT_EXT,
		     64, 64, 0, GL_RGB, GL_FLOAT, NULL);
	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT_EXT,
		     64, 64, 0, GL_RGB, GL_FLOAT, NULL);
	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	glCompressedTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_RGBA_BPTC_UNORM_EXT,
			       64, 64, 0, 4096, NULL);
	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	glCompressedTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM_EXT,
			       64, 64, 0, 4096, NULL);
	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	glCompressedTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT_EXT,
			       64, 64, 0, 4096, NULL);
	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	glCompressedTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT_EXT,
			       64, 64, 0, 4096, NULL);
	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	piglit_report_result(PIGLIT_PASS);
}

enum piglit_result
piglit_display(void)
{
	return PIGLIT_FAIL;
}
