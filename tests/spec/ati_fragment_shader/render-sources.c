/*
 * Copyright © 2017 Miklós Máté
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

/**
 * Tests rendering with GL_ATI_fragment_shader:
 * - various data sources for calculations in fragment shader
 *   - texture coordinates
 *   - texture sample
 *   - constant
 *   - primary color
 *   - secondary interpolator
 *   - one, zero
 * - switch between named fragment shaders
 * - use undefined default shader (rendering is undefined but shouldn't crash)
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;

PIGLIT_GL_TEST_CONFIG_END

static const float color1[] = {0.2, 0.3, 0.8};
static const float color2[] = {0.9, 0.8, 0.3};
static const float texcoord[] = {0.2, 0.7, 0.4};
static const float texcolor[] = {0.8, 0.1, 0.7};

static float result_const[] = {0.0, 0.0, 0.0};
static float result_tex[] = {0.0, 0.0, 0.0};
static float result_color[] = {0.0, 0.0, 0.0};

static GLuint tex;

enum {
	SHADER_TEX = 1,
	SHADER_CONST,
	SHADER_COLOR,
};

enum piglit_result
piglit_display(void)
{
	bool pass = true;

	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);

	glClearColor(1.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	glColor3fv(color1);
	glSecondaryColor3fvEXT(color2);
	glTexCoord3fv(texcoord);

	glEnable(GL_TEXTURE_2D);

	glEnable(GL_FRAGMENT_SHADER_ATI);
	glBindFragmentShaderATI(0);
	piglit_draw_rect(0, 0,
			 piglit_width / 4, piglit_height);
	glBindFragmentShaderATI(SHADER_CONST);
	piglit_draw_rect(piglit_width / 4, 0,
			 piglit_width / 4, piglit_height);
	glBindFragmentShaderATI(SHADER_COLOR);
	piglit_draw_rect(2 * piglit_width / 4, 0,
			 piglit_width / 4, piglit_height);
	glBindFragmentShaderATI(SHADER_TEX);
	piglit_draw_rect(3 * piglit_width / 4, 0,
			 piglit_width / 4, piglit_height);
	glDisable(GL_FRAGMENT_SHADER_ATI);

#if 0
	/* Mesa uses fixed-function when the shader is invalid, but
	 * it's undefined
	 */
	pass = pass && piglit_probe_rect_rgb(0, 0,
					     piglit_width / 4,
					     piglit_height, color1 * texcolor);
#endif
	pass = pass && piglit_probe_rect_rgb(piglit_width / 4, 0,
					     piglit_width / 4,
					     piglit_height, result_const);
	pass = pass && piglit_probe_rect_rgb(2 * piglit_width / 4, 0,
					     piglit_width / 4,
					     piglit_height, result_color);
	pass = pass && piglit_probe_rect_rgb(3 * piglit_width / 4, 0,
					     piglit_width / 4,
					     piglit_height, result_tex);

	piglit_present_results();

	pass &= piglit_check_gl_error(GL_NO_ERROR);

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	unsigned u;

	piglit_require_extension("GL_ATI_fragment_shader");

	/* create shaders that use all possible input sources:
	 * texcoord, sample, const, zero, one, pri&sec color
	 */

	glBindFragmentShaderATI(SHADER_TEX);
	glBeginFragmentShaderATI();
	glPassTexCoordATI(GL_REG_1_ATI, GL_TEXTURE0_ARB, GL_SWIZZLE_STR_ATI);
	glSampleMapATI(GL_REG_0_ATI, GL_TEXTURE0_ARB, GL_SWIZZLE_STR_ATI);
	glColorFragmentOp2ATI(GL_MUL_ATI, GL_REG_0_ATI, GL_NONE, GL_NONE,
			GL_REG_0_ATI, GL_NONE, GL_NONE,
			GL_REG_1_ATI, GL_NONE, GL_NONE);
	glEndFragmentShaderATI();

	glBindFragmentShaderATI(SHADER_CONST);
	glBeginFragmentShaderATI();
	glColorFragmentOp3ATI(GL_LERP_ATI, GL_REG_0_ATI, GL_NONE, GL_NONE,
			GL_CON_1_ATI, GL_NONE, GL_NONE,
			GL_ONE, GL_NONE, GL_NONE,
			GL_ZERO, GL_NONE, GL_NONE);
	glSetFragmentShaderConstantATI(GL_CON_1_ATI, color2);
	glEndFragmentShaderATI();

	glBindFragmentShaderATI(SHADER_COLOR);
	glBeginFragmentShaderATI();
	glColorFragmentOp2ATI(GL_MUL_ATI, GL_REG_0_ATI, GL_NONE, GL_NONE,
			GL_SECONDARY_INTERPOLATOR_ATI, GL_NONE, GL_NONE,
			GL_PRIMARY_COLOR_ARB, GL_NONE, GL_NONE);
	glEndFragmentShaderATI();

	/* create a texture */

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_FLOAT,
		     (void*)texcolor);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	/* compute the expected results */

	for (u=0; u<3; u++) {
		result_const[u] = color2[u];
		result_tex[u] = texcoord[u] * texcolor[u];
		result_color[u] = color1[u] * color2[u];
	}

	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);
}
