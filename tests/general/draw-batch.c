/*
 * Copyright © 2011 Marek Olšák <maraeo@gmail.com>
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
 *
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

void
piglit_init(int argc, char **argv)
{
	piglit_require_gl_version(14);
}

static void rotate_colors(float *array)
{
	unsigned i;
	float tmp[3];

	for (i = 0; i < 3; i++) {
		memcpy(tmp,		 array + i*5 + 2,  12);
		memcpy(array + i*5 + 2,  array + i*5 + 17, 12);
		memcpy(array + i*5 + 17, array + i*5 + 32, 12);
		memcpy(array + i*5 + 32, array + i*5 + 47, 12);
		memcpy(array + i*5 + 47, tmp,		   12);
	}
}

enum piglit_result
piglit_display(void)
{
	GLboolean pass = GL_TRUE;
	float array[] = {
		10, 10,	   1, 0, 0,
		27, 10,	   1, 0, 0,
		10, 30,	   1, 0, 0,

		30, 10,	   0, 1, 0,
		47, 10,	   0, 1, 0,
		30, 30,	   0, 1, 0,

		50, 10,	   0, 0, 1,
		67, 10,	   0, 0, 1,
		50, 30,	   0, 0, 1,

		70, 10,	   1, 0, 1,
		87, 10,	   1, 0, 1,
		70, 30,	   1, 0, 1
	};
	float seccol[] = {
		0.2, 0.2, 0.2,
		0.2, 0.2, 0.2,
		0.2, 0.2, 0.2,

		0.2, 0.2, 0.2,
		0.2, 0.2, 0.2,
		0.2, 0.2, 0.2,

		0.2, 0.2, 0.2,
		0.2, 0.2, 0.2,
		0.2, 0.2, 0.2,

		0.2, 0.2, 0.2,
		0.2, 0.2, 0.2,
		0.2, 0.2, 0.2,
	     };
	float c0[] = {1, 0.2, 0.2};
	float c1[] = {0.2, 1, 0.2};
	float c2[] = {0.2, 0.2, 1};
	float c3[] = {1, 0.2, 1};
	short indices[] = {
		0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11
	};
	int i, j;

	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);

	printf("From bottom to top:\n");

	glEnable(GL_COLOR_SUM);
	glLoadIdentity();

	/* State change: Vertex arrays. */
	glVertexPointer(2, GL_FLOAT, 20, array);
	glColorPointer(3, GL_FLOAT, 20, array + 2);
	glSecondaryColorPointer(3, GL_FLOAT, 0, seccol);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	glEnableClientState(GL_SECONDARY_COLOR_ARRAY);

	/* The vertex array state should be preserved after glClear. */
	glClear(GL_COLOR_BUFFER_BIT);

	/* Draw. */
	printf("DrawElements\n");
	for (i = 0; i < 4; i++)
		glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_SHORT, indices + i*3);

	/* State change: Constant buffer. */
	glTranslatef(0, 30, 0);

	rotate_colors(array);

	/* Draw. */
	printf("DrawArrays\n");
	for (i = 0; i < 4; i++)
		glDrawArrays(GL_TRIANGLES, i*3, 3);

	/* State change: Vertex arrays. */
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_SECONDARY_COLOR_ARRAY);

	/* State change: Constant buffer. */
	glTranslatef(0, 30, 0);

	rotate_colors(array);

	/* Draw. */
	printf("Begin/End\n");
	for (i = 0; i < 4; i++) {
		glBegin(GL_TRIANGLES);
		for (j = 0; j < 3; j++) {
			glColor3fv(array + i*15 + j*5 + 2);
			glSecondaryColor3fv(seccol);
			glVertex2fv(array + i*15 + j*5);
		}
		glEnd();
	}

	/* State change: Constant buffer. */
	glTranslatef(0, 30, 0);

	rotate_colors(array);

	/* Create display lists. */
	for (i = 0; i < 4; i++) {
		glNewList(i+1, GL_COMPILE);
		glBegin(GL_TRIANGLES);
		for (j = 0; j < 3; j++) {
			glColor3fv(array + i*15 + j*5 + 2);
			glSecondaryColor3fv(seccol);
			glVertex2fv(array + i*15 + j*5);
		}
		glEnd();
		glEndList();
	}

	/* Draw. */
	printf("CallList\n");
	for (i = 0; i < 4; i++) {
		glCallList(i+1);
	}

	pass = pass && piglit_probe_pixel_rgb(15, 15, c0);
	pass = pass && piglit_probe_pixel_rgb(35, 15, c1);
	pass = pass && piglit_probe_pixel_rgb(55, 15, c2);
	pass = pass && piglit_probe_pixel_rgb(75, 15, c3);

	pass = pass && piglit_probe_pixel_rgb(15, 45, c1);
	pass = pass && piglit_probe_pixel_rgb(35, 45, c2);
	pass = pass && piglit_probe_pixel_rgb(55, 45, c3);
	pass = pass && piglit_probe_pixel_rgb(75, 45, c0);

	pass = pass && piglit_probe_pixel_rgb(15, 75, c2);
	pass = pass && piglit_probe_pixel_rgb(35, 75, c3);
	pass = pass && piglit_probe_pixel_rgb(55, 75, c0);
	pass = pass && piglit_probe_pixel_rgb(75, 75, c1);

	pass = pass && piglit_probe_pixel_rgb(15, 105, c3);
	pass = pass && piglit_probe_pixel_rgb(35, 105, c0);
	pass = pass && piglit_probe_pixel_rgb(55, 105, c1);
	pass = pass && piglit_probe_pixel_rgb(75, 105, c2);

	piglit_present_results();
	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}
