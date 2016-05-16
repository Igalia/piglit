/*
 * Copyright 2016 VMware, Inc.
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
 * Basic test glCopyPixels in XOR mode.
 *
 * Brian Paul
 * May 2016
 */


#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN
	config.supports_gl_compat_version = 11;
	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;
PIGLIT_GL_TEST_CONFIG_END


static const float black[4] = {0, 0, 0, 0};


static void
draw_test_pattern(void)
{
	glPushMatrix();
	glTranslatef(-0.5, 0, 0);
	glScalef(0.5, 1, 1);

	/* draw background colored quad */
	glBegin(GL_TRIANGLE_FAN);
	glColor3f(1, 0, 0);
	glVertex2f(-1, -1);
	glColor3f(0, 1, 0);
	glVertex2f( 1, -1);
	glColor3f(0, 0, 1);
	glVertex2f( 1,  1);
	glColor3f(1, 1, 0);
	glVertex2f(-1,  1);
	glEnd();

	/* draw lines */
	glColor3f(1, 1, 1);
	glBegin(GL_LINE_STRIP);
	glVertex2f(0, -0.9);
	glVertex2f(0.9, 0);
	glVertex2f(0, 0.9);
	glVertex2f(-0.9, 0);
	glVertex2f(0, -0.9);
	glEnd();
	glPopMatrix();
}


enum piglit_result
piglit_display(void)
{
	bool pass = true;
	int x2 = piglit_width / 2;
	GLfloat pixel[4];

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-1, 1, -1, 1, -1, 1);

	glViewport(0, 0, piglit_width, piglit_height);

	glClear(GL_COLOR_BUFFER_BIT);

	draw_test_pattern();

	/* copy image from left to right side of window */
	glWindowPos2i(x2, 0);
	glCopyPixels(0, 0, x2, piglit_height, GL_COLOR);

	/* Check that the copy did something */
	glReadPixels(piglit_width * 3 / 4, piglit_height / 2, 1, 1,
		     GL_RGBA, GL_FLOAT, pixel);
	if (pixel[0] == 0.0 ||
	    pixel[1] == 0.0 ||
	    pixel[2] == 0.0) {
		printf("glCopyPixels appeared to fail.\n");
		pass = false;
	}

	/* copy the image again (left to right, but with XOR) */
	glLogicOp(GL_XOR);
	glEnable(GL_COLOR_LOGIC_OP);
	glCopyPixels(0, 0, x2, piglit_height, GL_COLOR);

	/* now the right half of the window show be black again */
	if (!piglit_probe_rect_rgba(x2, 0, x2, piglit_height, black)) {
		printf("XOR glCopyPixels failed to erase image\n");
		pass = false;
	}

	glDisable(GL_COLOR_LOGIC_OP);

	piglit_present_results();

	piglit_check_gl_error(GL_NO_ERROR);

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}


void
piglit_init(int argc, char **argv)
{
	/* nothing */
}
