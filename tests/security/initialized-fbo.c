/*
 * Copyright © 2012 VMware, Inc.
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
 * Test that FBO memory is initialized to a constant color and not stale
 * data that may show old contents of VRAM.
 *
 * To pass this test an OpenGL implementation should initialize the
 * contents of the new buffer to some fixed value (like all zeros).
 * But since that's not spec'd by OpenGL, we only return WARN instead of
 * FAIL if that's not the case.
 *
 * Brian Paul
 * June 2012
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_width = 512;
	config.window_height = 512;
	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END


static GLuint fbo, rb;


enum piglit_result
piglit_display(void)
{
	GLfloat firstPixel[4];
	bool pass = true;

	/* read from fbo, draw to window */
	glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
	glReadBuffer(GL_COLOR_ATTACHMENT0);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, piglit_winsys_fbo);
	glDrawBuffer(GL_BACK);

        /* init color buffer to red */
	glClearColor(1, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT);

	/* copy (undefined) fbo image to window */
	glWindowPos2i(0, 0);
	glCopyPixels(0, 0, piglit_width, piglit_height, GL_COLOR);

	glBindFramebuffer(GL_READ_FRAMEBUFFER, piglit_winsys_fbo);
	glReadBuffer(GL_BACK);

	glReadPixels(0, 0, 1, 1, GL_RGBA, GL_FLOAT, firstPixel);
	pass = piglit_probe_rect_rgb(0, 0, piglit_width, piglit_height,
				     firstPixel);

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_WARN;
}


void
piglit_init(int argc, char **argv)
{
	piglit_require_extension("GL_ARB_framebuffer_object");

	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glGenRenderbuffers(1, &rb);
	glBindRenderbuffer(GL_RENDERBUFFER, rb);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA,
			      piglit_width, piglit_height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
				  GL_RENDERBUFFER, rb);

	if (!piglit_check_gl_error(GL_NO_ERROR)) {
		printf("fbo creation error\n");
		piglit_report_result(PIGLIT_FAIL);
		return;
	}

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) !=
	    GL_FRAMEBUFFER_COMPLETE) {
		printf("fbo incomplete\n");
		piglit_report_result(PIGLIT_FAIL);
		return;
	}

	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);
	glViewport(0, 0, piglit_width, piglit_height);
}
