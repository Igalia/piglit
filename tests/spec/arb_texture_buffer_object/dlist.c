/*
 * Copyright © 2011 Intel Corporation
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

#include "piglit-util-gl.h"

/**
 * @file dlist.c
 *
 * Tests display list behavior for GL_ARB_texture_buffer_object.
 *
 * The new entrypoint was added to the list of buffer-object related
 * commands that are executed immediately and not added to the display
 * list.
 */

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;

PIGLIT_GL_TEST_CONFIG_END

enum piglit_result
piglit_display(void)
{
	/* UNREACHED */
	return PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	bool pass = true;
	GLuint list, tex, bo;
	GLint ret;

	piglit_require_gl_version(20);
	piglit_require_extension("GL_ARB_texture_buffer_object");

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_BUFFER, tex);

	glGenBuffers(1, &bo);
	glBindBuffer(GL_TEXTURE_BUFFER, bo);

	list = glGenLists(1);
	glNewList(list, GL_COMPILE);
	glTexBufferARB(GL_TEXTURE_BUFFER, GL_RGBA8, bo);
	glEndList();

	ret = 0xd0d0d0d0;
	glGetIntegerv(GL_TEXTURE_BUFFER_DATA_STORE_BINDING, &ret);
	if (ret != bo) {
		fprintf(stderr,
			"GL_TEXTURE_BUFFER_DATA_STORE after display list "
			"compile was %d, expected %d\n", ret, bo);
		pass = false;
	}

	/* Make sure the list is empty. */
	glTexBufferARB(GL_TEXTURE_BUFFER, GL_RGBA8, 0);
	glCallList(list);
	if (!piglit_check_gl_error(0))
		pass = false;

	glGetIntegerv(GL_TEXTURE_BUFFER_DATA_STORE_BINDING, &ret);
	if (ret != 0) {
		fprintf(stderr,
			"GL_TEXTURE_BUFFER_DATA_STORE after display list "
			"compile was %d, expected %d\n", ret, 0);
		pass = false;
	}

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}
