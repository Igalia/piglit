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
 */

/**
 * This test draws a point sprite with a checkerboard texture and tests whether
 * the correct colors were drawn using piglit_probe_pixel_rgb.
 *
 * \author Ben Holmes
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

static GLboolean Automatic = GL_FALSE;
static float maxSize = 0.0f;
static GLuint tex;

static void
Init(void)
{
	glewInit();
	piglit_require_extension("GL_ARB_point_sprite");

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, 400, 0, 300, -1, 1);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glEnable(GL_TEXTURE_2D);
	glEnable(GL_POINT_SPRITE_ARB);

	glGetFloatv(GL_POINT_SIZE_MAX, &maxSize);
	if (maxSize > 100)
		maxSize = 100;
	glPointSize(maxSize);

	glClearColor(0.2, 0.2, 0.2, 1.0);
	glColor3f(1.0, 1.0, 1.0);

}

static void
display(void)
{
	static const GLfloat black[3] = {0.0, 0.0, 0.0};
	GLboolean pass;

	glClear(GL_COLOR_BUFFER_BIT);
	glBindTexture(GL_TEXTURE_2D, tex);


	/* Make sure the point coordinate origin is set to the default location
	 * of upper left.
	 *
	 * OpenGL version must be at least 2.0 to support modifying
	 * GL_POINT_SPRITE_COORD_ORIGIN.
	 */
	if (GLEW_VERSION_2_0)
		glPointParameteri(GL_POINT_SPRITE_COORD_ORIGIN, GL_UPPER_LEFT);

	glBegin(GL_POINTS);
	glVertex2f(10.0f + (maxSize / 2),
		   10.0f + (maxSize / 2));
	glEnd();

	pass = piglit_probe_pixel_rgb(10.0f + (maxSize / 4),
				      10.0f + (3 * maxSize / 4),
				      black);


	/* Set the point coordinate origin to the lower left and check that the
	 * image is correctly flipped.
	 *
	 * OpenGL version must be at least 2.0 to support modifying
	 * GL_POINT_SPRITE_COORD_ORIGIN.
	 */
	if (GLEW_VERSION_2_0) {
		static const GLfloat white[3] = {1.0, 1.0, 1.0};


		glPointParameteri(GL_POINT_SPRITE_COORD_ORIGIN, GL_LOWER_LEFT);

		glBegin(GL_POINTS);
		glVertex2f(20.0f + maxSize + (maxSize / 2),
			   10.0f + (maxSize / 2));
		glEnd();

		pass = pass &&
			piglit_probe_pixel_rgb(20.0f + maxSize + (maxSize / 4),
					       10.0f + (3 * maxSize / 4),
					       white);
	}


	if (Automatic)
		piglit_report_result(pass ? PIGLIT_SUCCESS : PIGLIT_FAILURE);

	glutSwapBuffers();
}


static void
loadTex(void)
{
	#define height 2
	#define width 2
	int i, j;

	GLfloat texData[width][height][4];
	for (i = 0; i < width; i++) {
		for (j = 0; j < height; j++) {
			if ((i + j) & 1) {
				/* white */
				texData[i][j][0] = 1;
				texData[i][j][1] = 1;
				texData[i][j][2] = 1;
				texData[i][j][3] = 1;
			}
			else {
				/* black */
				texData[i][j][0] = 0;
				texData[i][j][1] = 0;
				texData[i][j][2] = 0;
				texData[i][j][3] = 0;
			}
		}
	}


	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexEnvi(GL_POINT_SPRITE, GL_COORD_REPLACE, GL_TRUE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA,
		     GL_FLOAT, texData);
}

int main(int argc, char **argv)
{
	glutInit(&argc, argv);
	if(argc==2 && !strncmp(argv[1],"-auto",5))
		Automatic = GL_TRUE;
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize(400, 300);
	glutCreateWindow("point_sprite");
	glutDisplayFunc(display);
	glutKeyboardFunc(piglit_escape_exit_key);

	Init();

	loadTex();

	glutMainLoop();

	return 0;
}
