/*
 * Copyright (C) 2016 VMware, Inc.
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
 * Test glViewport behaviour with GL_ARB_clip_control.
 * The position of the viewport in window coordinates should not be
 * effected by the GL_CLIP_ORIGIN state.
 *
 * See https://bugs.freedesktop.org/show_bug.cgi?id=93813
 *
 * Brian Paul
 * 9 Feb 2016
 */

#include "piglit-util-gl.h"


PIGLIT_GL_TEST_CONFIG_BEGIN
	config.supports_gl_compat_version = 20;
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;
PIGLIT_GL_TEST_CONFIG_END


static const float red[3] = { 1, 0, 0 };
static const float green[3] = { 0, 1, 0 };
static const float blue[3] = { 0, 0, 1 };
static const float white[3] = { 1, 1, 1 };


void
piglit_init(int argc, char **argv)
{
	piglit_require_extension("GL_ARB_clip_control");
	glEnable(GL_CULL_FACE);
}


/**
 * Draw this pattern in the current viewport region, regardless of
 * the clip control settings:
 *
 *   +---------+---------+
 *   |         |         |
 *   |   blue  |  white  |
 *   |         |         |
 *   +---------+---------+
 *   |         |         |
 *   |   red   |  green  |
 *   |         |         |
 *   +---------+---------+
 *
 * \param invert_y - if true, NDC_Y=-1=top, else NDC_Y=-1=bottom
 */
static void
draw_test_pattern(bool invert_y)
{
	/* Since the modelview and projection matrices are identity matrices,
	 * we're effectively drawing in Normalized Device Coordinates which
	 * range from [-1,1] in X and Y.
	 *
	 * Note: we're careful with our glRectf coordinates so that the
	 * rect is drawn front-facing.  If a rect is not drawn it must be
	 * because it was back-face culled by mistake.
	 */

	/* lower-left quadrant = red */
	glColor3fv(red);
	if (invert_y)
		glRectf(-1, 0, 0, 1);
	else
		glRectf(-1, -1, 0, 0);

	/* lower-right quadrant = green */
	glColor3fv(green);
	if (invert_y)
		glRectf(0, 0, 1, 1);
	else
		glRectf(0, -1, 1, 0);

	/* upper-left quadrant = blue */
	glColor3fv(blue);
	if (invert_y)
		glRectf(-1, -1, 0, 0);
	else
		glRectf(-1, 0, 0, 1);

	/* upper-right quadrant = white */
	glColor3fv(white);
	if (invert_y)
		glRectf(0, -1, 1, 0);
	else
		glRectf(0, 0, 1, 1);
}


static bool
check_test_pattern(int xpos, int ypos)
{
	bool pass = true;
	int half_w = piglit_width / 2;
	int half_h = piglit_height / 2;
	/* coords in center of the color swatches */
	int x0 = xpos + half_w / 4;
	int y0 = ypos + half_h / 4;
	int x1 = xpos + half_w * 3 / 4;
	int y1 = ypos + half_h * 3 / 4;

	if (!piglit_probe_pixel_rgb(x0, y0, red)) {
		printf("wrong color in lower-left quadrant of test pattern\n");
		pass = false;
	}

	if (!piglit_probe_pixel_rgb(x1, y0, green)) {
		printf("wrong color in lower-right quadrant of test pattern\n");
		pass = false;
	}

	if (!piglit_probe_pixel_rgb(x0, y1, blue)) {
		printf("wrong color in upper-left quadrant of test pattern\n");
		pass = false;
	}

	if (!piglit_probe_pixel_rgb(x1, y1, white)) {
		printf("wrong color in upper-right quadrant of test pattern\n");
		pass = false;
	}

	if (!pass) {
		GLint origin;
		glGetIntegerv(GL_CLIP_ORIGIN, &origin);
		printf("GL_CLIP_ORIGIN = %s\n",
		       piglit_get_gl_enum_name(origin));
	}

	return pass;
}


enum piglit_result
piglit_display(void)
{
	int half_w = piglit_width / 2;
	int half_h = piglit_height / 2;
	bool pass = true;

	/* Test normal GL coordinates */
	glClipControl(GL_LOWER_LEFT, GL_NEGATIVE_ONE_TO_ONE);
	glClear(GL_COLOR_BUFFER_BIT);

	/* Normal GL origin / Draw in upper-left screen quadrant */
	glViewport(0, half_h, half_w, half_h);
	draw_test_pattern(false);
	if (!check_test_pattern(0, half_h))
		pass = false;

	/* Normal GL origin / Draw in lower-right screen quadrant */
	glViewport(half_w, 0, half_w, half_h);
	draw_test_pattern(false);
	if (!check_test_pattern(half_w, 0))
		pass = false;


	/* Test inverted GL coordinates */
	glClipControl(GL_UPPER_LEFT, GL_NEGATIVE_ONE_TO_ONE);
	glClear(GL_COLOR_BUFFER_BIT);

	/* Inverted GL origin / Draw in upper-left screen quadrant */
	glViewport(0, half_h, half_w, half_h);
	draw_test_pattern(true);
	if (!check_test_pattern(0, half_h))
		pass = false;

	/* Inverted GL origin / Draw in lower-right screen quadrant */
	glViewport(half_w, 0, half_w, half_h);
	draw_test_pattern(true);
	if (!check_test_pattern(half_w, 0))
		pass = false;

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}
