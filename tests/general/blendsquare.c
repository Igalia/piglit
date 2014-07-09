/*
 * (C) Copyright IBM Corporation 2004
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * on the rights to use, copy, modify, merge, publish, distribute, sub
 * license, and/or sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.  IN NO EVENT SHALL
 * VA LINUX SYSTEM, IBM AND/OR THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/**
 * \file blendsquare.c
 * 
 * Simple test of GL_NV_blend_square functionality.  Four squares are drawn
 * with different blending modes, but all should be rendered with the same
 * final color.
 *
 * \author Ian Romanick <idr@us.ibm.com>
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_width = 400;
	config.window_height = 200;
	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

enum piglit_result
piglit_display(void)
{
	GLboolean pass = GL_TRUE;
	int w = (piglit_width - 50) / 4;
	int h = piglit_height - 20;
	int start_x = 10;
	int next_x = 10 + w;
	float expected[4] = {0.25, 0.25, 0.25, 0.25};

	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);

	glClearColor(0.2, 0.2, 0.8, 0);
	glClear(GL_COLOR_BUFFER_BIT);

	glBlendFunc(GL_ONE, GL_ZERO);
	glColor3f(0.5 * 0.5, 0.5 * 0.5, 0.5 * 0.5);
	piglit_draw_rect(start_x + next_x * 0, 10, w, h);

	glBlendFunc(GL_ONE, GL_ZERO);
	glColor3f(0.5, 0.5, 0.5);
	piglit_draw_rect(start_x + next_x * 1, 10, w, h);
	glBlendFunc(GL_DST_COLOR, GL_ZERO);
	piglit_draw_rect(start_x + next_x * 1, 10, w, h);

	glBlendFunc(GL_SRC_COLOR, GL_ZERO);
	piglit_draw_rect(start_x + next_x * 2, 10, w, h);

	glBlendFunc(GL_ONE, GL_ZERO);
	piglit_draw_rect(start_x + next_x * 3, 10, w, h);
	glBlendFunc(GL_ZERO, GL_DST_COLOR);
	piglit_draw_rect(start_x + next_x * 3, 10, w, h);

	pass = piglit_probe_pixel_rgb(15 + next_x * 0, piglit_height / 2,
				      expected) && pass;
	pass = piglit_probe_pixel_rgb(15 + next_x * 1, piglit_height / 2,
				      expected) && pass;
	pass = piglit_probe_pixel_rgb(15 + next_x * 2, piglit_height / 2,
				      expected) && pass;
	pass = piglit_probe_pixel_rgb(15 + next_x * 3, piglit_height / 2,
				      expected) && pass;

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	if (piglit_get_gl_version() < 14 && !piglit_is_extension_supported("GL_NV_blend_square")) {
		printf("Sorry, this program requires either OpenGL 1.4 or "
		       "GL_NV_blend_square\n");
		piglit_report_result(PIGLIT_SKIP);
	}

	printf("\nAll 4 quads should be the same color.  "
	       "The two on the left are drawn\n"
	       "without NV_blend_square functionality, and "
	       "the two on the right are drawn\n"
	       "with NV_blend_square functionality.  If the "
	       "two on the left are dark, but\n"
	       "the two on the right are not, then "
	       "NV_blend_square is broken.\n");
	glEnable(GL_BLEND);
}
