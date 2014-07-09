/*
 * Copyright © 2014 VMware, Inc.
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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

/*
 * Test GL_EXT_texture_swizzle API functions.
 * Brian Paul
 * 24 April 2014
 */


#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN
	config.supports_gl_compat_version = 12;
	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;
PIGLIT_GL_TEST_CONFIG_END


static bool
test_get(GLenum pname, GLint expected)
{
	GLint val;

	glGetTexParameteriv(GL_TEXTURE_2D, pname, &val);

	if (val != expected) {
		printf("glGetTexParameteriv(%s) returned %s instead of %s\n",
		       piglit_get_gl_enum_name(pname),
		       piglit_get_gl_enum_name(val),
		       piglit_get_gl_enum_name(expected));
		return false;
	}

	return true;
}


static bool
test_api(void)
{
	static const GLint swz[4] = { GL_BLUE, GL_GREEN, GL_ALPHA, GL_ZERO };
	GLint swzOut[4];

	/* test bad param value */
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_R_EXT, GL_RGBA);

	if (!piglit_check_gl_error(GL_INVALID_ENUM))
		return false;

	/* test good param values */
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_R_EXT, GL_ONE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_G_EXT, GL_ZERO);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_B_EXT, GL_RED);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_A_EXT, GL_BLUE);

	if (!piglit_check_gl_error(GL_NO_ERROR))
		return false;

	if (!test_get(GL_TEXTURE_SWIZZLE_R_EXT, GL_ONE))
		return false;

	if (!test_get(GL_TEXTURE_SWIZZLE_G_EXT, GL_ZERO))
		return false;

	if (!test_get(GL_TEXTURE_SWIZZLE_B_EXT, GL_RED))
		return false;

	if (!test_get(GL_TEXTURE_SWIZZLE_A_EXT, GL_BLUE))
		return false;

	/* set all at once */
	glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA_EXT, swz);

	if (!piglit_check_gl_error(GL_NO_ERROR))
		return false;

	glGetTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA_EXT, swzOut);
	if (swzOut[0] != swz[0] ||
	    swzOut[1] != swz[1] ||
	    swzOut[2] != swz[2] ||
	    swzOut[3] != swz[3]) {
		printf("glGetTexParameteriv(GL_TEXTURE_SWIZZLE_RGBA_EXT) failed\n");
		return false;
	}

	return true;
}


enum piglit_result
piglit_display(void)
{
	/* UNREACHED */
	return PIGLIT_FAIL;
}


void
piglit_init(int argc, char **argv)
{
	bool p;

	piglit_require_extension("GL_EXT_texture_swizzle");

	p = test_api();
	piglit_report_result(p ? PIGLIT_PASS : PIGLIT_FAIL);
}
