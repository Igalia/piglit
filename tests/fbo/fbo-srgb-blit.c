/*
 * Copyright © 2011 Henri Verbeet <hverbeet@gmail.com>
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
 *
 */

/** @file fbo-srgb-blit.c
 *
 * Test FBO blits between sRGB and linear textures. Blits should happen in
 * linear color space.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGB;

PIGLIT_GL_TEST_CONFIG_END

static GLuint src_tex, dst_tex;
static GLuint src_fbo, dst_fbo;
static uint32_t *tex_data;
static bool has_fb_srgb;

static void blit_rect(GLenum src_format, GLenum dst_format, float x, float y, float w, float h, bool stretch)
{
	glBindTexture(GL_TEXTURE_2D, src_tex);
	glTexImage2D(GL_TEXTURE_2D, 0, src_format, 16, 16, 0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, tex_data);

	glBindTexture(GL_TEXTURE_2D, dst_tex);
	glTexImage2D(GL_TEXTURE_2D, 0, dst_format, 16, 16, 0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, NULL);

	glBindFramebufferEXT(GL_READ_FRAMEBUFFER_EXT, src_fbo);
	glFramebufferTexture2DEXT(GL_READ_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, src_tex, 0);

	if (has_fb_srgb)
		glEnable(GL_FRAMEBUFFER_SRGB_EXT);
	glBindFramebufferEXT(GL_DRAW_FRAMEBUFFER_EXT, dst_fbo);
	glFramebufferTexture2DEXT(GL_DRAW_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, dst_tex, 0);

	if (stretch)
	{
		glBlitFramebufferEXT(7, 7, 9, 9, 0, 0, 8, 16, GL_COLOR_BUFFER_BIT, GL_LINEAR);
		if (has_fb_srgb)
			glDisable(GL_FRAMEBUFFER_SRGB_EXT);
		glBlitFramebufferEXT(7, 7, 9, 9, 8, 0, 16, 16, GL_COLOR_BUFFER_BIT, GL_LINEAR);
	}
	else
	{
		glBlitFramebufferEXT(0, 0, 8, 16, 0, 0, 8, 16, GL_COLOR_BUFFER_BIT, GL_LINEAR);
		if (has_fb_srgb)
			glDisable(GL_FRAMEBUFFER_SRGB_EXT);
		glBlitFramebufferEXT(8, 0, 16, 16, 8, 0, 16, 16, GL_COLOR_BUFFER_BIT, GL_LINEAR);
	}

	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, piglit_winsys_fbo);

	glBindTexture(GL_TEXTURE_2D, dst_tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glEnable(GL_TEXTURE_2D);

	piglit_draw_rect_tex(x, y, w, h, 0.0f, 0.0f, 1.0f, 1.0f);
}

enum piglit_result piglit_display(void)
{
	static const struct
	{
		int x;
		int y;
		float color[3];
	}
	expected[] =
	{
		{ 16, 32, {0.11f, 0.16f, 0.21f}},
		{ 48, 32, {0.11f, 0.16f, 0.21f}},
		{ 80, 32, {0.37f, 0.44f, 0.50f}},
		{112, 32, {0.37f, 0.44f, 0.50f}},
		{ 16, 96, {0.37f, 0.44f, 0.50f}},
		{ 48, 96, {0.37f, 0.44f, 0.50f}},
		{ 80, 96, {0.11f, 0.16f, 0.21f}},
		{112, 96, {0.11f, 0.16f, 0.21f}},
	};
	GLboolean pass = GL_TRUE;
	unsigned int i;

	blit_rect(GL_RGBA8, GL_SRGB8_ALPHA8, -1.0f, -1.0f, 1.0f, 1.0f, false);
	blit_rect(GL_SRGB8_ALPHA8, GL_RGBA8,  0.0f, -1.0f, 1.0f, 1.0f, false);
	blit_rect(GL_SRGB8_ALPHA8, GL_RGBA8, -1.0f,  0.0f, 1.0f, 1.0f, true);
	blit_rect(GL_RGBA8, GL_SRGB8_ALPHA8,  0.0f,  0.0f, 1.0f, 1.0f, true);

	for (i = 0; i < sizeof(expected) / sizeof(*expected); ++i)
	{
		pass &= piglit_probe_pixel_rgb(expected[i].x, expected[i].y, expected[i].color);
	}

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void piglit_init(int argc, char **argv)
{
	unsigned int i;

	piglit_require_extension("GL_EXT_framebuffer_object");
	piglit_require_extension("GL_EXT_framebuffer_blit");
	piglit_require_extension("GL_EXT_texture_sRGB");
	if (piglit_is_extension_supported("GL_EXT_framebuffer_sRGB"))
		has_fb_srgb = true;

	tex_data = malloc(16 * 16 * sizeof(*tex_data));
	for (i = 0; i < 16 * 16; ++i)
	{
		tex_data[i] = 0xff5f6f7f;
	}

	glGenTextures(1, &src_tex);
	glGenTextures(1, &dst_tex);
	glGenFramebuffersEXT(1, &src_fbo);
	glGenFramebuffersEXT(1, &dst_fbo);
}
