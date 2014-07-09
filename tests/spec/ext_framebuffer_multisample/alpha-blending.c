/*
 * Copyright © 2012 Intel Corporation
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

/** \file alpha-blending.c
 *
 * This test checks for Mesa bug 53077 (Output error with msaa when
 * both of framebuffer and source color's alpha are not 1).  The test
 * operates by drawing a partially transparent non-square rectangle to
 * a multisampled buffer (using a triangle fan) and then blitting the
 * result to the screen.  If the bug is present, artifacts will appear
 * along the rectangle diagonal due to alpha blending being performed
 * incorrectly while drawing the first triangle of the fan.
 *
 * See also https://bugs.freedesktop.org/show_bug.cgi?id=53077
 *
 * Note: when fast color clears are implemented for MSAA buffers, it's
 * possible that they will cover up this bug.  To avoid that, the test
 * can be supplied a command-line option of "slow_cc", which causes it
 * to use a clear color that cannot be fast cleared.
 */

#include "piglit-util-gl.h"

GLuint framebuffer, renderbuffer;
#define WIDTH 300
#define HEIGHT 350
int numSamples;
static bool slow_color_clear = false;

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_width = WIDTH;
	config.window_height = HEIGHT;
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;
	slow_color_clear = PIGLIT_STRIP_ARG("slow_cc");

PIGLIT_GL_TEST_CONFIG_END

void
piglit_init(int argc, char **argv)
{
	glGetIntegerv(GL_MAX_SAMPLES_EXT, &numSamples);

	glGenFramebuffersEXT(1, &framebuffer);
	glGenRenderbuffersEXT(1, &renderbuffer);
	glBindFramebufferEXT(GL_FRAMEBUFFER, framebuffer);
	glBindRenderbufferEXT(GL_RENDERBUFFER, renderbuffer);
	glRenderbufferStorageMultisampleEXT(GL_RENDERBUFFER, numSamples, GL_RGBA, WIDTH, HEIGHT);
	glFramebufferRenderbufferEXT(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, renderbuffer);
	glEnable (GL_MULTISAMPLE);
}

enum piglit_result
piglit_display()
{
	GLenum src_factor, dst_factor;
	float expected_color[] = { 0.0, 0.7, 0.0, 0.79 };
	int x0 = 140;
	int x1 = 220;
	int y0 = 55;
	int y1 = 165;
	bool pass = true;
	float vertex_data[][2] = {
		{ ((float) x0)/WIDTH * 2.0 - 1.0, ((float) y0)/HEIGHT * 2.0 - 1.0 },
		{ ((float) x1)/WIDTH * 2.0 - 1.0, ((float) y0)/HEIGHT * 2.0 - 1.0 },
		{ ((float) x1)/WIDTH * 2.0 - 1.0, ((float) y1)/HEIGHT * 2.0 - 1.0 },
		{ ((float) x0)/WIDTH * 2.0 - 1.0, ((float) y1)/HEIGHT * 2.0 - 1.0 },
	};

	glBindFramebufferEXT(GL_FRAMEBUFFER,framebuffer);

	if (slow_color_clear) {
		glColor4f (0.0, 1.0, 0.5, 0.7);
		glClearColor(0.0, 0.0, 0.5, 1.0);
		expected_color[2] = 0.5;
	} else {
		glColor4f (0.0, 1.0, 0.0, 0.7);
		glClearColor(0.0, 0.0, 0.0, 1.0);
	}
	glClear(GL_COLOR_BUFFER_BIT);

	glEnable(GL_BLEND);
	src_factor = GL_SRC_ALPHA;
	dst_factor = GL_ONE_MINUS_SRC_ALPHA;
	glBlendFunc (src_factor, dst_factor);
	glVertexPointer(2, GL_FLOAT, 0, vertex_data);
	glEnableClientState(GL_VERTEX_ARRAY);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	glBindFramebufferEXT(GL_READ_FRAMEBUFFER, framebuffer);
	glBindFramebufferEXT(GL_DRAW_FRAMEBUFFER, piglit_winsys_fbo);
	glBlitFramebuffer(0, 0, WIDTH, HEIGHT, 0, 0, WIDTH, HEIGHT, GL_COLOR_BUFFER_BIT, GL_NEAREST);
	glBindFramebuffer(GL_FRAMEBUFFER, piglit_winsys_fbo);

	pass = piglit_probe_rect_rgba(x0, y0, x1 - x0, y1 - y0, expected_color)
		&& pass;

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}
