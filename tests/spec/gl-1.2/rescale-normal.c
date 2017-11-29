/*
 * Copyright © 2017 Fabian Bieler
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
 * @file rescale-normal.c:  Rescale normals with fixed function pipeline.
 *
 * Set up scene with diffuse lighting and an isotropic modelview scale of 100.
 * Set the light color to 1% red, 100% green, 0% blue.
 * If the normal is scaled incorrectly in either direction, the sampled color
 * would be black or yellow, respectively, instead of green.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 12;
	config.window_visual = PIGLIT_GL_VISUAL_RGB;
	config.khr_no_error_support = PIGLIT_NO_ERRORS;

PIGLIT_GL_TEST_CONFIG_END

static const float green_with_a_smitch_of_red[] = {0.01, 1, 0, 1};

enum piglit_result
piglit_display(void)
{
	piglit_draw_rect(-0.01, -0.01, 0.02, 0.02);
	bool pass = piglit_probe_pixel_rgb(piglit_width / 2,
					   piglit_height / 2,
					   green_with_a_smitch_of_red);

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	const float black[] = {0, 0, 0, 1};
	const float white[] = {1, 1, 1, 1};

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);

	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, black);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, white);

	glLightfv(GL_LIGHT0, GL_DIFFUSE, green_with_a_smitch_of_red);

	glScalef(100.0, 100.0, 100.0);
	glEnable(GL_RESCALE_NORMAL);

	glNormal3f(0, 0, 1);
}
