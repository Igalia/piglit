/*
 * Copyright © 2010 Intel Corporation
 * Copyright © 2010 Marek Olšák <maraeo@gmail.com>
 * Copyright (c) 2017 VMware, Inc.
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
 *    Roland Scheidegger <sroland@vmware.com>
 *
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 30;

	/* Drivers that do not support GL_ARB_texture_non_power_of_two require
	 * window dimensions that are powers of two for this test.
	 */
	config.window_width = next_power_of_two(config.window_width);
	config.window_height = next_power_of_two(config.window_height);

	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;
	config.khr_no_error_support = PIGLIT_NO_ERRORS;

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


static int
get_texture_bits(GLenum target, GLenum size_enum, GLenum type_enum)
{
	GLint size = 0;
	GLint type = GL_NONE;
	glGetTexLevelParameteriv(target, 0, size_enum, &size);
	if (!size) {
		return size;
	}
	if (piglit_is_extension_supported("GL_EXT_texture_snorm") ||
            piglit_get_gl_version() >= 31) {
		glGetTexLevelParameteriv(target, 0, type_enum, &type);
		if (type == GL_SIGNED_NORMALIZED) {
			/* One bit is lost for the sign */
			size -= 1;
		}
	}
	return size;
}


float
calc_blend_factor(float src, float dst, float blendcol, GLenum factor)
{
	switch (factor) {
	case GL_ZERO:
		return 0.0f;
	case GL_ONE:
		return 1.0f;
	case GL_SRC_COLOR:
		return src;
	case GL_ONE_MINUS_SRC_COLOR:
		return 1.0f - src;
	case GL_DST_COLOR:
		return dst;
	case GL_ONE_MINUS_DST_COLOR:
		return 1.0f - dst;
	case GL_CONSTANT_COLOR:
		return blendcol;
	case GL_ONE_MINUS_CONSTANT_COLOR:
		return 1.0f - blendcol;
	default:
		assert(0);
	}
	return 0.0f;
}

/*
 * Calculate add blend func result. Pretty simplified, no separate a/rgb factors.
 */
void
blend_func_add(const float *src, const float *dst, const float *blendcol,
               GLenum src_factor, GLenum dst_factor, float *res)
{
	int i;
	for (i = 0; i < 4; i++) {
		float src_clamped = CLAMP(src[i], -1.0f, 1.0f);
		float dst_clamped = CLAMP(dst[i], -1.0f, 1.0f);
		float blendcol_clamped = 0.0f;
		float res_unclamped, s_factor, d_factor;
		if (blendcol)
			blendcol_clamped = CLAMP(blendcol[i], -1.0f, 1.0f);
		s_factor = calc_blend_factor(src_clamped, dst_clamped,
					     blendcol_clamped, src_factor);
		d_factor = calc_blend_factor(src_clamped, dst_clamped,
					     blendcol_clamped, dst_factor);
		res_unclamped = s_factor * src_clamped + d_factor * dst_clamped;
		res[i] = CLAMP(res_unclamped, -1.0f, 1.0f);
	}
}


enum piglit_result piglit_display(void)
{
	GLboolean pass = GL_TRUE;
	GLuint tex, fb;
	GLenum status;
	int r, g, b, a;
	struct blend_config {
		GLenum src_factor;
		GLenum dst_factor;
	};

	float res0[] = { 0.3, -0.3, 0.3, 0.0};

	float pos1[] = {-0.66, -1.0, 0.33, 2.0};
        float dst1[] = { 0.5, 0.4, -0.6, 0.2};
        float src1[] = { -0.2, 1.9, 0.8, -0.7};
	struct blend_config cnf1 = {GL_ONE_MINUS_SRC_COLOR, GL_ONE};
	float res1[4]; 

	float pos2[] = {-0.33, -1.0, 0.33, 2.0};
	float dst2[] = {1.9, -0.4, 0.7, 0.5};
        float src2[] = {-1.8, 0.3, 0.5, 0.9};
	struct blend_config cnf2 = {GL_DST_COLOR, GL_ONE_MINUS_DST_COLOR};
	float res2[4];

	float pos3[] = {0.0, -1.0, 0.33, 2.0};
	float dst3[] = {-0.6, 0.4, 0.8, 0.5};
	float src3[] = {0.8, 0.9, -0.7, 0.8};
	struct blend_config cnf3 = {GL_SRC_COLOR, GL_ONE_MINUS_SRC_COLOR};
	float res3[4];

	float pos4[] = {0.33, -1.0, 0.33, 2.0};
	float dst4[] = {0.9, 0.4, 0.7, 0.5};
	float src4[] = {0.8, 0.3, 0.5, -0.9};
	struct blend_config cnf4 = {GL_SRC_COLOR, GL_SRC_COLOR};
	float res4[4];

	float pos5[] = {0.66, -1.0, 0.33, 2.0};
	float dst5[] = {0.6, -0.3, 0.8, 0.5};
	float src5[] = {0.8, 0.1, 0.7, 0.8};
        float con5[] = {1.2, -1.8, 0.4, 0.6};
	struct blend_config cnf5 = {GL_ONE_MINUS_CONSTANT_COLOR, GL_ONE_MINUS_DST_COLOR};
	float res5[4];

	glGenFramebuffersEXT(1, &fb);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fb);
	glViewport(0, 0, piglit_width, piglit_height);

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8_SNORM,
		     piglit_width, piglit_height, 0,
		     GL_RGBA, GL_FLOAT, NULL);

	a = get_texture_bits(GL_TEXTURE_2D,
			     GL_TEXTURE_ALPHA_SIZE,
			     GL_TEXTURE_ALPHA_TYPE);
	r = get_texture_bits(GL_TEXTURE_2D,
			     GL_TEXTURE_RED_SIZE,
			     GL_TEXTURE_RED_TYPE);
	g = get_texture_bits(GL_TEXTURE_2D,
			     GL_TEXTURE_GREEN_SIZE,
			     GL_TEXTURE_GREEN_TYPE);
	b = get_texture_bits(GL_TEXTURE_2D,
			     GL_TEXTURE_BLUE_SIZE,
			     GL_TEXTURE_BLUE_TYPE);

	{
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
	if (r > 8)
		r = 8;
	if (g > 8)
		g = 8;
	if (b > 8)
		b = 8;
	if (a > 8)
		a = 8;

	piglit_set_tolerance_for_bits(r, g, b, a);

	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
				  GL_COLOR_ATTACHMENT0_EXT,
				  GL_TEXTURE_2D,
				  tex,
				  0);
	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
	if (status != GL_FRAMEBUFFER_COMPLETE_EXT) {
		printf(" - fbo incomplete (status = %s)\n",
		       piglit_get_gl_enum_name(status));
		piglit_report_subtest_result(PIGLIT_SKIP, "%s", "GL_RGBA8_SNORM");
		return PIGLIT_SKIP;
	}
        printf("\n");

	glClearColor(0.0, 0.0, 0.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT);

	glColor4fv(res0);
	piglit_draw_rect(-1.0, -1.0, 0.33, 2.0);

        blend(pos1, src1, dst1, NULL, cnf1.src_factor, cnf1.dst_factor);
	blend(pos2, src2, dst2, NULL, cnf2.src_factor, cnf2.dst_factor);
	blend(pos3, src3, dst3, NULL, cnf3.src_factor, cnf3.dst_factor);
	blend(pos4, src4, dst4, NULL, cnf4.src_factor, cnf4.dst_factor);
	blend(pos5, src5, dst5, con5, cnf5.src_factor, cnf5.dst_factor);

	if (!piglit_probe_pixel_rgba(piglit_width * 1 / 12, 0, res0)) {
		printf("  when testing FBO result, simple.\n");
		pass = GL_FALSE;
        }
	blend_func_add(src1, dst1, NULL, cnf1.src_factor, cnf1.dst_factor, res1);
	if (!piglit_probe_pixel_rgba(piglit_width * 3 / 12, 0, res1)) {
		printf("  when testing FBO result, blending with inv_src/one.\n");
		pass = GL_FALSE;
        }
	blend_func_add(src2, dst2, NULL, cnf2.src_factor, cnf2.dst_factor, res2);
	if (!piglit_probe_pixel_rgba(piglit_width * 5 / 12, 0, res2)) {
		printf("  when testing FBO result, blending with dst/inv_dst.\n");
		pass = GL_FALSE;
        }
	blend_func_add(src3, dst3, NULL, cnf3.src_factor, cnf3.dst_factor, res3);
	if (!piglit_probe_pixel_rgba(piglit_width * 7 / 12, 0, res3)) {
		printf("  when testing FBO result, blending with src/inv_src.\n");
		pass = GL_FALSE;
        }
	blend_func_add(src4, dst4, NULL, cnf4.src_factor, cnf4.dst_factor, res4);
	if (!piglit_probe_pixel_rgba(piglit_width * 9 / 12, 0, res4)) {
		printf("  when testing FBO result, blending with src/src.\n");
		pass = GL_FALSE;
	}
	blend_func_add(src5, dst5, con5, cnf5.src_factor, cnf5.dst_factor, res5);
	if (!piglit_probe_pixel_rgba(piglit_width * 11 / 12, 0, res5)) {
		printf("  when testing FBO result, blending with inv_constant/dst.\n");
		pass = GL_FALSE;
	}

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void piglit_init(int argc, char **argv)
{
	/*
	 * Either need GL_EXT_texture_snorm or GL 3.1 (which introduced
	 * snorm formats, but only the non-legacy ones).
	 * Note neither guarantees it's renderable (in fact GL 3.1 lists
	 * it explicitly as "texture only" but later versions just say
	 * not required for rendering). That would need
	 * GL_ARB_internalformat_query2.
	 */
	if (!piglit_is_extension_supported("GL_EXT_texture_snorm"))
		piglit_require_gl_version(31);

	glDisable(GL_DITHER);
	/*
	 * Note that all values entering blend will still be clamped
	 * implicitly to [-1,1] for snorm formats.
	 */
	glClampColor(GL_CLAMP_FRAGMENT_COLOR, GL_FALSE);
	glClampColor(GL_CLAMP_VERTEX_COLOR, GL_FALSE);
	glClampColor(GL_CLAMP_READ_COLOR, GL_FALSE);
}
