/*
 * Copyright © 2010 Intel Corporation
 * Copyright © 2010 Marek Olšák <maraeo@gmail.com>
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
 *    Marek Olšák <maraeo@gmail.com>
 *
 */

#include "piglit-util-gl.h"
#include "fbo-formats.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

static void blend(const float *rect, const float *src, const float *dst, const float *blendcol,
                  GLenum blendsrc, GLenum blenddst)
{
	glColor4fv(dst);
	piglit_draw_rect(rect[0], rect[1], rect[2], rect[3]);
	glEnable(GL_BLEND);
	glBlendFunc(blendsrc, blenddst);
	if (blendcol)
		glBlendColor(blendcol[0], blendcol[1], blendcol[2], blendcol[3]);
	glColor4fv(src);
	piglit_draw_rect(rect[0], rect[1], rect[2], rect[3]);
	glDisable(GL_BLEND);
}

#define fix_alpha(a) \
	(format->internalformat == GL_RGB5_A1 ? (a == 1 ? 1.0f : 0.0f) : \
	 format->internalformat == GL_RGB10_A2 ? (a == 1 ? 1.0f : \
						  a >= 0.666 ? 0.666f : \
						  a >= 0.333 ? 0.333f : 0.0f) : \
	 (a))

static int
get_texture_bits(GLenum target, GLenum size_enum, GLenum type_enum)
{
	GLint size = 0;
	GLint type = GL_NONE;
	glGetTexLevelParameteriv(target, 0, size_enum, &size);
	if (!size) {
		return size;
	}
	if (piglit_is_extension_supported("GL_EXT_texture_snorm")) {
		glGetTexLevelParameteriv(target, 0, type_enum, &type);
		if (type == GL_SIGNED_NORMALIZED) {
			/* One bit is lost for the sign */
			size -= 1;
		}
	}
	return size;
}

static enum piglit_result test_format(const struct format_desc *format)
{
	GLboolean pass = GL_TRUE;
	GLuint tex, fb;
	GLenum status;
	int r, g, b, l, a, i;

	float res0[] = {0.3, 0.3, 0.3, 0.0};

	float pos1[] = {-0.66, -1.0, 0.33, 2.0};
        float src1[] = {0.4, 0.9, 0.8, fix_alpha(0.7)};
        float dst1[] = {0.5, 0.4, 0.6, fix_alpha(0.2)};
        float con1[] = {0.2, 0.8, 0.4, fix_alpha(0.6)};
	float res1[] = {dst1[0]*(1-con1[0]) + src1[0]*con1[0],
                        dst1[1]*(1-con1[1]) + src1[1]*con1[1],
                        dst1[2]*(1-con1[2]) + src1[2]*con1[2],
                        dst1[3]*(1-con1[3]) + src1[3]*con1[3]};

	float pos2[] = {-0.33, -1.0, 0.33, 2.0};
	float dst2[] = {0.9, 0.4, 0.7, fix_alpha(0.5)};
        float src2[] = {0.8, 0.3, 0.5, fix_alpha(0.9)};
	float res2[] = {dst2[0]*(1-dst2[0]) + src2[0]*dst2[0],
			dst2[1]*(1-dst2[1]) + src2[1]*dst2[1],
			dst2[2]*(1-dst2[2]) + src2[2]*dst2[2],
			dst2[3]*(1-dst2[3]) + src2[3]*dst2[3]};

	float pos3[] = {0.0, -1.0, 0.33, 2.0};
	float dst3[] = {0.6, 0.4, 0.8, fix_alpha(0.5)};
	float src3[] = {0.8, 0.9, 0.7, fix_alpha(0.8)};
	float res3[] = {dst3[0]*(1-src3[0]) + src3[0]*src3[0],
			dst3[1]*(1-src3[1]) + src3[1]*src3[1],
			dst3[2]*(1-src3[2]) + src3[2]*src3[2],
			dst3[3]*(1-src3[3]) + src3[3]*src3[3]};

	float pos4[] = {0.33, -1.0, 0.33, 2.0};
	float dst4[] = {0.9, 0.4, 0.7, fix_alpha(0.5)};
	float src4[] = {0.8, 0.3, 0.5, fix_alpha(0.9)};
	float res4[] = {dst4[0]*(1-dst4[3]) + src4[0]*dst4[3],
			dst4[1]*(1-dst4[3]) + src4[1]*dst4[3],
			dst4[2]*(1-dst4[3]) + src4[2]*dst4[3],
			dst4[3]*(1-dst4[3]) + src4[3]*dst4[3]};
	float res4i = dst4[0]*(1-dst4[0]) + src4[0]*dst4[0]; /* intensity */
	float res4l = src4[0]; /* luminance without alpha */

	float pos5[] = {0.66, -1.0, 0.33, 2.0};
	float dst5[] = {0.6, 0.4, 0.8, fix_alpha(0.5)};
	float src5[] = {0.8, 0.9, 0.7, fix_alpha(0.8)};
	float res5[] = {dst5[0]*(1-src5[3]) + src5[0]*src5[3],
			dst5[1]*(1-src5[3]) + src5[1]*src5[3],
			dst5[2]*(1-src5[3]) + src5[2]*src5[3],
			dst5[3]*(1-src5[3]) + src5[3]*src5[3]};

        if (format->base_internal_format == GL_DEPTH_COMPONENT ||
            format->base_internal_format == GL_DEPTH_STENCIL)
		return PIGLIT_SKIP;

	glGenFramebuffersEXT(1, &fb);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fb);
	glViewport(0, 0, piglit_width, piglit_height);

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, format->internalformat,
		     piglit_width, piglit_height, 0,
		     GL_RGBA, GL_FLOAT, NULL);

	l = get_texture_bits(GL_TEXTURE_2D,
			     GL_TEXTURE_LUMINANCE_SIZE,
			     GL_TEXTURE_LUMINANCE_TYPE);
	a = get_texture_bits(GL_TEXTURE_2D,
			     GL_TEXTURE_ALPHA_SIZE,
			     GL_TEXTURE_ALPHA_TYPE);
	i = get_texture_bits(GL_TEXTURE_2D,
			     GL_TEXTURE_INTENSITY_SIZE,
			     GL_TEXTURE_INTENSITY_TYPE);
	r = get_texture_bits(GL_TEXTURE_2D,
			     GL_TEXTURE_RED_SIZE,
			     GL_TEXTURE_RED_TYPE);
	g = get_texture_bits(GL_TEXTURE_2D,
			     GL_TEXTURE_GREEN_SIZE,
			     GL_TEXTURE_GREEN_TYPE);
	b = get_texture_bits(GL_TEXTURE_2D,
			     GL_TEXTURE_BLUE_SIZE,
			     GL_TEXTURE_BLUE_TYPE);

	/* Compute expected result colors when reading back from a texture/FBO */
        if (i) {
		/* expected result = (I, 0, 0, 1) */
		res0[1] = res0[2] = 0.0;   res0[3] = 1.0;
		res1[1] = res1[2] = 0.0;   res1[3] = 1.0;
		res2[1] = res2[2] = 0.0;   res2[3] = 1.0;
		res3[1] = res3[2] = 0.0;   res3[3] = 1.0;
		res4[1] = res4[2] = 0.0;   res4[3] = 1.0;   res4[0] = res4i;
		res5[1] = res5[2] = 0.0;   res5[3] = 1.0;
	} else if (l) {
		/* expected result = (L, 0, 0, A) */
		res0[1] = res0[2] = 0.0;
		res1[1] = res1[2] = 0.0;
		res2[1] = res2[2] = 0.0;
		res3[1] = res3[2] = 0.0;
		res4[1] = res4[2] = 0.0;
		res5[1] = res5[2] = 0.0;
		if (!a) {
			res0[3] = 1;
			res1[3] = 1;
			res2[3] = 1;
			res3[3] = 1;
			res4[3] = 1; res4[0] = res4l;
			res5[3] = 1;
		}
        } else {
		if (!r) {
			res0[0] = 0;
			res1[0] = 0;
			res2[0] = 0;
			res3[0] = 0;
			res4[0] = 0;
			res5[0] = 0;
		}
		if (!g) {
			res0[1] = 0;
			res1[1] = 0;
			res2[1] = 0;
			res3[1] = 0;
			res4[1] = 0;
			res5[1] = 0;
		}
		if (!b) {
			res0[2] = 0;
			res1[2] = 0;
			res2[2] = 0;
			res3[2] = 0;
			res4[2] = 0;
			res5[2] = 0;
		}
		if (!a) {
			/* When there are no bits for the alpha channel, we
			 * always expect to read an alpha value of 1.0.
			 */
			res0[3] = 1;
			res1[3] = 1;
			res2[3] = 1;
			res3[3] = 1;
			res4[3] = 1;
			res5[3] = 1;

			/* Also blending with
			 * DST_ALPHA/ONE_MINUS_DST_ALPHA (as in case
			 * 4) with an implicit destination alpha value
			 * of 1.0 means that the result color should
			 * be identical to the source color, (if there
			 * are any bits to store that color that is).
			 */
			if (r)  {
				res4[0] = src4[0];
			}
			if (g) {
				res4[1] = src4[1];
			}
			if (b) {
				res4[2] = src4[2];
			}
		}
        }

	/* Clamp the bits for the framebuffer, except we aren't checking
	 * the actual framebuffer bits.
	 */
	if (l > 8)
		l = 8;
	if (i > 8)
		i = 8;
	if (r > 8)
		r = 8;
	if (g > 8)
		g = 8;
	if (b > 8)
		b = 8;
	if (a > 8)
		a = 8;

	if (format->internalformat == GL_R11F_G11F_B10F) {
		r = 6; /* precision of mantissa */
		g = 6;
		b = 5;
	}

        if (i) {
		piglit_set_tolerance_for_bits(i, i, i, i);
        } else if (l) {
		piglit_set_tolerance_for_bits(l, l, l, a);
        } else {
		piglit_set_tolerance_for_bits(r, g, b, a);
        }

	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
				  GL_COLOR_ATTACHMENT0_EXT,
				  GL_TEXTURE_2D,
				  tex,
				  0);
	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
	printf("Testing %s", format->name);
	if (status != GL_FRAMEBUFFER_COMPLETE_EXT) {
		printf(" - fbo incomplete (status = %s)\n",
		       piglit_get_gl_enum_name(status));
		piglit_report_subtest_result(PIGLIT_SKIP, "%s", format->name);
		return PIGLIT_SKIP;
	}
        printf("\n");

	glClearColor(0.0, 0.0, 0.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT);

	glColor4fv(res0);
	piglit_draw_rect(-1.0, -1.0, 0.33, 2.0);

        blend(pos1, src1, dst1, con1, GL_CONSTANT_COLOR, GL_ONE_MINUS_CONSTANT_COLOR);
	blend(pos2, src2, dst2, NULL, GL_DST_COLOR, GL_ONE_MINUS_DST_COLOR);
	blend(pos3, src3, dst3, NULL, GL_SRC_COLOR, GL_ONE_MINUS_SRC_COLOR);
	blend(pos4, src4, dst4, NULL, GL_DST_ALPHA, GL_ONE_MINUS_DST_ALPHA);
	blend(pos5, src5, dst5, NULL, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	if (!piglit_probe_pixel_rgba(piglit_width * 1 / 12, 0, res0)) {
		printf("  when testing FBO result, simple.\n");
		pass = GL_FALSE;
        }
	if (!piglit_probe_pixel_rgba(piglit_width * 3 / 12, 0, res1)) {
		printf("  when testing FBO result, blending with CONSTANT_COLOR.\n");
		pass = GL_FALSE;
        }
	if (!piglit_probe_pixel_rgba(piglit_width * 5 / 12, 0, res2)) {
		printf("  when testing FBO result, blending with DST_COLOR.\n");
		pass = GL_FALSE;
        }
	if (!piglit_probe_pixel_rgba(piglit_width * 7 / 12, 0, res3)) {
		printf("  when testing FBO result, blending with SRC_COLOR.\n");
		pass = GL_FALSE;
        }
	if (!piglit_probe_pixel_rgba(piglit_width * 9 / 12, 0, res4)) {
		printf("  when testing FBO result, blending with DST_ALPHA.\n");
		pass = GL_FALSE;
	}
	if (!piglit_probe_pixel_rgba(piglit_width * 11 / 12, 0, res5)) {
		printf("  when testing FBO result, blending with SRC_ALPHA.\n");
		pass = GL_FALSE;
	}

	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, piglit_winsys_fbo);
	glViewport(0, 0, piglit_width, piglit_height);

	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
	glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB,   GL_REPLACE);
	glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_REPLACE);

	glColor4f(1, 1, 1, 1);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, tex);
	piglit_draw_rect_tex(-1, -1, 2, 2,
			     0, 0, 1, 1);

	glDisable(GL_TEXTURE_2D);
	glDeleteTextures(1, &tex);
	glBindFramebufferEXT(GL_FRAMEBUFFER, piglit_winsys_fbo);
	glDeleteFramebuffersEXT(1, &fb);

	if (!pass) {
		piglit_present_results();
		piglit_report_subtest_result(PIGLIT_FAIL, "%s", format->name);
		return PIGLIT_FAIL;
	}

	/* Compute expected result colors when reading back from the window */
        if (i) {
		/* expected result = (I, I, I, I) */
		res0[3] = res0[2] = res0[1] = res0[0];
		res1[3] = res1[2] = res1[1] = res1[0];
		res2[3] = res2[2] = res2[1] = res2[0];
		res3[3] = res3[2] = res3[1] = res3[0];
		res4[3] = res4[2] = res4[1] = res4[0] = res4i;
		res5[3] = res5[2] = res5[1] = res5[0];
	} else if (l) {
		/* expected result = (L, L, L, A) */
		res0[2] = res0[1] = res0[0];
		res1[2] = res1[1] = res1[0];
		res2[2] = res2[1] = res2[0];
		res3[2] = res3[1] = res3[0];
		res4[2] = res4[1] = res4[0];
		res5[2] = res5[1] = res5[0];
	} else {
		/* leave 'res' colors as-is from above */
	}

	if (!piglit_probe_pixel_rgba(piglit_width * 1 / 12, 0, res0)) {
		printf("  when testing window result, simple.\n");
		pass = GL_FALSE;
        }
	if (!piglit_probe_pixel_rgba(piglit_width * 3 / 12, 0, res1)) {
		printf("  when testing window result, blending with CONSTANT_COLOR.\n");
		pass = GL_FALSE;
        }
	if (!piglit_probe_pixel_rgba(piglit_width * 5 / 12, 0, res2)) {
		printf("  when testing window result, blending DST_COLOR.\n");
		pass = GL_FALSE;
        }
	if (!piglit_probe_pixel_rgba(piglit_width * 7 / 12, 0, res3)) {
		printf("  when testing window result, blending SRC_COLOR.\n");
		pass = GL_FALSE;
        }
	if (!piglit_probe_pixel_rgba(piglit_width * 9 / 12, 0, res4)) {
		printf("  when testing window result, blending DST_ALPHA.\n");
		pass = GL_FALSE;
	}
	if (!piglit_probe_pixel_rgba(piglit_width * 11 / 12, 0, res5)) {
		printf("  when testing window result, blending SRC_ALPHA.\n");
		pass = GL_FALSE;
	}

	piglit_present_results();

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
	fbo_formats_init(argc, argv, GL_TRUE);
	glDisable(GL_DITHER);
}
