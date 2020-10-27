/*
 * Copyright © 2020 Intel Corporation
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
 */

/**
 * \file errors.c
 * Check for errors required by the spec.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 12;
	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

void
piglit_init(int argc, char **argv)
{
	bool pass = true;

	piglit_require_extension("GL_NV_primitive_restart");

	/* The GL_NV_primitive_restart spec says:
	 *
	 *    The error INVALID_OPERATION is generated if PrimitiveRestartNV
	 *    is called outside the execution of Begin and the corresponding
	 *    execution of End.
	 */
	printf("Trying glPrimitiveRestartNV outside glBegin/glEnd...\n");
	glPrimitiveRestartNV();
	pass = piglit_check_gl_error(GL_INVALID_OPERATION) && pass;
	printf("\n");

	/* Ensure that the error state is cleared before the next test. */
	piglit_reset_gl_error();

	/* The GL_NV_primitive_restart spec says:
	 *
	 *    The error INVALID_OPERATION is generated if
	 *    PrimitiveRestartIndexNV is called between the execution of Begin
	 *    and the corresponding execution of End.
	 */
	printf("Trying glPrimitiveRestartIndexNV inside glBegin/glEnd...\n");
	glBegin(GL_TRIANGLE_STRIP);
	glPrimitiveRestartIndexNV(0);
	glEnd();

	/* Note: it is illegal to call glGetError between glBegin and glEnd. */
	pass = piglit_check_gl_error(GL_INVALID_OPERATION) && pass;
	printf("\n");

	/* Ensure that the error state is cleared before the next test. */
	piglit_reset_gl_error();

	/* Similar to the previous test, but try while compiling a display
	 * list.  The GL_NV_primitive_restart spec says:
	 *
	 *    PrimitiveRestartIndexNV is not compiled into display lists, but
	 *    is executed immediately.
	 */
	printf("Trying glPrimitiveRestartIndexNV inside glBegin/glEnd during "
	       "display list compilation...\n");
	glNewList(1, GL_COMPILE);
	glBegin(GL_TRIANGLE_STRIP);

	/* No error should be generated.  The glBegin is not "executed," so it
	 * does not affect the glPrimitiveRestartIndexNV call or the
	 * glGetError call.
	 */
	glPrimitiveRestartIndexNV(0);
	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

	glEnd();
	glEndList();

	glCallList(1);
	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;
	printf("\n");

	/* Ensure that the error state is cleared before the next test. */
	piglit_reset_gl_error();

	if (piglit_get_gl_version() >= 31) {
		/* Section 10.7.5 ("Commands Allowed Between Begin and End")
		 * of the OpenGL 4.6 Compatibility Profile spec says:
		 *
		 *    The only GL commands that are allowed within any Begin /
		 *    End pairs are [long list of things that does not include
		 *    PrimitiveRestartIndex].
		 */
		printf("Trying glPrimitiveRestartIndex inside glBegin/glEnd...\n");
		glBegin(GL_TRIANGLE_STRIP);
		glPrimitiveRestartIndex(0);
		glEnd();

		/* Note: it is illegal to call glGetError between glBegin and
		 * glEnd.
		 */
		pass = piglit_check_gl_error(GL_INVALID_OPERATION) && pass;
		printf("\n");

		/* Ensure that the error state is cleared before the next
		 * test.
		 */
		piglit_reset_gl_error();

		/* Similar to the previous test, but try while compiling a
		 * display list.  Section 21.4.1 ("Commands Not Usable In
		 * Display Lists") of the OpenGL 4.6 Compatibility Profile
		 * spec says:
		 *
		 *    Vertex arrays: ..., PrimitiveRestartIndex
		 */
		printf("Trying glPrimitiveRestartIndex inside glBegin/glEnd "
		       "during display list compilation...\n");
		glNewList(1, GL_COMPILE);
		glBegin(GL_TRIANGLE_STRIP);

		/* No error should be generated.  The glBegin is not "executed,"
		 * so it does not affect the glPrimitiveRestartIndex call or
		 * the glGetError call.
		 */
		glPrimitiveRestartIndex(0);
		pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

		glEnd();
		glEndList();

		glCallList(1);
		pass = piglit_check_gl_error(GL_NO_ERROR) && pass;
		printf("\n");

		/* Ensure that the error state is cleared before the next
		 * test.
		 */
		piglit_reset_gl_error();
	}

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}

enum piglit_result
piglit_display()
{
	/* UNREACHABLE */
	return PIGLIT_FAIL;
}
