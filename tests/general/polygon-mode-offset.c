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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 */


/**
 * Test glPolygonMode + glPolygonOffset.
 *
 * Brian Paul
 * Feb 2013
 */


#include "piglit-util-gl.h"


PIGLIT_GL_TEST_CONFIG_BEGIN
	config.supports_gl_compat_version = 10;
	config.window_visual = (PIGLIT_GL_VISUAL_RGB |
				PIGLIT_GL_VISUAL_DOUBLE |
				PIGLIT_GL_VISUAL_DEPTH);
PIGLIT_GL_TEST_CONFIG_END



/**
 * Check that we drew a white outline around the blue polygon
 */
static bool
check_lines_visible(int number)
{
	static const float white[3] = {1, 1, 1};
	static const float blue[3] = {0, 0, 1};
	const int w = piglit_width, h = piglit_height;
	const int mx = w / 2, my = h / 2;
	float p[4];
	bool pass = true;

	/* probe bottom */
	if (!piglit_probe_pixel_rgb_silent(mx, 1, white, p)) {
		piglit_report_subtest_result(PIGLIT_FAIL,
			"config %d: Expected white pixel on bottom edge",
			number);
		pass = false;
	}

	/* probe top */
	if (!piglit_probe_pixel_rgb_silent(mx, h-2, white, p)) {
		piglit_report_subtest_result(PIGLIT_FAIL,
			"config %d: Expected white pixel on top edge",
			number);
		pass = false;
	}

	/* probe left */
	if (!piglit_probe_pixel_rgb_silent(1, my, white, p)) {
		piglit_report_subtest_result(PIGLIT_FAIL,
			"config %d: Expected white pixel on left edge",
			number);
		pass = false;
	}

	/* probe right */
	if (!piglit_probe_pixel_rgb_silent(w-2, my, white, p)) {
		piglit_report_subtest_result(PIGLIT_FAIL,
			"config %d: Expected white pixel on right edge",
			number);
		pass = false;
	}

	/* probe center */
	if (!piglit_probe_pixel_rgb_silent(mx, my, blue, p)) {
		piglit_report_subtest_result(PIGLIT_FAIL,
			"config %d: Expected blue pixel in center",
			number);
		pass = false;
	}

	return pass;
}


/** Draw rect with clockwise vertices */
static void
rect_cw(float coords[2][2])
{
	glBegin(GL_POLYGON);
	glVertex2f(coords[0][0], coords[0][0]);
	glVertex2f(coords[0][0], coords[1][0]);
	glVertex2f(coords[1][0], coords[1][0]);
	glVertex2f(coords[1][0], coords[0][0]);
	glEnd();
}


/** Draw rect with counter clockwise vertices */
static void
rect_ccw(float coords[2][2])
{
	glBegin(GL_POLYGON);
	glVertex2f(coords[0][0], coords[0][0]);
	glVertex2f(coords[1][0], coords[0][0]);
	glVertex2f(coords[1][0], coords[1][0]);
	glVertex2f(coords[0][0], coords[1][0]);
	glEnd();
}


enum color { WHITE, BLUE };

struct test_config {
	GLenum offsetEnable;
	float offsetFactor, offsetUnits;
	/* first prim: */
	GLenum frontMode1, backMode1;
	enum color color1;
	GLenum winding1;
	/* second prim: */
	GLenum frontMode2, backMode2;
	enum color color2;
	GLenum winding2;
};


/**
 * For all these test configurations, we should wind up drawing a
 * blue filled quad with a white outline.
 */
static const struct test_config configs[] = {
	{
		GL_POLYGON_OFFSET_FILL, 1.0, 1.0,
		/* first prim */
		GL_LINE, GL_LINE, WHITE, GL_CCW,
		/* second prim */
		GL_FILL, GL_FILL, BLUE, GL_CCW
	},
	{
		GL_POLYGON_OFFSET_FILL, 1.0, 1.0,
		/* first prim */
		GL_FILL, GL_FILL, BLUE, GL_CCW,
		/* second prim */
		GL_LINE, GL_LINE, WHITE, GL_CCW
	},
	{
		GL_POLYGON_OFFSET_FILL, 1.0, 1.0,
		/* first prim */
		GL_FILL, GL_LINE, BLUE, GL_CCW,
		/* second prim */
		GL_FILL, GL_LINE, WHITE, GL_CW
	},
	{
		GL_POLYGON_OFFSET_FILL, 1.0, 1.0,
		/* first prim */
		GL_LINE, GL_FILL, WHITE, GL_CCW,
		/* second prim */
		GL_LINE, GL_FILL, BLUE, GL_CW
	},
	{
		GL_POLYGON_OFFSET_LINE, 1.0, -1.0,
		/* first prim */
		GL_LINE, GL_FILL, WHITE, GL_CCW,
		/* second prim */
		GL_LINE, GL_FILL, BLUE, GL_CW
	},
	{
		GL_POLYGON_OFFSET_LINE, 1.0, -1.0,
		/* first prim */
		GL_LINE, GL_FILL, BLUE, GL_CW,
		/* second prim */
		GL_LINE, GL_FILL, WHITE, GL_CCW
	},
	{
		GL_POLYGON_OFFSET_LINE, 1.0, -1.0,
		/* first prim */
		GL_FILL, GL_LINE, BLUE, GL_CCW,
		/* second prim */
		GL_FILL, GL_LINE, WHITE, GL_CW
	}
};


/** Test one configuration */
static bool
test(int config_number)
{
	const struct test_config *config = &configs[config_number];
	bool pass;
	float white_coords[2][2], blue_coords[2][2], (*coords)[2];

	/* for drawing the filled quad (cover the whole window) */
	blue_coords[0][0] = 0;
	blue_coords[0][1] = 0;
	blue_coords[1][0] = piglit_width;
	blue_coords[1][1] = piglit_height;

	/* for drawing the outline (2 pixels smaller than the window size) */
	white_coords[0][0] = 1;
	white_coords[0][1] = 1;
	white_coords[1][0] = piglit_width - 2;
	white_coords[1][1] = piglit_height - 2;

	assert(config->offsetEnable == GL_POLYGON_OFFSET_FILL ||
	       config->offsetEnable == GL_POLYGON_OFFSET_LINE ||
	       config->offsetEnable == GL_POLYGON_OFFSET_POINT);

	assert(config->frontMode1 == GL_LINE ||
	       config->frontMode1 == GL_FILL ||
	       config->frontMode1 == GL_POINT);

	assert(config->backMode1 == GL_LINE ||
	       config->backMode1 == GL_FILL ||
	       config->backMode1 == GL_POINT);

	assert(config->winding1 == GL_CW ||
	       config->winding1 == GL_CCW);

	assert(config->winding2 == GL_CW ||
	       config->winding2 == GL_CCW);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(config->offsetEnable);
	glPolygonOffset(config->offsetFactor, config->offsetUnits);

	/* draw first prim */
	glPolygonMode(GL_FRONT, config->frontMode1);
	glPolygonMode(GL_BACK, config->backMode1);
	if (config->color1 == WHITE) {
		glColor3f(1, 1, 1);
		coords = white_coords;
	}
	else {
		glColor3f(0, 0, 1);
		coords = blue_coords;
	}

	if (config->winding1 == GL_CW)
		rect_cw(coords);
	else
		rect_ccw(coords);

	/* draw second prim */
	glPolygonMode(GL_FRONT, config->frontMode2);
	glPolygonMode(GL_BACK, config->backMode2);
	if (config->color2 == WHITE) {
		glColor3f(1, 1, 1);
		coords = white_coords;
	}
	else {
		glColor3f(0, 0, 1);
		coords = blue_coords;
	}

	if (config->winding2 == GL_CW)
		rect_cw(coords);
	else
		rect_ccw(coords);

	/* check results */
	pass = check_lines_visible(config_number);

	piglit_present_results();

	glDisable(config->offsetEnable);

	return pass;
}


enum piglit_result
piglit_display(void)
{
	int i;
	bool pass = true;

	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);

	/* Sub-pixel translation so that lines hit specific pixels */
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(0.375, 0.375, 0.0);

	for (i = 0; i < ARRAY_SIZE(configs); i++) {
		pass = test(i) && pass;
	}

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}


void
piglit_init(int argc, char **argv)
{
	glClearColor(1, 0, 0, 0);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
}
