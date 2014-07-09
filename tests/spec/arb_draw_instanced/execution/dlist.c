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
 * Tests display list behavior for GL_ARB_draw_instanced.
 *
 * From the spec:
 *
 *     "The error INVALID_OPERATION is generated if
 *      DrawArraysInstancedARB or DrawElementsInstancedARB is called
 *      during display list compilation."
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
	GLuint list;

	piglit_require_extension("GL_ARB_draw_instanced");

	list = glGenLists(1);
	glNewList(list, GL_COMPILE);

	glDrawArraysInstancedARB(GL_TRIANGLES, 0, 2, 3);
	if (!piglit_check_gl_error(GL_INVALID_OPERATION))
		pass = false;

	glDrawElementsInstancedARB(GL_TRIANGLES, 2, GL_UNSIGNED_INT, NULL, 3);
	if (!piglit_check_gl_error(GL_INVALID_OPERATION))
		pass = false;

	glEndList();

	/* Make sure the list is empty. */
	glCallList(list);
	if (!piglit_check_gl_error(0))
		pass = false;

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}
