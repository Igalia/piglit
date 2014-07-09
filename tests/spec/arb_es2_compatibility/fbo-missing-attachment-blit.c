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
 * \file fbo-missing-attachment-blit.c
 * Verify that a color blit to a depth-only FBO doesn't crash
 *
 * From the ARB_ES2_compatibility spec:
 *
 *     "(8) How should we handle draw buffer completeness?
 *
 *     RESOLVED: Remove draw/readbuffer completeness checks, and treat
 *     drawbuffers referring to missing attachments as if they were NONE."
 *
 * From the ARB_framebuffer_object spec:
 *
 *     "If a buffer is specified in <mask> and does not exist in both the
 *     read and draw framebuffers, the corresponding bit is silently
 *     ignored."
 *
 * It is valid to have a depth-only FBO that has the draw buffer set to values
 * other than \c GL_NONE.  However, doing operations that would read from or
 * draw to these missing attachments should treat them as though they were
 * \c GL_NONE (i.e., don't crash).
 *
 * This test can run in four modes:
 *
 *  - From an FBO missing the color attachment with the ES2 rules.
 *  - From an FBO missing the color attachment without the ES2 rules.
 *  - To an FBO missing the color attachment with the ES2 rules.
 *  - To an FBO missing the color attachment without the ES2 rules.
 *
 * See also https://bugs.freedesktop.org/show_bug.cgi?id=37739.
 *
 * \author Ian Romanick <ian.d.romanick@intel.com>
 */
#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_RGB;

PIGLIT_GL_TEST_CONFIG_END

enum piglit_result
piglit_display(void)
{
	return PIGLIT_FAIL;
}

bool
do_blit_test(bool use_es2, bool from_missing_to_complete)
{
	GLuint rb[3];
	GLuint fb[2];
	GLenum status;
	GLenum err;
	const unsigned src = from_missing_to_complete ? 0 : 1;
	const unsigned dst = 1 - src;
	const char *const names[] = {
		"buffer with missing attachment",
		"complete buffer"
	};
	bool pass = true;

	printf("Testing blit from %s to %s...\n", names[src], names[dst]);

	/* Create a depth-only FBO and a depth/color FBO.
	 */
	glGenRenderbuffers(ARRAY_SIZE(rb), rb);

	glBindRenderbuffer(GL_RENDERBUFFER, rb[0]);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24,
			      piglit_width, piglit_height);
	glBindRenderbuffer(GL_RENDERBUFFER, rb[1]);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24,
			      piglit_width, piglit_height);
	glBindRenderbuffer(GL_RENDERBUFFER, rb[2]);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA,
			      piglit_width, piglit_height);

	glGenFramebuffers(ARRAY_SIZE(fb), fb);
	glBindFramebuffer(GL_FRAMEBUFFER, fb[0]);
	glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
				  GL_RENDERBUFFER, rb[0]);
	if (!use_es2) {
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
	}

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fb[1]);
	glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
				  GL_RENDERBUFFER, rb[1]);
	glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
				  GL_RENDERBUFFER, rb[2]);

	err = glGetError();
	if (err != 0) {
		fprintf(stderr, "Unexpected GL error state 0x%04x\n", err);
		return false;
	}

	/* Check completeness of the source surface.
	 */
	glBindFramebuffer(GL_READ_FRAMEBUFFER, fb[src]);
	status = glCheckFramebufferStatus(GL_READ_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE) {
		fprintf(stderr, "Read FBO erroneously incomplete: 0x%04x\n",
			status);
		return false;
	}

	/* In the source surface, clear the depth buffer and draw a single
	 * rectangle with a constant depth value.  The depth test setting here
	 * is correct.
	 */
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fb[src]);
	glClearDepth(0.0);
	glClear(GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_ALWAYS);

	piglit_draw_rect_z(0.5, -0.5, -0.5, 1.0, 1.0);

	/* Check completeness of the destination surface.
	 */
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fb[dst]);
	status = glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER);

	if (status != GL_FRAMEBUFFER_COMPLETE) {
		fprintf(stderr, "Draw FBO erroneously incomplete: 0x%04x\n",
			status);
		return false;
	}

	glBlitFramebuffer(0, 0, piglit_width, piglit_height,
			  0, 0, piglit_width, piglit_height,
			  GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT,
			  GL_NEAREST);
	err = glGetError();
	if (err != 0) {
		fprintf(stderr, "Unexpected GL error state 0x%04x\n", err);
		return false;
	}

	/* Probe depth values from the destination buffer to make sure the
	 * depth part of the blit actually happened.
	 */
	glBindFramebuffer(GL_READ_FRAMEBUFFER, fb[dst]);
	pass = piglit_probe_rect_depth(0.25 * piglit_width,
				       0.25 * piglit_height,
				       0.4 * piglit_width,
				       0.4 * piglit_height,
				       0.75);
	pass = piglit_probe_rect_depth(0,
				       0,
				       piglit_width,
				       0.2 * piglit_height,
				       0.0)
		&& pass;

	glBindFramebuffer(GL_READ_FRAMEBUFFER, piglit_winsys_fbo);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, piglit_winsys_fbo);
	glDeleteFramebuffers(ARRAY_SIZE(fb), fb);
	glDeleteRenderbuffers(ARRAY_SIZE(rb), rb);

	return pass;
}

void
piglit_init(int argc, char **argv)
{
	bool pass;
	bool use_es2 = false;
	unsigned arg;

	if (argc > 1 && strcmp(argv[1], "es2") == 0) {
		use_es2 = true;
		arg = 2;
	} else {
		arg = 1;
	}

	piglit_require_extension("GL_ARB_framebuffer_object");

	if (use_es2)
		piglit_require_extension("GL_ARB_ES2_compatibility");

	if (argc > arg) {
		pass = do_blit_test(use_es2,
				    strcmp(argv[arg], "from") == 0);
	} else {
		pass = do_blit_test(use_es2, true);
		pass = do_blit_test(use_es2, false) && pass;
	}

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}
