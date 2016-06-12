/*
 * Copyright (C) 2016 Ilia Mirkin
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

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 30;
	config.supports_gl_es_version = 30;
	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

enum piglit_result
piglit_display(void)
{
	return PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	bool pass = true;
	GLint box[9 * 4] = {0};
	int max;
	piglit_require_extension("GL_EXT_window_rectangles");

	glGetIntegerv(GL_MAX_WINDOW_RECTANGLES_EXT, &max);

	glWindowRectanglesEXT(0, 0, NULL);
	if (!piglit_check_gl_error(GL_INVALID_ENUM))
		pass = false;

	glWindowRectanglesEXT(GL_EXCLUSIVE_EXT, -1, NULL);
	if (!piglit_check_gl_error(GL_INVALID_VALUE))
		pass = false;

	if (max < 9) {
		GLint t[4];
		glWindowRectanglesEXT(GL_EXCLUSIVE_EXT, max + 1, box);
		if (!piglit_check_gl_error(GL_INVALID_VALUE))
			pass = false;

		glGetIntegeri_v(GL_WINDOW_RECTANGLE_EXT, max + 1, t);
		if (!piglit_check_gl_error(GL_INVALID_VALUE))
			pass = false;
	}

	if (max > 9)
		max = 9;

	box[2] = -1;
	glWindowRectanglesEXT(GL_EXCLUSIVE_EXT, max, box);
	if (!piglit_check_gl_error(GL_INVALID_VALUE))
		pass = false;
	box[2] = 0;
	box[3] = -1;
	glWindowRectanglesEXT(GL_EXCLUSIVE_EXT, max, box);
	if (!piglit_check_gl_error(GL_INVALID_VALUE))
		pass = false;

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}
