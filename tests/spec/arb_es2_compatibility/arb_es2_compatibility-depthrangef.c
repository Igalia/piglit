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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

/** @file arb_es2_compatibility-depthrangef.c
 *
 * Tests that ARB_ES2_compatibility adds glDepthRangef (as opposed to
 * glDepthRange) and that it works.  Based on general/depth-clamp-range.c.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_width = 150;
	config.window_height = 150;
	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_DEPTH;

PIGLIT_GL_TEST_CONFIG_END

void
piglit_init(int argc, char **argv)
{
	piglit_require_extension("GL_ARB_ES2_compatibility");
}

enum piglit_result
piglit_display(void)
{
#ifdef GL_ARB_ES2_compatibility
	GLboolean pass = GL_TRUE;
	float red[4] =   {1.0, 0.0, 0.0, 0.0};
	float green[4] = {0.0, 1.0, 0.0, 0.0};
	float blue[4] =  {0.0, 0.0, 1.0, 0.0};

	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);

	glClearColor(0.0, 0.0, 1.0, 0.0);

	glClearDepthf(0.5);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	/* Keep in mind that the ortho projection flips near and far's signs,
	 * so 1.0 to quad()'s z maps to glDepthRange's near, and -1.0 maps to
	 * glDepthRange's far.
	 */

	glColor4fv(green);
	glDepthRangef(0, 1);
	piglit_draw_rect_z(0.5,
			   0, 0,
			   piglit_width / 2, piglit_height);

	glColor4fv(red);
	glDepthRangef(1, 0);
	piglit_draw_rect_z(0.5,
			   piglit_width / 2, 0,
			   piglit_width, piglit_height);

	pass = piglit_probe_pixel_rgb(piglit_width * 1 / 4, piglit_height / 2,
				      green) && pass;
	pass = piglit_probe_pixel_rgb(piglit_width * 3 / 4, piglit_height / 2,
				      blue) && pass;

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
#else
	return PIGLIT_SKIP;
#endif /* GL_ARB_ES2_compatibility */
}
