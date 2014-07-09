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
 * @file renderbuffer-samples.c
 *
 * Tests that asking for samples gives the correct number of
 * GL_RENDERBUFFER_SAMPLES.
 *
 * From the EXT_framebuffer_multisample spec:
 *
 *     "If <samples> is zero, then RENDERBUFFER_SAMPLES_EXT is set to
 *      zero.  Otherwise <samples> represents a request for a desired
 *      minimum number of samples. Since different implementations may
 *      support different sample counts for multisampled rendering,
 *      the actual number of samples allocated for the renderbuffer
 *      image is implementation dependent.  However, the resulting
 *      value for RENDERBUFFER_SAMPLES_EXT is guaranteed to be greater
 *      than or equal to <samples> and no more than the next larger
 *      sample count supported by the implementation.
 *
 * Note also this issue:
 *
 * "    (2)  What happens when <samples> is zero or one?
 *
 *           RESOLVED, 0 = single sample, 1 = minimum multisample
 *
 *           Resolved by consensus, May 9, 2005
 *
 *           Zero means single sample, as if RenderbufferStorageEXT
 *           had been called instead of
 *           RenderbufferStorageMultisampleEXT.  One means minimum
 *           number of samples supported by implementation.
 *
 *           There was a question if one should mean the same thing as
 *           single-sample (one sample), or if it should mean the
 *           minimum supported number of samples for multisample
 *           rendering.  The rules for rasterizing in "multisample"
 *           mode are different than "non-multisample" mode.  In the
 *           end, we decided that some implementations may wish to
 *           support a "one-sample" multisample buffer to allow for
 *           multipass multisampling where the sample location can be
 *           varied either by the implementation or perhaps explicitly
 *           by a "multisample location" extension."
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
	GLint max_samples, samples, prev_rb_samples = 0;
	GLuint rb;
	bool pass = true;

	piglit_require_extension("GL_EXT_framebuffer_multisample");

	glGetIntegerv(GL_MAX_SAMPLES, &max_samples);

	glGenRenderbuffersEXT(1, &rb);
	glBindRenderbufferEXT(GL_RENDERBUFFER, rb);
	printf("%10s %10s\n", "requested", "result");
	for (samples = 0; samples <= max_samples; samples++) {
		GLint rb_samples;

		glRenderbufferStorageMultisampleEXT(GL_RENDERBUFFER,
						    samples,
						    GL_RGBA,
						    1, 1);

		glGetRenderbufferParameterivEXT(GL_RENDERBUFFER,
						GL_RENDERBUFFER_SAMPLES,
						&rb_samples);

		if ((rb_samples < prev_rb_samples) ||
		    (samples == 0 && rb_samples != 0) ||
		    (samples > 0 && rb_samples < samples)) {
			fprintf(stderr, "%10d %10d (ERROR)\n", samples, rb_samples);
			pass = false;
		} else {
			printf("%10d %10d\n", samples, rb_samples);
		}

		prev_rb_samples = rb_samples;
	}
	glDeleteRenderbuffersEXT(1, &rb);

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}
