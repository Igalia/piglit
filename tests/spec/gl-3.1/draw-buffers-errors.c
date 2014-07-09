/**
 * Copyright © 2013 Intel Corporation
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
 * Test that DrawBuffers() returns correct error message for different values
 *
 * Section 4.2.1 (Selecting a Buffer for Writing) of OpenGL 3.1 spec says:
 *
 * "For both the default framebuffer and framebuffer objects, the constants
 *  FRONT, BACK, LEFT, RIGHT, and FRONT_AND_BACK are not valid in the bufs
 *  array passed to DrawBuffers, and will result in the error INVALID_ENUM."
 *
 * "If the GL is bound to the default framebuffer and DrawBuffers is supplied
 *  with a constant (other than NONE) that does not indicate any of the color
 *  buffers allocated to the GL context by the window system, the error
 *  INVALID_OPERATION will be generated."
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 31;
	config.supports_gl_core_version = 31;

	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END


static const GLenum invalids[] = {
	GL_FRONT,
	GL_BACK,
	GL_LEFT,
	GL_RIGHT,
	GL_FRONT_AND_BACK
};

static const GLenum valids[] = {
	GL_NONE,
	GL_FRONT_LEFT,
	GL_FRONT_RIGHT,
	GL_BACK_LEFT,
	GL_BACK_RIGHT
};

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
	unsigned i;

	for (i = 0; i < ARRAY_SIZE(valids); i++) {
		GLenum err = 0;
		glDrawBuffers(1, &valids[i]);
		err = glGetError();
		/* err = INVALID_OPERATION if that color buffer is not
		 * allocated to the window system
		 */
		if (err != GL_NONE && err != GL_INVALID_OPERATION) {
			printf("Expected GL_NONE or GL_INVALID_OPERATION with"
				" %s but received: %s\n",
				piglit_get_gl_enum_name(valids[i]),
				piglit_get_gl_enum_name(err));
			pass = false;
		}
	}

	for (i = 0; i < ARRAY_SIZE(invalids); i++) {
		GLenum err = 0;
		glDrawBuffers(1, &invalids[i]);
		err = glGetError();
		if (err != GL_INVALID_ENUM) {
			printf("Expected GL_INVALID_ENUM with %s but "
				"received: %s\n",
				piglit_get_gl_enum_name(invalids[i]),
				piglit_get_gl_enum_name(err));
			pass = false;
		}
	}

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}
