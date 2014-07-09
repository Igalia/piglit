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
 * @file negative-copyteximage.c
 *
 * From the EXT_framebuffer_multisample spec:
 *
 *     "Finally, the behavior of several GL operations is specified
 *      "as if the arguments were passed to CopyPixels."  These
 *      operations include: CopyTex{Sub}Image*, CopyColor{Sub}Table,
 *      and CopyConvolutionFilter*.  INVALID_FRAMEBUFFER_OPERATION_EXT
 *      will be generated if an attempt is made to execute one of
 *      these operations, or CopyPixels, while the object bound to
 *      READ_FRAMEBUFFER_BINDING_EXT (section 4.4) is not "framebuffer
 *      complete" (as defined in section 4.4.4.2).  INVALID_OPERATION
 *      will be generated if the object bound to
 *      READ_FRAMEBUFFER_BINDING_EXT is "framebuffer complete" and the
 *      value of SAMPLE_BUFFERS is greater than zero."
 *
 * The Errors section says that these and ReadPixels report
 * "INVALID_OPERATION_EXT", but that appears to be a typo.
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
	GLint max_samples;
	GLuint rb, fb, tex;
	GLenum status;
	bool pass = true;

	piglit_require_extension("GL_EXT_framebuffer_multisample");

	glGetIntegerv(GL_MAX_SAMPLES, &max_samples);

	glGenFramebuffersEXT(1, &fb);
	glBindFramebufferEXT(GL_FRAMEBUFFER, fb);

	glGenRenderbuffersEXT(1, &rb);
	glBindRenderbufferEXT(GL_RENDERBUFFER, rb);
	glRenderbufferStorageMultisampleEXT(GL_RENDERBUFFER, max_samples,
					    GL_RGBA, 1, 1);

	glFramebufferRenderbufferEXT(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
				     GL_RENDERBUFFER, rb);

	glDrawBuffer(GL_COLOR_ATTACHMENT0);
	glReadBuffer(GL_COLOR_ATTACHMENT0);

	status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE_EXT) {
		fprintf(stderr, "FBO incomplete\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);

	/* Finally, the actual test! */
	glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 0, 0, 1, 1, 0);
	if (!piglit_check_gl_error(GL_INVALID_OPERATION))
		piglit_report_result(PIGLIT_FAIL);

	glDeleteTextures(1, &tex);
	glDeleteRenderbuffersEXT(1, &rb);
	glDeleteFramebuffersEXT(1, &fb);

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}
