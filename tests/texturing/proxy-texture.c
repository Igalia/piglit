/*
 * Copyright (c) 2012 VMware, Inc.
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
 * NON-INFRINGEMENT.  IN NO EVENT SHALL THE AUTHORS AND/OR THEIR
 * SUPPLIERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

/**
 * Tests proxy texture error handling.
 * \author Brian Paul
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_RGB;

PIGLIT_GL_TEST_CONFIG_END


static void
init_proxy_texture(void)
{
	/* Create good 8x8 proxy texture image */
	glTexImage2D(GL_PROXY_TEXTURE_2D, 0, GL_RGBA, 8, 8, 0,
		     GL_RGBA, GL_FLOAT, NULL);
	piglit_check_gl_error(GL_NO_ERROR);
}


static bool
check_no_proxy_change_(int line)
{
	GLint w;
	glGetTexLevelParameteriv(GL_PROXY_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &w);
	/* The proxy texture image width should be 8 */
	if (w != 8) {
		printf("Proxy texture was mistakenly changed!");
		return false;
	}
	return true;
}

#define check_no_proxy_change()  check_no_proxy_change_(__LINE__)


/**
 * Check that the proxy texture width and height are zero.
 */
static bool
check_proxy_zeroed(void)
{
	GLint w, h;
	glGetTexLevelParameteriv(GL_PROXY_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &w);
	glGetTexLevelParameteriv(GL_PROXY_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &h);
	/* The proxy texture image width should not be zero, not 8 */
	if (w != 0 || h != 0) {
		printf("Proxy texture size wasn't zero-ed out!");
		return false;
	}
	return true;
}


static bool
do_proxy_tests(void)
{
	bool pass = true;
	GLint maxSize;

	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxSize);

	init_proxy_texture();

	/* bad level => GL_INVALID_VALUE */
	glTexImage2D(GL_PROXY_TEXTURE_2D, 5555, GL_RGBA, 8, 8, 0,
		     GL_RGBA, GL_FLOAT, NULL);
	pass = piglit_check_gl_error(GL_INVALID_VALUE) && pass;
	pass = check_no_proxy_change() && pass;

	/* bad width => GL_INVALID_VALUE */
	glTexImage2D(GL_PROXY_TEXTURE_2D, 0, GL_RGBA, -8, 8, 0,
		     GL_RGBA, GL_FLOAT, NULL);
	pass = piglit_check_gl_error(GL_INVALID_VALUE) && pass;
	pass = check_no_proxy_change() && pass;

	/* bad border => GL_INVALID_VALUE */
	glTexImage2D(GL_PROXY_TEXTURE_2D, 0, GL_RGBA, 8, 8, 2,
		     GL_RGBA, GL_FLOAT, NULL);
	pass = piglit_check_gl_error(GL_INVALID_VALUE) && pass;
	pass = check_no_proxy_change() && pass;

	/* bad format+type => GL_INVALID_OPERATION */
	glTexImage2D(GL_PROXY_TEXTURE_2D, 0, GL_RGBA, 8, 8, 0,
		     GL_DEPTH_COMPONENT, GL_UNSIGNED_INT_8_8_8_8, NULL);
	pass = piglit_check_gl_error(GL_INVALID_OPERATION) && pass;
	pass = check_no_proxy_change() && pass;

	/* Test real proxy behaviour here.  Use too big width, height.
	 * This should not generate a GL error but it should zero-out the
	 * proxy image dims.
	 */
	glTexImage2D(GL_PROXY_TEXTURE_2D, 0, GL_RGBA, maxSize*2, maxSize*2, 0,
		     GL_RGBA, GL_FLOAT, NULL);
	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;
	pass = check_proxy_zeroed() && pass;

	return pass;
}


enum piglit_result
piglit_display(void)
{
	/* nothing */
	return PIGLIT_PASS;
}


void
piglit_init(int argc, char **argv)
{
	if (do_proxy_tests())
		piglit_report_result(PIGLIT_PASS);
	else
		piglit_report_result(PIGLIT_FAIL);
}
