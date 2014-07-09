/*
 * Copyright © 2014 VMware, Inc.
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

/*
 * Test flat-shaded clipped line color.
 * Exercises provoking vertex, line smooth, line width, etc.
 *
 * Author: Brian Paul
 */


#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN
	config.supports_gl_compat_version = 10;
	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;
PIGLIT_GL_TEST_CONFIG_END


/* far left, far right verts */
static const float verts[2][2] = {
	{ -10.0, 0.0 }, { 10.0, 0.0 }
};

static const float colors[2][3] = {
	{ 1.0, 0.0, 0.0 }, { 0.0, 1.0, 0.0 }
};

static const GLuint forward_order[2] = { 0, 1 };
static const GLuint backward_order[2] = { 1, 0 };

static bool have_pv = false;


static bool
test_one(int order, const float expected[3])
{
	bool pass = false;
	int dy, y = piglit_height / 2;

	glClear(GL_COLOR_BUFFER_BIT);

	/* draw horizontal line across middle of window */
	if (order == 0)
		glDrawElements(GL_LINES, 2, GL_UNSIGNED_INT, forward_order);
	else
		glDrawElements(GL_LINES, 2, GL_UNSIGNED_INT, backward_order);

	/* To be resilient in the face of different line rasterization,
	 * try several Y values to find where the line was drawn.
	 */
	for (dy = -1; dy <= 1; dy++) {
		GLfloat color[3];
		glReadPixels(0, y + dy, 1, 1, GL_RGB, GL_FLOAT, color);
		if (color[0] || color[1] || color[2]) {
			/* found non-black pixel */
			/* test all pixels across middle of window */
			pass = piglit_probe_rect_rgb(0, y + dy, /* x, y */
						     piglit_width, 1, /* w, h */
						     expected);
			break;
		}
	}

	piglit_present_results();

	return pass;
}


void
piglit_init(int argc, char **argv)
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glEnable(GL_VERTEX_ARRAY);
	glEnable(GL_COLOR_ARRAY);
	glVertexPointer(2, GL_FLOAT, 0, verts);
	glColorPointer(3, GL_FLOAT, 0, colors);

	glShadeModel(GL_FLAT);

	if (piglit_is_extension_supported("GL_ARB_provoking_vertex")) {
		have_pv = true;
	}
	else if (piglit_is_extension_supported("GL_EXT_provoking_vertex")) {
		have_pv = true;
	}

	if (have_pv)
		printf("Have provoking vertex.\n");
}


enum piglit_result
piglit_display(void)
{
	int direction, smooth, pv, width;
	bool p, pass = true;

	for (pv = 0; pv <= (int) have_pv; pv++) {
		if (pv == 1)
			glProvokingVertex(GL_FIRST_VERTEX_CONVENTION_EXT);

		for (width = 1; width <= 5; width += 4) {
			glLineWidth((float) width);

			for (smooth = 0; smooth <= 1; smooth++) {
				if (smooth)
					glEnable(GL_LINE_SMOOTH);
				else
					glDisable(GL_LINE_SMOOTH);

				for (direction = 0; direction <= 1; direction++) {
					/* Determine which vertex color should
					 * have been used for the line.
					 */
					int c = !(direction ^ pv);

					p = test_one(direction, colors[c]);
					if (!p) {
						printf("failure (pv = %d, dir = %d,"
						       " smooth = %d, width = %d)\n",
						       pv, direction, smooth, width);
					}
					pass = pass && p;
				}
			}
		}
	}

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}
