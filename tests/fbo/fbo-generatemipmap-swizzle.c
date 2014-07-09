/*
 * Copyright Â© 2009 Intel Corporation
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
 * Authors:
 *    Robert Bragg <robert@sixbynine.org>
 *    Eric Anholt <eric@anholt.net>
 *
 */

/** @file fbo-generatemipmap-swizzle.c
 *
 * Tests that glGenerateMipmapEXT works correctly on a 2D texture with a
 * swizzle set via ARB_texture_swizzle
 *
 * Compare this test to fbo-generatemipmap.c.
 */

#include "piglit-util-gl.h"

#define TEX_WIDTH 256
#define TEX_HEIGHT 256

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_width = 700;
	config.window_height = 300;
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGB;

PIGLIT_GL_TEST_CONFIG_END

static const float red[] =   {1, 0, 0, 0};
static const float green[] = {0, 1, 0, 0};

static const GLint swizzle[4] = { GL_ZERO, GL_RED, GL_ZERO, GL_ZERO };

static GLuint
create_fbo(void)
{
	GLuint tex, fb;
	GLenum status;
	int i, dim;

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzle);

	for (i = 0, dim = TEX_WIDTH; dim >0; i++, dim /= 2) {
		glTexImage2D(GL_TEXTURE_2D, i, GL_RED,
			     dim, dim,
			     0,
			     GL_RED, GL_UNSIGNED_BYTE, NULL);
	}
	assert(glGetError() == 0);

	glGenFramebuffersEXT(1, &fb);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fb);
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
				  GL_COLOR_ATTACHMENT0_EXT,
				  GL_TEXTURE_2D,
				  tex,
				  0);
	assert(glGetError() == 0);

	status = glCheckFramebufferStatusEXT (GL_FRAMEBUFFER_EXT);
	if (status != GL_FRAMEBUFFER_COMPLETE_EXT) {
		fprintf(stderr, "FBO incomplete\n");
		goto done;
	}

	glViewport(0, 0, TEX_WIDTH, TEX_HEIGHT);
	piglit_ortho_projection(TEX_WIDTH, TEX_HEIGHT, GL_FALSE);

	glColor4fv(red);
	piglit_draw_rect(0, 0, TEX_WIDTH, TEX_HEIGHT);

	glGenerateMipmapEXT(GL_TEXTURE_2D);
done:
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, piglit_winsys_fbo);
	glDeleteFramebuffersEXT(1, &fb);

	return tex;
}

static void
draw_mipmap(int x, int y, int dim)
{
	glViewport(0, 0, piglit_width, piglit_height);
	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);

	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, piglit_winsys_fbo);

	glEnable(GL_TEXTURE_2D);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	piglit_draw_rect_tex(x, y, dim, dim,
			     0, 0, 1, 1);
}

static GLboolean
test_mipmap_drawing(int start_x, int start_y, int dim)
{
	return piglit_probe_rect_rgb(start_x, start_y, dim, dim, green);
}

enum piglit_result
piglit_display(void)
{
	GLboolean pass = GL_TRUE;
	int dim;
	GLuint tex;
	int x;

	glClearColor(0.5, 0.5, 0.5, 0.5);
	glClear(GL_COLOR_BUFFER_BIT);

	tex = create_fbo();

	x = 1;
	for (dim = TEX_WIDTH; dim > 1; dim /= 2) {
		draw_mipmap(x, 1, dim);
		x += dim + 1;
	}

	x = 1;
	for (dim = TEX_WIDTH; dim > 1; dim /= 2) {
		pass &= test_mipmap_drawing(x, 1, dim);
		x += dim + 1;
	}

	glDeleteTextures(1, &tex);

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void piglit_init(int argc, char **argv)
{
	piglit_require_extension("GL_EXT_framebuffer_object");
	piglit_require_extension("GL_ARB_texture_swizzle");
	piglit_require_extension("GL_ARB_texture_rg");
}
