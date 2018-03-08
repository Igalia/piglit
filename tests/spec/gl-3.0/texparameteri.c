/*
 * Copyright © 2018 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * on the rights to use, copy, modify, merge, publish, distribute, sub
 * license, and/or sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NON-INFRINGEMENT.  IN NO EVENT SHALL VMWARE AND/OR THEIR SUPPLIERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/**
 * @file
 * Basic tests of OpenGL 3.0 gl{Get,}TexParameterI{iv,uiv} functions
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 30;
	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

#ifndef GL_TEXTURE_CROP_RECT_OES
#define GL_TEXTURE_CROP_RECT_OES          0x8B9D
#endif

enum piglit_result
piglit_display(void)
{
	return PIGLIT_FAIL;
}

static bool
check_values(const char *msg, const unsigned *expected, const unsigned *got)
{
	if (memcmp(expected, got, 4 * sizeof(*expected)) != 0) {
		fprintf(stderr, "%s.\n"
			"         Got: 0x%08x 0x%08x 0x%08x 0x%08x\n"
			"    Expected: 0x%08x 0x%08x 0x%08x 0x%08x\n",
			msg,
			got[0],
			got[1],
			got[2],
			got[3],
			expected[0],
			expected[1],
			expected[2],
			expected[3]);
		return false;
	}

	return true;
}

void
piglit_init(int argc, char **argv)
{
	static const GLuint uint_border[4] = {
		0x80706050, 0x40302010,
		0x08070605, 0x04030201
	};
	static const GLint int_border[4] = { -1, -2, -3, -4 };
	static const GLuint bad[4] = {
		0x0badc0de, 0x0badc0de, 0x0badc0de, 0x0badc0de
	};
	GLuint uint_return[4];
	GLint int_return[4];
	GLuint tex[2];
	bool pass = true;

	(void) argc;
	(void) argv;

	glGenTextures(ARRAY_SIZE(tex), tex);

	/* Try the unsigned integer texture. */
	glBindTexture(GL_TEXTURE_2D, tex[0]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32UI, 16, 16, 0,
		     GL_RGBA_INTEGER, GL_UNSIGNED_INT, NULL);
	glTexParameterIuiv(GL_TEXTURE_2D,
			   GL_TEXTURE_BORDER_COLOR,
			   uint_border);

	glGetTexParameterIuiv(GL_TEXTURE_2D,
			      GL_TEXTURE_BORDER_COLOR,
			      uint_return);

	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

	if (!check_values("uint border color mismatch",
			  uint_border, uint_return))
		pass = false;

	memcpy(uint_return, bad, sizeof(bad));
	glGetTexParameterIuiv(GL_TEXTURE_2D,
			      0xbeef0000 | GL_TEXTURE_BORDER_COLOR,
			      uint_return);

	pass = piglit_check_gl_error(GL_INVALID_ENUM) && pass;

	if (!check_values("Wrote data during GL error "
			  "glGetTexParameterIuiv(0xbeef0000 | GL_TEXTURE_BORDER_COLOR)",
			  bad, uint_return))
		pass = false;

	/* GL_TEXTURE_CROP_RECT_OES only exists in OpenGL ES 1.x. */
	memcpy(uint_return, bad, sizeof(bad));
	glGetTexParameterIuiv(GL_TEXTURE_2D,
			      GL_TEXTURE_CROP_RECT_OES,
			      uint_return);

	pass = piglit_check_gl_error(GL_INVALID_ENUM) && pass;

	if (!check_values("Wrote data during GL error "
			  "glGetTexParameterIuiv(GL_TEXTURE_CROP_RECT_OES)",
			  bad, uint_return))
		pass = false;

	/* Try the signed integer texture. */
	glBindTexture(GL_TEXTURE_2D, tex[1]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32I, 16, 16, 0,
		     GL_RGBA_INTEGER, GL_INT, NULL);
	glTexParameterIiv(GL_TEXTURE_2D,
			  GL_TEXTURE_BORDER_COLOR,
			  int_border);

	glGetTexParameterIiv(GL_TEXTURE_2D,
			     GL_TEXTURE_BORDER_COLOR,
			     int_return);

	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

	if (!check_values("int border color mismatch",
			  (unsigned *) int_border, (unsigned *) int_return))
		pass = false;

	memcpy(int_return, bad, sizeof(bad));
	glGetTexParameterIiv(GL_TEXTURE_2D,
			     0xbeef0000 | GL_TEXTURE_BORDER_COLOR,
			     int_return);

	pass = piglit_check_gl_error(GL_INVALID_ENUM) && pass;

	if (!check_values("Wrote data during GL error "
			  "glGetTexParameterIiv(0xbeef0000 | GL_TEXTURE_BORDER_COLOR)",
			  bad, (unsigned *) int_return))
		pass = false;

	/* GL_TEXTURE_CROP_RECT_OES only exists in OpenGL ES 1.x. */
	memcpy(int_return, bad, sizeof(bad));
	glGetTexParameterIiv(GL_TEXTURE_2D,
			     GL_TEXTURE_CROP_RECT_OES,
			     int_return);

	pass = piglit_check_gl_error(GL_INVALID_ENUM) && pass;

	if (!check_values("Wrote data during GL error "
			  "glGetTexParameterIiv(GL_TEXTURE_CROP_RECT_OES)",
			  bad, (unsigned *) int_return))
		pass = false;

	glBindTexture(GL_TEXTURE_2D, 0);
	glDeleteTextures(ARRAY_SIZE(tex), tex);

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}
