/*
 * Copyright © 2009-2010 Intel Corporation
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

/** @file fbo-generatemipmap-formats.c
 *
 * Tests that glGenerateMipmapEXT works correctly on 2D textures of various
 * internalformats.
 */

#include "piglit-util-gl.h"
#include "fbo-formats.h"

static bool npot;
static int tex_width = 256;
static int tex_height = 256;

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_width = 700;
	config.window_height = 300;
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;

PIGLIT_GL_TEST_CONFIG_END

static void set_npot(bool new_npot)
{
	npot = new_npot;
	if (npot) {
		tex_width = 293;
		tex_height = 277;
	} else {
		tex_width = 256;
		tex_height = 256;
	}
}

static void
key_func(unsigned char key, int x, int y)
{
	switch (key) {
	case 'd':
		if (piglit_is_extension_supported("GL_ARB_texture_non_power_of_two"))
			set_npot(!npot);
		break;
	}
	fbo_formats_key_func(key, x, y);
}

static int
create_tex(GLenum internalformat, GLenum baseformat, GLenum basetype)
{
	GLuint tex;
	int i;
	GLenum type, format;

	if ((baseformat == GL_DEPTH_COMPONENT) || (baseformat == GL_DEPTH_STENCIL)) {
		tex = piglit_depth_texture(GL_TEXTURE_2D, internalformat,
					   tex_width, tex_height, 1, GL_FALSE);
		if (!piglit_check_gl_error(GL_NO_ERROR))
		        piglit_report_result(PIGLIT_FAIL);
		if (internalformat == GL_DEPTH32F_STENCIL8) {
			format = GL_DEPTH_STENCIL;
			type = GL_FLOAT_32_UNSIGNED_INT_24_8_REV;
		} else if (baseformat == GL_DEPTH_STENCIL) {
			format = GL_DEPTH_STENCIL;
			type = GL_UNSIGNED_INT_24_8;
		} else if (baseformat == GL_DEPTH_COMPONENT) {
			format = GL_DEPTH_COMPONENT;
			type = GL_FLOAT;
		} else {
			assert(0);
			format = GL_NONE;
			type = GL_NONE;
		}
	} else {
		tex = piglit_rgbw_texture(internalformat,
					  tex_width, tex_height, GL_FALSE,
					  GL_TRUE, basetype);
		format = GL_RGBA;
		type = GL_FLOAT;
	}

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
			GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
			GL_NEAREST_MIPMAP_NEAREST);

	for (i = 1; (tex_width >> i) || (tex_height >> i); i++) {
		glTexImage2D(GL_TEXTURE_2D, i, internalformat,
			     (tex_width >> i) ? (tex_width >> i) : 1,
			     (tex_height >> i) ? (tex_height >> i) : 1,
			     0,
			     format, type, NULL);
	}
	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	glGenerateMipmapEXT(GL_TEXTURE_2D);

	return tex;
}

static void _glTexEnv4f(GLenum target, GLenum pname,
		       GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
	float v[4] = {x, y, z, w};
	glTexEnvfv(target, pname, v);
}

static void
draw_mipmap(int x, int y, int level, GLenum basetype)
{
	int r, g, b, l, a, d, i;

	glViewport(0, 0, piglit_width, piglit_height);
	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);

	if (piglit_is_extension_supported("GL_ARB_depth_texture")) {
		glGetTexLevelParameteriv(GL_TEXTURE_2D, level,
					 GL_TEXTURE_DEPTH_SIZE, &d);
	} else {
		d = 0;
	}
	glGetTexLevelParameteriv(GL_TEXTURE_2D, level,
				 GL_TEXTURE_LUMINANCE_SIZE, &l);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, level,
				 GL_TEXTURE_ALPHA_SIZE, &a);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, level,
				 GL_TEXTURE_INTENSITY_SIZE, &i);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, level,
				 GL_TEXTURE_RED_SIZE, &r);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, level,
				 GL_TEXTURE_GREEN_SIZE, &g);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, level,
				 GL_TEXTURE_BLUE_SIZE, &b);

	/* Don't expect unclamped values for float depth buffers. */
	if (d && basetype == GL_FLOAT) {
		basetype = GL_UNSIGNED_NORMALIZED;
	}

	glEnable(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	switch (basetype) {
	case GL_UNSIGNED_NORMALIZED:
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB,   GL_REPLACE);
		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_REPLACE);
		break;

	case GL_SIGNED_NORMALIZED:
	case GL_FLOAT:
		/* TEX*CONST + COLOR*(1-CONST)
		 *
		 * Default:
		 *    CONST = 1
		 *
		 * Signed normalized:
		 *    Convert [-1, 1] to [0, 1] using:
		 *    x * 0.5 + 0.5
		 *
		 *    CONST = 0.5
		 *    COLOR = 1
		 *
		 * Float:
		 *    Convert [-5, 5] to [0, 1] using:
		 *    x * 0.1 + 0.5
		 *
		 *    CONST = 0.1
		 *    COLOR = 0.5/0.9
		 */
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB,   GL_INTERPOLATE);
		glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_INTERPOLATE);
		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB,   GL_TEXTURE);
		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA, GL_TEXTURE);
		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB,   GL_PRIMARY_COLOR);
		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_ALPHA, GL_PRIMARY_COLOR);
		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_RGB,   GL_CONSTANT);
		glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE2_ALPHA, GL_CONSTANT);
		glTexEnvf(GL_TEXTURE_ENV, GL_OPERAND0_RGB,   GL_SRC_COLOR);
		glTexEnvf(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA, GL_SRC_ALPHA);
		glTexEnvf(GL_TEXTURE_ENV, GL_OPERAND1_RGB,   GL_SRC_COLOR);
		glTexEnvf(GL_TEXTURE_ENV, GL_OPERAND1_ALPHA, GL_SRC_ALPHA);
		glTexEnvf(GL_TEXTURE_ENV, GL_OPERAND2_RGB,   GL_SRC_COLOR);
		glTexEnvf(GL_TEXTURE_ENV, GL_OPERAND2_ALPHA, GL_SRC_ALPHA);

		if (basetype == GL_FLOAT) {
			glColor4f(0.5/0.9, 0.5/0.9, 0.5/0.9, 0.5/0.9);
			_glTexEnv4f(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR,
				    r||l||i||d ? 0.1 : 1,
				    g||l||i||d ? 0.1 : 1,
				    b||l||i||d ? 0.1 : 1,
				    a||i       ? 0.1 : 1);
		} else {
			glColor4f(1, 1, 1, 1);
			_glTexEnv4f(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR,
				    r||l||i||d ? 0.5 : 1,
				    g||l||i||d ? 0.5 : 1,
				    b||l||i||d ? 0.5 : 1,
				    a||i       ? 0.5 : 1);
		}
		if (!piglit_check_gl_error(GL_NO_ERROR))
		        piglit_report_result(PIGLIT_FAIL);
		break;

	default:
		assert(0);
	}


	piglit_draw_rect_tex(x, y,
			     (tex_width >> level) ? (tex_width >> level) : 1,
			     (tex_height >> level) ? (tex_height >> level) : 1,
			     0, 0, 1, 1);
}

static GLboolean
test_mipmap_drawing(int x, int y, int level, GLuint internalformat)
{
	GLboolean pass = GL_TRUE;
	int w = (tex_width >> level) ? (tex_width >> level) : 1;
	int h = (tex_height >> level) ? (tex_height >> level) : 1;
	int x1 = x, y1 = y, x2 = x + w/2, y2 = y + h/2;
	float r[] = {1, 0, 0, 0};
	float g[] = {0, 1, 0, 0.25};
	float b[] = {0, 0, 1, 0.5};
	float wh[] = {1, 1, 1, 1};
	GLint r_size, g_size, b_size, l_size, a_size, d_size, i_size;
	GLint compressed;

	glGetTexLevelParameteriv(GL_TEXTURE_2D, level,
				 GL_TEXTURE_COMPRESSED, &compressed);
	if (compressed && (w < h ? w : h) < 8)
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
		if (level < 3)
			piglit_set_tolerance_for_bits(8, 8, 8, 8);
		else if (level < 6)
			piglit_set_tolerance_for_bits(7, 7, 7, 7);
		else
			piglit_set_tolerance_for_bits(4, 4, 4, 4);
		for (x1 = x; x1 < x + w; x1++) {
			float val = (x1 - x + 0.5) / w;
			float color[3] = {val, val, val};
			pass = pass && piglit_probe_rect_rgb(x1, y, 1, h,
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
			wh[3] = 1.0;
		}
	} else if (a_size && !r_size && !l_size) {
		r[0] = 0.0;
		r[1] = 0.0;
		r[2] = 0.0;
		g[0] = 0.0;
		g[1] = 0.0;
		g[2] = 0.0;
		b[0] = 0.0;
		b[1] = 0.0;
		b[2] = 0.0;
		wh[0] = 0.0;
		wh[1] = 0.0;
		wh[2] = 0.0;
	} else {
		if (!r_size) {
			r[0] = 0.0;
			wh[0] = 0.0;
		}

		if (!g_size) {
			g[1] = 0.0;
			wh[1] = 0.0;
		}

		if (!b_size) {
			b[2] = 0.0;
			wh[2] = 0.0;
		}
		if (!a_size) {
			r[3] = 1.0;
			g[3] = 1.0;
			b[3] = 1.0;
			wh[3] = 1.0;
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

	if (compressed) {
		l_size = MIN2(7, l_size);
		i_size = MIN2(7, i_size);
		r_size = MIN2(7, r_size);
		g_size = MIN2(7, g_size);
		b_size = MIN2(7, b_size);
		a_size = MIN2(7, a_size);
	}

	if (i_size) {
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
	} else if (internalformat == GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT) {
		/* The texture is uploaded with values in the range
		 * -5->+5 and these get mapped to 0,1 when drawing.
		 * However when compressing to the unsigned float
		 * compressed format the -5 values will get clamped to
		 * 0 which comes out as 0.5 */
		r[1] = r[2] = g[0] = g[2] = b[0] = b[1] = 0.5f;
	}

	if (tex_width == 256) {
		pass = pass && piglit_probe_rect_rgba(x1, y1, w/2, h/2, r);
		pass = pass && piglit_probe_rect_rgba(x2, y1, w/2, h/2, g);
		pass = pass && piglit_probe_rect_rgba(x1, y2, w/2, h/2, b);
		pass = pass && piglit_probe_rect_rgba(x2, y2, w/2, h/2, wh);
	} else if (w > 1 && h > 1) {
		if (compressed) {
			/* DXT1 RGBA blurs the pixels in the NPOT case. */
			if (w <= 7 || h <= 7) {
				return pass;
			}
			pass = pass && piglit_probe_rect_rgba(x1,   y1,   w/2-4, h/2-4, r);
			pass = pass && piglit_probe_rect_rgba(x2+4, y1,   w/2-4, h/2-4, g);
			pass = pass && piglit_probe_rect_rgba(x1,   y2+4, w/2-4, h/2-4, b);
			pass = pass && piglit_probe_rect_rgba(x2+4, y2+4, w/2-4, h/2-4, wh);
		} else {
			/* There may be inaccuracies with NPOT sampling in the middle of the texture. */
			pass = pass && piglit_probe_rect_rgba(x1,   y1,   w/2-1, h/2-1, r);
			pass = pass && piglit_probe_rect_rgba(x2+1, y1,   w/2-1, h/2-1, g);
			pass = pass && piglit_probe_rect_rgba(x1,   y2+1, w/2-1, h/2-1, b);
			pass = pass && piglit_probe_rect_rgba(x2+1, y2+1, w/2-1, h/2-1, wh);
		}
	}

	return pass;
}

static GLboolean
test_format(const struct format_desc *format, GLenum basetype)
{
	GLuint tex;
	int x;
	int level;
	GLboolean pass = GL_TRUE;

	if (basetype == GL_INT) {
		printf("Skipping mipmap generation for integer texture.\n");
		return GL_TRUE;
	}

	printf("Testing %s%s\n", format->name, tex_width == 256 ? "" : " (NPOT)");
	tex = create_tex(format->internalformat, format->base_internal_format,
			 basetype);

	x = 1;
	for (level = 0; (tex_width >> level) || (tex_height >> level); level++) {
		draw_mipmap(x, 1, level, basetype);
		x += (tex_width >> level) + 1;
	}

	x = 1;
	for (level = 0; (tex_width >> level) || (tex_height >> level); level++) {
		pass = pass && test_mipmap_drawing(x, 1, level,
						   format->internalformat);
		x += (tex_width >> level) + 1;
	}

	glDeleteTextures(1, &tex);

	piglit_report_subtest_result(pass ? PIGLIT_PASS : PIGLIT_FAIL,
				     "%s%s", format->name,
				     npot ? " NPOT" : "");

	return pass;
}

enum piglit_result
piglit_display(void)
{
	struct format_desc format;
	GLboolean pass = GL_TRUE;
	int i;
	(void)fbo_formats_display;

	glClearColor(0.5, 0.5, 0.5, 0.5);
	glClear(GL_COLOR_BUFFER_BIT);

	if (piglit_automatic) {
		for (i = 0; i < test_sets[test_index].num_formats; i++) {
			format = test_sets[test_index].format[i];
			/* Skip testing textures with depth-stencil internal
			 * formats as they are not allowed in glGenerateMipmap.
			 */
			if (format.base_internal_format == GL_DEPTH_STENCIL ||
			    format.base_internal_format == GL_STENCIL_INDEX)
				continue;

			pass = test_format(&format,
					   test_sets[test_index].basetype) && pass;
		}
		if (piglit_is_extension_supported("GL_ARB_texture_non_power_of_two")) {
			set_npot(GL_TRUE);
			for (i = 0; i < test_sets[test_index].num_formats; i++) {
				format = test_sets[test_index].format[i];
				if (format.base_internal_format == GL_DEPTH_STENCIL ||
				    format.base_internal_format == GL_STENCIL_INDEX)
					continue;

				pass = test_format(&format,
						   test_sets[test_index].basetype) && pass;
			}
			set_npot(GL_FALSE);
		}
	} else {
		format = test_sets[test_index].format[format_index];
		if (format.base_internal_format != GL_DEPTH_STENCIL &&
		    format.base_internal_format != GL_STENCIL_INDEX)
			pass = test_format(&format,
					   test_sets[test_index].basetype);
	}

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}


void piglit_init(int argc, char **argv)
{
	fbo_formats_init(argc, argv, GL_FALSE);

	if (!piglit_automatic) {
		piglit_set_keyboard_func(key_func);
		printf("    -n   Next test set.\n"
		       "    -N   Previous test set.\n"
		       "    -m   Next format in the set.\n"
		       "    -M   Previous format in the set.\n"
		       "    -d   Switch between POT and NPOT\n");
	}
}
