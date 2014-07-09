/*
 * Copyright © 2010 Intel Corporation
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
 *
 * Authors:
 *    Shuang He <shuang.he@intel.com>
 */

/**
 * \file object_purgeable-api-pbo.c
 * Simple test of the API for GL_APPLE_object_purgeable with GL_ARB_pixel_buffer_object.
 */

#include "piglit-util-gl.h"
#include "object_purgeable.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_RGB;

PIGLIT_GL_TEST_CONFIG_END

void
piglit_init(int argc, char **argv)
{
	(void) argc;
	(void) argv;

	init_ObjectPurgeableAPI();
	piglit_automatic = GL_TRUE;

	piglit_require_extension("GL_ARB_pixel_buffer_object");
}


enum piglit_result
piglit_display(void)
{
	GLuint pbo;
	GLboolean pass;

	glGenBuffersARB(1, &pbo);
	glBindBufferARB(GL_PIXEL_PACK_BUFFER_ARB, pbo);
	glBufferDataARB(GL_PIXEL_PACK_BUFFER_ARB, 100*100, NULL,
			GL_STATIC_DRAW_ARB);
	glBindBufferARB(GL_PIXEL_PACK_BUFFER_ARB, 0);

	pass = test_Purgeable(pbo, GL_BUFFER_OBJECT_APPLE);

	glDeleteBuffersARB(1, &pbo);
	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}
