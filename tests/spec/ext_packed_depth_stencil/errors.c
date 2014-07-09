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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

/**
 * Test GL_EXT_packed_depth_stencil for API error handling.
 * Based on an original Glean test written by Brian Paul.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN
	config.supports_gl_compat_version = 12;
	config.window_visual = (PIGLIT_GL_VISUAL_RGBA |
				PIGLIT_GL_VISUAL_DEPTH |
				PIGLIT_GL_VISUAL_STENCIL);
PIGLIT_GL_TEST_CONFIG_END


static bool
test_drawpixels(void)
{
	GLuint p[1];

	glDrawPixels(1, 1, GL_DEPTH_STENCIL_EXT, GL_UNSIGNED_INT, p);
	if (!piglit_check_gl_error(GL_INVALID_ENUM))
		return false;

	glDrawPixels(1, 1, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT_24_8_EXT, p);
	if (!piglit_check_gl_error(GL_INVALID_OPERATION))
		return false;

	return true;
}


static bool
test_readpixels(void)
{
	GLuint p[1];

	glReadPixels(0, 0, 1, 1, GL_DEPTH_STENCIL_EXT, GL_FLOAT, p);
	if (!piglit_check_gl_error(GL_INVALID_ENUM))
		return false;

	glReadPixels(0, 0, 1, 1, GL_STENCIL_INDEX, GL_UNSIGNED_INT_24_8_EXT, p);
	if (!piglit_check_gl_error(GL_INVALID_OPERATION))
		return false;

	return true;
}


static bool
test_texture(void)
{
	glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8_EXT,
			 0, 0, 1, 1, 0);
	if (!piglit_check_gl_error(GL_NO_ERROR))
		return false;

	glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, 1, 1);
	if (!piglit_check_gl_error(GL_NO_ERROR))
		return false;

	return true;
}


void
piglit_init(int argc, char **argv)
{
	bool pass = true;

	piglit_require_extension("GL_EXT_packed_depth_stencil");

	pass = test_drawpixels() && pass;
	pass = test_readpixels() && pass;
	pass = test_texture() && pass;

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}


enum piglit_result
piglit_display(void)
{
	/* unused */
	return PIGLIT_FAIL;
}
