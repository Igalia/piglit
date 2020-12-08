/*
 * Copyright 2020 Google
 *
 * Based on framebuffer-blit-levels.c, which has
 * Copyright © 2012 Intel Corporation
 *
 * SPDX-License-Identifier: MIT
 */

/** \file framebuffer-blit-mipmap.c
 *
 * This test uses glBlitFramebuffer to generate the mipmap level-by-level.
 *
 * The test operates as follows:
 *
 * - A 2D test texture is created with all miplevels present.
 *
 * - The test texture is populated with a deterministic pattern of data.  For
 *   the first miplevel, this is done by simply uploading the data pattern
 *   using glTexImage2D.  For the rest miplevels, this is done by blitting
 *   from the previous miplevel.
 *
 * - The data in the test texture is then verified.  This is done by a direct
 *   call to glReadPixels().
 */

#include "piglit-util-gl.h"

#define LOG2_SIZE 7
#define SIZE (1 << LOG2_SIZE)
#define NUM_LEVELS (LOG2_SIZE + 1)

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_width = SIZE * 2 + 50;
	config.window_height = SIZE + 50;
	config.window_visual = PIGLIT_GL_VISUAL_RGBA |
	                       PIGLIT_GL_VISUAL_DOUBLE;
	config.khr_no_error_support = PIGLIT_NO_ERRORS;

PIGLIT_GL_TEST_CONFIG_END

static const float red[] =   {1, 0, 0, 1};
static const float green[] = {0, 1, 0, 1};
static const float blue[] =  {0, 0, 1, 1};
static const float white[] = {1, 1, 1, 1};

static bool
probe_mipmap(unsigned x, unsigned y, unsigned level)
{
	unsigned s = SIZE >> level;
	bool pass = true;

	if (s > 1) {
		printf("Testing level %d: %dx%d\n", level, s, s);

		s /= 2;
		pass &= piglit_probe_rect_rgba(x + 0, y + 0, s, s, red);
		pass &= piglit_probe_rect_rgba(x + s, y + 0, s, s, green);
		pass &= piglit_probe_rect_rgba(x + 0, y + s, s, s, blue);
		pass &= piglit_probe_rect_rgba(x + s, y + s, s, s, white);
	} else {
		printf("Skipping level %d: %dx%d\n", level, s, s);
	}

	return pass;
}

static void
draw_mipmap(unsigned x, unsigned y, unsigned level)
{
	const unsigned s = SIZE >> level;
	piglit_draw_rect_tex(x, y, s, s, 0, 0, 1, 1);
}

static GLuint
create_test_texture(void)
{
	GLuint tex;
	GLuint src_fbo;
	GLuint dst_fbo;
	unsigned level;

	tex = piglit_rgbw_texture(GL_RGBA,
			          SIZE,
			          SIZE,
			          GL_FALSE,
			          GL_FALSE,
			          GL_UNSIGNED_BYTE);
	for (level = 1; level < NUM_LEVELS; level++) {
		glTexImage2D(GL_TEXTURE_2D, level,
			     GL_RGBA,
			     SIZE >> level, SIZE >> level,
			     0 /* border */,
			     GL_RGBA,
			     GL_UNSIGNED_BYTE,
			     NULL /* data */);
	}

	glGenFramebuffers(1, &src_fbo);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, src_fbo);

	glGenFramebuffers(1, &dst_fbo);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, dst_fbo);

	/* generate the mipmap */
	for (level = 1; level < NUM_LEVELS; level++) {
		const unsigned src_size = SIZE >> (level - 1);
		const unsigned dst_size = SIZE >> level;

		glFramebufferTexture2D(GL_READ_FRAMEBUFFER,
				       GL_COLOR_ATTACHMENT0,
				       GL_TEXTURE_2D, tex,
				       level - 1);
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER,
				       GL_COLOR_ATTACHMENT0,
				       GL_TEXTURE_2D,
				       tex,
				       level);

		glBlitFramebuffer(0, 0, src_size, src_size,
				  0, 0, dst_size, dst_size,
				  GL_COLOR_BUFFER_BIT, GL_NEAREST);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDeleteFramebuffers(1, &src_fbo);
	glDeleteFramebuffers(1, &dst_fbo);

	return tex;
}

enum piglit_result
piglit_display()
{
	bool pass = true;
	GLuint tex;
	unsigned level;
	unsigned x;

	tex = create_test_texture();

	glBindFramebufferEXT(GL_FRAMEBUFFER, piglit_winsys_fbo);

	glClearColor(0.5, 0.5, 0.5, 0.5);
	glClear(GL_COLOR_BUFFER_BIT);

	glViewport(0, 0, piglit_width, piglit_height);
	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);

	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glTexParameteri(GL_TEXTURE_2D,
			GL_TEXTURE_MIN_FILTER,
			GL_NEAREST_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D,
			GL_TEXTURE_MAG_FILTER,
			GL_NEAREST);
	glEnable(GL_TEXTURE_2D);

	x = 1;
	for (level = 0; level < NUM_LEVELS; level++) {
		draw_mipmap(x, 1, level);
		x += (SIZE >> level) + 1;
	}

	x = 1;
	for (level = 0; level < NUM_LEVELS; level++) {
		pass &= probe_mipmap(x, 1, level);
		x += (SIZE >> level) + 1;
	}

	glDeleteTextures(1, &tex);

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	piglit_require_extension("GL_ARB_framebuffer_object");
}
