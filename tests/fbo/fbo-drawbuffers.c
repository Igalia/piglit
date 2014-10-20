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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 *
 * Authors:
 *    Eric Anholt <eric@anholt.net>
 *
 */

/** @file fbo-drawbuffers.c
 *
 * Tests that drawing the same color to two render targets with
 * ARB_draw_buffers and fixed function fragment works.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_RGBA |
			       PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

static GLuint
attach_texture(int i)
{
	GLuint tex;

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
		     piglit_width, piglit_height, 0,
		     GL_RGBA, GL_UNSIGNED_BYTE, NULL);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
				  GL_COLOR_ATTACHMENT0_EXT + i,
				  GL_TEXTURE_2D,
				  tex,
				  0);
	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	return tex;
}

static GLboolean test_masked_clear;

enum piglit_result
piglit_display(void)
{
	GLboolean pass = GL_TRUE;
	GLuint tex0, tex1, fb;
	GLenum status;
	float green[] = {0, 1, 0, 0};
	float masked_clear[] = {0.1, 0.7, 0.3, 0.4};
	float *expected;
	const GLenum attachments[] = {
		GL_COLOR_ATTACHMENT0_EXT,
		GL_COLOR_ATTACHMENT1_EXT,
	};

	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);

	glGenFramebuffersEXT(1, &fb);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fb);

	tex0 = attach_texture(0);
	tex1 = attach_texture(1);

	glDrawBuffersARB(2, attachments);

	status = glCheckFramebufferStatusEXT (GL_FRAMEBUFFER_EXT);
	if (status != GL_FRAMEBUFFER_COMPLETE_EXT) {
		fprintf(stderr, "fbo incomplete (status = 0x%04x)\n", status);
		piglit_report_result(PIGLIT_SKIP);
	}

	/* Clear render targets (textures) to red */
	if (test_masked_clear) {
		glClearColor(0.1, 0.2, 0.3, 0.4);
		glClear(GL_COLOR_BUFFER_BIT);

		glColorMask(GL_FALSE, GL_TRUE, GL_FALSE, GL_FALSE);
		glClearColor(0.6, 0.7, 0.8, 0.9);
		glClear(GL_COLOR_BUFFER_BIT);
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

		expected = masked_clear;
	} else {
		glClearColor(1.0, 0.0, 0.0, 0.0);
		glClear(GL_COLOR_BUFFER_BIT);

		glColor4fv(green);
		piglit_draw_rect(0, 0, piglit_width, piglit_height);

		expected = green;
	}

	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, piglit_winsys_fbo);

	/* Draw the two green textures to halves of the window. */
	glEnable(GL_TEXTURE_2D);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glBindTexture(GL_TEXTURE_2D, tex0);
	piglit_draw_rect_tex(0, 0,
			     piglit_width / 2, piglit_height,
			     0, 0, 1, 1);
	glBindTexture(GL_TEXTURE_2D, tex1);
	piglit_draw_rect_tex(piglit_width / 2, 0,
			     piglit_width, piglit_height,
			     0, 0, 1, 1);
	glDisable(GL_TEXTURE_2D);
	glDeleteTextures(1, &tex0);
	glDeleteTextures(1, &tex1);
	glDeleteFramebuffersEXT(1, &fb);

	pass = pass && piglit_probe_rect_rgba(0, 0, piglit_width, piglit_height, expected);

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	GLint num;
	int i;

	for (i = 1; i < argc; i++) {
		if (strcmp(argv[i], "masked-clear") == 0) {
			puts("Testing masked glClear.");
			test_masked_clear = GL_TRUE;
		}
	}

	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);

	piglit_require_extension("GL_EXT_framebuffer_object");
	piglit_require_extension("GL_ARB_draw_buffers");

	glGetIntegerv(GL_MAX_DRAW_BUFFERS_ARB, &num);
	if (num < 2)
		piglit_report_result(PIGLIT_SKIP);
}
