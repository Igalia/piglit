/*
 * Copyright © 2016 VMware, Inc.
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
 * Verifies that mipmap generation uses the right format (from view,
 * not what was originally specified).
 */

#include "piglit-util-gl.h"
#include "common.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 20;

	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;

	config.khr_no_error_support = PIGLIT_NO_ERRORS;

PIGLIT_GL_TEST_CONFIG_END

/**
 * Create view with different view format and generate mipmap.
 */
static bool
test_mipgen(void)
{
	GLuint tex, new_tex;
	GLint width = 4, height = 4, levels = 2;
	bool pass = true;

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);

	glTexStorage2D(GL_TEXTURE_2D, levels, GL_R8, width, height);

	/* averaging these as snorm values should give 0 */
	GLubyte buf[4][4] =
		{{0xFF, 0x01, 0xFF, 0x01},
		 {0xFF, 0x01, 0xFF, 0x01},
		 {0xFF, 0x01, 0xFF, 0x01},
		 {0xFF, 0x01, 0xFF, 0x01}};
	GLbyte res[4];

	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height,
			GL_RED, GL_UNSIGNED_BYTE, buf);

	glGenTextures(1, &new_tex);

	glTextureView(new_tex, GL_TEXTURE_2D, tex,  GL_R8_SNORM, 0, 2, 0, 1);
	glBindTexture(GL_TEXTURE_2D, new_tex);
	glGenerateMipmap(GL_TEXTURE_2D);
	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glGetTexImage(GL_TEXTURE_2D, 1, GL_RED, GL_BYTE, &res);
	pass = !res[0] && !res[1] && !res[2] && !res[3];

	if (!pass) {
		printf("expected 0, got %d %d %d %d\n",
		       res[0], res[1], res[2], res[3]);
	}

	glDeleteTextures(1, &new_tex);
	glDeleteTextures(1, &tex);

	return pass;
}

enum piglit_result
piglit_display(void)
{
       return PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	bool pass = true;

	piglit_require_extension("GL_ARB_texture_storage");
	piglit_require_extension("GL_ARB_texture_view");

	pass = test_mipgen() && piglit_check_gl_error(GL_NO_ERROR);
	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}
