/* Copyright © 2011 Intel Corporation
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
 * \file clearbuffer-display-list.c
 * Verify glClearBuffer functions are supported in display lists
 *
 * This test works by generating display lists with glClearBufferfv() in
 * GL_COMPILE, GL_COMPILE_AND_EXECUTE modes and attempting to clear the color
 * buffer using display lists
 *
 * \author Anuj Phogat
 */

#include "piglit-util-gl.h"
#include "clearbuffer-common.h"

void piglit_init(int argc, char **argv)
{
	static const float initial_color[4]  = { 0.0, 0.0, 0.0, 0.0 };
	static const float first[4]  = { 0.5, 0.4, 0.3, 1.0 };
	static const float second[4] = { 0.8, 0.0, 0.8, 1.0 };
	static const float third[4] = { 1.0, 0.3, 0.7, 1.0 };
	GLuint index;
	bool pass = true;

	piglit_require_gl_version(30);
	glClear(GL_COLOR_BUFFER_BIT);

	/* Probe the color buffer before creating display list. Default clear
	 * color is (0.0, 0.0, 0.0, 0.0)
	 */
	pass = piglit_probe_rect_rgba(0, 0,
				      piglit_width, piglit_height,
				      initial_color)
		&& pass;
	/* Generate two display lists */
	index = glGenLists(2);
	/* Create a new list in compile mode */
	glNewList(index, GL_COMPILE);
	glClearBufferfv(GL_COLOR,
			0,
			first);
	glEndList();
	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

	/* To ensure that glNewList() just compiles the list in GL_COMPILE
	 * mode, probe the color buffer after glEndList()
	 */
	pass = piglit_probe_rect_rgba(0, 0,
				      piglit_width, piglit_height,
				      initial_color)
		&& pass;

	glCallList(index);

	/* Probe the color buffer after glCallList() */
	pass = piglit_probe_rect_rgba(0, 0,
				      piglit_width, piglit_height,
				      first)
		&& pass;

	/* Create a new list in compile and execute mode */
	glNewList(index + 1, GL_COMPILE_AND_EXECUTE);
	glClearBufferfv(GL_COLOR,
			0,
			second);
	glEndList();

	/* Probe the color buffer after display list is executed */
	pass = piglit_probe_rect_rgba(0, 0,
				      piglit_width, piglit_height,
				      second)
		&& pass;
	/* To ensure that glNewList() is also able to compile the list in
	 * GL_COMPILE_AND_EXECUTE mode, clear the buffer to a unique color,
	 * call glCallList() and probe the color buffer again
	 */
	glClearBufferfv(GL_COLOR,
			0,
			third);
	/* Probe the color buffer before glCallList() */
	pass = piglit_probe_rect_rgba(0, 0,
				      piglit_width, piglit_height,
				      third)
		&& pass;
	glCallList(index + 1);
	/* Probe the color buffer after glCallList() */
	pass = piglit_probe_rect_rgba(0, 0,
				      piglit_width, piglit_height,
				      second)
		&& pass;
	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;
	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}
