/* Copyright © 2011 Intel Corporation
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
 * \file bindfragdata-invalid-parameters.c
 * Verify that passing invalid parameters to glBindFragDataLocation generates
 * the correct errors.
 *
 * \author Ian Romanick
 */
#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;
	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

enum piglit_result
piglit_display(void)
{
	return PIGLIT_FAIL;
}

void piglit_init(int argc, char **argv)
{
	GLint max_draw_buffers;
	GLuint prog;

	piglit_require_gl_version(30);

	glGetIntegerv(GL_MAX_DRAW_BUFFERS, &max_draw_buffers);

	/* Page 237 (page 253 of the PDF) of the OpenGL 3.0 spec says:
	 *
	 *     "BindFragDataLocation may be issued before any shader objects
	 *     are attached to a program object."
	 *
	 * As a result, all of the invalid location tests can be performed
	 * without a shader at all.  Only a program object is necessary.
	 */
	prog = glCreateProgram();
	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	/* Page 236 (page 252 of the PDF) of the OpenGL 3.0 spec says:
	 *
	 *     "The error INVALID VALUE is generated if colorNumber is equal
	 *     or greater than MAX DRAW BUFFERS."
	 *
	 * Since the colorNumber parameter is unsigned, this statement means
	 * an error should be generated if a negative number is used.
	 */
	printf("Trying location = -1...\n");
	glBindFragDataLocation(prog, -1, "foo");
	if (!piglit_check_gl_error(GL_INVALID_VALUE))
		piglit_report_result(PIGLIT_FAIL);

	printf("Trying location = GL_MAX_DRAW_BUFFERS...\n");
	glBindFragDataLocation(prog, max_draw_buffers, "foo");
	if (!piglit_check_gl_error(GL_INVALID_VALUE))
		piglit_report_result(PIGLIT_FAIL);

	/* Page 236 (page 252 of the PDF) of the OpenGL 3.0 spec says:
	 *
	 *     "The error INVALID_OPERATION is generated if name starts with
	 *     the reserved gl prefix."
	 *
	 * This was changed in a later version of the spec.  Page 279 (page
	 * 296 of the PDF) of the OpenGL 4.2 Core spec says:
	 *
	 *     "The error INVALID_OPERATION is generated if name starts with
	 *     the reserved gl_ prefix."
	 *
	 * The OpenGL 4.2 spec also matches the specified behavior of
	 * glBindAttribLocation as far back as OpenGL 2.0.
	 */
	printf("Trying name = `gl_FragColor'...\n");
	glBindFragDataLocation(prog, 0, "gl_FragColor");
	if (!piglit_check_gl_error(GL_INVALID_OPERATION))
		piglit_report_result(PIGLIT_FAIL);

	printf("Trying name = `gl_FragDepth'...\n");
	glBindFragDataLocation(prog, 0, "gl_FragDepth");
	if (!piglit_check_gl_error(GL_INVALID_OPERATION))
		piglit_report_result(PIGLIT_FAIL);

	printf("Trying name = `gl_'...\n");
	glBindFragDataLocation(prog, 0, "gl_");
	if (!piglit_check_gl_error(GL_INVALID_OPERATION))
		piglit_report_result(PIGLIT_FAIL);

	piglit_report_result(PIGLIT_PASS);
}
