/*
 * Copyright © 2011 Intel Corporation
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

#include "piglit-util-gl.h"

/**
 * @file discard-drawpixels.c
 *
 * Tests that GL_RASTERIZER_DISCARD appropriately affects glDrawPixels().
 *
 * From the EXT_transform_feedback spec:
 *
 *     "Primitives can be optionally discarded before rasterization by
 *      calling Enable and Disable with RASTERIZER_DISCARD_EXT. When
 *      enabled, primitives are discared right before the
 *      rasterization stage, but after the optional transform feedback
 *      stage. When disabled, primitives are passed through to the
 *      rasterization stage to be processed
 *      normally. RASTERIZER_DISCARD_EXT applies to the DrawPixels,
 *      CopyPixels, Bitmap, Clear and Accum commands as well."
 */

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;

PIGLIT_GL_TEST_CONFIG_END

enum piglit_result
piglit_display(void)
{
	bool pass = true;
	float green[4] = {0.0, 1.0, 0.0, 0.0};
	float *buf;
	int i;

	buf = malloc(sizeof(float) * 4 * piglit_width * piglit_height);

	/* should draw full screen. */
	for (i = 0; i < piglit_width * piglit_height; i++) {
		buf[i * 4 + 0] = 0.0;
		buf[i * 4 + 1] = 1.0;
		buf[i * 4 + 2] = 0.0;
		buf[i * 4 + 3] = 0.0;
	}
	glDisable(GL_RASTERIZER_DISCARD);
	glRasterPos2i(-1, -1);
	glDrawPixels(piglit_width, piglit_height, GL_RGBA, GL_FLOAT, buf);

	/* should not draw full screen. */
	for (i = 0; i < piglit_width * piglit_height; i++) {
		buf[i * 4 + 0] = 1.0;
		buf[i * 4 + 1] = 0.0;
		buf[i * 4 + 2] = 0.0;
		buf[i * 4 + 3] = 0.0;
	}
	glEnable(GL_RASTERIZER_DISCARD);
	glRasterPos2i(-1, -1);
	glDrawPixels(piglit_width, piglit_height, GL_RGBA, GL_FLOAT, buf);

	pass = piglit_probe_rect_rgba(0, 0, piglit_width, piglit_height, green);

	piglit_present_results();

	free(buf);

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	piglit_require_transform_feedback();
}
