/*
 * Copyright © 2018 Danylo Piliaiev
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
 * Test that provoking vertex works as expected when part of the geometry
 * is clipped when flat shading is enabled.
 *
 * https://bugs.freedesktop.org/show_bug.cgi?id=103047
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN
	config.supports_gl_compat_version = 10;
	config.khr_no_error_support = PIGLIT_NO_ERRORS;
PIGLIT_GL_TEST_CONFIG_END

void
piglit_init(int argc, char **argv)
{
	piglit_require_extension("GL_EXT_provoking_vertex");
	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);

	glShadeModel(GL_FLAT);
}

enum piglit_result
piglit_display(void)
{
	static const float red[3] = {1, 0, 0},
		green[3] = {0, 1, 0},
		blue[3] = {0, 0, 1},
		yellow[3] = {1, 1, 0},
		cyan[3] = {0, 1, 1};

	bool pass = true;

	glClear(GL_COLOR_BUFFER_BIT);
	glProvokingVertexEXT(GL_LAST_VERTEX_CONVENTION_EXT);

	const int y1 = piglit_height / 3;

	glBegin(GL_TRIANGLE_STRIP);
		glColor3fv(cyan);
		glVertex3i(piglit_width + 1, y1, 0);
		glColor3fv(yellow);
		glVertex3i(piglit_width + 2, y1, 0);
		glColor3fv(blue);
		glVertex3i(piglit_width + 3, y1, 0);
		glColor3fv(green);
		glVertex3i(piglit_width / 2, y1 * 2, 0);
		glColor3fv(red);
		glVertex3i(piglit_width - 1, y1 * 2, 0);
	glEnd();

	pass = pass && piglit_probe_pixel_rgb(piglit_width - 2, y1 * 3 / 2, red);

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}
