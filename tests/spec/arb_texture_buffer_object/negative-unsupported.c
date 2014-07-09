/* Copyright © 2012 Intel Corporation
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

/** @file negative-unsupported.c
 * Verify that the GL_TEXTURE_BUFFER target cannot be used when the
 * extension is not supported.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;

PIGLIT_GL_TEST_CONFIG_END

enum piglit_result
piglit_display(void)
{
	return PIGLIT_FAIL; /* UNREACHED */
}


void
piglit_init(int argc, char **argv)
{
	GLuint tex;
	GLuint bo;
	bool pass = true;

	piglit_require_not_extension("GL_ARB_texture_buffer_object");
	if (piglit_get_gl_version() >= 31)
		piglit_report_result(PIGLIT_SKIP);

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_BUFFER, tex);
	pass = piglit_check_gl_error(GL_INVALID_ENUM) && pass;

	glGenBuffers(1, &bo);
	glBindBuffer(GL_TEXTURE_BUFFER, bo);
	pass = piglit_check_gl_error(GL_INVALID_ENUM) && pass;

	glDeleteBuffers(1, &bo);
	glDeleteTextures(1, &tex);
	piglit_report_result(PIGLIT_PASS);
}
