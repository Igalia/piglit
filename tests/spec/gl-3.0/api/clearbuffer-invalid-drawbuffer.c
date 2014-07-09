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

/**
 * \file clearbuffer-invalid-drawbuffer.c
 * Probe various invalid drawbuffer settings for glClearBuffer
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
	static const float zero_f[4] = {
		0.0f, 0.0f, 0.0f, 0.0f
	};

	static const int zero_i[4] = {
		0, 0, 0, 0
	};

	GLint max_draw_buffers;
	unsigned i;

	piglit_require_gl_version(30);

	glGetIntegerv(GL_MAX_DRAW_BUFFERS, &max_draw_buffers);

	/* Page 264 (page 280 of the PDF) of the OpenGL 3.0 spec says:
	 *
	 *     "ClearBuffer generates an INVALID VALUE error if buffer is
	 *     COLOR and drawbuffer is less than zero, or greater than the
	 *     value of MAX DRAW BUFFERS minus one; or if buffer is DEPTH,
	 *     STENCIL, or DEPTH STENCIL and drawbuffer is not zero."
	 */
	if (!piglit_automatic)
		printf("Trying invalid drawbuffer with GL_DEPTH...\n");

	glClearBufferfv(GL_DEPTH, 1, zero_f);
	if (!piglit_check_gl_error(GL_INVALID_VALUE))
		piglit_report_result(PIGLIT_FAIL);

	glClearBufferfv(GL_DEPTH, -1, zero_f);
	if (!piglit_check_gl_error(GL_INVALID_VALUE))
		piglit_report_result(PIGLIT_FAIL);

	glClearBufferiv(GL_DEPTH, 1, zero_i);
	if (!piglit_check_gl_error(GL_INVALID_VALUE))
		piglit_report_result(PIGLIT_FAIL);

	glClearBufferiv(GL_DEPTH, -1, zero_i);
	if (!piglit_check_gl_error(GL_INVALID_VALUE))
		piglit_report_result(PIGLIT_FAIL);

	glClearBufferuiv(GL_DEPTH, 1, (GLuint *) zero_i);
	if (!piglit_check_gl_error(GL_INVALID_VALUE))
		piglit_report_result(PIGLIT_FAIL);

	glClearBufferuiv(GL_DEPTH, -1, (GLuint *) zero_i);
	if (!piglit_check_gl_error(GL_INVALID_VALUE))
		piglit_report_result(PIGLIT_FAIL);

	if (!piglit_automatic)
		printf("Trying invalid drawbuffer with GL_STENCIL...\n");

	glClearBufferfv(GL_STENCIL, 1, zero_f);
	if (!piglit_check_gl_error(GL_INVALID_VALUE))
		piglit_report_result(PIGLIT_FAIL);

	glClearBufferfv(GL_STENCIL, -1, zero_f);
	if (!piglit_check_gl_error(GL_INVALID_VALUE))
		piglit_report_result(PIGLIT_FAIL);

	glClearBufferiv(GL_STENCIL, 1, zero_i);
	if (!piglit_check_gl_error(GL_INVALID_VALUE))
		piglit_report_result(PIGLIT_FAIL);

	glClearBufferiv(GL_STENCIL, -1, zero_i);
	if (!piglit_check_gl_error(GL_INVALID_VALUE))
		piglit_report_result(PIGLIT_FAIL);

	glClearBufferuiv(GL_STENCIL, 1, (GLuint *) zero_i);
	if (!piglit_check_gl_error(GL_INVALID_VALUE))
		piglit_report_result(PIGLIT_FAIL);

	glClearBufferuiv(GL_STENCIL, -1, (GLuint *) zero_i);
	if (!piglit_check_gl_error(GL_INVALID_VALUE))
		piglit_report_result(PIGLIT_FAIL);

	/* Page 263 (page 279 of the PDF) of the OpenGL 3.0 spec says:
	 *
	 *     "The command
	 *
	 *          void ClearBufferfi( enum buffer, int drawbuffer,
	 *                              float depth, int stencil );
	 *
	 *     clears both depth and stencil buffers of the currently bound
	 *     draw framebuffer.  buffer must be DEPTH STENCIL and drawbuffer
	 *     must be zero."
	 */
	if (!piglit_automatic)
		printf("Trying invalid drawbuffer with GL_DEPTH_STENCIL...\n");

	glClearBufferfi(GL_DEPTH_STENCIL, 1, 0.0f, 0);
	if (!piglit_check_gl_error(GL_INVALID_VALUE))
		piglit_report_result(PIGLIT_FAIL);

	glClearBufferfi(GL_DEPTH_STENCIL, -1, 0.0f, 0);
	if (!piglit_check_gl_error(GL_INVALID_VALUE))
		piglit_report_result(PIGLIT_FAIL);

	if (!piglit_automatic)
		printf("Trying invalid drawbuffer with GL_COLOR...\n");

	glClearBufferfv(GL_COLOR, -1, zero_f);
	if (!piglit_check_gl_error(GL_INVALID_VALUE))
		piglit_report_result(PIGLIT_FAIL);

	glClearBufferfv(GL_COLOR, max_draw_buffers, zero_f);
	if (!piglit_check_gl_error(GL_INVALID_VALUE))
		piglit_report_result(PIGLIT_FAIL);

	glClearBufferiv(GL_COLOR, -1, zero_i);
	if (!piglit_check_gl_error(GL_INVALID_VALUE))
		piglit_report_result(PIGLIT_FAIL);

	glClearBufferiv(GL_COLOR, max_draw_buffers, zero_i);
	if (!piglit_check_gl_error(GL_INVALID_VALUE))
		piglit_report_result(PIGLIT_FAIL);

	glClearBufferuiv(GL_COLOR, -1, (GLuint *) zero_i);
	if (!piglit_check_gl_error(GL_INVALID_VALUE))
		piglit_report_result(PIGLIT_FAIL);

	glClearBufferuiv(GL_COLOR, max_draw_buffers, (GLuint *) zero_i);
	if (!piglit_check_gl_error(GL_INVALID_VALUE))
		piglit_report_result(PIGLIT_FAIL);

	for (i = 0; i < max_draw_buffers; i++) {
		/* It seems reasonable that someone might accidentally use
		 * GL_DRAW_BUFFERi instead of just i.  Make sure that also
		 * generates the expected error.
		 */
		glClearBufferfv(GL_COLOR, GL_DRAW_BUFFER0 + i, zero_f);
		if (!piglit_check_gl_error(GL_INVALID_VALUE))
			piglit_report_result(PIGLIT_FAIL);

		glClearBufferiv(GL_COLOR, GL_DRAW_BUFFER0 + i, zero_i);
		if (!piglit_check_gl_error(GL_INVALID_VALUE))
			piglit_report_result(PIGLIT_FAIL);

		glClearBufferuiv(GL_COLOR, GL_DRAW_BUFFER0 + i,
				 (GLuint *) zero_i);
		if (!piglit_check_gl_error(GL_INVALID_VALUE))
			piglit_report_result(PIGLIT_FAIL);
	}

	/* Now try the valid cases and assert no error.
	 */
	if (!piglit_automatic)
		printf("Trying valid drawbuffer with everything...\n");

	for (i = 0; i < max_draw_buffers; i++) {
		glClearBufferfv(GL_COLOR, i, zero_f);
		if (!piglit_check_gl_error(GL_NO_ERROR))
			piglit_report_result(PIGLIT_FAIL);

		glClearBufferiv(GL_COLOR, i, zero_i);
		if (!piglit_check_gl_error(GL_NO_ERROR))
			piglit_report_result(PIGLIT_FAIL);

		glClearBufferuiv(GL_COLOR, i, (GLuint *) zero_i);
		if (!piglit_check_gl_error(GL_NO_ERROR))
			piglit_report_result(PIGLIT_FAIL);
	}

	glClearBufferfv(GL_DEPTH, 0, zero_f);
	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	glClearBufferiv(GL_DEPTH, 0, zero_i);
	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	glClearBufferuiv(GL_DEPTH, 0, (GLuint *) zero_i);
	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	glClearBufferfv(GL_STENCIL, 0, zero_f);
	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	glClearBufferiv(GL_STENCIL, 0, zero_i);
	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	glClearBufferuiv(GL_STENCIL, 0, (GLuint *) zero_i);
	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	glClearBufferfi(GL_DEPTH_STENCIL, 0, 0.0f, 0);
	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	piglit_report_result(PIGLIT_PASS);
}
