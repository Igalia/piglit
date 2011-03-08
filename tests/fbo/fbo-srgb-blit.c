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

#include "piglit-util.h"

int piglit_width = 128;
int piglit_height = 128;
int piglit_window_mode = GLUT_DOUBLE | GLUT_RGB;

static GLuint src_tex, dst_tex;
static GLuint src_fbo, dst_fbo;
static uint32_t *tex_data;

static void blit_rect(GLenum src_format, GLenum dst_format, float x, float y, float w, float h, bool stretch)
{
	glBindTexture(GL_TEXTURE_2D, src_tex);
	glTexImage2D(GL_TEXTURE_2D, 0, src_format, 16, 16, 0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, tex_data);

	glBindTexture(GL_TEXTURE_2D, dst_tex);
	glTexImage2D(GL_TEXTURE_2D, 0, dst_format, 16, 16, 0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, NULL);

	glBindFramebufferEXT(GL_READ_FRAMEBUFFER_EXT, src_fbo);
	glFramebufferTexture2DEXT(GL_READ_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, src_tex, 0);

	glBindFramebufferEXT(GL_DRAW_FRAMEBUFFER_EXT, dst_fbo);
	glFramebufferTexture2DEXT(GL_DRAW_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, dst_tex, 0);

	if (stretch)
		glBlitFramebufferEXT(7, 7, 9, 9, 0, 0, 16, 16, GL_COLOR_BUFFER_BIT, GL_LINEAR);
	else
		glBlitFramebufferEXT(0, 0, 16, 16, 0, 0, 16, 16, GL_COLOR_BUFFER_BIT, GL_LINEAR);

	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

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
		{32, 32, {0.11f, 0.16f, 0.21f}},
		{32, 96, {0.37f, 0.44f, 0.50f}},
		{96, 96, {0.11f, 0.16f, 0.21f}},
		{96, 32, {0.37f, 0.44f, 0.50f}},
	};
	GLboolean pass = GL_TRUE;
	unsigned int i;

	blit_rect(GL_RGBA8, GL_SRGB8_ALPHA8, -1.0f, -1.0f, 1.0f, 1.0f, false);
	blit_rect(GL_SRGB8_ALPHA8, GL_RGBA8, -1.0f,  0.0f, 1.0f, 1.0f, true);
	blit_rect(GL_RGBA8, GL_SRGB8_ALPHA8,  0.0f,  0.0f, 1.0f, 1.0f, true);
	blit_rect(GL_SRGB8_ALPHA8, GL_RGBA8,  0.0f, -1.0f, 1.0f, 1.0f, false);

	for (i = 0; i < sizeof(expected) / sizeof(*expected); ++i)
	{
		pass &= piglit_probe_pixel_rgb(expected[i].x, expected[i].y, expected[i].color);
	}

	glutSwapBuffers();

	return pass ? PIGLIT_SUCCESS : PIGLIT_FAILURE;
}

void piglit_init(int argc, char **argv)
{
	unsigned int i;

	piglit_require_extension("GL_EXT_framebuffer_object");
	piglit_require_extension("GL_EXT_framebuffer_blit");
	piglit_require_extension("GL_EXT_texture_sRGB");

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
