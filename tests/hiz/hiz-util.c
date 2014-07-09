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
#include "piglit-util-gl.h"
#include "hiz/hiz-util.h"

#define hiz_probe_common(probe_func, expect) \
	bool pass = true; \
	const float dx = piglit_width / 9.0; \
	const float dy = piglit_height / 9.0; \
	\
	int ix; \
	int iy; \
	\
	for (iy = 0; iy < 3; ++iy) for (ix = 0; ix < 3; ++ix) {	\
		int x = (3 * ix + 1) * dx; \
		int y = (3 * iy + 1) * dy; \
		int i = 3 * (2 - iy) + ix;		     \
		pass &= probe_func(x, y, dx, dy, expect[i]); \
	} \
	\
	return pass;


bool
hiz_probe_color_buffer(const float *expected_colors[])
{
	hiz_probe_common(piglit_probe_rect_rgb, expected_colors);
}

bool
hiz_probe_depth_buffer(const float expected_depths[])
{
	hiz_probe_common(piglit_probe_rect_depth, expected_depths);
}

bool
hiz_probe_stencil_buffer(const unsigned expected_stencil[])
{
	hiz_probe_common(piglit_probe_rect_stencil, expected_stencil);
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
         	if (!piglit_check_gl_error(0))
			piglit_report_result(PIGLIT_FAIL);
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
         	if (!piglit_check_gl_error(0))
			piglit_report_result(PIGLIT_FAIL);
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
         	if (!piglit_check_gl_error(0))
			piglit_report_result(PIGLIT_FAIL);
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
         	if (!piglit_check_gl_error(0))
			piglit_report_result(PIGLIT_FAIL);
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

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, piglit_winsys_fbo);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, piglit_winsys_fbo);

 	if (!piglit_check_gl_error(0))
		piglit_report_result(PIGLIT_FAIL);
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
	static const float *expect_color[9] = {
		hiz_grey,  hiz_blue,  hiz_blue,
		hiz_green, hiz_green, hiz_blue,
		hiz_green, hiz_green, hiz_grey,
	};

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

	return hiz_probe_color_buffer(expect_color);
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
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, piglit_winsys_fbo);
		glBlitFramebuffer(0, 0, piglit_width, piglit_height,
			          0, 0, piglit_width, piglit_height,
			          GL_COLOR_BUFFER_BIT, GL_NEAREST);
		piglit_present_results();
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
	}

	hiz_delete_fbo(fbo);

	return pass;
}

bool
hiz_run_test_depth_test_window()
{
	bool pass = hiz_run_test_depth_test_common();
	if (!piglit_automatic)
		piglit_present_results();
	return pass;
}

/** \} */

/* ------------------------------------------------------------------------ */

/**
 * \name hiz_run_depth_read utilties
 *
 * Utilities for testing depth reads.
 *
 * \{
 */

/**
 * Common functionality needed by hiz_run_test_depth_read_fbo() and
 * hiz_run_test_depth_read_window().
 */
static bool
hiz_run_test_depth_read_common()
{
	static const float *expect_color[9] = {
		hiz_grey,  hiz_blue,  hiz_blue,
		hiz_green, hiz_green, hiz_blue,
		hiz_green, hiz_green, hiz_grey,
	};

	static const float expect_depth[9] = {
		hiz_clear_z, hiz_blue_z,  hiz_blue_z,
		hiz_green_z, hiz_green_z, hiz_blue_z,
		hiz_green_z, hiz_green_z, hiz_clear_z,
	};


	const float width_3 = piglit_width / 3.0;
	const float height_3 = piglit_height / 3.0;

	glDisable(GL_STENCIL_TEST);

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

	if (!hiz_probe_color_buffer(expect_color))
		return false;
	return hiz_probe_depth_buffer(expect_depth);
}

bool
hiz_run_test_depth_read_fbo(const struct hiz_fbo_options *fbo_options)
{
	bool pass = true;
	GLuint fbo = 0;

	piglit_require_extension("GL_ARB_framebuffer_object");

	/* Create and bind FBO. */
	fbo = hiz_make_fbo(fbo_options);
	assert(fbo != 0);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);

	pass = hiz_run_test_depth_read_common();

	if (!piglit_automatic) {
		/* Blit the FBO to the window FB so we can see the results. */
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, piglit_winsys_fbo);
		glBlitFramebuffer(0, 0, piglit_width, piglit_height,
			          0, 0, piglit_width, piglit_height,
			          GL_COLOR_BUFFER_BIT, GL_NEAREST);
		piglit_present_results();
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
	}

	hiz_delete_fbo(fbo);

	return pass;
}

bool
hiz_run_test_depth_read_window()
{
	bool pass = hiz_run_test_depth_read_common();
	if (!piglit_automatic)
		piglit_present_results();
	return pass;
}

/** \} */

/* ------------------------------------------------------------------------ */

/**
 * \name hiz_run_stencil_test utilties
 *
 * Utilities for testing stencil testing.
 *
 * \{
 */

/**
 * Common functionality needed by hiz_run_test_stencil_test_fbo() and
 * hiz_run_test_stencil_test_window().
 */
static bool
hiz_run_test_stencil_test_common()
{
	static const float *expected_colors[9] = {
		hiz_grey,  hiz_blue,  hiz_grey,
		hiz_green, hiz_blue,  hiz_grey,
		hiz_green, hiz_green, hiz_grey,
	};

	const float dx = piglit_width / 3.0;
	const float dy = piglit_height / 3.0;

	/* Set up depth state. */
	glDisable(GL_DEPTH_TEST);
	glClearDepth(hiz_clear_z);

	/* Set up stencil state. */
	glEnable(GL_STENCIL_TEST);
	glClearStencil(3); /* 3 is a good canary. */
	glStencilFunc(GL_LESS, 3, ~0);
	glStencilOp(GL_INCR, GL_INCR, GL_INCR);

	glClearColor(hiz_grey[0], hiz_grey[1], hiz_grey[2], hiz_grey[3]);
	glClear(GL_COLOR_BUFFER_BIT
		| GL_DEPTH_BUFFER_BIT
		| GL_STENCIL_BUFFER_BIT);

	glViewport(0, 0, piglit_width, piglit_height);
	piglit_ortho_projection(piglit_width, piglit_height, false);

	/* Draw rect 1. */
	glColor4fv(hiz_grey);
	piglit_draw_rect(0 * dx, 0 * dy, /* x, y */
			 2 * dx, 3 * dy); /* w, h */

	/* Draw rect 2. */
	glColor4fv(hiz_green);
	piglit_draw_rect(0 * dx, 0 * dy,  /* x, y */
			 2 * dx, 2 * dy); /* w, h */

	/* Draw rect 3. */
	glColor4fv(hiz_blue);
	piglit_draw_rect(1 * dx, 1 * dy,   /* x, y */
		         2 * dx, 2 * dy);  /* w, h */

	if (!piglit_check_gl_error(0))
		return false;

	return hiz_probe_color_buffer(expected_colors);
}

bool
hiz_run_test_stencil_test_fbo(const struct hiz_fbo_options *fbo_options)
{
	bool pass = true;
	GLuint fbo = 0;

	piglit_require_extension("GL_ARB_framebuffer_object");

	/* Create and bind FBO. */
	fbo = hiz_make_fbo(fbo_options);
	assert(fbo != 0);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);

	pass = hiz_run_test_stencil_test_common();

	if (!piglit_automatic) {
		/* Blit the FBO to the window FB so we can see the results. */
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, piglit_winsys_fbo);
		glBlitFramebuffer(0, 0, piglit_width, piglit_height,
			          0, 0, piglit_width, piglit_height,
			          GL_COLOR_BUFFER_BIT, GL_NEAREST);
		piglit_present_results();
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
	}

	hiz_delete_fbo(fbo);

	return pass;
}

bool
hiz_run_test_stencil_test_window()
{
	bool pass = hiz_run_test_stencil_test_common();
	if (!piglit_automatic)
		piglit_present_results();
	return pass;
}

/** \} */

/* ------------------------------------------------------------------------ */

/**
 * \name hiz_run_stencil_read utilties
 *
 * Utilities for testing the reading of the stencil buffer.
 *
 * \{
 */

/**
 * Common functionality needed by hiz_run_test_stencil_read_fbo() and
 * hiz_run_test_stencil_read_window().
 */
static bool
hiz_run_test_stencil_read_common()
{
	static const float *expected_colors[9] = {
		hiz_grey,  hiz_blue,  hiz_grey,
		hiz_green, hiz_blue,  hiz_grey,
		hiz_green, hiz_green, hiz_grey,
	};

	static const unsigned expected_stencil[9] = {
		4, 5, 4,
		5, 6, 4,
		5, 5, 3,
	};

	const float dx = piglit_width / 3.0;
	const float dy = piglit_height / 3.0;

	/* Set up depth state. */
	glDisable(GL_DEPTH_TEST);
	glClearDepth(hiz_clear_z);

	/* Set up stencil state. */
	glEnable(GL_STENCIL_TEST);
	glClearStencil(3); /* 3 is a good canary. */
	glStencilFunc(GL_LESS, 3, ~0);
	glStencilOp(GL_INCR, GL_INCR, GL_INCR);

	glClearColor(hiz_grey[0], hiz_grey[1], hiz_grey[2], hiz_grey[3]);
	glClear(GL_COLOR_BUFFER_BIT
		| GL_DEPTH_BUFFER_BIT
		| GL_STENCIL_BUFFER_BIT);

	glViewport(0, 0, piglit_width, piglit_height);
	piglit_ortho_projection(piglit_width, piglit_height, false);

	/* Draw rect 1. */
	glColor4fv(hiz_grey);
	piglit_draw_rect(0 * dx, 0 * dy, /* x, y */
			 2 * dx, 3 * dy); /* w, h */

	/* Draw rect 2. */
	glColor4fv(hiz_green);
	piglit_draw_rect(0 * dx, 0 * dy,  /* x, y */
			 2 * dx, 2 * dy); /* w, h */

	/* Draw rect 3. */
	glColor4fv(hiz_blue);
	piglit_draw_rect(1 * dx, 1 * dy,   /* x, y */
		         2 * dx, 2 * dy);  /* w, h */

	if (!piglit_check_gl_error(0))
		return false;

	if (!hiz_probe_color_buffer(expected_colors))
		return false;
	return hiz_probe_stencil_buffer(expected_stencil);
}

bool
hiz_run_test_stencil_read_fbo(const struct hiz_fbo_options *fbo_options)
{
	bool pass = true;
	GLuint fbo = 0;

	piglit_require_extension("GL_ARB_framebuffer_object");

	/* Create and bind FBO. */
	fbo = hiz_make_fbo(fbo_options);
	assert(fbo != 0);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);

	pass = hiz_run_test_stencil_read_common();

	if (!piglit_automatic) {
		/* Blit the FBO to the window FB so we can see the results. */
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, piglit_winsys_fbo);
		glBlitFramebuffer(0, 0, piglit_width, piglit_height,
			          0, 0, piglit_width, piglit_height,
			          GL_COLOR_BUFFER_BIT, GL_NEAREST);
		piglit_present_results();
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
	}

	hiz_delete_fbo(fbo);

	return pass;
}

bool
hiz_run_test_stencil_read_window()
{
	bool pass = hiz_run_test_stencil_read_common();
	if (!piglit_automatic)
		piglit_present_results();
	return pass;
}

/** \} */

bool
hiz_run_test_depth_stencil_test_fbo(const struct hiz_fbo_options *fbo_options)
{
	bool pass = true;

	GLuint fbo = 0;
	bool has_depth_buffer = fbo_options->depth_format
				|| fbo_options->depth_stencil_format;
	bool has_stencil_buffer = fbo_options->stencil_format
				  || fbo_options->depth_stencil_format;

	const float dx = piglit_width / 3.0;
	const float dy = piglit_height / 3.0;

	static const float **expected_colors = NULL;

	static const float *expected_colors_d1s0[9] = {
		hiz_grey,  hiz_blue,  hiz_blue,
		hiz_green, hiz_green, hiz_blue,
		hiz_green, hiz_green, hiz_grey,
	};

	static const float *expected_colors_d0s1[9] = {
		hiz_grey,  hiz_blue,  hiz_grey,
		hiz_green, hiz_blue,  hiz_grey,
		hiz_green, hiz_green, hiz_grey,
	};

	static const float *expected_colors_d1s1[9] = {
		hiz_grey,  hiz_blue,  hiz_grey,
		hiz_green, hiz_green, hiz_grey,
		hiz_green, hiz_green, hiz_grey,
	};

	if (has_depth_buffer && !has_stencil_buffer)
	   expected_colors = expected_colors_d1s0;
	else if (!has_depth_buffer && has_stencil_buffer)
	   expected_colors = expected_colors_d0s1;
	else if (has_depth_buffer && has_stencil_buffer)
	   expected_colors = expected_colors_d1s1;

	piglit_require_extension("GL_ARB_framebuffer_object");

	/* Create and bind FBO. */
	fbo = hiz_make_fbo(fbo_options);
	assert(fbo != 0);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);

	/* Set up depth state. */
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glClearDepth(hiz_clear_z);

	/* Set up stencil state. The test for 3 < stencil with the
	 * buffer cleared to 3 means the first primitive drawn will be
	 * stenciled out.
	 */
	glEnable(GL_STENCIL_TEST);
	glClearStencil(3); /* 3 is a good canary. */
	glStencilFunc(GL_LESS, 3, ~0);
	glStencilOp(GL_INCR, GL_INCR, GL_INCR);

	glClearColor(hiz_grey[0], hiz_grey[1], hiz_grey[2], hiz_grey[3]);
	glClear(GL_COLOR_BUFFER_BIT
		| GL_DEPTH_BUFFER_BIT
		| GL_STENCIL_BUFFER_BIT);

	glViewport(0, 0, piglit_width, piglit_height);
	piglit_ortho_projection(piglit_width, piglit_height, false);

	/* Draw a rect 1 on left 2/3 of the screen with clear color,
	 * letting the next drawing there pass stencil.
	 */
	glColor4fv(hiz_grey);
	glDepthRange(hiz_clear_z, hiz_clear_z);
	piglit_draw_rect(0 * dx, 0 * dy, /* x, y */
			 2 * dx, 3 * dy); /* w, h */

	/* Draw rect 2. This should pass with or without stencil. */
	glColor4fv(hiz_green);
	glDepthRange(hiz_green_z, hiz_green_z);
	piglit_draw_rect(0 * dx, 0 * dy,  /* x, y */
			 2 * dx, 2 * dy); /* w, h */

	/* Draw rect 3. This should draw only the left half if stencil
	 * is present (due to rect 1 covering only that much), and
	 * should draw over rect 2 only if depth is not present.
	 */
	glColor4fv(hiz_blue);
	glDepthRange(hiz_blue_z, hiz_blue_z);
	piglit_draw_rect(1 * dx, 1 * dy,   /* x, y */
		         2 * dx, 2 * dy);  /* w, h */

 	pass = piglit_check_gl_error(0);

	pass = hiz_probe_color_buffer(expected_colors) && pass;

	if (!piglit_automatic) {
		/* Blit the FBO to the window FB so we can see the results. */
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, piglit_winsys_fbo);
		glBlitFramebuffer(0, 0, piglit_width, piglit_height,
			          0, 0, piglit_width, piglit_height,
			          GL_COLOR_BUFFER_BIT, GL_NEAREST);
		piglit_present_results();
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
	}

	hiz_delete_fbo(fbo);

	return pass;
}
