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
 */

/**
 * Test OpenGL scissor for polygon rendering.
 * This is a replacement for the old glean scissor test.
 *
 * Brian Paul
 * Feb 2013
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN
	config.supports_gl_compat_version = 10;
	config.window_visual = PIGLIT_GL_VISUAL_RGBA;
PIGLIT_GL_TEST_CONFIG_END


static const GLubyte black[4] = { 0, 0, 0, 0 };
static const GLubyte white[4] = { 255, 255, 255, 255 };


static bool
color_equal(const GLubyte c1[4], const GLubyte c2[4])
{
	return (c1[0] == c2[0] &&
		c1[1] == c2[1] &&
		c1[2] == c2[2] &&
		c1[3] == c2[3]);
}


static bool
check_result(int x, int y, int w, int h, const GLubyte *image)
{
	int i, j;

	for (j = 0; j < piglit_height; j++) {
		for (i = 0; i < piglit_width; i++) {
			const GLubyte *p = image + (j * piglit_width + i) * 4;
			if (i >= x && i < x + w &&
			    j >= y && j < y + h) {
				/* inside the scissor rect */
				if (!color_equal(p, white)) {
					fprintf(stderr, "Expected white at (%d, %d),"
						" found black\n", i, j);
					return false;
				}
			}
			else {
				/* outside the scissor rect */
				if (!color_equal(p, black)) {
					fprintf(stderr, "Expected black at (%d, %d),"
						" found white\n", i, j);
					return false;
				}
			}
		}
	}

	return true;
}


enum piglit_result
piglit_display(void)
{
	GLubyte *image = malloc(piglit_width * piglit_height * 4);
	int dx, dy;
	int x, y, w, h;
	bool pass = true;

	/* compute position step */
	dx = piglit_width / 8;
	dy = piglit_height / 8;

	/* compute scissor size */
	w = piglit_width / 5;
	h = piglit_height / 5;

	glColor4ubv(white);

	/* loop over scissor positions */
	for (y = 0; y < piglit_height; y += dy) {
		for (x = 0; x < piglit_width; x += dx) {
			glClear(GL_COLOR_BUFFER_BIT);

			glEnable(GL_SCISSOR_TEST);
			glScissor(x, y, w, h);

			/* draw window-sized quad */
			piglit_draw_rect(-1, -1, 2, 2);

			glDisable(GL_SCISSOR_TEST);

			glReadPixels(0, 0, piglit_width, piglit_height,
				     GL_RGBA, GL_UNSIGNED_BYTE, image);

			piglit_present_results();

			pass = check_result(x, y, w, h, image);
			if (!pass)
				goto end;
		}
	}

end:
	free(image);

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}


void
piglit_init(int argc, char**argv)
{
	/* nothing */
}
