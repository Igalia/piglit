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

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_width = 200;
	config.window_height = 200;
	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_DEPTH;

PIGLIT_GL_TEST_CONFIG_END



/** Display contents of the depth buffer as grayscale color */
static void
display_depth(const GLfloat *buf)
{
	glWindowPos2i(0, 0);
	glDrawPixels(piglit_width, piglit_height,
		     GL_LUMINANCE, GL_FLOAT, buf);
}


/** Test glClear(GL_DEPTH_BUFFER_BIT) + glReadPixels */
static bool
test_z_clear(void)
{
	GLfloat *buf = malloc(piglit_width * piglit_height * sizeof(GLfloat));
	float z;
	double diff, tolerance;
	GLint zBits, i;

	glGetIntegerv(GL_DEPTH_BITS, &zBits);

	/* allow 1-bit error */
	tolerance = 1.0 / (1 << (zBits - 1));

	for (z = 0.0f; z <= 1.0f; z += 0.125) {
		glClearDepth(z);
		glClear(GL_DEPTH_BUFFER_BIT);

		glReadPixels(0, 0, piglit_width, piglit_height,
			     GL_DEPTH_COMPONENT, GL_FLOAT, buf);

		if (!piglit_automatic) {
			display_depth(buf);
			piglit_present_results();
		}

		/* Make sure all the values are the same */
		for (i = 1; i < piglit_width * piglit_height; i++) {
			if (buf[i] != buf[0]) {
				printf("depth[%d]=%f != depth[0]=%f\n",
				       i, buf[i], buf[0]);
				free(buf);
				return false;
			}
		}

		/* Check that the depth value read back is within tolerance */
		diff = buf[0] - z;
		if (diff > tolerance) {
			printf("Depth buffer clear failed!\n");
			printf("Expected %f, found %f\n", z, buf[0]);
			free(buf);
			return false;
		}
	}

	free(buf);
	return true;
}


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


static bool
test_z_gradient(void)
{
	const GLfloat epsilon = 2.0 / piglit_width;
	GLfloat *buf, *row;
	bool pass = true;
	int pos, i;

	/* draw full-window quad with z increasing left to right */
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	draw_z_gradient(-1.0, 1.0);
	glDisable(GL_DEPTH_TEST);

	buf = (GLfloat *) malloc(piglit_width * piglit_height * sizeof(GLfloat));

	/* read whole buffer */
	glReadPixels(0, 0, piglit_width, piglit_height,
		     GL_DEPTH_COMPONENT, GL_FLOAT, buf);

	/* examine a horizontal row at mid-Y */
	row = buf + piglit_width * piglit_height / 2;

	pos = 0;
	if (fabs(row[pos] - 0.0) > epsilon) {
		printf("Left-most Z value should be close to 0.0, found %f\n",
		       row[pos]);
		pass = false;
	}

	pos = piglit_width / 2;
	if (fabs(row[pos] - 0.5) > epsilon) {
		printf("Middle Z value should be close to 0.5, found %f\n",
		       row[pos]);
		pass = false;
	}

	pos = piglit_width - 1;
	if (fabs(row[pos] - 1.0) > epsilon) {
		printf("Left-most Z value should be close to 1.0, found %f\n",
		       row[pos]);
		pass = false;
	}

	/* check for monotonicity */
	for (i = 1; i < piglit_width; i++) {
		if (row[i - 1] > row[i]) {
			printf("Z values aren't increasing from left to right. row[%d]=%f > row[%d]=%f\n",
			       i-1, row[i-1], i, row[i]);
			pass = false;
			break;
		}
	}

	if (!piglit_automatic) {
		display_depth(buf);
		piglit_present_results();
	}

	free(buf);

	return pass;
}


enum piglit_result
piglit_display(void)
{
	if (!test_z_clear())
		return PIGLIT_FAIL;

	if (!test_z_gradient())
		return PIGLIT_FAIL;

	return PIGLIT_PASS;
}


void
piglit_init(int argc, char **argv)
{
}
