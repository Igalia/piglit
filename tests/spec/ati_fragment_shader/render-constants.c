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
 * - using local and global constants
 * - updating global constants
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;

PIGLIT_GL_TEST_CONFIG_END

static const float color1[] = {0.4, 0.2, 0.6};
static const float color2[] = {0.7, 0.2, 0.3};
static const float color3[] = {0.1, 0.7, 0.2};
static const float color4[] = {0.8, 0.1, 0.7};

static float result1p3[] = {0.0, 0.0, 0.0};
static float result2p3[] = {0.0, 0.0, 0.0};
static float result4p3[] = {0.0, 0.0, 0.0};

static const unsigned s_local = 42;
static const unsigned s_global = 13;

enum piglit_result
piglit_display(void)
{
	bool pass = true;

	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);

	glClearColor(1.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	glEnable(GL_FRAGMENT_SHADER_ATI);
	glSetFragmentShaderConstantATI(GL_CON_7_ATI, color2);
	glSetFragmentShaderConstantATI(GL_CON_4_ATI, color3);
	glBindFragmentShaderATI(s_global);
	piglit_draw_rect(0, 0, piglit_width / 4, piglit_height); /* 2+3 */
	glBindFragmentShaderATI(s_local);
	piglit_draw_rect(piglit_width / 4, 0,
			 piglit_width / 4, piglit_height); /* 1+3 */
	glBindFragmentShaderATI(s_global);
	glSetFragmentShaderConstantATI(GL_CON_7_ATI, color4);
	piglit_draw_rect(2 * piglit_width / 4, 0,
			 piglit_width / 4, piglit_height); /* 4+3 */
	glBindFragmentShaderATI(s_local);
	piglit_draw_rect(3 * piglit_width / 4, 0,
			 piglit_width / 4, piglit_height); /*1+3 */
	glDisable(GL_FRAGMENT_SHADER_ATI);

	pass = pass && piglit_probe_rect_rgb(0, 0,
					     piglit_width / 4,
					     piglit_height, result2p3);
	pass = pass && piglit_probe_rect_rgb(piglit_width / 4, 0,
					     piglit_width / 4,
					     piglit_height, result1p3);
	pass = pass && piglit_probe_rect_rgb(2 * piglit_width / 4, 0,
					     piglit_width / 4,
					     piglit_height, result4p3);
	pass = pass && piglit_probe_rect_rgb(3 * piglit_width / 4, 0,
					     piglit_width / 4,
					     piglit_height, result1p3);

	piglit_present_results();

	pass &= piglit_check_gl_error(GL_NO_ERROR);

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	unsigned u;

	piglit_require_extension("GL_ATI_fragment_shader");

	/* create identical shaders, but one of them has a local constant */

	glBindFragmentShaderATI(s_global);
	glBeginFragmentShaderATI();
	glColorFragmentOp2ATI(GL_ADD_ATI, GL_REG_0_ATI, GL_NONE, GL_NONE,
			GL_CON_7_ATI, GL_NONE, GL_NONE,
			GL_CON_4_ATI, GL_NONE, GL_NONE);
	glEndFragmentShaderATI();

	glBindFragmentShaderATI(s_local);
	glBeginFragmentShaderATI();
	glColorFragmentOp2ATI(GL_ADD_ATI, GL_REG_0_ATI, GL_NONE, GL_NONE,
			GL_CON_7_ATI, GL_NONE, GL_NONE,
			GL_CON_4_ATI, GL_NONE, GL_NONE);
	glSetFragmentShaderConstantATI(GL_CON_7_ATI, color1);
	glEndFragmentShaderATI();

	/* compute the expected results */

	for (u=0; u<3; u++) {
		result1p3[u] = color1[u]+color3[u];
		result2p3[u] = color2[u]+color3[u];
		result4p3[u] = color4[u]+color3[u];
	}

	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);
}
