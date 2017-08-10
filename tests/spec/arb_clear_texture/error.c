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

/** @file error.c
 *
 * Tests the various error conditions that glClearTexSubImage should
 * signal.
 */

#include "common.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 14;

	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

	config.khr_no_error_support = PIGLIT_HAS_ERRORS;

PIGLIT_GL_TEST_CONFIG_END

static bool
test_sub_clear(void)
{
	GLuint tex;
	bool pass = true;

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexImage2D(GL_TEXTURE_2D,
		     0, /* level */
		     GL_RGBA,
		     4, 4, /* width/height */
		     0, /* border */
		     GL_RGBA,
		     GL_UNSIGNED_BYTE,
		     NULL);
	glTexImage2D(GL_TEXTURE_2D,
		     1, /* level */
		     GL_RGBA,
		     2, 2, /* width/height */
		     0, /* border */
		     GL_RGBA,
		     GL_UNSIGNED_BYTE,
		     NULL);
	glTexImage2D(GL_TEXTURE_2D,
		     2, /* level */
		     GL_RGBA,
		     1, 1, /* width/height */
		     0, /* border */
		     GL_RGBA,
		     GL_UNSIGNED_BYTE,
		     NULL);
	glBindTexture(GL_TEXTURE_2D, 0);

	/* test invalid x */
	glClearTexSubImage(tex,
			   0, /* level */
			   -1, 0, 0, /* x/y/z */
			   1, 1, 1, /* width/height/depth */
			   GL_RGBA, GL_UNSIGNED_BYTE,
			   NULL /* data */);
	pass &= piglit_check_gl_error(GL_INVALID_OPERATION);

	/* test invalid y */
	glClearTexSubImage(tex,
			   0, /* level */
			   0, -1, 0, /* x/y/z */
			   1, 1, 1, /* width/height/depth */
			   GL_RGBA, GL_UNSIGNED_BYTE,
			   NULL /* data */);
	pass &= piglit_check_gl_error(GL_INVALID_OPERATION);

	/* test invalid z */
	glClearTexSubImage(tex,
			   0, /* level */
			   0, 0, -1, /* x/y/z */
			   1, 1, 1, /* width/height/depth */
			   GL_RGBA, GL_UNSIGNED_BYTE,
			   NULL /* data */);
	pass &= piglit_check_gl_error(GL_INVALID_OPERATION);

	/* test invalid width */
	glClearTexSubImage(tex,
			   0, /* level */
			   1, 1, 0, /* x/y/z */
			   4, 1, 1, /* width/height/depth */
			   GL_RGBA, GL_UNSIGNED_BYTE,
			   NULL /* data */);
	pass &= piglit_check_gl_error(GL_INVALID_OPERATION);

	/* Test invalid height */
	glClearTexSubImage(tex,
			   0, /* level */
			   1, 1, 0, /* x/y/z */
			   1, 4, 1, /* width/height/depth */
			   GL_RGBA, GL_UNSIGNED_BYTE,
			   NULL /* data */);
	pass &= piglit_check_gl_error(GL_INVALID_OPERATION);

	/* Test invalid depth */
	glClearTexSubImage(tex,
			   0, /* level */
			   1, 1, 0, /* x/y/z */
			   1, 1, 2, /* width/height/depth */
			   GL_RGBA, GL_UNSIGNED_BYTE,
			   NULL /* data */);
	pass &= piglit_check_gl_error(GL_INVALID_OPERATION);

	/* test clearing invalid region of level 1 */
	glClearTexSubImage(tex,
			   1, /* level */
			   1, 1, 0, /* x/y/z */
			   2, 3, 1, /* width/height/depth */
			   GL_RGBA, GL_UNSIGNED_BYTE,
			   NULL /* data */);
	pass &= piglit_check_gl_error(GL_INVALID_OPERATION);

	/* Test no error */
	glClearTexSubImage(tex,
			   0, /* level */
			   1, 1, 0, /* x/y/z */
			   2, 3, 1, /* width/height/depth */
			   GL_RGBA, GL_UNSIGNED_BYTE,
			   NULL /* data */);
	pass &= piglit_check_gl_error(GL_NO_ERROR);

	glDeleteTextures(1, &tex);

	return pass;
}


static bool
test_texture_view(void)
{
	GLuint tex, view;
	bool pass = true;

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);

	glTexStorage2D(GL_TEXTURE_2D, 3, GL_RGBA8, 4, 4);

	/* create view of levels 1..2 as 0..1 */
	glGenTextures(1, &view);
	glTextureView(view, GL_TEXTURE_2D, tex,
		      GL_RGBA8, 1, 2, 0, 1);

	glBindTexture(GL_TEXTURE_2D, view);

	pass &= piglit_check_gl_error(GL_NO_ERROR);

	/* try to clear invalid level */
	glClearTexSubImage(view,
			   2, /* level */
			   0, 0, 0, /* x/y/z */
			   1, 1, 1, /* width/height/depth */
			   GL_RGBA, GL_UNSIGNED_BYTE,
			   NULL /* data */);
	pass &= piglit_check_gl_error(GL_INVALID_OPERATION);

	/* clearing all of level 0 should work */
	glClearTexSubImage(view,
			   0, /* level */
			   0, 0, 0, /* x/y/z */
			   2, 2, 1, /* width/height/depth */
			   GL_RGBA, GL_UNSIGNED_BYTE,
			   NULL /* data */);
	pass &= piglit_check_gl_error(GL_NO_ERROR);

	/* try to clear invalid region of level 0 */
	glClearTexSubImage(view,
			   0, /* level */
			   0, 0, 0, /* x/y/z */
			   4, 4, 1, /* width/height/depth */
			   GL_RGBA, GL_UNSIGNED_BYTE,
			   NULL /* data */);
	pass &= piglit_check_gl_error(GL_INVALID_OPERATION);

	return pass;
}


void
piglit_init(int argc, char **argv)
{
	GLuint tex;
	bool pass = true;

	/* glClearTexture is either in the GL_ARB_clear_texture
	 * extension or in core in GL 4.4
	 */
	if (piglit_get_gl_version() < 44 &&
	    !piglit_is_extension_supported("GL_ARB_clear_texture")) {
		printf("OpenGL 4.4 or GL_ARB_clear_texture is required.\n");
		piglit_report_result(PIGLIT_SKIP);
	}

	/* Create a texture using the zero texture */
	glTexImage2D(GL_TEXTURE_2D,
		     0, /* level */
		     GL_RGBA,
		     1, 1, /* width/height */
		     0, /* border */
		     GL_RGBA,
		     GL_UNSIGNED_BYTE,
		     NULL);

	/* Using the zero texture should result in an error even if it
	 * is a valid texture */
	glClearTexImage(0, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	pass &= piglit_check_gl_error(GL_INVALID_OPERATION);

	/* We shouldn't be able to use a texture number that doesn't
	 * exist yet */
	glClearTexImage(100, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	pass &= piglit_check_gl_error(GL_INVALID_OPERATION);

	glGenTextures(1, &tex);

	/* We shouldn't be able to use a texture that doesn't have any
	 * data yet */
	glClearTexImage(tex, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	pass &= piglit_check_gl_error(GL_INVALID_OPERATION);

	/* Set level 1 */
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexImage2D(GL_TEXTURE_2D,
		     1, /* level */
		     GL_RGBA,
		     1, 1, /* width/height */
		     0, /* border */
		     GL_RGBA,
		     GL_UNSIGNED_BYTE,
		     NULL);

	/* We shouldn't be able to clear a level that doesn't have
	 * data yet */
	glClearTexImage(tex, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	pass &= piglit_check_gl_error(GL_INVALID_OPERATION);

	/* But we should be able to clear level 1 */
	glClearTexImage(tex, 1, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	pass &= piglit_check_gl_error(GL_NO_ERROR);

	glBindTexture(GL_TEXTURE_2D, 0);
	glDeleteTextures(1, &tex);

	pass &= test_sub_clear();

	pass &= test_invalid_format(GL_DEPTH_COMPONENT,
				    GL_DEPTH_COMPONENT,
				    GL_UNSIGNED_INT,
				    GL_RGBA,
				    GL_UNSIGNED_BYTE);

	pass &= test_invalid_format(GL_RGBA,
				    GL_RGBA,
				    GL_UNSIGNED_BYTE,
				    GL_DEPTH_COMPONENT,
				    GL_UNSIGNED_INT);

	if (piglit_is_extension_supported("GL_ARB_texture_view")) {
		pass &= test_texture_view();
	}

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}

enum piglit_result
piglit_display(void)
{
	/* unused */
	return PIGLIT_FAIL;
}
