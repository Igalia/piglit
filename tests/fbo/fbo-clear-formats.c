/*
 * Copyright © 2009-2011 Intel Corporation
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
 *    Eric Anholt <eric@anholt.net>
 *
 */

/** @file fbo-clear-formats.c
 *
 * Tests that glClear works correctly on all levels of 2D
 * texture-based FBOs of various internalformats.
 */

#include "piglit-util-gl.h"
#include "fbo-formats.h"

#define TEX_WIDTH 256
#define TEX_HEIGHT 256

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_width = 700;
	config.window_height = 300;
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_STENCIL |
			       PIGLIT_GL_VISUAL_RGBA;

PIGLIT_GL_TEST_CONFIG_END

static bool clear_stencil = false;

/* Do piglit_rgbw_texture() image but using glClear */
static bool
do_rgba_clear(GLenum format, GLuint tex, int level, int size)
{
	float red[4]   = {1.0, 0.0, 0.0, 0.0};
	float green[4] = {0.0, 1.0, 0.0, 0.25};
	float blue[4]  = {0.0, 0.0, 1.0, 0.5};
	float white[4] = {1.0, 1.0, 1.0, 1.0};
	float black[4] = {0.0, 0.0, 0.0, 0.0};
	float *color;
	GLuint fb;
	GLenum status;

	glGenFramebuffersEXT(1, &fb);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fb);

	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
				  GL_COLOR_ATTACHMENT0_EXT,
				  GL_TEXTURE_2D,
				  tex,
				  level);

	status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
	if (status != GL_FRAMEBUFFER_COMPLETE_EXT) {
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, piglit_winsys_fbo);
		glDeleteFramebuffersEXT(1, &fb);
		return false;
	}

	/* Handle the small sizes of compressed mipmap blocks */
	switch (format) {
	case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:
	case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
	case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:
	case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:
	case GL_COMPRESSED_RGB_FXT1_3DFX:
	case GL_COMPRESSED_RGBA_FXT1_3DFX:
		if (size == 4)
			color = red;
		else if (size == 2)
			color = green;
		else if (size == 1)
			color = blue;
		else {
			assert(0);
			color = black;
		}
		glClearColor(color[0], color[1], color[2], color[3]);
		glClear(GL_COLOR_BUFFER_BIT);
		return true;
	}

	glEnable(GL_SCISSOR_TEST);

	glScissor(0, 0, size / 2, size / 2);
	glClearColor(red[0], red[1], red[2], red[3]);
	glClear(GL_COLOR_BUFFER_BIT);

	glScissor(size / 2, 0, size / 2, size / 2);
	glClearColor(green[0], green[1], green[2], green[3]);
	glClear(GL_COLOR_BUFFER_BIT);

	glScissor(0, size / 2, size / 2, size / 2);
	glClearColor(blue[0], blue[1], blue[2], blue[3]);
	glClear(GL_COLOR_BUFFER_BIT);

	glScissor(size / 2, size / 2, size / 2, size / 2);
	glClearColor(white[0], white[1], white[2], white[3]);
	glClear(GL_COLOR_BUFFER_BIT);

	glDisable(GL_SCISSOR_TEST);

	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, piglit_winsys_fbo);
	glDeleteFramebuffersEXT(1, &fb);

	return true;
}

static bool
do_depth_clear(GLenum format, GLuint tex, int level, int size)
{
	GLuint fb;
	GLenum status;
	GLint draw_buffer, read_buffer;
	int x;

	glGetIntegerv(GL_DRAW_BUFFER, &draw_buffer);
	glGetIntegerv(GL_READ_BUFFER, &read_buffer);

	glGenFramebuffersEXT(1, &fb);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fb);

	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
				  GL_DEPTH_ATTACHMENT_EXT,
				  GL_TEXTURE_2D,
				  tex,
				  level);

	status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
	if (status != GL_FRAMEBUFFER_COMPLETE_EXT) {
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, piglit_winsys_fbo);
		glDeleteFramebuffersEXT(1, &fb);
		return false;
	}

	glEnable(GL_SCISSOR_TEST);

	for (x = 0; x < size; x++) {
		float val = (x + 0.5) / (size);
		glScissor(x, 0, 1, size);
		glClearDepth(val);
		glClear(GL_DEPTH_BUFFER_BIT);
	}

	glDisable(GL_SCISSOR_TEST);

	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, piglit_winsys_fbo);
	glDeleteFramebuffersEXT(1, &fb);

	glDrawBuffer(draw_buffer);
	glReadBuffer(read_buffer);

	return true;
}

static bool
do_stencil_clear(GLenum format, GLuint tex, int level, int size)
{
	GLuint fb;
	GLenum status;
	GLint draw_buffer, read_buffer;
	int x;

	glGetIntegerv(GL_DRAW_BUFFER, &draw_buffer);
	glGetIntegerv(GL_READ_BUFFER, &read_buffer);

	glGenFramebuffersEXT(1, &fb);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fb);

	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
				  GL_DEPTH_STENCIL_ATTACHMENT,
				  GL_TEXTURE_2D,
				  tex,
				  level);

	status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
	if (status != GL_FRAMEBUFFER_COMPLETE_EXT) {
		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, piglit_winsys_fbo);
		glDeleteFramebuffersEXT(1, &fb);
		return false;
	}

	glEnable(GL_SCISSOR_TEST);

	for (x = 0; x < size; x++) {
		unsigned val = ((x + 0.5) / (size)) * 0xff;
		glScissor(x, 0, 1, size);
		glClearStencil(val);
		glClear(GL_STENCIL_BUFFER_BIT);
	}

	glDisable(GL_SCISSOR_TEST);

	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, piglit_winsys_fbo);
	glDeleteFramebuffersEXT(1, &fb);

	glDrawBuffer(draw_buffer);
	glReadBuffer(read_buffer);

        if (!piglit_check_gl_error(GL_NO_ERROR)) {
		/* Should be no error at this point.  If there is, report failure */
		piglit_report_result(PIGLIT_FAIL);
        }

	return true;
}

static GLuint
create_tex(GLenum internalformat, GLenum baseformat)
{
	GLuint tex;
	int level, dim;
	GLenum type, format;

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);

	if (internalformat == GL_DEPTH32F_STENCIL8) {
		format = GL_DEPTH_STENCIL;
		type = GL_FLOAT_32_UNSIGNED_INT_24_8_REV;
	} else if (baseformat == GL_DEPTH_COMPONENT) {
		format = GL_DEPTH_COMPONENT;
		type = GL_FLOAT;
	} else if (baseformat == GL_DEPTH_STENCIL) {
		format = GL_DEPTH_STENCIL_EXT;
		type = GL_UNSIGNED_INT_24_8_EXT;
	} else {
		format = GL_RGBA;
		type = GL_FLOAT;
	}

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
			GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
			GL_LINEAR_MIPMAP_NEAREST);

	for (level = 0, dim = TEX_WIDTH; dim > 0; level++, dim /= 2) {
		glTexImage2D(GL_TEXTURE_2D, level, internalformat,
			     dim, dim,
			     0,
			     format, type, NULL);
	}

	for (level = 0, dim = TEX_WIDTH; dim > 0; level++, dim /= 2) {
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, level);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, level);

		if (clear_stencil) {
			if (!do_stencil_clear(format, tex, level, dim)) {
				glDeleteTextures(1, &tex);
				return 0;
			}
		} else if (baseformat == GL_DEPTH_COMPONENT ||
			   baseformat == GL_DEPTH_STENCIL) {
			if (!do_depth_clear(format, tex, level, dim)) {
				glDeleteTextures(1, &tex);
				return 0;
			}
		} else {
			if (!do_rgba_clear(format, tex, level, dim)) {
				glDeleteTextures(1, &tex);
				return 0;
			}
		}
	}

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, level - 1);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);

        if (!piglit_check_gl_error(GL_NO_ERROR)) {
		/* Should be no error at this point.  If there is, report failure */
		piglit_report_result(PIGLIT_FAIL);
        }
           
	return tex;
}

static void
draw_mipmap(int x, int y, int dim)
{
	glEnable(GL_TEXTURE_2D);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	piglit_draw_rect_tex(x, y, dim, dim,
			     0, 0, 1, 1);

	glDisable(GL_TEXTURE_2D);
}

static void
draw_stencil_mipmap(int x, int y, int dim, GLuint tex, GLuint level)
{
	GLuint fbo;
	GLint draw_buffer, read_buffer;

	glGetIntegerv(GL_DRAW_BUFFER, &draw_buffer);
	glGetIntegerv(GL_READ_BUFFER, &read_buffer);

	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);

	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
			       GL_TEXTURE_2D, tex, level);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, piglit_winsys_fbo);
        glWindowPos2i(x, y);
        glCopyPixels(0, 0, dim, dim, GL_STENCIL);
        if (!piglit_check_gl_error(GL_NO_ERROR)) {
		/* The blit shouldn't generate an error.  If it does, report failure */
		piglit_report_result(PIGLIT_FAIL);
        }

	glBindFramebuffer(GL_FRAMEBUFFER, piglit_winsys_fbo);
	glDeleteFramebuffers(1, &fbo);

	glDrawBuffer(draw_buffer);
	glReadBuffer(read_buffer);
}

static void
visualize_stencil()
{
	unsigned i;

	glEnable(GL_STENCIL_TEST);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
	for (i = 0; i <= 0xff; i++) {
		glStencilFunc(GL_EQUAL, i, ~0);
		glColor4ub(i, i, i, 255);
		piglit_draw_rect(0, 0, piglit_width, piglit_height);
	}
	glDisable(GL_STENCIL_TEST);
	glColor4ub(255, 255, 255, 255);
}

static GLboolean
test_mipmap_drawing(int x, int y, int dim, int level, GLuint internalformat)
{
	GLboolean pass = GL_TRUE;
	int half = dim / 2;
	int x1 = x, y1 = y, x2 = x + half, y2 = y + half;
	float r[] = {1, 0, 0, 0};
	float g[] = {0, 1, 0, 0.25};
	float b[] = {0, 0, 1, 0.5};
	float w[] = {1, 1, 1, 1};
	GLint r_size, g_size, b_size, l_size, a_size, d_size, i_size;
	GLint compressed;

	glGetTexLevelParameteriv(GL_TEXTURE_2D, level,
				 GL_TEXTURE_COMPRESSED, &compressed);
	if (compressed && dim < 8)
		return GL_TRUE;

	if (piglit_is_extension_supported("GL_ARB_depth_texture")) {
		glGetTexLevelParameteriv(GL_TEXTURE_2D, level,
					 GL_TEXTURE_DEPTH_SIZE, &d_size);
	} else {
		d_size = 0;
	}
	glGetTexLevelParameteriv(GL_TEXTURE_2D, level,
				 GL_TEXTURE_LUMINANCE_SIZE, &l_size);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, level,
				 GL_TEXTURE_ALPHA_SIZE, &a_size);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, level,
				 GL_TEXTURE_INTENSITY_SIZE, &i_size);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, level,
				 GL_TEXTURE_RED_SIZE, &r_size);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, level,
				 GL_TEXTURE_GREEN_SIZE, &g_size);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, level,
				 GL_TEXTURE_BLUE_SIZE, &b_size);

	if (d_size) {
		for (x1 = x; x1 < x + dim; x1++) {
			float val = (x1 - x + 0.5) / (dim);
			float color[3] = {val, val, val};
			pass = pass && piglit_probe_rect_rgb(x1, y, 1, dim,
							     color);
		}
		return pass;
	}

	if (i_size || l_size) {
		r[0] = 1.0;
		r[1] = 1.0;
		r[2] = 1.0;

		g[0] = 0.0;
		g[1] = 0.0;
		g[2] = 0.0;

		b[0] = 0.0;
		b[1] = 0.0;
		b[2] = 0.0;

		if (i_size) {
			r[3] = 1.0;
			g[3] = 0.0;
			b[3] = 0.0;
		} else if (l_size && !a_size) {
			r[3] = 1.0;
			g[3] = 1.0;
			b[3] = 1.0;
			w[3] = 1.0;
		}
	} else if (a_size && !r_size && !l_size) {
		r[0] = 1.0;
		r[1] = 1.0;
		r[2] = 1.0;
		g[0] = 1.0;
		g[1] = 1.0;
		g[2] = 1.0;
		b[0] = 1.0;
		b[1] = 1.0;
		b[2] = 1.0;
	} else {
		if (!r_size) {
			r[0] = 0.0;
			w[0] = 0.0;
		}

		if (!g_size) {
			g[1] = 0.0;
			w[1] = 0.0;
		}

		if (!b_size) {
			b[2] = 0.0;
			w[2] = 0.0;
		}
		if (!a_size) {
			r[3] = 1.0;
			g[3] = 1.0;
			b[3] = 1.0;
			w[3] = 1.0;
		}
	}

	/* Clamp the bits for the framebuffer, except we aren't checking
	 * the actual framebuffer bits.
	 */
	if (l_size > 8)
		l_size = 8;
	if (i_size > 8)
		i_size = 8;
	if (r_size > 8)
		r_size = 8;
	if (g_size > 8)
		g_size = 8;
	if (b_size > 8)
		b_size = 8;
	if (a_size > 8)
		a_size = 8;

	if (d_size) {
		piglit_set_tolerance_for_bits(8, 8, 8, 8);
	} else if (i_size) {
		piglit_set_tolerance_for_bits(i_size, i_size, i_size, i_size);
	} else if (l_size) {
		piglit_set_tolerance_for_bits(l_size, l_size, l_size, a_size);
	} else {
		piglit_set_tolerance_for_bits(r_size, g_size, b_size, a_size);
	}

	if (internalformat == GL_COMPRESSED_RGBA_S3TC_DXT1_EXT ||
	    internalformat == GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT) {
		/* If alpha in DXT1 is < 0.5, the whole pixel should be black. */
		r[0] = r[1] = r[2] = r[3] = 0;
		g[0] = g[1] = g[2] = g[3] = 0;
		/* If alpha in DXT1 is >= 0.5, it should be white. */
		b[3] = 1;
	}

	pass = pass && piglit_probe_rect_rgba(x1, y1, half, half, r);
	pass = pass && piglit_probe_rect_rgba(x2, y1, half, half, g);
	pass = pass && piglit_probe_rect_rgba(x1, y2, half, half, b);
	pass = pass && piglit_probe_rect_rgba(x2, y2, half, half, w);

	return pass;
}

static enum piglit_result
test_format(const struct format_desc *format)
{
	int dim;
	GLuint tex;
	int x;
	int level;
	GLboolean pass = GL_TRUE;

	printf("Testing %s", format->name);

	if (clear_stencil && format->base_internal_format != GL_DEPTH_STENCIL) {
		printf(" - no stencil.\n");
		return PIGLIT_SKIP;
	}

	tex = create_tex(format->internalformat, format->base_internal_format);
	if (tex == 0) {
		printf(" - FBO incomplete\n");
		piglit_report_subtest_result(PIGLIT_SKIP,
					     "%s (fbo incomplete)",
					     format->name);
		return PIGLIT_SKIP;
	}
	printf("\n");

	if (clear_stencil) {
		glClearStencil(0x0);
		glClear(GL_STENCIL_BUFFER_BIT);
	}

	glViewport(0, 0, piglit_width, piglit_height);
	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);

	x = 1;
	level = 0;
	for (dim = TEX_WIDTH; dim > 1; dim /= 2) {
		if (clear_stencil)
			draw_stencil_mipmap(x, 1, dim, tex, level);
		else
			draw_mipmap(x, 1, dim);
		x += dim + 1;
		level++;
	}

	if (clear_stencil)
		visualize_stencil();

	x = 1;
	level = 0;
	for (dim = TEX_WIDTH; dim > 1; dim /= 2) {
		pass = pass && test_mipmap_drawing(x, 1, dim, level,
						   format->internalformat);
		x += dim + 1;
		level++;
	}

	glDeleteTextures(1, &tex);

	piglit_report_subtest_result(pass ? PIGLIT_PASS : PIGLIT_FAIL,
				     "%s", format->name);
	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

enum piglit_result piglit_display(void)
{
	return fbo_formats_display(test_format);
}

void piglit_init(int argc, char **argv)
{
	if (argc == 3 && strcmp(argv[2], "stencil") == 0)
		clear_stencil = true;

	if (clear_stencil)
		piglit_require_extension("GL_ARB_framebuffer_object");

	fbo_formats_init(clear_stencil ? 2 : argc, argv, GL_TRUE);
}
