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
 * \file fbo-scissor-blit.c
 *
 * Since window system framebuffers use a different coordinate system
 * than fbo's, it is important to check that glBlitFramebuffer()
 * interprets scissor coordinates correctly depending whether the
 * destination framebuffer is an fbo or a window.  This test verifies
 * proper scissor operation in both cases.
 *
 * The test takes a single command-line argument: "window" to test
 * scissoring in a window, and "fbo" to test scissoring in an fbo.  In
 * the fbo case, the final image is blitted to the window afterwards
 * (without scissoring it) so that failures can be easily diagnosed.
 */

#include "piglit-util-gl.h"

static const int width = 128, height = 128;

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_width = width;
	config.window_height = height;
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;

PIGLIT_GL_TEST_CONFIG_END
static GLuint src_fbo;
static GLuint ref_fbo;
static GLuint dst_fbo;


static GLuint
setup_framebuffer()
{
	GLuint fbo, rb;
	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
	glGenRenderbuffers(1, &rb);
	glBindRenderbuffer(GL_RENDERBUFFER, rb);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA, width, height);
	glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
				  GL_RENDERBUFFER, rb);
	if (glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER)
	    != GL_FRAMEBUFFER_COMPLETE) {
		printf("Framebuffer incomplete\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	return fbo;
}


static void
print_usage_and_exit(char *prog_name)
{
	printf("Usage: %s <dst_fb_type>\n"
	       "  where <dst_fb_type> is one of:\n"
	       "    fbo\n"
	       "    window\n",
	       prog_name);
	piglit_report_result(PIGLIT_FAIL);
}


void
piglit_init(int argc, char **argv)
{
	bool blit_to_fbo;

	if (argc != 2)
		print_usage_and_exit(argv[0]);

	if (strcmp(argv[1], "window") == 0)
		blit_to_fbo = false;
	else if (strcmp(argv[1], "fbo") == 0)
		blit_to_fbo = true;
	else {
		blit_to_fbo = false;
		print_usage_and_exit(argv[0]);
	}

	piglit_require_extension("GL_ARB_framebuffer_object");

	src_fbo = setup_framebuffer();
	ref_fbo = setup_framebuffer();
	if (blit_to_fbo)
		dst_fbo = setup_framebuffer();
	else
		dst_fbo = 0;
}


enum piglit_result
piglit_display(void)
{
	float red[4]   = {1.0, 0.0, 0.0, 0.0};
	float green[4] = {0.0, 1.0, 0.0, 0.25};
	float blue[4]  = {0.0, 0.0, 1.0, 0.5};
	float white[4] = {1.0, 1.0, 1.0, 1.0};
	float grey[4]  = {0.5, 0.5, 0.5, 0.5};
	bool pass;
        float *ref_image;

	/* Draw the source image to src_fbo */
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, src_fbo);
	glEnable(GL_SCISSOR_TEST);

	glScissor(0, 0, width / 2, height / 2);
	glClearColor(red[0], red[1], red[2], red[3]);
	glClear(GL_COLOR_BUFFER_BIT);

	glScissor(width / 2, 0, width / 2, height / 2);
	glClearColor(green[0], green[1], green[2], green[3]);
	glClear(GL_COLOR_BUFFER_BIT);

	glScissor(0, height / 2, width / 2, height / 2);
	glClearColor(blue[0], blue[1], blue[2], blue[3]);
	glClear(GL_COLOR_BUFFER_BIT);

	glScissor(width / 2, height / 2, width / 2, height / 2);
	glClearColor(white[0], white[1], white[2], white[3]);
	glClear(GL_COLOR_BUFFER_BIT);

	glDisable(GL_SCISSOR_TEST);

	glClearColor(grey[0], grey[1], grey[2], grey[3]);

	/* Blit to dst_fbo, scissoring the image in an asymmetrical way. */
	glBindFramebuffer(GL_READ_FRAMEBUFFER, src_fbo);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, dst_fbo);
	glClear(GL_COLOR_BUFFER_BIT);
	glEnable(GL_SCISSOR_TEST);
	glScissor(10, 20, width - 40, height - 60);
	glBlitFramebuffer(0, 0, width, height,
			  0, 0, width, height,
			  GL_COLOR_BUFFER_BIT, GL_NEAREST);
	glDisable(GL_SCISSOR_TEST);

	/* Blit to ref_fbo, simulating the correct scissoring effect
	 * by manually adjusting the coordinates.
	 */
	glBindFramebuffer(GL_READ_FRAMEBUFFER, src_fbo);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, ref_fbo);
	glClear(GL_COLOR_BUFFER_BIT);
	glBlitFramebuffer(10, 20, width - 30, height - 40,
			  10, 20, width - 30, height - 40,
			  GL_COLOR_BUFFER_BIT, GL_NEAREST);

	/* Read the reference image from ref_fbo */
	ref_image = malloc(sizeof(float) * 4 * width * height);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, ref_fbo);
	glReadPixels(0, 0, width, height, GL_RGBA, GL_FLOAT, ref_image);

	/* Compare the image in dst_fbo with the reference image */
	glBindFramebuffer(GL_READ_FRAMEBUFFER, dst_fbo);
	pass = piglit_probe_image_rgba(0, 0, width, height, ref_image);

	if (dst_fbo != 0) {
		/* Show the contents of dst_fbo in the window */
		glBindFramebuffer(GL_READ_FRAMEBUFFER, dst_fbo);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, piglit_winsys_fbo);
		glBlitFramebuffer(0, 0, width, height,
				  0, 0, width, height,
				  GL_COLOR_BUFFER_BIT, GL_NEAREST);
	}

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}
