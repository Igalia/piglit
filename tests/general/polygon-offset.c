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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Authors:
 *    Keith Packard <keithp@keithp.com>
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_width = 400;
	config.window_height = 300;
	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_DEPTH;

PIGLIT_GL_TEST_CONFIG_END

void
piglit_init(int argc, char **argv)
{
	piglit_ortho_projection(piglit_width, piglit_height, false);

	glShadeModel(GL_FLAT);
	glClearColor(0.2, 0.2, 0.2, 1.0);

}

float red[3] = {1.0, 0.0, 0.0};
float blue[3] = {0.0, 0.0, 1.0};

#define SIZE	40
#define SPACE	50

#define X(x)	((x) * SPACE + SIZE/2)
#define Y(y)	((y) * SPACE + SIZE/2)

int	line_width;

static void
do_rect(int x, int y, float color[3], int mode)
{
	glColor3fv(color);
	glPolygonMode(GL_FRONT_AND_BACK, mode);
	piglit_draw_rect_z(0, X(x), Y(y), SIZE, SIZE);
}

static int
check(int x, int y, float color[3])
{
	return piglit_probe_pixel_rgb(X(x) + SIZE-line_width/4, Y(y) + SIZE - line_width/4, color);
}

static int poly_mode[3] = { GL_FILL, GL_LINE, GL_POINT };

enum piglit_result
piglit_display(void)
{
	int x = 0, y = 0;
	int line_width_range[2];
	int point_size_range[2];

	bool pass = true;
	int first_mode;
	int second_mode;
	int over;

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glGetIntegerv(GL_ALIASED_LINE_WIDTH_RANGE, line_width_range);
	line_width = line_width_range[1];
	glGetIntegerv(GL_ALIASED_POINT_SIZE_RANGE, point_size_range);
	if (line_width > point_size_range[1])
		line_width = point_size_range[1];
	glLineWidth(line_width);
	glPointSize(line_width);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	glEnable(GL_POLYGON_OFFSET_FILL);
	glEnable(GL_POLYGON_OFFSET_POINT);
	glEnable(GL_POLYGON_OFFSET_LINE);

	for (first_mode = 0; first_mode < 3; first_mode++) {
		for (second_mode = 0; second_mode < 3; second_mode++) {
			for (over = 0; over < 2; over++) {
				glPolygonOffset(0.0, 0.0);
				do_rect(x, y, red, poly_mode[first_mode]);
				glPolygonOffset(0.0, over ? -1.0 : 1.0);
				do_rect(x, y, blue, poly_mode[second_mode]);
				pass = pass && check(x, y, over ? blue : red);

				if (++x == 4) {
					x = 0;
					y++;
				}
			}
		}
	}

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}
