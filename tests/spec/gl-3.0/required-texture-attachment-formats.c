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
 * @file required-texture-attachment-formats.c
 *
 * Tests that the color-and-texturing required sized internal formats
 * for GL 3.0 are supported as texture attachments.  See page 294 of
 * the GL 3.0 pdf (20080923) "Required Framebuffer Formats" and page
 * 180 "Required Texture Formats".
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
	GLuint tex, fbo;
	int i, c;

	piglit_require_gl_version(30);

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);

	for (i = 0; required_formats[i].token != GL_NONE; i++) {
		GLenum format, type, attachment, status;
		const struct sized_internalformat *f;

		if (!valid_for_gl_version(&required_formats[i], target_version))
			continue;

		if (!required_formats[i].rb_required)
			continue;

		f = get_sized_internalformat(required_formats[i].token);

		if (f->token == GL_DEPTH24_STENCIL8 ||
		    f->token == GL_DEPTH32F_STENCIL8) {
			format = GL_DEPTH_STENCIL;
			type = GL_UNSIGNED_INT_24_8;
			attachment = GL_DEPTH_STENCIL_ATTACHMENT;
		} else if (get_channel_size(f, D)) {
			format = GL_DEPTH_COMPONENT;
			type = GL_FLOAT;
			attachment = GL_DEPTH_ATTACHMENT;
		} else {
			format = GL_RGBA;
			type = GL_FLOAT;
			attachment = GL_COLOR_ATTACHMENT0;

			/* Have to specify integer data for integer textures. */
			for (c = R; c <= I; c++) {
				if (get_channel_type(f, c) == GL_UNSIGNED_INT ||
				    get_channel_type(f, c) == GL_INT) {
					format = GL_RGBA_INTEGER;
					type = GL_UNSIGNED_INT;
					break;
				}
			}
		}

		glTexImage2D(GL_TEXTURE_2D, 0, f->token,
			     1, 1, 0,
			     format, type, NULL);

		if (glGetError() != 0) {
			printf("Unexpected error creating %s texture\n",
			       f->name);
			pass = false;
			continue;
		}

		/* Testing of the sizes/types of the channels is left
		 * up to the required-sized-texture-formats test.
		 */

		glGenFramebuffers(1, &fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);

		glFramebufferTexture2D(GL_FRAMEBUFFER,
				       attachment,
				       GL_TEXTURE_2D,
				       tex, 0);

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

		status = glCheckFramebufferStatus(GL_FRAMEBUFFER_EXT);
		if (status != GL_FRAMEBUFFER_COMPLETE) {
			fprintf(stderr, "%s fbo incomplete (status = 0x%04x)\n",
				f->name, status);
			pass = false;
		} else {
			printf("%s: fbo complete\n", f->name);
		}

		glDeleteFramebuffers(1, &fbo);
	}

	glDeleteTextures(1, &tex);

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}

PIGLIT_GL_TEST_CONFIG_BEGIN
	setup_required_size_test(argc, argv, &config);
	target_version = MAX2(config.supports_gl_compat_version,
			      config.supports_gl_core_version);
PIGLIT_GL_TEST_CONFIG_END
