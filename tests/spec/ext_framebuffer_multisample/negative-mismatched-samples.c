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

#include "piglit-util-gl.h"

/**
 * @file samples.c
 *
 * From the EXT_framebuffer_multisample spec:
 *
 *     "Modification to 4.4.4.2 (Framebuffer Completeness)
 *
 *          Add an entry to the bullet list:
 *
 *          * The value of RENDERBUFFER_SAMPLES_EXT is the same for all attached
 *            images.
 *            { FRAMEBUFFER_INCOMPLETE_MULTISAMPLE_EXT }"
 */

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;

PIGLIT_GL_TEST_CONFIG_END

enum piglit_result
piglit_display(void)
{
	/* UNREACHED */
	return PIGLIT_FAIL;
}

static bool
test_buffers(GLuint rb0, GLuint samples0,
	     GLuint rb1, GLuint samples1)
{
	GLenum status;

	if (rb0 == rb1)
		return true;

	glFramebufferRenderbufferEXT(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
				     GL_RENDERBUFFER, rb0);
	glFramebufferRenderbufferEXT(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1,
				     GL_RENDERBUFFER, rb1);

	status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER);
	if (samples0 != samples1) {
		if (status != GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE) {
			fprintf(stderr,
				"Framebuffer with %d and %d samples: "
				"reported 0x%x, not "
				"GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE",
				samples0, samples1, status);
			return false;
		}
	} else if (status != GL_FRAMEBUFFER_COMPLETE) {
		fprintf(stderr,
			"Framebuffer with %d and %d samples incomplete: "
			"reported 0x%x, not GL_FRAMEBUFFER_COMPLETE\n",
			samples0, samples1, status);
		return false;
	}
	return true;
}

void
piglit_init(int argc, char **argv)
{
	GLint max_samples, max_draw_buffers;
	GLuint *rb, fb;
	GLint *rb_samples;
	bool pass = true;
	int i, buf0, buf1;
	static const GLenum buffers[2] = {
		GL_COLOR_ATTACHMENT0,
		GL_COLOR_ATTACHMENT1
	};

	piglit_require_extension("GL_EXT_framebuffer_multisample");
	piglit_require_extension("GL_ARB_draw_buffers");

	glGetIntegerv(GL_MAX_DRAW_BUFFERS, &max_draw_buffers);
	if (max_draw_buffers < 2) {
		printf("test requires 2 draw buffers.\n");
		piglit_report_result(PIGLIT_SKIP);
	}

	glGetIntegerv(GL_MAX_SAMPLES, &max_samples);

	rb = malloc(max_samples * sizeof(*rb));
	rb_samples = malloc(max_samples * sizeof(*rb_samples));

	glGenFramebuffersEXT(1, &fb);
	glBindFramebufferEXT(GL_FRAMEBUFFER, fb);

	glDrawBuffers(2, buffers);
	glReadBuffer(GL_COLOR_ATTACHMENT0);

	glGenRenderbuffers(max_samples, rb);

	for (i = 0; i < max_samples; i++) {
		glBindRenderbufferEXT(GL_RENDERBUFFER, rb[i]);
		glRenderbufferStorageMultisampleEXT(GL_RENDERBUFFER,
						    i,
						    GL_RGBA, 1, 1);

		glGetRenderbufferParameterivEXT(GL_RENDERBUFFER,
						GL_RENDERBUFFER_SAMPLES,
						&rb_samples[i]);
	}

	for (buf0 = 0; buf0 < max_samples; buf0++) {
		for (buf1 = 0; buf1 < max_samples; buf1++) {
			pass = test_buffers(rb[buf0], rb_samples[buf0],
					    rb[buf1], rb_samples[buf1]) && pass;
		}
	}

	glDeleteFramebuffersEXT(1, &fb);
	glDeleteRenderbuffersEXT(max_samples, rb);

	free(rb);
	free(rb_samples);

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}
