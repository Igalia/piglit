/*
 * Copyright 2012 VMware, Inc.
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
 * Test glTexImage with an image too large for the given mipmap level.
 *
 * Page 157 of the OpenGL 2.1 spec says:
 *
 * "In a similar fashion, the maximum allowable width of a one- or
 * two- dimensional texture image, and the maximum allowable height of a
 * two- dimensional texture image, must be at least 2k−lod + 2bt for
 * image arrays of level 0 through k, where k is the log base 2 of MAX
 * TEXTURE SIZE. The maximum allowable width and height of a cube map
 * texture must be the same, and must be at least 2k−lod + 2bt for image
 * arrays level 0 through k, where k is the log base 2 of MAX CUBE MAP
 * TEXTURE SIZE."
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END
enum piglit_result
piglit_display(void)
{
	/* no op */
	return PIGLIT_PASS;
}


void
piglit_init(int argc, char **argv)
{
	GLint maxSize;
	GLuint tex;
	bool pass;

	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxSize);
	printf("GL_MAX_TEXTURE_SIZE = %d\n", maxSize);

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);

        /*
         * Note: we don't try to create any maxSize by maxSize textures
         * since we may not have enough texture memory.
         */

	/*
         * For level 0, maxSize by 1 (and vice-versa) should be OK.
         */
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
		     maxSize, 1, 0,
		     GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	pass = piglit_check_gl_error(GL_NO_ERROR);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
		     1, maxSize, 0,
		     GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

        /*
         * For level 1, maxSize by 1 (and vice versa) should fail.
	 */
	glTexImage2D(GL_TEXTURE_2D, 1, GL_RGBA,
		     maxSize, 1, 0,
		     GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	pass = piglit_check_gl_error(GL_INVALID_VALUE) & pass;

	glTexImage2D(GL_TEXTURE_2D, 1, GL_RGBA,
		     1, maxSize, 0,
		     GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	pass = piglit_check_gl_error(GL_INVALID_VALUE) & pass;

        /*
         * For level 2, maxSize/2 by 1 (and vice versa) should fail.
         */
	glTexImage2D(GL_TEXTURE_2D, 2, GL_RGBA,
		     maxSize/2, 1, 0,
		     GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	pass = piglit_check_gl_error(GL_INVALID_VALUE) & pass;

	glTexImage2D(GL_TEXTURE_2D, 2, GL_RGBA,
		     1, maxSize/2, 0,
		     GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	pass = piglit_check_gl_error(GL_INVALID_VALUE) & pass;

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}
