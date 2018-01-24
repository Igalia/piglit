/*
 * Copyright 2018 VMware, Inc.
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
 * Test GL_COLOR_MATERIAL with vertex arrays
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN
	config.supports_gl_compat_version = 11;
	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;
	config.khr_no_error_support = PIGLIT_NO_ERRORS;
PIGLIT_GL_TEST_CONFIG_END


static const float white[] = {1.0, 1.0, 1.0, 1.0};
static const float black[] = {0.0, 0.0, 0.0, 0.0};


static bool
test(bool use_dlist)
{
	static const float almost_green[] = {0.0, 0.8, 0.0, 0.8};
	static const float pos[4][2] = {
		{ -1, -1 },
		{ 1, -1 },
		{ 1, 1 },
		{ -1, 1 }
	};
	static const float color[4][4] = {
		{ 0.0, 0.4, 0.0, 0.8 },
		{ 0.0, 0.4, 0.0, 0.8 },
		{ 0.0, 0.4, 0.0, 0.8 },
		{ 0.0, 0.4, 0.0, 0.8 }
	};
	GLuint list = 0;
	bool pass = true;

	glClearColor(0.5, 0.5, 0.5, 0.5);
	glClear(GL_COLOR_BUFFER_BIT);

	if (use_dlist) {
		list = glGenLists(1);
		glNewList(1, GL_COMPILE);
	}

	/* Change material per vertex */
	glColorPointer(4, GL_FLOAT, 0, color);
	glEnable(GL_COLOR_ARRAY);
	glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
	glEnable(GL_COLOR_MATERIAL);

	glVertexPointer(2, GL_FLOAT, 0, pos);
	glEnable(GL_VERTEX_ARRAY);

	glNormal3f(0, 0, 1);

	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	if (use_dlist) {
		glEndList();
		glCallList(list);
		glDeleteLists(list, 1);
	}

	pass = piglit_probe_rect_rgba(0, 0, piglit_width, piglit_height,
				      almost_green);
	piglit_present_results();

	if (!pass) {
		printf("Fail while testing %s\n",
		       use_dlist ? "display list" : "immediate mode");
	}

	return pass;
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
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glLightfv(GL_LIGHT0, GL_AMBIENT, white);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, white);
	glLightfv(GL_LIGHT0, GL_SPECULAR, black);
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, black);
}
