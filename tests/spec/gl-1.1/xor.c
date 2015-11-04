/*
 * Copyright 2015 VMware, Inc.
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
 * Test drawing in XOR mode.  XOR mode is often used for "rubber-band"
 * selection boxes, etc. in CAD apps.  Test that this basically works.
 *
 * Brian Paul
 * Nov, 4 2015
 */


#include "piglit-util-gl.h"


/* Note: RGBA logicops were added in GL 1.1 */

PIGLIT_GL_TEST_CONFIG_BEGIN
	config.supports_gl_compat_version = 11;
	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;
PIGLIT_GL_TEST_CONFIG_END


static void
draw_lines(void)
{
	glBegin(GL_LINE_STRIP);
	glVertex2f(0, -0.9);
	glVertex2f(0.9, 0);
	glVertex2f(0, 0.9);
	glVertex2f(-0.9, 0);
	glVertex2f(0, -0.9);
	glEnd();
}


static bool
test(float line_width, const float color[4])
{
	int image_bytes = piglit_width * piglit_height * 4 * sizeof(GLubyte);
	GLubyte *ref_image, *test_image;
	bool pass = true;

	ref_image = malloc(image_bytes);
	test_image = malloc(image_bytes);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-1, 1, -1, 1, -1, 1);

	glViewport(0, 0, piglit_width, piglit_height);

	glClear(GL_COLOR_BUFFER_BIT);

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

	/* Read reference image */
	glReadPixels(0, 0, piglit_width, piglit_height,
		     GL_RGBA, GL_UNSIGNED_BYTE, ref_image);

	/* draw XOR lines */
	glLogicOp(GL_XOR);
	glEnable(GL_COLOR_LOGIC_OP);
	glColor4fv(color);
	glLineWidth(line_width);
	draw_lines();

	glReadPixels(0, 0, piglit_width, piglit_height,
		     GL_RGBA, GL_UNSIGNED_BYTE, test_image);
	/* images should differ */
	if (memcmp(ref_image, test_image, image_bytes) == 0) {
		printf("Failure: images should differ after drawing XOR lines.\n"
		       "(line width %g, color %g, %g, %g, %g)\n",
		       line_width, color[0], color[1], color[2], color[3]);
		pass = false;
	}

	/* draw lines again - should "erase" previous lines */
	draw_lines();

	glReadPixels(0, 0, piglit_width, piglit_height,
		     GL_RGBA, GL_UNSIGNED_BYTE, test_image);
	/* images should match */
	if (memcmp(ref_image, test_image, image_bytes) != 0) {
		printf("Failure: images should match after drawing XOR lines twice.\n"
		       "(line width %g, color %g, %g, %g, %g)\n",
		       line_width, color[0], color[1], color[2], color[3]);
		pass = false;
	}

	glDisable(GL_COLOR_LOGIC_OP);

	free(test_image);
	free(ref_image);

	piglit_present_results();

	piglit_check_gl_error(GL_NO_ERROR);

	return pass;
}


enum piglit_result
piglit_display(void)
{
	static const float white[4] = { 1, 1, 1, 1};
	static const float greenish[4] = { 0.25, 1, 0.5, 0.5 };
	bool pass = true;

	pass = test(1.0, white) && pass;
	pass = test(4.0, white) && pass;
	pass = test(1.0, greenish) && pass;
	pass = test(4.0, greenish) && pass;

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}


void
piglit_init(int argc, char **argv)
{
	/* nothing */
}
