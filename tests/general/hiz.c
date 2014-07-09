/*
 * Copyright © 2010 Marek Olšák <maraeo@gmail.com>
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
 *
 * Authors:
 *    Marek Olšák <maraeo@gmail.com>
 *
 */

/** @file r300-hiz-bug.c
 *
 * Tests that two overlapping triangles are rendered correctly.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_width = 400;
	config.window_height = 400;
	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DEPTH | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

enum {
	INSIDE,
	EDGE,
	OUTSIDE
};

int tri_point_intersect_2d(const float v0[2],
			   const float v1[2],
			   const float v2[2],
			   const float p[2],
                           float dist_eps)
{
	float n0[2];
	float n1[2];
	float n2[3];
	float l0, l1, l2;
	float c0, c1, c2;
	float d0, d1, d2;

	/* compute edge normals */
	n0[0] = v1[1] - v0[1];
	n0[1] = v0[0] - v1[0];

	n1[0] = v2[1] - v1[1];
	n1[1] = v1[0] - v2[0];

	n2[0] = v0[1] - v2[1];
	n2[1] = v2[0] - v0[0];

	/* compute inverse lengths of the normals */
	l0 = 1 / sqrt(n0[0]*n0[0] + n0[1]*n0[1]);
	l1 = 1 / sqrt(n1[0]*n1[0] + n1[1]*n1[1]);
	l2 = 1 / sqrt(n2[0]*n2[0] + n2[1]*n2[1]);

	/* normalize the normals */
	n0[0] *= l0;
	n0[1] *= l0;
	n1[0] *= l1;
	n1[1] *= l1;
	n2[0] *= l2;
	n2[1] *= l2;

	/* compute negative dot products between normals and vertices
	 * to get ray equations in the form nx*x + ny*y + c = 0. */
	c0 = - n0[0]*v0[0] - n0[1]*v0[1];
	c1 = - n1[0]*v1[0] - n1[1]*v1[1];
	c2 = - n2[0]*v2[0] - n2[1]*v2[1];

	/* compute the distances between the point and the edges. */
	d0 = n0[0]*p[0] + n0[1]*p[1] + c0;
	d1 = n1[0]*p[0] + n1[1]*p[1] + c1;
	d2 = n2[0]*p[0] + n2[1]*p[1] + c2;

	/* the point is inside the triangle */
	if (d0 < -dist_eps && d1 < -dist_eps && d2 < -dist_eps) {
		return INSIDE;
	}

	/* the point is outside the triangle */
	if (d0 > dist_eps || d1 > dist_eps || d2 > dist_eps) {
		return OUTSIDE;
	}

	return EDGE;
}

GLboolean pix_equal(int x, int y, const float probe[3], const float expected[3])
{
	int i;
	GLboolean ret = GL_TRUE;

	for (i = 0; i < 3; i++) {
		if (fabs(probe[i] - expected[i]) > 0.01) {
			ret = GL_FALSE;
			break;
		}
	}

	if (!ret) {
		printf("Probe color at (%i,%i)\n", x, y);
		printf("  Expected: %f %f %f\n", expected[0], expected[1], expected[2]);
		printf("  Observed: %f %f %f\n", probe[0], probe[1], probe[2]);
	}
	return ret;
}

GLboolean test_less()
{
	const float bg[3] = {0.1, 0.1, 0.1};
	const float c1[3] = {1.0, 0.3, 0.3};
	const float v11[3] = {0, 1,   -1};
	const float v12[3] = {0, 0,   -1};
	const float v13[3] = {1, 0.5,  1};
	const float c2[3] = {0.0, 1.0, 1};
	const float v21[3] = {1, 1,   0};
	const float v22[3] = {0, 0.5, 0};
	const float v23[3] = {1, 0,   0};
	float *pix;
	int i,j;
        float dist_eps = 1.0f / MIN2(piglit_width, piglit_height);

	glClearDepth(1);
	glDepthFunc(GL_LESS);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glBegin (GL_TRIANGLES);
	glColor3fv(c1);
	glVertex3fv(v11);
	glVertex3fv(v12);
	glVertex3fv(v13);
	glEnd();

	glBegin (GL_TRIANGLES);
	glColor3fv(c2);
	glVertex3fv(v21);
	glVertex3fv(v22);
	glVertex3fv(v23);
	glEnd();

	pix = malloc(piglit_width * piglit_height * 3 * sizeof(float));
	glReadPixels(0, 0, piglit_width, piglit_height, GL_RGB, GL_FLOAT, pix);

	/* check pixels */
	for (j = 0; j < piglit_height; j++) {
		for (i = 0; i < piglit_width; i++) {
			float *px = &pix[(j*piglit_width + i) * 3];
			float p[2];
			int t1_intersect, t2_intersect;

			p[0] = i / (float)(piglit_width-1);
			p[1] = j / (float)(piglit_height-1);

			t1_intersect = tri_point_intersect_2d(v11, v12, v13, p, dist_eps);
			t2_intersect = tri_point_intersect_2d(v21, v22, v23, p, dist_eps);

			if (t1_intersect == EDGE || t2_intersect == EDGE) {
				//printf(" ");
				continue;
			}

			if (t1_intersect == INSIDE) {
				if (t2_intersect == INSIDE) {
					if (fabs(p[0] - 0.5) < dist_eps) {
						//printf(" ");
						continue;
					}
					if (p[0] < 0.5) {
						//printf("2");
						if (!pix_equal(i, j, px, c2)) {
							free(pix);
							return GL_FALSE;
						}
					} else {
						//printf("1");
						if (!pix_equal(i, j, px, c1)) {
							free(pix);
							return GL_FALSE;
						}
					}
				} else {
					//printf("1");
					if (!pix_equal(i, j, px, c1)) {
						free(pix);
						return GL_FALSE;
					}
				}
			} else {
				if (t2_intersect == INSIDE) {
					//printf("2");
					if (!pix_equal(i, j, px, c2)) {
						free(pix);
						return GL_FALSE;
					}
				} else {
					//printf("0");
					if (!pix_equal(i, j, px, bg)) {
						free(pix);
						return GL_FALSE;
					}
				}
			}

			/*if (i == piglit_width-1)
				printf("\n");*/
		}
	}

	free(pix);
	return GL_TRUE;
}

enum piglit_result piglit_display()
{
	GLboolean pass = GL_TRUE;

	pass = pass && test_less();
	piglit_present_results();
	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void piglit_init(int argc, char**argv)
{
	glClearColor(0.1, 0.1, 0.1, 0.1);
	glEnable(GL_DEPTH_TEST);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0, 1.0, 0.0, 1.0, -1, 1);

	printf("First the red triangle is drawn, then the blue one.\n");
}
