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
 * Test glDrawPixels(GL_DEPTH_COMPONENT)
 * Brian Paul
 * June 2012
 *
 * We don't rely on glReadPixels(GL_DEPTH_COMPONENT) in case it's not
 * working.  Instead we test by drawing an image into the depth buffer
 * while setting the color buffer to white.  Next, we draw quads just
 * in front and behind where we expect the Z values to be.  The quad
 * behind should be invisible while the quad in front should be totally
 * visible.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_width = 200;
	config.window_height = 200;
	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_DEPTH;

PIGLIT_GL_TEST_CONFIG_END

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
	/* a tight epsilon isn't important for this test */
	const GLfloat epsilon = 4.0 / piglit_width;
	static const GLfloat white[4] = { 1, 1, 1, 1 };
	static const GLfloat green[4] = { 0, 1, 0, 1 };
	static const GLfloat red[4] = { 1, 0, 0, 1 };
	GLfloat *buf;
	bool pass = true;
	int i, j;
	float zLeft, zRight;

	/* For both glDrawPixels and the polygon rendering below we
	 * use a range of Z values in [0, 1] where 0=near and 1=far.
	 * So object Z coords are the same as normalized depth coords.
	 */

	zLeft = epsilon;
	zRight = 1.0 - epsilon;

	/* create image of Z values increasing from left to right */
	buf = (GLfloat *)
		malloc(piglit_width * piglit_height * sizeof(GLfloat));
	for (j = 0; j < piglit_height; j++) {
		for (i = 0; i < piglit_width; i++) {
			float z = i / (float) (piglit_width - 1);
			z = zLeft + z * (zRight - zLeft);
			buf[j * piglit_width + i] = z;
		}
	}

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	/* glDrawPixels the Z gradient image */
	glColor4fv(white);
	glWindowPos2i(0, 0);
	glDrawPixels(piglit_width, piglit_height,
		     GL_DEPTH_COMPONENT, GL_FLOAT, buf);

	free(buf);

	/* draw a red quad behind the Z gradient - it should not be visible */
	glColor4fv(red);
	draw_z_gradient(zLeft + epsilon, zRight + epsilon);
	if (!piglit_probe_rect_rgb(0, 0, piglit_width, piglit_height, white)) {
		printf("Quad behind test failed\n");
		pass = false;
	}

	/* draw green quad in front of the Z gradient - it should be visible */
	glColor4fv(green);
	draw_z_gradient(zLeft - epsilon, zRight - epsilon);
	if (!piglit_probe_rect_rgb(0, 0, piglit_width, piglit_height, green)) {
		printf("Quad in front test failed\n");
		pass = false;
	}

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}


void
piglit_init(int argc, char **argv)
{
	/* Setup projection matrix such that zObj=0 becomes zBuffer=0
	 * and zObj=1 becomes zBuffer=1 (identity transform).
	 * So, glOrtho maps zObj=0 to zNDC=-1 and maps zObj=1 to zNDC=1.
	 * Then, zNDC=-1 maps to zBuffer=0 and zNDC=1 maps to zBuffer=1.
	 */
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-1.0, 1.0, -1.0, 1.0, 0.0, -1.0);

	glEnable(GL_DEPTH_TEST);
}
