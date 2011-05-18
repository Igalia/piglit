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
 *
 * Authors:
 *     Chad Versace <chad.versace@intel.com>
 */

/**
 * \file hiz-util.c
 * \copybrief hiz-util.h
 * \author Chad Versace <chad.versace@intel.com>
 */

#include <assert.h>
#include "piglit-util.h"
#include "hiz/hiz-util.h"

bool
hiz_probe_rects()
{
	bool pass = true;
	const float width_9 = piglit_width / 9.0;
	const float height_9 = piglit_height / 9.0;

	/*
	 * For the color and depth probing below, the read buffer is
	 * decomposed as follows: Let (w,h) be the read buffer's dimentions.
	 * Divide the read buffer into 9 large rectangles of dimension (w/3,
	 * h/3). Subdivide each large rectangle into 9 small rectangles of
	 * dimension (w/9, h/9).
	 */

	/*
	 * Probe the color of the center small rectangle of each large
	 * rectangle. The large rectangles are traversed in column major
	 * order, bottom to top and left to right.
	 *
	 * Such exhaustive probing is necessary because HiZ corruption may
	 * cause global artifacts in the scene.
	 */
	pass &= piglit_probe_rect_rgba(1 * width_9, 1 * height_9,
	                                   width_9,     height_9, hiz_green);
	pass &= piglit_probe_rect_rgba(1 * width_9, 4 * height_9,
	                                   width_9,     height_9, hiz_green);
	pass &= piglit_probe_rect_rgba(1 * width_9, 7 * height_9,
	                                   width_9,     height_9, hiz_grey);
	pass &= piglit_probe_rect_rgba(4 * width_9, 1 * height_9,
	                                   width_9,     height_9, hiz_green);
	pass &= piglit_probe_rect_rgba(4 * width_9, 4 * height_9,
	                                   width_9,     height_9, hiz_green);
	pass &= piglit_probe_rect_rgba(4 * width_9, 7 * height_9,
	                                   width_9,     height_9, hiz_blue);
	pass &= piglit_probe_rect_rgba(7 * width_9, 1 * height_9,
	                                   width_9,     height_9, hiz_grey);
	pass &= piglit_probe_rect_rgba(7 * width_9, 4 * height_9,
	                                   width_9,     height_9, hiz_blue);
	pass &= piglit_probe_rect_rgba(7 * width_9, 7 * height_9,
	                                   width_9,     height_9, hiz_blue);

	/* Now probe the depths. */
	pass &= piglit_probe_rect_depth(1 * width_9, 1 * height_9,
	                                    width_9,     height_9, hiz_green_z);
	pass &= piglit_probe_rect_depth(1 * width_9, 4 * height_9,
	                                    width_9,     height_9, hiz_green_z);
	pass &= piglit_probe_rect_depth(1 * width_9, 7 * height_9,
	                                    width_9,     height_9, hiz_clear_z);
	pass &= piglit_probe_rect_depth(4 * width_9, 1 * height_9,
	                                    width_9,     height_9, hiz_green_z);
	pass &= piglit_probe_rect_depth(4 * width_9, 4 * height_9,
	                                    width_9,     height_9, hiz_green_z);
	pass &= piglit_probe_rect_depth(4 * width_9, 7 * height_9,
	                                    width_9,     height_9, hiz_blue_z);
	pass &= piglit_probe_rect_depth(7 * width_9, 1 * height_9,
	                                    width_9,     height_9, hiz_clear_z);
	pass &= piglit_probe_rect_depth(7 * width_9, 4 * height_9,
	                                    width_9,     height_9, hiz_blue_z);
	pass &= piglit_probe_rect_depth(7 * width_9, 7 * height_9,
	                                    width_9,     height_9, hiz_blue_z);

	return pass;
}


GLuint
hiz_make_fbo(const struct hiz_fbo_options *options)
{
	GLuint fb = 0;
	GLenum fb_status;
	GLuint color_rb = 0;
	GLuint depth_rb = 0;
	GLuint stencil_rb = 0;
	GLuint depth_stencil_rb = 0;

	glGenFramebuffers(1, &fb);
	glBindFramebuffer(GL_FRAMEBUFFER, fb);

	/* Bind color attachment. */
	if (options->color_format != 0) {
		glGenRenderbuffers(1, &color_rb);
		glBindRenderbuffer(GL_RENDERBUFFER, color_rb);
		glRenderbufferStorage(GL_RENDERBUFFER,
		                      options->color_format,
		                      piglit_width, piglit_height);
		glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER,
		                          GL_COLOR_ATTACHMENT0,
		                          GL_RENDERBUFFER,
		                          color_rb);
		assert(glGetError() == 0);
	}

	/* Bind depth attachment. */
	if (options->depth_format != 0) {
		glGenRenderbuffers(1, &depth_rb);
		glBindRenderbuffer(GL_RENDERBUFFER, depth_rb);
		glRenderbufferStorage(GL_RENDERBUFFER,
		                      options->depth_format,
		                      piglit_width, piglit_height);
		glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER,
		                          GL_DEPTH_ATTACHMENT,
		                          GL_RENDERBUFFER,
		                          depth_rb);
		assert(glGetError() == 0);
	}

	/* Bind stencil attachment. */
	if (options->stencil_format != 0) {
		glGenRenderbuffers(1, &stencil_rb);
		glBindRenderbuffer(GL_RENDERBUFFER, stencil_rb);
		glRenderbufferStorage(GL_RENDERBUFFER,
		                      options->stencil_format,
		                      piglit_width, piglit_height);
		glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER,
		                          GL_STENCIL_ATTACHMENT,
		                          GL_RENDERBUFFER,
		                          stencil_rb);
		assert(glGetError() == 0);
	}

	/* Bind depth/stencil attachment. */
	if (options->depth_stencil_format != 0) {
		glGenRenderbuffers(1, &depth_stencil_rb);
		glBindRenderbuffer(GL_RENDERBUFFER, depth_stencil_rb);
		glRenderbufferStorage(GL_RENDERBUFFER,
		                      options->depth_stencil_format,
		                      piglit_width, piglit_height);
		glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER,
		                          GL_DEPTH_STENCIL_ATTACHMENT,
		                          GL_RENDERBUFFER,
		                          depth_stencil_rb);
		assert(glGetError() == 0);
	}

	fb_status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (fb_status != GL_FRAMEBUFFER_COMPLETE) {
		printf("error: FBO incomplete (status = 0x%04x)\n", fb_status);
		piglit_report_result(PIGLIT_SKIP);
	}

	return fb;
}

void
hiz_delete_fbo(GLuint fbo)
{
	const GLenum *i;
	const GLenum attachments[] = {
		GL_COLOR_ATTACHMENT0,
		GL_DEPTH_STENCIL_ATTACHMENT,
		GL_DEPTH_ATTACHMENT,
		GL_STENCIL_ATTACHMENT,
		0
	};

	for (i = attachments; *i != 0; ++i) {
		GLuint name;
		glGetFramebufferAttachmentParameteriv(
			GL_DRAW_FRAMEBUFFER,
			GL_COLOR_ATTACHMENT0,
			GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME,
			(GLint*) &name);
		if (name != 0)
			glDeleteRenderbuffers(1, &name);
	}

	glDeleteFramebuffers(1, &fbo);

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);

	assert(!glGetError());
}

/* ------------------------------------------------------------------------ */

/**
 * \name hiz_run_depth_test utilties
 *
 * Utilities for testing depth testing.
 *
 * \{
 */

/**
 * Common functionality needed by hiz_run_test_depth_test_fbo() and
 * hiz_run_test_depth_test_window().
 */
static bool
hiz_run_test_depth_test_common()
{
	const float width_3 = piglit_width / 3.0;
	const float height_3 = piglit_height / 3.0;

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glClearDepth(hiz_clear_z);
	glClearColor(hiz_grey[0], hiz_grey[1], hiz_grey[2], hiz_grey[3]);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glViewport(0, 0, piglit_width, piglit_height);
	piglit_ortho_projection(piglit_width, piglit_height, false);

	glColor4fv(hiz_green);
	glDepthRange(hiz_green_z, hiz_green_z);
	piglit_draw_rect(0 * width_3, 0 * width_3,   /* x, y */
		         2 * width_3, 2 * height_3); /* width, height */

	glColor4fv(hiz_blue);
	glDepthRange(hiz_blue_z, hiz_blue_z);
	piglit_draw_rect(1 * width_3, 1 * height_3,   /* x, y */
		         2 * width_3, 2 * height_3); /* width, height */

	glClearDepth(1.0);
	glDepthRange(0, 1);

	return hiz_probe_rects();
}

bool
hiz_run_test_depth_test_fbo(const struct hiz_fbo_options *fbo_options)
{
	bool pass = true;
	GLuint fbo = 0;

	piglit_require_extension("GL_ARB_framebuffer_object");

	/* Create and bind FBO. */
	fbo = hiz_make_fbo(fbo_options);
	assert(fbo != 0);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);

	pass = hiz_run_test_depth_test_common();

	if (!piglit_automatic) {
		/* Blit the FBO to the window FB so we can see the results. */
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		glBlitFramebuffer(0, 0, piglit_width, piglit_height,
			          0, 0, piglit_width, piglit_height,
			          GL_COLOR_BUFFER_BIT, GL_NEAREST);
		glutSwapBuffers();
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
	}

	hiz_delete_fbo(fbo);

	return pass;
}

bool
hiz_run_test_depth_test_window() {
	bool pass = hiz_run_test_depth_test_common();
	if (!piglit_automatic)
		glutSwapBuffers();
	return pass;
}

/** \} */
