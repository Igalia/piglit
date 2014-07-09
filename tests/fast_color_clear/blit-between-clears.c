/*
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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

/**
 * \file blit-between-clears.c
 *
 * Some implementations (i965/gen7+ in particular) contain logic to
 * avoid performing a redundant fast color clear on a buffer that is
 * already in the cleared state.  This test verifies that blitting to
 * a buffer takes it out of the cleared state, so a subsequent fast
 * color clear will take effect.
 */

#include "piglit-util-gl.h"

#define RB_WIDTH 512
#define RB_HEIGHT 512

PIGLIT_GL_TEST_CONFIG_BEGIN
	config.supports_gl_compat_version = 11;
	config.window_width = RB_WIDTH;
	config.window_height = RB_HEIGHT;
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;
PIGLIT_GL_TEST_CONFIG_END


static GLuint fb;


void
piglit_init(int argc, char **argv)
{
	GLuint rb;
	GLenum fb_status;

	/* Requirements */
	piglit_require_gl_version(11);
	piglit_require_extension("GL_ARB_framebuffer_object");

	/* Set up framebuffer */
	glGenFramebuffers(1, &fb);
	glBindFramebuffer(GL_FRAMEBUFFER, fb);
	glGenRenderbuffers(1, &rb);
	glBindRenderbuffer(GL_RENDERBUFFER, rb);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA,
			      RB_WIDTH, RB_HEIGHT);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
				  GL_RENDERBUFFER, rb);
	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);
	fb_status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (fb_status != GL_FRAMEBUFFER_COMPLETE) {
		printf("Framebuffer status: %s\n",
		       piglit_get_gl_enum_name(fb_status));
		piglit_report_result(PIGLIT_FAIL);
	}
}


enum piglit_result
piglit_display(void)
{
	bool pass = true;
	static const GLfloat green[] = { 0.0, 1.0, 0.0, 1.0 };

	/* Fast clear the auxiliary framebuffer to red. */
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fb);
	glClearColor(1, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT);

	/* Fast clear the window system framebuffer to green. */
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, piglit_winsys_fbo);
	glClearColor(0, 1, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT);

	/* Blit the auxiliary framebuffer to the window system
	 * framebuffer, turning it red.
	 */
	glBindFramebuffer(GL_READ_FRAMEBUFFER, fb);
	glBlitFramebuffer(0, 0, RB_WIDTH, RB_HEIGHT,
			  0, 0, piglit_width, piglit_height,
			  GL_COLOR_BUFFER_BIT, GL_NEAREST);

	/* Fast clear the window system framebuffer back to green. */
	glClear(GL_COLOR_BUFFER_BIT);

	/* Verify that the second clear actually took effect. */
	glBindFramebuffer(GL_READ_FRAMEBUFFER, piglit_winsys_fbo);
	pass = piglit_probe_rect_rgba(0, 0, piglit_width, piglit_height,
				      green) && pass;

	if (!piglit_check_gl_error(GL_NO_ERROR))
		pass = false;

	piglit_present_results();
	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}
