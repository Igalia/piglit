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
 * @file get.c
 *
 * Tests glGetIntegerv queries not covered by other tests.
 */

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;
	config.supports_gl_core_version = 31;

	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;

PIGLIT_GL_TEST_CONFIG_END

enum piglit_result
piglit_display(void)
{
	/* UNREACHED */
	return PIGLIT_FAIL;
}

static bool
_expect(int line, GLenum token, GLint val)
{
	GLint ret = 0xd0d0d0d0;

	glGetIntegerv(token, &ret);
	if (ret != val) {
		fprintf(stderr,
			"line %d: %s was %s, expected %s\n",
			line,
			piglit_get_gl_enum_name(token),
			piglit_get_gl_enum_name(ret),
			piglit_get_gl_enum_name(val));
		return false;
	}

	return true;
}
#define expect(token, val) _expect(__LINE__, token, val)

void
piglit_init(int argc, char **argv)
{
	bool pass = true;
	GLuint tex, bo;

	piglit_require_gl_version(20);
	piglit_require_extension("GL_ARB_texture_buffer_object");

	glGenTextures(1, &tex);
	glGenBuffers(1, &bo);

	pass = expect(GL_TEXTURE_BINDING_BUFFER, 0) && pass;
	glBindTexture(GL_TEXTURE_BUFFER, tex);
	pass = expect(GL_TEXTURE_BINDING_BUFFER, tex) && pass;

	pass = expect(GL_TEXTURE_BUFFER, 0) && pass;
	glBindBuffer(GL_TEXTURE_BUFFER, bo);
	pass = expect(GL_TEXTURE_BUFFER, bo) && pass;

	pass = expect(GL_TEXTURE_BUFFER_FORMAT_ARB,
		      piglit_is_core_profile ? GL_R8 : GL_LUMINANCE8) && pass;

	glTexBufferARB(GL_TEXTURE_BUFFER, GL_RGBA8, 0);
	pass = expect(GL_TEXTURE_BUFFER_FORMAT_ARB, GL_RGBA8) && pass;

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}

