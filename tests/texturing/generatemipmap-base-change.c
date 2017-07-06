/*
 * Copyright 2016 VMware, Inc.
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
 * Test that glGenerateMipmap works properly (doesn't crash) when called a
 * second time on a texture after we change the base image's size or format.
 *
 * The command line takes two parameters:
 *   size   - test base level size change
 *   format - test base level format change
 */


#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN
	config.supports_gl_compat_version = 10;
	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;
	config.khr_no_error_support = PIGLIT_NO_ERRORS;
PIGLIT_GL_TEST_CONFIG_END


void
piglit_init(int argc, char **argv)
{
#define MAX_SIZE 8
	GLubyte img[MAX_SIZE * MAX_SIZE * 4];
	bool change_size = false, change_format = false;
	int i;
	GLuint tex;
	GLsizei base_size, w0 = 0, h0 = 0;
	bool pass = true;

	/* We require GL 3.0 or GL_EXT_framebuffer_object */
	if (piglit_get_gl_version() < 30 &&
		!piglit_is_extension_supported("GL_EXT_framebuffer_object")) {
		piglit_report_result(PIGLIT_SKIP);
	}

	/* parse args */
	for (i = 1; i < argc; i++) {
		if (strcmp(argv[i], "size") == 0) {
			change_size = true;
		} else if (strcmp(argv[i], "format") == 0) {
			change_format = true;
		}
	}

	if (!change_size && !change_format) {
		printf("Missing required argument: 'size' or 'format'\n");
		piglit_report_result(PIGLIT_SKIP);
	}

	/* init image data */
	for (i = 0; i < sizeof(img); i++)
		img[i] = 128;


	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);

	// Create initial texture mipmap (base_size x base_size)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	base_size = change_size ? MAX_SIZE / 2 : MAX_SIZE;
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, base_size, base_size, 0,
		     GL_RGBA, GL_UNSIGNED_BYTE, img);
	glGenerateMipmap(GL_TEXTURE_2D);

	if (change_format) {
		/* Change format of first level */
		glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, base_size, base_size,
			     0, GL_RGBA, GL_UNSIGNED_BYTE, img);
	}
	else if (change_size) {
		/* Change base level to be larger */
		assert(base_size * 2 == MAX_SIZE);
		base_size = MAX_SIZE;
		glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, base_size, base_size,
			     0, GL_RGBA, GL_UNSIGNED_BYTE, img);
	}

	/* See if mipmap generation works (may crash/assert in Mesa) */
	glGenerateMipmap(GL_TEXTURE_2D);

	/* check level sizes */
	for (i = 0; i < 3; i++) {
		GLint w, h;
		glGetTexLevelParameteriv(GL_TEXTURE_2D, i, GL_TEXTURE_WIDTH, &w);
		glGetTexLevelParameteriv(GL_TEXTURE_2D, i, GL_TEXTURE_HEIGHT, &h);
		if (i == 0) {
			w0 = w;
			h0 = h;
		}
		else {
			if (w != w0 >> i || h != h0 >> i) {
				printf("Incorrect mipmap level size: level %d", i);
				printf(" Found %d x %d, expected %d x %d\n", w, h,
				       w0 >> i , h0 >> i);
				pass = false;
			}
		}
	}

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}


enum piglit_result
piglit_display(void)
{
	// no-op
	return PIGLIT_PASS;
}
