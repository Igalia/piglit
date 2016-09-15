/*
 * Copyright © 2016 VMware, Inc.
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
 * Test that enabling GL_ALPHA_TO_COVERAGE has no effect for non-MSAA
 * rendering.
 */


#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN
	config.supports_gl_compat_version = 13;
	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;
PIGLIT_GL_TEST_CONFIG_END


void
piglit_init(int argc, char **argv)
{
	/* nothing */
}

static bool
test_coverage_nop(bool msaa_enabled)
{
	float alpha;

	if (msaa_enabled)
		glEnable(GL_MULTISAMPLE);
	else
		glDisable(GL_MULTISAMPLE);

	/* Render quad with varying alpha value.  Make sure it draws
	 * and is not missing because of some kind of MSAA coverage bug.
	 */
	for (alpha = 1.0; alpha >= 0.0; alpha -= 1.0/128) {
		float expected[4] = {alpha, alpha, alpha, alpha};

		glClear(GL_COLOR_BUFFER_BIT);
		glColor4fv(expected);
		piglit_draw_rect(-1, -1, 2, 2);

		if (!piglit_probe_rect_rgba(0, 0, piglit_width, piglit_height,
					    expected)) {
			printf("Rect did not draw for alpha = %g "
			       "with GL_MULTISAMPLE %s\n",
			       alpha,
			       msaa_enabled ? "enabled" : "disabled");
			return false;
		}

		piglit_present_results();
	}

	return true;
}


enum piglit_result
piglit_display(void)
{
	GLint samples;
	bool pass;

	glGetIntegerv(GL_SAMPLES, &samples);
	if (samples != 0) {
		printf("Unexpected GL_SAMPLES = %d\n", samples);
		return PIGLIT_FAIL;
	}

	glGetIntegerv(GL_SAMPLE_BUFFERS, &samples);
	if (samples != 0) {
		printf("Unexpected GL_SAMPLE_BUFFERS = %d\n", samples);
		return PIGLIT_FAIL;
	}

	glClearColor(1, 0, 0, 0);

	/* Enabling GL_SAMPLE_ALPHA_TO_COVERAGE should have no effect
	 * with a non-MSAA drawing surface.
	 */
	glEnable(GL_SAMPLE_ALPHA_TO_COVERAGE);

	pass = test_coverage_nop(true);
	pass = test_coverage_nop(false) && pass;

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}
