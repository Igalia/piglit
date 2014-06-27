/*
 * Copyright 2014 Intel Corporation
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

/** @file simple.c
 *
 * A simple test of glCopyImageSubData that copies a square from one
 * 2D texture to another and back.  This test exercises texture to texture,
 * texture to renderbuffer, renderbuffer to texture, and renderbuffer to
 * renderbuffer copies.  This test also exercises copying from one texture
 * or renderbuffer to the same texture or renderbuffer
 */
#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 13;

	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

static const float green[3] = {0.0, 1.0, 0.0};
static const float red[3] = {1.0, 0.0, 0.0};
static const float blue[3] = {0.0, 0.0, 1.0};

int renderbuffers = 0;

void
piglit_init(int argc, char **argv)
{
	int i;

	piglit_require_extension("GL_ARB_copy_image");
	piglit_require_extension("GL_EXT_framebuffer_object");

	for (i = 0; i < argc; ++i) {
		if (strcmp(argv[i], "--tex-to-tex") == 0)
			renderbuffers = 0;
		else if (strcmp(argv[i], "--tex-to-rb") == 0)
			renderbuffers = 1;
		else if (strcmp(argv[i], "--rb-to-rb") == 0)
			renderbuffers = 2;
	}
}

struct image {
	GLuint name, fbo;
	GLenum target;
};

static void
image_init(struct image *image, GLenum target, GLenum internalformat)
{
	image->target = target;

	glGenFramebuffersEXT(1, &image->fbo);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, image->fbo);

	if (image->target == GL_RENDERBUFFER_EXT) {
		glGenRenderbuffersEXT(1, &image->name);
		glBindRenderbufferEXT(GL_RENDERBUFFER, image->name);
		glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT,
					 internalformat, 64, 64);
		glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT,
					     GL_COLOR_ATTACHMENT0_EXT,
					     image->target, image->name);
	} else {
		glGenTextures(1, &image->name);
		glBindTexture(image->target, image->name);
		glTexImage2D(image->target, 0, internalformat,
			     64, 64, 0, internalformat, GL_BYTE, NULL);
		glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
					  GL_COLOR_ATTACHMENT0_EXT,
					  image->target, image->name, 0);
	}
}

static void
image_destroy(struct image *image)
{
	if (image->target == GL_RENDERBUFFER_EXT)
		glDeleteRenderbuffersEXT(1, &image->name);
	else
		glDeleteTextures(1, &image->name);

	glDeleteFramebuffers(1, &image->fbo);
}

static void
image_fill(struct image *image, const float *color)
{
	glClearColor(color[0], color[1], color[2], 1.0f);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, image->fbo);
	glClear(GL_COLOR_BUFFER_BIT);
}

static void
image_draw(struct image *image, int x, int y)
{
	if (image->target == GL_RENDERBUFFER_EXT) {
		glBindFramebufferEXT(GL_READ_FRAMEBUFFER_EXT, image->fbo);
		glBlitFramebufferEXT(0, 0, 64, 64, x, y, x + 64, y + 64,
				     GL_COLOR_BUFFER_BIT, GL_NEAREST);
	} else {
		/* Set up or GL environment for rendering */
		piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);
		glEnable(GL_TEXTURE_2D);
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

		glBindTexture(image->target, image->name);
		piglit_draw_rect_tex(x, y, 64, 64, 0, 0, 1, 1);

		glDisable(GL_TEXTURE_2D);
	}
}

enum piglit_result
piglit_display(void)
{
	bool pass = true;
	struct image images[2];

	if (renderbuffers >= 1)
		images[0].target = GL_RENDERBUFFER_EXT;
	else
		images[0].target = GL_TEXTURE_2D;

	if (renderbuffers == 2)
		images[1].target = GL_RENDERBUFFER_EXT;
	else
		images[1].target = GL_TEXTURE_2D;

	image_init(&images[0], images[0].target, GL_RGB);
	image_init(&images[1], images[1].target, GL_RGB);

	image_fill(&images[0], green);
	image_fill(&images[1], red);

	glCopyImageSubData(images[0].name, images[0].target, 0, 0, 0, 0,
			   images[1].name, images[1].target, 0, 17, 11, 0,
			   32, 32, 1);
	pass &= piglit_check_gl_error(GL_NO_ERROR);

	/* We should now have a green square on red */
	glBindFramebufferEXT(GL_READ_FRAMEBUFFER_EXT, images[1].fbo);
	pass &= piglit_probe_rect_rgb(17, 11, 32, 32, green);
	pass &= piglit_probe_rect_rgb(0, 0, 64, 11, red);
	pass &= piglit_probe_rect_rgb(0, 11, 17, 32, red);
	pass &= piglit_probe_rect_rgb(49, 11, 15, 32, red);
	pass &= piglit_probe_rect_rgb(0, 43, 64, 21, red);

	image_fill(&images[0], blue);
	glBindFramebufferEXT(GL_READ_FRAMEBUFFER_EXT, images[0].fbo);
	pass &= piglit_probe_rect_rgb(0, 0, 64, 64, blue);

	glCopyImageSubData(images[1].name, images[1].target, 0, 17, 11, 0,
			   images[0].name, images[0].target, 0, 0, 32, 0,
			   32, 32, 1);
	pass &= piglit_check_gl_error(GL_NO_ERROR);

	/* This should be a green square on blue (no red!) */
	glBindFramebufferEXT(GL_READ_FRAMEBUFFER_EXT, images[0].fbo);
	pass &= piglit_probe_rect_rgb(0, 32, 32, 32, green);
	pass &= piglit_probe_rect_rgb(0, 0, 64, 32, blue);
	pass &= piglit_probe_rect_rgb(32, 32, 32, 32, blue);

	glCopyImageSubData(images[0].name, images[0].target, 0, 0, 32, 0,
			   images[0].name, images[0].target, 0, 32, 0, 0,
			   32, 32, 1);
	pass &= piglit_check_gl_error(GL_NO_ERROR);

	/* This should be a blue/green checkerboard */
	glBindFramebufferEXT(GL_READ_FRAMEBUFFER_EXT, images[0].fbo);
	pass &= piglit_probe_rect_rgb(0, 0, 32, 32, blue);
	pass &= piglit_probe_rect_rgb(0, 32, 32, 32, green);
	pass &= piglit_probe_rect_rgb(32, 0, 32, 32, green);
	pass &= piglit_probe_rect_rgb(32, 32, 32, 32, blue);

	if (!piglit_automatic) {
		glBindFramebufferEXT(GL_DRAW_FRAMEBUFFER_EXT,
				     piglit_winsys_fbo);

		image_draw(&images[1], 0, 0);
		image_draw(&images[0], 64, 0);

		piglit_present_results();
	}

	image_destroy(&images[0]);
	image_destroy(&images[1]);

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}
