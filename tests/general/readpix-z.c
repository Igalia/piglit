/*
 * Copyright 2012 VMware, Inc.
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
 */

/**
 * Test glReadPixels(GL_DEPTH_COMPONENT)
 * Brian Paul
 * June 2012
 */

#include "piglit-util-gl-common.h"

PIGLIT_GL_TEST_MAIN(200 /*window_width*/,
                    200 /*window_height*/,
                    PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_DEPTH);

static void
draw_z_gradient(GLfloat zLeft, GLfloat zRight)
{
	GLfloat verts[4][3];

	verts[0][0] = -1.0;  verts[0][1] = -1.0;  verts[0][2] = zLeft;
	verts[1][0] =  1.0;  verts[1][1] = -1.0;  verts[1][2] = zRight;
	verts[2][0] =  1.0;  verts[2][1] =  1.0;  verts[2][2] = zRight;
	verts[3][0] = -1.0;  verts[3][1] =  1.0;  verts[3][2] = zLeft;

	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, verts);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}


enum piglit_result
piglit_display(void)
{
	const GLfloat epsilon = 2.0 / piglit_width;
	GLfloat *buf;
	bool pass = true;
	int pos, i;

	/* draw full-window quad with z increasing left to right */
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	draw_z_gradient(-1.0, 1.0);

	buf = (GLfloat *) malloc(piglit_width * sizeof(GLfloat));

	glReadPixels(0, piglit_height / 2, piglit_width, 1,
		     GL_DEPTH_COMPONENT, GL_FLOAT, buf);

	pos = 0;
	if (fabs(buf[pos] - 0.0) > epsilon) {
		printf("Left-most Z value should be close to 0.0, found %f\n",
		       buf[pos]);
		pass = false;
	}

	pos = piglit_width / 2;
	if (fabs(buf[pos] - 0.5) > epsilon) {
		printf("Middle Z value should be close to 0.5, found %f\n",
		       buf[pos]);
		pass = false;
	}

	pos = piglit_width - 1;
	if (fabs(buf[pos] - 1.0) > epsilon) {
		printf("Left-most Z value should be close to 1.0, found %f\n",
		       buf[pos]);
		pass = false;
	}

	/* check for monotonicity */
	for (i = 1; i < piglit_width; i++) {
		if (buf[i - 1] > buf[i]) {
			printf("Z values aren't increasing from left to right. buf[%d]=%f > buf[%d]=%f\n",
			       i-1, buf[i-1], i, buf[i]);
			pass = false;
			break;
		}
	}

	free(buf);

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}


void
piglit_init(int argc, char **argv)
{
}
