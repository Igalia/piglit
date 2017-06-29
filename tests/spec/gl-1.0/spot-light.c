/*
 * Copyright (C) 2016 Daniel Scharrer <daniel@constexpr.org>
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

/** @file spot-light.c
 *
 * This is a simple sanity test for fixed function spot lights in OpenGL.
 *
 * It tests that vertices directly in front of the spot light are lit with
 * full intensity and that lighting of vertices beyond the spot cutoff,
 * and especially of those behind the spot light, is not affected by the
 * spot light. This is done for three spot lights with different exponents.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;
	config.khr_no_error_support = PIGLIT_NO_ERRORS;

PIGLIT_GL_TEST_CONFIG_END

static const GLfloat pos[4] = {15.0, 15.0, 0.0, 1.0}; /* center */
static const GLfloat light0_dir[3] = {-1.0, 0.0, 0.0}; /* left */
static const GLfloat light1_dir[3] = {0.0, 1.0, 0.0}; /* up */
static const GLfloat light2_dir[3] = {1.0, 0.0, 0.0}; /* right */
static const GLfloat light0_ambient[4] = {1.0, 0.0, 0.0, 1.0};
static const GLfloat light1_ambient[4] = {0.0, 1.0, 0.0, 1.0};
static const GLfloat light2_ambient[4] = {0.0, 0.0, 1.0, 1.0};
static const GLfloat global_ambient[4] = {0.2, 0.2, 0.2, 1.0};

static const GLfloat expected_left[4] = {1.0, 0.2, 0.2, 1.0};
static const GLfloat expected_bottom[4] = {0.2, 0.2, 0.2, 1.0};
static const GLfloat expected_right[4] = {0.2, 0.2, 1.0, 1.0};
static const GLfloat expected_top[4] = {0.2, 1.0, 0.2, 1.0};
static const GLfloat expected_bottom_left[4] = {0.2, 0.2, 0.2, 1.0};
static const GLfloat expected_top_left[4] = {0.2, 1.0, 0.2, 1.0};
static const GLfloat expected_bottom_right[4] = {0.2, 0.2, 0.2, 1.0};
static const GLfloat expected_top_right[4] = {0.2, 1.0, 0.2, 1.0};

enum piglit_result
piglit_display(void)
{
	bool pass = true;

	glClear(GL_COLOR_BUFFER_BIT);

	glPointSize(3);
	glBegin(GL_POINTS);
	for (int x = 0; x < 11; x++) {
		for (int y = 0; y < 11; y++) {
			glVertex2f(3 * x, 3 * y);
		}
	}
	glEnd();

	pass = piglit_probe_pixel_rgba( 0, 15, expected_left) && pass;
	pass = piglit_probe_pixel_rgba(15,  0, expected_bottom) && pass;
	pass = piglit_probe_pixel_rgba(30, 15, expected_right) && pass;
	pass = piglit_probe_pixel_rgba(15, 30, expected_top) && pass;
	pass = piglit_probe_pixel_rgba( 0,  0, expected_bottom_left) && pass;
	pass = piglit_probe_pixel_rgba( 0, 30, expected_top_left) && pass;
	pass = piglit_probe_pixel_rgba(30,  0, expected_bottom_right) && pass;
	pass = piglit_probe_pixel_rgba(30, 30, expected_top_right) && pass;

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}


void
piglit_init(int argc, char **argv)
{
	GLfloat zero[4] = {0.0, 0.0, 0.0, 1.0};
	GLfloat one[4] = {1.0, 1.0, 1.0, 1.0};

	glClearColor(0.0, 0.0, 0.0, 1.0);

	glEnable(GL_LIGHT0);
	glLightf(GL_LIGHT0, GL_SPOT_CUTOFF, 44.0);
	glLightf(GL_LIGHT0, GL_SPOT_EXPONENT, 1.0);
	glLightfv(GL_LIGHT0, GL_POSITION, pos);
	glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, light0_dir);
	glLightfv(GL_LIGHT0, GL_AMBIENT, light0_ambient);

	glEnable(GL_LIGHT1);
	glLightf(GL_LIGHT1, GL_SPOT_CUTOFF, 60.0);
	glLightf(GL_LIGHT1, GL_SPOT_EXPONENT, 0.0);
	glLightfv(GL_LIGHT1, GL_POSITION, pos);
	glLightfv(GL_LIGHT1, GL_SPOT_DIRECTION, light1_dir);
	glLightfv(GL_LIGHT1, GL_AMBIENT, light1_ambient);

	glEnable(GL_LIGHT2);
	glLightf(GL_LIGHT2, GL_SPOT_CUTOFF, 44.0);
	glLightf(GL_LIGHT2, GL_SPOT_EXPONENT, 5.0);
	glLightfv(GL_LIGHT2, GL_POSITION, pos);
	glLightfv(GL_LIGHT2, GL_SPOT_DIRECTION, light2_dir);
	glLightfv(GL_LIGHT2, GL_AMBIENT, light2_ambient);

	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, global_ambient);
	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 0);

	/* We are not interested in testing diffuse lighting, enable only the ambient term. */
	glMaterialfv(GL_FRONT, GL_DIFFUSE, zero);
	glMaterialfv(GL_FRONT, GL_AMBIENT, one);

	glEnable(GL_LIGHTING);

	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);
}
