/*
 * Copyright (C) 2018 VMware, Inc.
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
 * Test glDrawArrays with non-zero start parameter, with and without
 * display lists.
 */

#include "piglit-util-gl.h"


PIGLIT_GL_TEST_CONFIG_BEGIN
	config.supports_gl_compat_version = 14;
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;
	config.khr_no_error_support = PIGLIT_NO_ERRORS;
PIGLIT_GL_TEST_CONFIG_END


static const float verts[][2] = {
	{ -0.75, -0.25 },
	{ -0.25, -0.25 },
	{ -0.25,  0.25 },
	{ -0.75,  0.25 },
	{  0.1f, -0.9f },
	{  0.9f, -0.9f },
	{  0.9f,  0.9f },
	{  0.1f,  0.9f }
};


static const float white[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
static const float black[4] = { 0.0f, 0.0f, 0.0f, 0.0f };


static bool
test(bool use_dlist)
{
	GLuint list1 = 0, list2 = 0;
	bool pass;

	glClear(GL_COLOR_BUFFER_BIT);

	if (use_dlist) {
		list1 = glGenLists(1);
		glNewList(list1, GL_COMPILE);
	}

	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(2, GL_FLOAT, 0, verts);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	if (use_dlist) {
		glEndList();

		list2 = glGenLists(1);
		glNewList(list2, GL_COMPILE);
	}

	glDrawArrays(GL_TRIANGLE_FAN, 4, 4);

	if (use_dlist) {
		glEndList();

		glCallList(list1);
		glCallList(list2);

		glDeleteLists(list1, 1);
		glDeleteLists(list2, 1);
	}

	pass = piglit_probe_pixel_rgba(piglit_width*1/4, piglit_height/2,
				       white);
	pass = piglit_probe_pixel_rgba(piglit_width*3/4, piglit_height/2,
				       white) && pass;
	pass = piglit_probe_pixel_rgba(piglit_width*3/4, 2, black) && pass;

	piglit_present_results();

	if (!pass) {
		printf("Fail while testing %s\n",
		       use_dlist ? "display list" : "immediate mode");
	}

	return true;
}


enum piglit_result
piglit_display(void)
{
	bool pass = test(false);
	pass = test(true) && pass;
	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}


void
piglit_init(int argc, char **argv)
{
	/* nothing */
}
