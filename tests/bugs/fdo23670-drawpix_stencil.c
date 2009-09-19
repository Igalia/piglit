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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <GL/glew.h>
#if defined(__APPLE__)
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#include "piglit-util.h"

#define WIN_WIDTH 100
#define WIN_HEIGHT 100

static GLboolean Automatic = GL_FALSE;

static void
init(void)
{
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0, WIN_WIDTH, 0, WIN_HEIGHT, -1, 1);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
}

static void
display(void)
{
	GLubyte stencil_rect[20 * 20];
	int i;
	int x, y;
	GLboolean pass = GL_TRUE;
	static float red[] = {1.0, 0.0, 0,0, 0.0};
	static float black[] = {0.0, 0.0, 0,0, 0.0};

	glClearStencil(0);
	glClearDepth(1.0);
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glDisable(GL_DITHER);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, 20);

	for (i=0; i<20*20; i++)
		if (i < 20*20/2)
			stencil_rect[i] = 1;
		else
			stencil_rect[i] = 0;


	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	glEnable(GL_STENCIL_TEST);
	glStencilFunc(GL_LESS, 0, (GLuint)~0);
	glRasterPos2i(50, 50);
	glDrawPixels(20, 20, GL_STENCIL_INDEX, GL_UNSIGNED_BYTE, stencil_rect);
	glColor3f(1.0, 0.0, 0.0);
	glRectf(50, 50, 50+20, 50+20);
	glDisable(GL_STENCIL_TEST);

	for (x = 50; x < 50 + 20; x++) {
		for (y = 50; y < 50 + 10; y++) {
			pass &= piglit_probe_pixel_rgb(x, y, red);
		}
		for (y = 50 + 10; y < 50 + 20; y++) {
			pass &= piglit_probe_pixel_rgb(x, y, black);
		}
	}

	glutSwapBuffers();

	if (Automatic) {
		piglit_report_result(pass ? PIGLIT_SUCCESS : PIGLIT_FAILURE);
	}
}

int main(int argc, char **argv)
{
	int i;
	for(i = 1; i < argc; ++i) {
		if (!strcmp(argv[i], "-auto"))
			Automatic = 1;
		else
			printf("Unknown option: %s\n", argv[i]);
	}

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_STENCIL);
	glutInitWindowSize(WIN_WIDTH, WIN_HEIGHT);
	glutCreateWindow("fdo23670-drawpix_stencil");
	glutKeyboardFunc(piglit_escape_exit_key);
	glutDisplayFunc(display);

	init();

	glutMainLoop();

	return 0;

}
