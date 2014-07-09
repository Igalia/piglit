/*
 * Copyright © 2013 Intel Corporation
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

/** @file teximage-multisample.c
 *
 * Section 3.8.4(TEXTURING) From GL spec 3.2 core:
 * Functions added 'glTexImage2DMultisample'
 *
 * For TexImage2DMultisample, target must be TEXTURE_2D_MULTISAMPLE or
 * PROXY_TEXTURE_2D_MULTISAMPLE.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;
	config.supports_gl_core_version = 31;

PIGLIT_GL_TEST_CONFIG_END

void
piglit_init(int argc, char **argv)
{
	bool pass = true;
	GLuint textures[3];

	if(piglit_get_gl_version() < 32) {
		piglit_require_extension("GL_ARB_texture_multisample");
	}

	glGenTextures(3, textures);

	/* Pass a Texture 2D Multisample */
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, textures[0]);
	glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGB,
				1024, 1024, GL_FALSE);
	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

	/* Pass a Proxy Texture 2d Multisample */
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, textures[1]);
	glTexImage2DMultisample(GL_PROXY_TEXTURE_2D_MULTISAMPLE, 4, GL_RGB,
				1024, 1024, GL_FALSE);
	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

	/* Pass an Invalid Enum */
	glBindTexture(GL_TEXTURE_2D, textures[2]);
	glTexImage2DMultisample(GL_TEXTURE_2D, 4, GL_RGB,
				1024, 1024, GL_FALSE);
	pass = piglit_check_gl_error(GL_INVALID_ENUM) && pass;

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}

enum piglit_result
piglit_display(void)
{
	return PIGLIT_FAIL;
}
