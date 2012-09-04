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

#include "piglit-util-gl-common.h"

PIGLIT_GL_TEST_MAIN(100, 100, GLUT_RGBA | GLUT_DOUBLE)

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

	/* The max texture size should be OK for mipmap level zero. */
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
		     maxSize, maxSize, 0,
		     GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	pass = piglit_check_gl_error(GL_NO_ERROR);

	/* Setting the level 1 image to the max texture size should be
	 * an error.
	 */
	glTexImage2D(GL_TEXTURE_2D, 1, GL_RGBA,
		     maxSize, maxSize, 0,
		     GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	pass = piglit_check_gl_error(GL_INVALID_VALUE) & pass;

	/* Setting the level 2 image to half the max texture size should be
	 * an error also.
	 */
	glTexImage2D(GL_TEXTURE_2D, 2, GL_RGBA,
		     maxSize/2, maxSize/2, 0,
		     GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	pass = piglit_check_gl_error(GL_INVALID_VALUE) & pass;

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}
