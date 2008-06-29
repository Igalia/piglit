/*
 * Copyright (c) The Piglit project 2008
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * on the rights to use, copy, modify, merge, publish, distribute, sub
 * license, and/or sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.  IN NO EVENT SHALL
 * VA LINUX SYSTEM, IBM AND/OR THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/**
 * @file
 * Test whether 1D textures correctly ignore the T coordinate wrap mode.
 *
 * Since 1D textures are genuine one-dimensional objects, the T coordinate
 * shouldn't affect them at all. However, R300 simulates them as flat
 * 2D textures, which caused incorrect sampling of border colors.
 */

#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <GL/glut.h>

#include "piglit-util.h"


static int Width = 256, Height = 128;
static int Automatic = 0;

static const GLfloat TextureColor[3] = { 1.0, 0.5, 0.0 };


static void test(GLenum wrapt, int cellx, int celly)
{
	int sx, sy;

	glPushMatrix();
	glTranslatef(cellx*0.25, celly*0.5, 0.0);
	glScalef(0.25, 0.5, 1.0);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_T, wrapt);
	glBegin(GL_QUADS);
		glTexCoord2f(0, 0); glVertex2f(0, 0);
		glTexCoord2f(1, 0); glVertex2f(1, 0);
		glTexCoord2f(1, 1); glVertex2f(1, 1);
		glTexCoord2f(0, 1); glVertex2f(0, 1);
	glEnd();
	glPopMatrix();

	glReadBuffer(GL_BACK);

	/* Take more than one sample, just to be sure */
	for(sy = 0; sy < 4; ++sy) {
		for(sx = 0; sx < 4; ++sx) {
			int x = (cellx*5 + sx + 1)*Width/20;
			int y = (celly*5 + sy + 1)*Height/10;

			if (!piglit_probe_pixel_rgb(x, y, TextureColor)) {
				fprintf(stderr, "Fail in cell %i,%i\n", cellx, celly);

				if (Automatic)
					piglit_report_result(PIGLIT_FAILURE);
			}
		}
	}
}

static void Redisplay(void)
{
	int x, y;

	glClearColor(0.5, 0.5, 0.5, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	test(GL_REPEAT, 0, 0);
	test(GL_CLAMP, 1, 0);
	test(GL_CLAMP_TO_EDGE, 2, 0);
	test(GL_CLAMP_TO_BORDER, 3, 0);
	test(GL_MIRRORED_REPEAT, 0, 1);
	test(GL_MIRROR_CLAMP_EXT, 1, 1);
	test(GL_MIRROR_CLAMP_TO_EDGE_EXT, 2, 1);
	test(GL_MIRROR_CLAMP_TO_BORDER_EXT, 3, 1);

	glutSwapBuffers();

	if (Automatic)
		piglit_report_result(PIGLIT_SUCCESS);
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


static void Init(void)
{
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexImage1D(GL_TEXTURE_1D, 0, GL_RGB, 1, 0, GL_RGB, GL_FLOAT, TextureColor);
	glEnable(GL_TEXTURE_1D);

	Reshape(Width,Height);
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
		printf("You should see a flat orange color\n");
		glutKeyboardFunc(Key);
	}
	Init();
	glutMainLoop();
	return 0;
}

