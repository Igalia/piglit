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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Authors:
 *    Shuang he <shuang.he@intel.com>
 */


#include <GL/glut.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>


#define WIN_WIDTH 100
#define WIN_HEIGHT 100


static void
init(void)
{
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0, WIN_WIDTH, 0, WIN_HEIGHT, 2, -2);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
}

static void
display(void)
{
	static float white[] = {1.0, 1.0, 1.0, 0.0};
	static float red[] = {1.0, 0.0, 0.0, 0.0};
	static float green[] = {0.0, 1.0, 0.0, 0.0};
	static float blue[] = {0.0, 0.0, 1.0, 0.0};

	glClearColor(1.0, 1.0, 1.0, 1.0);
	glDisable(GL_DITHER);
	glEnable(GL_DEPTH_TEST);

	glClear(GL_COLOR_BUFFER_BIT);
	glDepthFunc(GL_ALWAYS);
	glRasterPos3f(0.0, 0.0, 0.5);
	glDrawPixels(1, 1, GL_RGBA, GL_FLOAT, white);
	glRasterPos3f(2.0, 0.0, 0.5);
	glDrawPixels(1, 1, GL_RGBA, GL_FLOAT, white);

	glDepthFunc(GL_LESS);
	glRasterPos3f(0.0, 0.0, 0.0);
	glDrawPixels(1, 1, GL_RGBA, GL_FLOAT, red);
	glRasterPos3f(2.0, 0.0, 1.0);
	glDrawPixels(1, 1, GL_RGBA, GL_FLOAT, blue);


	glutSwapBuffers();
}

int main(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_STENCIL);
	glutInitWindowSize(WIN_WIDTH, WIN_HEIGHT);
	glutCreateWindow("fdo23670-depth_test");
	glutDisplayFunc(display);

	init();

	glutMainLoop();

	return 0;

}
