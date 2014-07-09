/*
 * Copyright © 2012 Intel Corporation
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

/** @file get-renderbuffer-internalformat.c
 *
 * Test that
 * glGetRenderbufferParameteriv(GL_RENDERBUFFER_INTERNAL_FORMAT)
 * returns the original internalFormat.
 */

#include <stdio.h>
#include "piglit-util-gl.h"
#include "../../fbo/fbo-formats.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGB;

PIGLIT_GL_TEST_CONFIG_END

enum piglit_result
piglit_display()
{
	/* Unreached. */
	return PIGLIT_FAIL;
}

static enum piglit_result
test_format(const struct format_desc *format)
{
	GLuint rb;
	GLint internalformat;

	/* These texture image formats are not color-renderable
	 * internalformats for renderbuffers.
	 */
	if (format->internalformat >= 1 &&
	    format->internalformat <= 4)
		return PIGLIT_SKIP;

	printf("Testing %s: ", format->name);
	glGenRenderbuffers(1, &rb);
	glBindRenderbuffer(GL_RENDERBUFFER, rb);
	glRenderbufferStorage(GL_RENDERBUFFER, format->internalformat, 1, 1);
	if (glGetError() == GL_INVALID_ENUM) {
		printf("SKIP\n");
		return PIGLIT_SKIP;
	}

	glGetRenderbufferParameteriv(GL_RENDERBUFFER,
				     GL_RENDERBUFFER_INTERNAL_FORMAT,
				     &internalformat);

	if (internalformat == format->internalformat) {
		printf("OK\n");
		return PIGLIT_PASS;
	} else {
		printf("FAIL (%s instead of %s)\n",
		       piglit_get_gl_enum_name(internalformat),
		       piglit_get_gl_enum_name(format->internalformat));
		return PIGLIT_FAIL;
	}
}

void piglit_init(int argc, char **argv)
{
	piglit_automatic = true;
	fbo_formats_init(argc, argv, GL_TRUE);
	piglit_report_result(fbo_formats_display(test_format));
}
