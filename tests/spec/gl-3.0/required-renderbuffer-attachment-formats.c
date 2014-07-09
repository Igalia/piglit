/* Copyright © 2011 Intel Corporation
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
#include "sized-internalformats.h"

/**
 * @file required-renderbuffer-attachment-formats.c
 *
 * Tests that the color-and-texturing required sized internal formats
 * for GL 3.0 are supported as renderbuffer attachments.  See page 294
 * of the GL 3.0 pdf (20080923) "Required Framebuffer Formats" and
 * page 180 "Required Texture Formats".
 */

static int target_version;

enum piglit_result
piglit_display(void)
{
	/* UNREACHED */
	return PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	bool pass = true;
	GLuint rb, fbo;
	int i;

	piglit_require_gl_version(30);

	glGenRenderbuffers(1, &rb);
	glBindRenderbuffer(GL_RENDERBUFFER, rb);

	for (i = 0; required_formats[i].token != GL_NONE; i++) {
		GLenum attachment, status;
		const struct sized_internalformat *f;

		if (!valid_for_gl_version(&required_formats[i], target_version))
			continue;

		if (!required_formats[i].rb_required)
			continue;

		f = get_sized_internalformat(required_formats[i].token);

		if (f->token == GL_DEPTH24_STENCIL8 ||
		    f->token == GL_DEPTH32F_STENCIL8) {
			attachment = GL_DEPTH_STENCIL_ATTACHMENT;
		} else if (get_channel_size(f, D)) {
			attachment = GL_DEPTH_ATTACHMENT;
		} else {
			attachment = GL_COLOR_ATTACHMENT0;
		}

		glRenderbufferStorage(GL_RENDERBUFFER, f->token, 1, 1);

		/* We don't test the sizes of the channels, because
		 * the spec allows the implementation to choose
		 * resolution pretty much however it feels (GL 2.x
		 * texturing-style).  From page 284 of the GL 3.0
		 * spec:
		 *
		 *     "A GL implementation may vary its allocation of
		 *      internal component resolution based on any
		 *      RenderbufferStorage parameter (except target),
		 *      but the allocation and chosen internal format
		 *      must not be a function of any other state and
		 *      cannot be changed once they are established."
		 */

		if (glGetError() != 0) {
			printf("Unexpected error creating %s texture\n",
			       f->name);
			pass = false;
			continue;
		}

		glGenFramebuffers(1, &fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);

		glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachment,
					  GL_RENDERBUFFER, rb);

		if (glGetError() != 0) {
			printf("Unexpected error binding %s texture\n",
			       f->name);
			pass = false;
			continue;
		}

		if (attachment == GL_COLOR_ATTACHMENT0) {
			glDrawBuffer(GL_COLOR_ATTACHMENT0);
		} else {
			glDrawBuffer(GL_NONE);
		}

		status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if (status != GL_FRAMEBUFFER_COMPLETE) {
			fprintf(stderr, "%s fbo incomplete (status = 0x%04x)\n",
				f->name, status);
			pass = false;
		} else {
			printf("%s: fbo complete\n", f->name);
		}

		glDeleteFramebuffers(1, &fbo);
	}

	glDeleteRenderbuffers(1, &rb);

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}

PIGLIT_GL_TEST_CONFIG_BEGIN
	setup_required_size_test(argc, argv, &config);
	target_version = MAX2(config.supports_gl_compat_version,
			      config.supports_gl_core_version);
PIGLIT_GL_TEST_CONFIG_END
