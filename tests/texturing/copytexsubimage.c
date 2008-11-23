/*
 * Copyright © 2008 Intel Corporation
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

#include "GL/glut.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define WIN_WIDTH 200
#define WIN_HEIGHT 200

static GLboolean Automatic = GL_FALSE;

static void rect(int x1, int y1, int x2, int y2)
{
	glBegin(GL_POLYGON);
	glVertex2f(x1, y1);
	glVertex2f(x1, y2);
	glVertex2f(x2, y2);
	glVertex2f(x2, y1);
	glEnd();
}
static GLboolean inrect(int x, int y, int x1, int y1, int x2, int y2)
{
	if (x >= x1 && x < x2 && y >= y1 && y < y2)
		return GL_TRUE;
	else
		return GL_FALSE;
}

static GLboolean
check_results(int dstx, int dsty, int w, int h)
{
	GLfloat results[w * h][4];
	GLboolean pass = GL_TRUE;
	int x, y;

	/* Check the results */
	glReadPixels(dstx, dsty, w, h, GL_RGBA, GL_FLOAT, results);
	for (y = 0; y < h; y++) {
		for (x = 0; x < w; x++) {
			GLfloat expected[3];

			if (inrect(x, y, 5, h/2, w - 5, h - 5)) {
				expected[0] = 0.0;
				expected[1] = 0.0;
				expected[2] = 1.0;
			} else if (inrect(x, y, 5, 5, w - 5, h/2)) {
				expected[0] = 0.0;
				expected[1] = 1.0;
				expected[2] = 0.0;
			} else {
				expected[0] = 1.0;
				expected[1] = 0.0;
				expected[2] = 0.0;
			}

			if (results[y * w + x][0] != expected[0] ||
			    results[y * w + x][1] != expected[1] ||
			    results[y * w + x][2] != expected[2]) {
				printf("Expected at (%d,%d): %f,%f,%f\n",
				       x, y,
				       expected[0], expected[1], expected[2]);
				printf("Probed at   (%d,%d): %f,%f,%f\n",
				       x, y,
				       results[y * w + x][0],
				       results[y * w + x][1],
				       results[y * w + x][2]);
				pass = GL_FALSE;
			}
		}
	}

	return pass;
}

static void display()
{
	int srcx = 20, srcy = 20, srcw = 32, srch = 32;
	int dstx = 80, dsty = 20;
	int dstx2 = 140, dsty2 = 20;
	int texname, x, y;
	GLboolean pass = GL_TRUE;

	glClearColor(0.5, 0.5, 0.5, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	/* Draw the object we're going to copy */
	glColor3f(1.0, 0.0, 0.0);
	rect(srcx, srcy, srcx + srcw, srcy + srch);
	glColor3f(0.0, 1.0, 0.0);
	rect(srcx + 5, srcy + 5, srcx + srcw - 5, srcy + srch/2);
	glColor3f(0.0, 0.0, 1.0);
	rect(srcx + 5, srcy + srch/2, srcx + srcw - 5, srcy + srch - 5);

	/* Create a texture image and copy it in */
	glGenTextures(1, &texname);
	glBindTexture(GL_TEXTURE_2D, texname);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glEnable(GL_TEXTURE_2D);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, srcw, srch, 0,
		     GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glCopyTexSubImage2D(GL_TEXTURE_2D, 0,
			    0, 0, /* offset in image */
			    srcx, srcy, /* offset in readbuffer */
			    srcw, srch);

	/* Draw the texture image out */
	glBegin(GL_POLYGON);
	glTexCoord2f(0.0, 0.0); glVertex2f(dstx, dsty);
	glTexCoord2f(0.0, 1.0); glVertex2f(dstx, dsty + srch);
	glTexCoord2f(1.0, 1.0); glVertex2f(dstx + srcw, dsty + srch);
	glTexCoord2f(1.0, 0.0); glVertex2f(dstx + srcw, dsty);
	glEnd();

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, srcw, srch, 0,
		     GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glCopyTexSubImage2D(GL_TEXTURE_2D, 0,
			    0, 0, /* offset in image */
			    srcx, srcy, /* offset in readbuffer */
			    srcw / 2, srch / 2);
	glCopyTexSubImage2D(GL_TEXTURE_2D, 0,
			    srcw / 2, 0, /* offset in image */
			    srcx + srcw / 2, srcy, /* offset in readbuffer */
			    srcw / 2, srch / 2);
	glCopyTexSubImage2D(GL_TEXTURE_2D, 0,
			    0, srch / 2, /* offset in image */
			    srcx, srcy + srch / 2, /* offset in readbuffer */
			    srcw / 2, srch / 2);
	glCopyTexSubImage2D(GL_TEXTURE_2D, 0,
			    srcw / 2, srch / 2, /* offset in image */
			    srcx + srcw / 2, srcy + srch / 2, /* offset in readbuffer */
			    srcw / 2, srch / 2);

	/* Draw the texture image out */
	glBegin(GL_POLYGON);
	glTexCoord2f(0.0, 0.0); glVertex2f(dstx2, dsty2);
	glTexCoord2f(0.0, 1.0); glVertex2f(dstx2, dsty2 + srch);
	glTexCoord2f(1.0, 1.0); glVertex2f(dstx2 + srcw, dsty2 + srch);
	glTexCoord2f(1.0, 0.0); glVertex2f(dstx2 + srcw, dsty2);
	glEnd();

	glDisable(GL_TEXTURE_2D);
	glDeleteTextures(1, &texname);
	pass &= check_results(dstx, dsty, srcw, srch);
	pass &= check_results(dstx2, dsty2, srcw, srch);

	if (Automatic) {
		printf("PIGLIT: {'result': '%s' }\n",
		       pass ? "pass" : "fail");
		exit(pass ? 0 : 1);
	}

	glutSwapBuffers();
}

static void init()
{
	glMatrixMode( GL_PROJECTION );
	glPushMatrix();
	glLoadIdentity();
	glOrtho( 0, WIN_WIDTH, 0, WIN_HEIGHT, -1, 1 );

	glMatrixMode( GL_MODELVIEW );
	glPushMatrix();
	glLoadIdentity();
}

int main(int argc, char**argv)
{
	glutInit(&argc, argv);
	if (argc == 2 && !strcmp(argv[1], "-auto"))
		Automatic = 1;
	glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize (WIN_WIDTH, WIN_HEIGHT);
	glutInitWindowPosition (100, 100);
	glutCreateWindow ("cubemap");
	init();
	glutDisplayFunc(display);
	glutMainLoop();
}
