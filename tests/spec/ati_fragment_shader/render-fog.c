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
 * Tests rendering with GL_ATI_fragment_shader: using fog.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;

PIGLIT_GL_TEST_CONFIG_END

static const float color1[] = {0.2, 0.3, 0.8};
static const float color2[] = {0.9, 0.8, 0.3};

static float resultLin[] = {0.0, 0.0, 0.0};
static float resultExp[] = {0.0, 0.0, 0.0};
static float resultEx2[] = {0.0, 0.0, 0.0};

static const float z = 0.8;
static const float dens = 0.4;

enum piglit_result
piglit_display(void)
{
	bool pass = true;

	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);

	glClearColor(1.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	glColor3fv(color2);

	glFogfv(GL_FOG_COLOR, color1);
	glFogf(GL_FOG_DENSITY, dens);
	glFogf(GL_FOG_START, 0.0);
	glFogf(GL_FOG_END, 1.0);
	glHint(GL_FOG_HINT, GL_NICEST);

	glEnable(GL_FRAGMENT_SHADER_ATI);
	glEnable(GL_FOG);
	glFogi(GL_FOG_MODE, GL_LINEAR);
	piglit_draw_rect_z(z, 0, 0,
			   piglit_width / 4, piglit_height);
	glFogi(GL_FOG_MODE, GL_EXP);
	piglit_draw_rect_z(z, piglit_width / 4, 0,
			   piglit_width / 4, piglit_height);
	glFogi(GL_FOG_MODE, GL_EXP2);
	piglit_draw_rect_z(z, 2 * piglit_width / 4, 0,
			   piglit_width / 4, piglit_height);
	glDisable(GL_FOG);
	piglit_draw_rect_z(z, 3 * piglit_width / 4, 0,
			   piglit_width / 4, piglit_height);
	glDisable(GL_FRAGMENT_SHADER_ATI);

	pass = pass && piglit_probe_rect_rgb(0, 0,
					     piglit_width / 4,
					     piglit_height, resultLin);
	pass = pass && piglit_probe_rect_rgb(piglit_width / 4, 0,
					     piglit_width / 4,
					     piglit_height, resultExp);
	pass = pass && piglit_probe_rect_rgb(2 * piglit_width / 4, 0,
					     piglit_width / 4,
					     piglit_height, resultEx2);
	pass = pass && piglit_probe_rect_rgb(3 * piglit_width / 4, 0,
					     piglit_width / 4,
					     piglit_height, color2);

	piglit_present_results();

	pass &= piglit_check_gl_error(GL_NO_ERROR);

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	unsigned u;
	float f;

	piglit_require_extension("GL_ATI_fragment_shader");

	glBeginFragmentShaderATI();
	glColorFragmentOp1ATI(GL_MOV_ATI, GL_REG_0_ATI, GL_NONE, GL_NONE,
			GL_PRIMARY_COLOR_ARB, GL_NONE, GL_NONE);
	glEndFragmentShaderATI();

	/* compute the expected results */

	for (u=0; u<3; u++) {
		f = (1.0-z)/(1.0-0.0);
		resultLin[u] = f*color2[u] + (1.0-f)*color1[u];
		f = exp(-dens*z);
		resultExp[u] = f*color2[u] + (1.0-f)*color1[u];
		f = exp(-pow(dens*z,2.0));
		resultEx2[u] = f*color2[u] + (1.0-f)*color1[u];
	}

	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);
}
