/**
 * Copyright © 2013 Intel Corporation
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
 * Test that Clear() clears no buffers when passed 0
 *
 * Section 4.2.3(Clearing the Buffers) of OpenGL 3.2 Core says:
 * "The value to which each buffer is cleared depends on the setting of the
 *  clear value for that buffer. If buf is zero, no buffers are cleared."
 *
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 32;
        config.supports_gl_core_version = 32;

        config.window_visual = PIGLIT_GL_VISUAL_RGB |
				PIGLIT_GL_VISUAL_DOUBLE |
				PIGLIT_GL_VISUAL_DEPTH |
				PIGLIT_GL_VISUAL_STENCIL;

PIGLIT_GL_TEST_CONFIG_END

void
piglit_init(int argc, char **argv)
{
}

enum piglit_result
piglit_display(void)
{
	bool pass = true;
	float green[] = {0, 1, 0};

	glEnable(GL_DEPTH_TEST);

	/* Set first base values to the buffers */
	glClearColor(0.0, 1.0, 0.0, 1.0);
	glClearDepth(.8);
	glClearStencil(1);
	glClear(GL_COLOR_BUFFER_BIT |
		GL_DEPTH_BUFFER_BIT |
		GL_STENCIL_BUFFER_BIT);

	/* Set a second value to the buffers */
	glClearColor(1.0, 0.0, 0.0, 1.0);
	glClearDepth(.2);
	glClearStencil(2);

	glClear(0);
	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

	/* if probe returns the first value, glClear(0) didn't clear values */
	pass = piglit_probe_rect_rgb(0, 0, piglit_width, piglit_height, green)
		 && pass;
	pass = piglit_probe_rect_depth(0, 0, piglit_width, piglit_height, .8)
		 && pass;
	pass = piglit_probe_rect_stencil(0, 0, piglit_width, piglit_height, 1)
		 && pass;

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}
