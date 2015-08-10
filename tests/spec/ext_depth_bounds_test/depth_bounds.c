/*
 * Copyright © 2015 Advanced Micro Devices, Inc.
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
 *    Marek Olšák <maraeo@gmail.com>
 */

#include "piglit-util-gl.h"
#include <time.h>

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;
	config.window_width = 200;
	config.window_height = 200;
	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE |
			       PIGLIT_GL_VISUAL_DEPTH;

PIGLIT_GL_TEST_CONFIG_END

static bool inplace;
static const float size = 20;
static float white_color[3] = {1, 1, 1};
static float clear_color[3] = {0.1, 0.1, 0.1};

void
piglit_init(int argc, char **argv)
{
	inplace = argc == 2 && strcmp(argv[1], "-inplace") == 0;

	piglit_require_extension("GL_EXT_depth_bounds_test");
	piglit_gen_ortho_projection(0, piglit_width, 0, piglit_height, 0, -1, 0);
	srand(123456789);
	glDepthFunc(GL_ALWAYS);
}

enum {
	PASS,
	FAIL,
};

static struct {
	unsigned expected;
	float z0, z1, z2, z3;
	float min, max;
} test[] = {
	{PASS, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0},
	{PASS, 1.0, 1.0, 1.0, 1.0, 0.0, 1.0},
	{PASS, 0.0, 0.0, 0.0, 0.0, 0.0, 1.0},

	{PASS, 1.0, 1.0, 1.0, 1.0, 0.5, 1.0},
	{PASS, 0.7, 0.7, 0.7, 0.7, 0.5, 1.0},
	{PASS, 0.5, 0.5, 0.5, 0.5, 0.5, 1.0},
	{FAIL, 0.3, 0.3, 0.3, 0.3, 0.5, 1.0},
	{FAIL, 0.0, 0.0, 0.0, 0.0, 0.5, 1.0},

	{FAIL, 1.0, 1.0, 1.0, 1.0, 0.0, 0.5},
	{FAIL, 0.7, 0.7, 0.7, 0.7, 0.0, 0.5},
	{PASS, 0.5, 0.5, 0.5, 0.5, 0.0, 0.5},
	{PASS, 0.3, 0.3, 0.3, 0.3, 0.0, 0.5},
	{PASS, 0.0, 0.0, 0.0, 0.0, 0.0, 0.5},

	{FAIL, 0.29, 0.29, 0.29, 0.29, 0.3, 0.5},
	{PASS, 0.31, 0.31, 0.31, 0.31, 0.3, 0.5},
	{PASS, 0.49, 0.49, 0.49, 0.49, 0.3, 0.5},
	{FAIL, 0.51, 0.51, 0.51, 0.51, 0.3, 0.5},

	{PASS, 0.65, 0.65, 0.65, 0.65, 0.6, 0.7},
	{FAIL, 0.90, 0.90, 0.90, 0.90, 0.6, 0.7},
	{FAIL, 0.55, 0.55, 0.55, 0.55, 0.6, 0.7},
};

static void
check_rect(int i, bool *pass)
{
	float x = size * (i % 10);
	float y = size * (i / 10);

	printf("Test %i, bounds=(%.2f, %.2f), z=(%.2f, %.2f, %.2f, %.2f)\n",
	       i, test[i].min, test[i].max, test[i].z0, test[i].z1,
	       test[i].z2, test[i].z3);

	switch (test[i].expected) {
	case PASS:
		*pass = piglit_probe_rect_rgb(x, y, size, size, white_color) && *pass;
		break;
	case FAIL:
		*pass = piglit_probe_rect_rgb(x, y, size, size, clear_color) && *pass;
		break;
	}
}

enum piglit_result
piglit_display(void)
{
	bool pass = GL_TRUE;
	int i;

	glClearColor(clear_color[0], clear_color[1], clear_color[2], 1);
	glClearDepth(1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glColor3fv(white_color);
	for (i = 0; i < ARRAY_SIZE(test); i++) {
		float x = size * (i % 10);
		float y = size * (i / 10);

		glColorMask(0, 0, 0, 0);
		glEnable(GL_DEPTH_TEST);
		glBegin(GL_QUADS);
		glVertex3f(x,      y,      test[i].z0);
		glVertex3f(x+size, y,      test[i].z1);
		glVertex3f(x+size, y+size, test[i].z2);
		glVertex3f(x,      y+size, test[i].z3);
		glEnd();
		glDisable(GL_DEPTH_TEST);

		glColorMask(1, 1, 1, 1);
		glEnable(GL_DEPTH_BOUNDS_TEST_EXT);
		glDepthBoundsEXT(test[i].min, test[i].max);
		glBegin(GL_QUADS);
		/* use a random depth, it doesn't matter */
		glVertex3f(x,      y,      (rand() % 11) * 0.1);
		glVertex3f(x+size, y,      (rand() % 11) * 0.1);
		glVertex3f(x+size, y+size, (rand() % 11) * 0.1);
		glVertex3f(x,      y+size, (rand() % 11) * 0.1);
		glEnd();
		glDisable(GL_DEPTH_BOUNDS_TEST_EXT);

		if (inplace)
			check_rect(i, &pass);
	}

	if (!inplace)
		for (i = 0; i < ARRAY_SIZE(test); i++)
			check_rect(i, &pass);

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}
