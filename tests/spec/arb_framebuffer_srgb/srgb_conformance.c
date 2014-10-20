/*
 * Copyright © 2013 VMware, Inc.
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
 *    Roland Scheidegger <sroland@vmware.com>
 *
 * Tests accuracy of srgb->linear and linear->srgb conversion,
 * according to d3d10 rules:
 * 1) srgb->linear is permitted a tolerance of 0.5 on the srgb side
 * (meaning the result converted back to srgb (but before float->int
 * conversion) using perfect formula must not deviate more than 0.5)
 * 2) linear->srgb is permitted a tolerance of 0.6 compared to using
 * perfect formula
 * 3) additionally all srgb values must stay the same when doing
 * srgb to linear and then linear to srgb conversion.
 * This test does not actually verify 2) (which would need exhaustive
 * test of all float values) so if some floats outside these generated
 * by srgb->linear conversion get mapped to arbitrary values that will
 * go unnoticed. Likewise, correct behavior for floats outside 0.0-1.0
 * is not verified (including INFs and NaNs - the former should get clamped
 * to 0/255 in the end, NaNs should also get mapped to 0).
 *
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_width = 16;
	config.window_height = 16;
	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

static enum piglit_result test_format(void)
{
	GLboolean pass = GL_TRUE;
	GLuint texsrgb, texfb, fb;
	GLenum status;
	int i;
	float tex_vals[256][4];
	float readf[256][4];
	float tolerance;
	GLubyte readb[256][4];
	float maxErr = 0.0f;

	for (i = 0; i < 256; i++) {
		tex_vals[i][0] = (float)i / 255.0f;
		tex_vals[i][1] = tex_vals[i][2] = 0.0f;
		tex_vals[i][3] = 1.0f;
	}

	/* initialize texture */
	glGenTextures(1, &texsrgb);
	glBindTexture(GL_TEXTURE_2D, texsrgb);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8_ALPHA8_EXT,
		     16, 16, 0,
		     GL_RGBA, GL_FLOAT, &tex_vals[0][0]);

	glGenFramebuffersEXT(1, &fb);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fb);
	glViewport(0, 0, piglit_width, piglit_height);

	/* draw into float framebuffer and verify results */
	glGenTextures(1, &texfb);
	glBindTexture(GL_TEXTURE_2D, texfb);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F_ARB,
		     piglit_width, piglit_height, 0,
		     GL_RGBA, GL_FLOAT, NULL);

	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
				  GL_COLOR_ATTACHMENT0_EXT,
				  GL_TEXTURE_2D,
				  texfb,
				  0);
	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
	printf("Testing using fb float format");
	if (status != GL_FRAMEBUFFER_COMPLETE_EXT) {
		printf(" - fbo incomplete (status = %s)\n",
		       piglit_get_gl_enum_name(status));
		piglit_report_subtest_result(PIGLIT_SKIP, "float fb");
		return PIGLIT_SKIP;
	}
        printf("\n");

	glColor4f(1, 1, 1, 1);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texsrgb);

	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
	glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB,   GL_REPLACE);
	glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_REPLACE);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	piglit_draw_rect_tex(-1, -1, 2, 2,
			     0, 0, 1, 1);

	glDisable(GL_TEXTURE_2D);

	/* have to make up our own error measuring, since we
	   measure error on srgb side (so with values mapped back to srgb
	   using accurate formula) */

	tolerance = 0.5f; /* as required by d3d10 */
	glReadPixels(0, 0, 16, 16, GL_RGBA, GL_FLOAT, &readf[0][0]);

	for (i = 0; i < 256; i++) {
		float err = fabs(piglit_linear_to_srgb(readf[i][0]) - (float)i);
		if (0)
			printf("readback: %f observed: %f expected: %f\n", readf[i][0],
				piglit_linear_to_srgb(readf[i][0]), (float)i);
		if (err > maxErr) {
			maxErr = err;
		}
		if (err > tolerance) {
			printf("  failed when testing srgb->float result\n");
			pass = GL_FALSE;
			break;
		}
	}
	printf("max error srgb->linear was %f\n", maxErr);

	piglit_present_results();

	piglit_report_subtest_result(pass ? PIGLIT_PASS : PIGLIT_FAIL,
				     "srgb->linear");

	/* draw into srgb framebuffer and verify results */
	glBindTexture(GL_TEXTURE_2D, texfb);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8_ALPHA8_EXT,
		     piglit_width, piglit_height, 0,
		     GL_RGBA, GL_FLOAT, NULL);

	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
				  GL_COLOR_ATTACHMENT0_EXT,
				  GL_TEXTURE_2D,
				  texfb,
				  0);

	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	glEnable(GL_FRAMEBUFFER_SRGB_EXT);
	status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
	printf("Testing using fb srgb format");
	if (status != GL_FRAMEBUFFER_COMPLETE_EXT) {
		printf(" - fbo incomplete (status = %s)\n",
		       piglit_get_gl_enum_name(status));
		piglit_report_subtest_result(PIGLIT_SKIP, "srgb fb");
		return PIGLIT_SKIP;
	}
        printf("\n");

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texsrgb);
	piglit_draw_rect_tex(-1, -1, 2, 2,
			     0, 0, 1, 1);
	glDisable(GL_TEXTURE_2D);

	tolerance = 0.6f; /* as required by d3d10 */
	/* TODO: should test this tolerance too really right now only test
	   the previously converted from srgb values, so we only enforce
	   any value stays the same when doing srgb->linear->srgb, which
	   corresponds to 0.5 tolerance but only for these exact values. */
	glReadPixels(0, 0, 16, 16, GL_RGBA, GL_UNSIGNED_BYTE, &readb[0][0]);

	for (i = 0; i < 256; i++) {
		if (0)
			printf("observed: %d expected: %d\n", readb[i][0], i);
		if (readb[i][0] != i) {
			printf("  failed when testing srgb->float->srgb result\n");
			pass = GL_FALSE;
			break;
		}
	}

	piglit_present_results();

	piglit_report_subtest_result(pass ? PIGLIT_PASS : PIGLIT_FAIL,
				     "srgb->linear->srgb");

	glDeleteTextures(1, &texfb);
	glDeleteTextures(1, &texsrgb);
	glDeleteFramebuffersEXT(1, &fb);

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

enum piglit_result piglit_display(void)
{
	return test_format();
}

void piglit_init(int argc, char **argv)
{
	piglit_require_extension("GL_EXT_framebuffer_object");
	piglit_require_extension("GL_ARB_texture_env_combine");
	piglit_require_extension("GL_EXT_texture_sRGB");
	piglit_require_extension("GL_ARB_framebuffer_sRGB");
	piglit_require_extension("GL_ARB_color_buffer_float");
	glDisable(GL_DITHER);
}
