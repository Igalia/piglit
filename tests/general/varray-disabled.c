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

/**
 * @file varray-disabled.c
 *
 * Test whether no vertices are drawn when we call DrawArrays with no
 * vertex array enabled.
 *
 * http://bugs.freedesktop.org/show_bug.cgi?id=19911
 */

#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <GL/glut.h>

#include "piglit-util.h"

static int Width = 128, Height = 128;
static int Automatic = 0;

static void
set_colors(GLfloat *color_array, GLfloat *color)
{
	int i;

	for (i = 0; i < 4; i++) {
		color_array[i * 4 + 0] = color[0];
		color_array[i * 4 + 1] = color[1];
		color_array[i * 4 + 2] = color[2];
		color_array[i * 4 + 3] = 1.0;
	}
}

static void Redisplay(void)
{
	GLfloat vertices[4][2];
	GLfloat colors[16];
	GLboolean pass = GL_TRUE;
	GLfloat red[3]    = {1.0, 0.0, 0.0};
	GLfloat green[3]  = {0.0, 1.0, 0.0};
	GLfloat blue[3]   = {0.0, 0.0, 1.0};
	GLfloat black[3]  = {0.0, 0.0, 0.0};

	glClearColor(0.0, 0.0, 0.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT);

	glColorPointer(4, GL_FLOAT, 0, colors);
	glEnableClientState(GL_COLOR_ARRAY);

	/* Draw the vertices enabled once on the left side for sanity */
	vertices[0][0] = 0.0;
	vertices[0][1] = 0.0;
	vertices[1][0] = 0.3;
	vertices[1][1] = 0.0;
	vertices[2][0] = 0.3;
	vertices[2][1] = 1.0;
	vertices[3][0] = 0.0;
	vertices[3][1] = 1.0;
	glVertexPointer(2, GL_FLOAT, 0, vertices);
	glEnableClientState(GL_VERTEX_ARRAY);
	set_colors(colors, red);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	/* Now disable and draw again. */
	vertices[0][0] = 0.3;
	vertices[0][1] = 0.0;
	vertices[1][0] = 0.7;
	vertices[1][1] = 0.0;
	vertices[2][0] = 0.7;
	vertices[2][1] = 1.0;
	vertices[3][0] = 0.3;
	vertices[3][1] = 1.0;
	/* This NULL pointer set was key in triggering the bug reported. */
	glVertexPointer (2, GL_FLOAT, 0, NULL);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	set_colors(colors, green);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	/* Now draw again enabled, to make sure the hardware hasn't given
	 * up on us.
	 */
	vertices[0][0] = 0.7;
	vertices[0][1] = 0.0;
	vertices[1][0] = 1.0;
	vertices[1][1] = 0.0;
	vertices[2][0] = 1.0;
	vertices[2][1] = 1.0;
	vertices[3][0] = 0.7;
	vertices[3][1] = 1.0;
	glVertexPointer(2, GL_FLOAT, 0, vertices);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	set_colors(colors, blue);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	glutSwapBuffers();

	pass &= piglit_probe_pixel_rgb(Width * 1 / 6, Height / 2, red);
	pass &= piglit_probe_pixel_rgb(Width * 3 / 6, Height / 2, black);
	pass &= piglit_probe_pixel_rgb(Width * 5 / 6, Height / 2, blue);
	if (Automatic) {
		int i;

		if (pass)
			piglit_report_result(PIGLIT_SUCCESS);
		else
			piglit_report_result(PIGLIT_FAILURE);
		exit(pass ? 0 : 1);
	}
}


static void Reshape(int width, int height)
{
	Width = width;
	Height = height;
	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0, 1.0, 0.0, 1.0, -1.0, 1.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

static void Key(unsigned char key, int x, int y)
{
	(void) x;
	(void) y;
	switch (key) {
	case 27:
		exit(0);
		break;
	}
	glutPostRedisplay();
}

int main(int argc, char *argv[])
{
	int i;
	glutInit(&argc, argv);
	if (argc == 2 && !strcmp(argv[1], "-auto"))
		Automatic = 1;
	glutInitWindowPosition(0, 0);
	glutInitWindowSize(Width, Height);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
	glutCreateWindow(argv[0]);
	glutReshapeFunc(Reshape);
	glutDisplayFunc(Redisplay);
	if (!Automatic) {
		printf("Escape to quit\n");
		glutKeyboardFunc(Key);
	}
	Reshape(Width,Height);
	glutMainLoop();
	return 0;
}

