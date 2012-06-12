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

/**
 * @file blit-mismatched-formats.cpp
 *
 * This test verifies if glBlitFramebuffer() throws GL_INVALID_OPERATION with
 * non-matching formats for multisample framebuffer objects.
 *
 * We initialize two FBOs with minimum supported sample count and different
 * buffer formats, do blitting operation between them and then query the gl
 * error.
 *
 * Author: Anuj Phogat <anuj.phogat@gmail.com>
 */

#include "common.h"

PIGLIT_GL_TEST_MAIN(256 /*window_width*/,
                    256 /*window_height*/,
                    GLUT_DOUBLE | GLUT_RGBA);

const int pattern_width = 256; const int pattern_height = 256;
Fbo src_fbo, dst_fbo;

static GLenum color_formats[] = {
	GL_RED,
	GL_RG,
	GL_RGB,
	GL_ALPHA};

enum piglit_result
piglit_display()
{
	bool pass = true;
	GLuint i;

	FboConfig config_ms(1 , pattern_width, pattern_height);

	for(i = 0; i < ARRAY_SIZE(color_formats); i++) {
		config_ms.color_internalformat = color_formats[i];
		src_fbo.setup(config_ms);

		if (!piglit_check_gl_error(GL_NO_ERROR)) {
			printf("Error setting up renderbuffer color format\n");
			piglit_report_result(PIGLIT_FAIL);
		}

		/* Blit multisample-to-multisample with non-matching formats */
		glBindFramebuffer(GL_READ_FRAMEBUFFER, src_fbo.handle);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, dst_fbo.handle);
		glBlitFramebuffer(0, 0, pattern_width, pattern_height,
				  0, 0, pattern_width, pattern_height,
				  GL_COLOR_BUFFER_BIT, GL_NEAREST);

		/* Blitting between different formats is not allowed for
		 * multisample frambuffers. Here GL_INVALID_OPERATION is an
		 * expected gl error.
		 */
		pass = piglit_check_gl_error(GL_INVALID_OPERATION) && pass;
	}

	return (pass ? PIGLIT_PASS : PIGLIT_FAIL);
}

void
piglit_init(int argc, char **argv)
{
	piglit_require_gl_version(30);

	/* Passing sample count = 1 will create the FBOs with minimum supported
	 * sample count. Both FBOs are created with GL_RGBA format by default.
	 */
	src_fbo.setup(FboConfig(1 /* sample count */,
				pattern_width,
				pattern_height));

	dst_fbo.setup(FboConfig(1 /* sample count */,
				pattern_width,
				pattern_height));

	if (!piglit_check_gl_error(GL_NO_ERROR)) {
		printf("Error setting up frame buffer objects\n");
		piglit_report_result(PIGLIT_FAIL);
	}
}
