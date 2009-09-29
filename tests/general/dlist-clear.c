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

/** @file dlist-clear.c
 *
 * Tests that clears and primitives get stored properly in a
 * COMPILE_AND_EXECUTE display list.  Caught a regression in the intel driver
 * with the new metaops clear code.
 */

#include "piglit-util.h"

#define WIN_WIDTH 100
#define WIN_HEIGHT 100

static GLboolean Automatic = GL_FALSE;
static int win_width, win_height;

static void display(void)
{
	GLboolean pass = GL_TRUE;
	int x, y;
	static float red[]   = {1.0, 0.0, 0.0, 0.0};
	static float green[] = {0.0, 1.0, 0.0, 0.0};
	static float blue[]  = {0.0, 0.0, 1.0, 0.0};

	glClearColor(0.5, 0.0, 0.0, 0.0);
	glColor4fv(red);

	/* Make a list containing a clear and a rectangle.  It'll draw
	 * colors we don't expect to see, due to COMPILE_AND_EXECUTE.
	 */
	glNewList(1, GL_COMPILE_AND_EXECUTE);
	/* Even though we don't use depth, GL_DEPTH_BUFFER_BIT is what
	 * triggered the metaops clear path which messed up the display list.
	 */
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glBegin(GL_QUADS);
		glVertex2f(10, 10);
		glVertex2f(20, 10);
		glVertex2f(20, 20);
		glVertex2f(10, 20);
	glEnd();
	glEndList();

	/* Now, set up our expected colors, translate the dlist's rectangle
	 * over a little, and do the draw we actually expect to see.
	 */
	glClearColor(0.0, 1.0, 0.0, 0.0);
	glColor4fv(blue);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(20, 0, 0);

	glCallList(1);

	for (y = 0; y < win_height; y++) {
		for (x = 0; x < win_width; x++) {
			float *expected;

			if (x >= 30 && x < 40 && y >= 10 && y < 20)
				expected = blue;
			else
				expected = green;

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

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(WIN_WIDTH, WIN_HEIGHT);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("dlist-clear");
	glutKeyboardFunc(piglit_escape_exit_key);
	init();
	glutDisplayFunc(display);
	glutMainLoop();

	return 0;
}
