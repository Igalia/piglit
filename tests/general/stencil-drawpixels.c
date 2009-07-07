/*
 * Copyright © 2009 Intel Corporation
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
 *    Eric Anholt <eric@anholt.net>
 *
 */

/** @file stencil-drawpixels.c
 *
 * Tests that glDrawPixels(GL_STENCIL) minimally works.
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#if defined(__APPLE__)
#include <GLUT/glut.h>
#else
#include "GL/glut.h"
#endif
#include "piglit-util.h"

#define WIN_WIDTH 100
#define WIN_HEIGHT 100

static GLboolean Automatic = GL_FALSE;
static int win_width, win_height;

static void display()
{
	GLboolean pass = GL_TRUE;
	int x, y, i;
	static float red[]   = {1.0, 0.0, 0.0, 0.0};
	static float green[] = {0.0, 1.0, 0.0, 0.0};
	static float blue[]  = {0.0, 0.0, 1.0, 0.0};
	static float square[100];

	/* whole window gray -- none should be visible */
	glClearColor(0.5, 0.5, 0.5, 0.0);
	glClear(GL_COLOR_BUFFER_BIT);

	/* Clear stencil to 0, which will be drawn red */
	glClearStencil(0);
	glClear(GL_STENCIL_BUFFER_BIT);

	/* quad at 10, 10 that will be drawn green. */
	for (i = 0; i < 100; i++)
		square[i] = 1.0;
	glRasterPos2i(10, 10);
	glDrawPixels(10, 10, GL_STENCIL_INDEX, GL_FLOAT, square);

	/* quad at 30, 10 that will be drawn blue. */
	for (i = 0; i < 100; i++)
		square[i] = 2.0;
	glRasterPos2i(30, 10);
	glDrawPixels(10, 10, GL_STENCIL_INDEX, GL_FLOAT, square);

	glDisable(GL_SCISSOR_TEST);
	glEnable(GL_STENCIL_TEST);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

	/* First quad -- stencil == 0 gets red */
	glStencilFunc(GL_EQUAL, 0, ~0);
	glColor4fv(red);
	piglit_draw_rect(0, 0, win_width, win_height);

	/* Second quad -- stencil == 1 gets green */
	glStencilFunc(GL_EQUAL, 1, ~0);
	glColor4fv(green);
	piglit_draw_rect(0, 0, win_width, win_height);

	/* Last quad -- stencil == 2 gets blue */
	glStencilFunc(GL_EQUAL, 2, ~0);
	glColor4fv(blue);
	piglit_draw_rect(0, 0, win_width, win_height);

	assert(glGetError() == 0);

	for (y = 0; y < win_height; y++) {
		for (x = 0; x < win_width; x++) {
			float *expected;

			if (x >= 10 && x < 20 && y >= 10 && y < 20)
				expected = green;
			else if (x >= 30 && x < 40 && y >= 10 && y < 20)
				expected = blue;
			else
				expected = red;

			pass &= piglit_probe_pixel_rgb(x, y, expected);
		}
	}

	glutSwapBuffers();

	if (Automatic) {
		piglit_report_result(pass ? PIGLIT_SUCCESS : PIGLIT_FAILURE);
	}
}


static void reshape(int width, int height)
{
	win_width = width;
	win_height = height;

	glViewport(0, 0, width, height);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0, width, 0.0, height, -1.0, 1.0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

static void
init(void)
{
	reshape(WIN_WIDTH, WIN_HEIGHT);
}

int main(int argc, char**argv)
{
	int i;
	glutInit(&argc, argv);

	for(i = 1; i < argc; ++i) {
		if (!strcmp(argv[i], "-auto"))
			Automatic = 1;
		else
			printf("Unknown option: %s\n", argv[i]);
	}

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_STENCIL);
	glutInitWindowSize(WIN_WIDTH, WIN_HEIGHT);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("stencil-drawpixels");
	glutKeyboardFunc(piglit_escape_exit_key);
	init();
	glutDisplayFunc(display);
	glutMainLoop();

	return 0;
}
