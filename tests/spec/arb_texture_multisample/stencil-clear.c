/*
 * Copyright 2016 Advanced Micro Devices, Inc.
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
 *
 */

#include "piglit-util-gl.h"

/* File: stencil-clear.c
 *
 * Tests whether clearing a multisample stencil texture, followed by a blit
 * and subsequent rendering works correctly.
 */

#define TEX_WIDTH 256
#define TEX_HEIGHT 256

PIGLIT_GL_TEST_CONFIG_BEGIN

    config.supports_gl_compat_version = 30;
    config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;
    config.window_width = TEX_WIDTH;
    config.window_height = TEX_HEIGHT;
    config.khr_no_error_support = PIGLIT_NO_ERRORS;

PIGLIT_GL_TEST_CONFIG_END

static GLuint fbo, fbo_copy;

static void
usage(void)
{
	fprintf(stderr, "usage: arb_texture_multisample-stencil-clear [samples N]\n");
	exit(1);
}

enum piglit_result
piglit_display(void)
{
	static const float black[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
	bool pass = true;

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
	glViewport(0, 0, TEX_WIDTH, TEX_HEIGHT);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClearDepth(0.0f);
	glClearStencil(0);
	glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo_copy);

	/* This blit is there on purpose to trigger a bug in stencil decompress
	 * on Radeon. The blit destination is not used.
	 */
	glBlitFramebuffer(0, 0, TEX_WIDTH, TEX_WIDTH, 0, 0, TEX_WIDTH, TEX_WIDTH, GL_STENCIL_BUFFER_BIT, GL_NEAREST);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	glEnable(GL_STENCIL_TEST);
	glStencilMask(255);
	glStencilFunc(GL_LEQUAL, 1, 255);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

	glColor4f(1.0, 1.0, 1.0, 1.0);
	glBegin(GL_TRIANGLE_FAN);
		glVertex3f( 0.0174f, -0.00413f, 1.0f);
		glVertex3f(-1.0f, -1.0f,  1.0f);
		glVertex3f( 1.0f, -1.0f, -1.0f);
		glVertex3f( 1.0f,  1.0f,  1.0f);
		glVertex3f(-1.0f,  1.0f, -1.0f);
		glVertex3f(-1.0f, -1.0f, -1.0f);
	glEnd();

	glDisable(GL_STENCIL_TEST);

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, piglit_winsys_fbo);
	glClearColor(1.0, 0.0, 1.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
	glBlitFramebuffer(0, 0, TEX_WIDTH, TEX_HEIGHT, 0, 0, TEX_WIDTH, TEX_HEIGHT,
			  GL_COLOR_BUFFER_BIT, GL_NEAREST);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, piglit_winsys_fbo);

	pass = piglit_probe_rect_rgb(0, 0, TEX_WIDTH, TEX_HEIGHT, black) && pass;

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

GLuint create_fbo(unsigned num_samples)
{
	GLenum tex_target;
	GLuint texColor;
	GLuint texZS;
	GLuint fbo;

	/* setup an fbo with (optionally multisample) textures */
	glGenTextures(1, &texColor);
	glGenTextures(1, &texZS);

	if (num_samples != 0) {
		tex_target = GL_TEXTURE_2D_MULTISAMPLE;

		glBindTexture(tex_target, texZS);
		glTexImage2DMultisample(
			tex_target, num_samples, GL_DEPTH32F_STENCIL8,
			TEX_WIDTH, TEX_HEIGHT, GL_TRUE);

		glBindTexture(tex_target, texColor);
		glTexImage2DMultisample(
			tex_target, num_samples, GL_RGBA8,
			TEX_WIDTH, TEX_HEIGHT, GL_TRUE);
	} else {
		tex_target = GL_TEXTURE_2D;

		glBindTexture(tex_target, texZS);
		glTexParameteri(tex_target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(tex_target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(tex_target, 0, GL_DEPTH32F_STENCIL8,
			     TEX_WIDTH, TEX_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

		glBindTexture(tex_target, texColor);
		glTexParameteri(tex_target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(tex_target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(tex_target, 0, GL_RGBA8, TEX_WIDTH, TEX_HEIGHT, 0,
			     GL_RGBA, GL_FLOAT, NULL);
	}

	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER,
			GL_DEPTH_STENCIL_ATTACHMENT,
			tex_target, texZS, 0);
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER,
			GL_COLOR_ATTACHMENT0,
			tex_target, texColor, 0);

	if (!piglit_check_gl_error(GL_NO_ERROR)) {
		printf("Error during tex/fbo setup; no point continuing.\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	return fbo;
}

void
piglit_init(int argc, char **argv)
{
	unsigned num_samples = 4;

	piglit_require_extension("GL_ARB_texture_multisample");

	for (int i = 1; i < argc; ++i) {
		if (!strcmp(argv[i], "samples")) {
			++i;
			if (i >= argc)
				usage();
			num_samples = atoi(argv[i]);
		} else
			usage();
	}

	printf("Number of samples: %u\n", num_samples);

	fbo = create_fbo(num_samples);
	fbo_copy = create_fbo(0);
}
