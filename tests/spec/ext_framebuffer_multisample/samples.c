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
 *     "The values of SAMPLE_BUFFERS and SAMPLES are derived from the
 *      attachments of the currently bound framebuffer object.  If the
 *      current DRAW_FRAMEBUFFER_BINDING_EXT is not "framebuffer
 *      complete", then both SAMPLE_BUFFERS and SAMPLES are undefined.
 *      Otherwise, SAMPLES is equal to the value of
 *      RENDERBUFFER_SAMPLES_EXT for the attached images (which all
 *      must have the same value for RENDERBUFFER_SAMPLES_EXT).
 *      Further, SAMPLE_BUFFERS is one if SAMPLES is non-zero.
 *      Otherwise, SAMPLE_BUFFERS is zero."
 *
 * See also negative-mismatched-samples.c.
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

void
piglit_init(int argc, char **argv)
{
	GLint max_samples, sample_buffers, samples, rb_samples;
	GLuint rb, fb;
	GLenum status;
	bool pass = true;
	int i;

	piglit_require_extension("GL_EXT_framebuffer_multisample");

	glGetIntegerv(GL_MAX_SAMPLES, &max_samples);

	glGenFramebuffersEXT(1, &fb);
	glBindFramebufferEXT(GL_FRAMEBUFFER, fb);

	glDrawBuffer(GL_COLOR_ATTACHMENT0);
	glReadBuffer(GL_COLOR_ATTACHMENT0);

	for (i = 0; i < max_samples; i++) {
		glGenRenderbuffersEXT(1, &rb);
		glBindRenderbufferEXT(GL_RENDERBUFFER, rb);
		glRenderbufferStorageMultisampleEXT(GL_RENDERBUFFER,
						    max_samples,
						    GL_RGBA, 1, 1);

		glFramebufferRenderbufferEXT(GL_FRAMEBUFFER,
					     GL_COLOR_ATTACHMENT0,
					     GL_RENDERBUFFER, rb);

		status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER);
		if (status != GL_FRAMEBUFFER_COMPLETE_EXT) {
			fprintf(stderr, "FBO incomplete\n");
			piglit_report_result(PIGLIT_FAIL);
		}

		glGetRenderbufferParameterivEXT(GL_RENDERBUFFER,
						GL_RENDERBUFFER_SAMPLES,
						&rb_samples);

		glGetIntegerv(GL_SAMPLES, &samples);
		if (rb_samples != samples) {
			fprintf(stderr,
				"FBO reported GL_SAMPLES %d for "
				"rb samples %d\n",
				samples, rb_samples);
			pass = false;
		}

		glGetIntegerv(GL_SAMPLE_BUFFERS, &sample_buffers);
		if ((rb_samples != 0) != (sample_buffers == 1)) {
			fprintf(stderr,
				"FBO reported GL_SAMPLE_BUFFERS %d for "
				"rb samples %d\n",
				sample_buffers, samples);
			pass = false;
		}

		glDeleteRenderbuffersEXT(1, &rb);
	}

	glDeleteFramebuffersEXT(1, &fb);

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}
