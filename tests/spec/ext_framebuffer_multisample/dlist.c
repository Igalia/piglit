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
 * Tests that glRenderbufferStorageMultisampleEXT is executed
immediately instead of being compiled into display lists.
 *
 * From the EXT_framebuffer_multisample spec:
 *
 *     "Certain commands, when called while compiling a display list,
 *      are not compiled into the display list but are executed
 *      immediately.  These are: ...,
 *      RenderbufferStorageMultisampleEXT..."
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
	GLint max_samples, rb_samples;
	GLuint rb, list;
	GLint width;
	bool pass = true;

	piglit_require_extension("GL_EXT_framebuffer_multisample");

	glGetIntegerv(GL_MAX_SAMPLES, &max_samples);
	if (!piglit_check_gl_error(0))
		piglit_report_result(PIGLIT_FAIL);

	glGenRenderbuffersEXT(1, &rb);
	glBindRenderbufferEXT(GL_RENDERBUFFER, rb);

	/* Make the list.  The Storage should be called during compile. */
	list = glGenLists(1);
	glNewList(list, GL_COMPILE);
	glRenderbufferStorageMultisampleEXT(GL_RENDERBUFFER,
					    max_samples,
					    GL_RGBA,
					    1, 1);
	glEndList();

	/* Make sure that the command occurred during the compile. */
	glGetRenderbufferParameterivEXT(GL_RENDERBUFFER,
					GL_RENDERBUFFER_SAMPLES,
					&rb_samples);
	if (rb_samples != max_samples) {
		fprintf(stderr, "glRenderbufferStorageMultisampleEXT not called during "
			"display list compile\n");
		pass = false;
	}


	/* Now, make sure that it doesn't occur at execute.  Start
	 * with storage of a different size so we can distinguish.
	 */
	glRenderbufferStorageMultisampleEXT(GL_RENDERBUFFER,
					    max_samples,
					    GL_RGBA,
					    2, 2);

	glCallList(list);
	glGetRenderbufferParameterivEXT(GL_RENDERBUFFER,
					GL_RENDERBUFFER_WIDTH,
					&width);

	if (width != 2) {
		fprintf(stderr, "glRenderbufferStorageMultisampleEXT called "
			"during display list execute");
		pass = false;
	}

	glDeleteRenderbuffersEXT(1, &rb);

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}
