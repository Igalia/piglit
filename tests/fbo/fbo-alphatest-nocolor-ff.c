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

/** @file fbo-alphatest-nocolor-ff.c
 *
 * Tests that rendering to a depth texture with no color buffer bound
 * and alpha testing enabled using fixed function does the alpha
 * testing correctly.
 */

#include "piglit-util-gl.h"

#define BUF_WIDTH 32

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_DEPTH | PIGLIT_GL_VISUAL_RGBA;

PIGLIT_GL_TEST_CONFIG_END

static void create_fbo(GLuint *out_tex)
{
	GLuint tex, fb;
	GLenum status;

	/* Create the depth-stencil buffer. */
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
		     BUF_WIDTH, BUF_WIDTH, 0,
		     GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	/* Create the FBO. */
	glGenFramebuffersEXT(1, &fb);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fb);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
				  GL_DEPTH_ATTACHMENT_EXT,
				  GL_TEXTURE_2D,
				  tex,
				  0);

	status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
	if (status != GL_FRAMEBUFFER_COMPLETE_EXT) {
		piglit_report_result(PIGLIT_SKIP);
	}

	glViewport(0, 0, BUF_WIDTH, BUF_WIDTH);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_ALWAYS);
	glClearDepth(0.0);
	glClear(GL_DEPTH_BUFFER_BIT);

	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	glEnable(GL_ALPHA_TEST);
	glAlphaFunc(GL_GREATER, 0.5f);

	/* Does not draw (depth texture = 1.0) */
	glColor4f(0.0, 1.0, 0.0, 0.0);
	piglit_draw_rect_z(1.0,
			   -1.0, -1.0, 1.0, 2.0);

	/* Draws (depth texture = 0.0). */
	glColor4f(0.0, 1.0, 0.0, 1.0);
	piglit_draw_rect_z(1.0,
			   0.0, -1.0, 1.0, 2.0);

	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, piglit_winsys_fbo);
	glDeleteFramebuffersEXT(1, &fb);

	glDisable(GL_ALPHA_TEST);
	glDisable(GL_DEPTH_TEST);

	*out_tex = tex;
}

enum piglit_result piglit_display(void)
{
	GLboolean pass = GL_TRUE;
	float black[] = {0, 0, 0, 0};
	float white[] = {1, 1, 1, 1};
	GLuint tex;

	glClearColor(0.0, 1.0, 0.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT);

	create_fbo(&tex);

	glBindTexture(GL_TEXTURE_2D, tex);
	glViewport(0, 0, piglit_width, piglit_height);

	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, piglit_winsys_fbo);

	glEnable(GL_TEXTURE_2D);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE, GL_INTENSITY);

	piglit_draw_rect_tex(-1, -1, 2, 2,
			     0, 0, 1, 1);

	glDisable(GL_TEXTURE_2D);

	pass = pass && piglit_probe_rect_rgba(0, 0,
					      piglit_width / 2, piglit_height,
					      black);
	pass = pass && piglit_probe_rect_rgba(piglit_width / 2, 0,
					      piglit_width / 2, piglit_height,
					      white);

	glDeleteTextures(1, &tex);

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void piglit_init(int argc, char **argv)
{
	piglit_require_extension("GL_EXT_framebuffer_object");
	piglit_require_extension("GL_ARB_depth_texture");
}
