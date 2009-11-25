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

#include "piglit-util.h"
#include "piglit-framework.h"

#define BOX_SIZE 100
#define TEST_COLS 2
#define TEST_ROWS 1

int piglit_window_mode = GLUT_DOUBLE | GLUT_RGB;
int piglit_width = 1 + ((BOX_SIZE + 1) * TEST_COLS);
int piglit_height = 1 + ((BOX_SIZE + 1) * TEST_ROWS);

static float maxSize = 0.0f;
static GLuint tex;

static void
loadTex(void);

void
piglit_init(int argc, char **argv)
{
	piglit_require_extension("GL_ARB_point_sprite");

	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);

	glEnable(GL_TEXTURE_2D);
	glEnable(GL_POINT_SPRITE_ARB);

	glGetFloatv(GL_POINT_SIZE_MAX, &maxSize);
	if (maxSize > BOX_SIZE)
		maxSize = BOX_SIZE;
	glPointSize(maxSize);

	glClearColor(0.2, 0.2, 0.2, 1.0);
	glColor3f(1.0, 1.0, 1.0);

	loadTex();
}

enum piglit_result
piglit_display(void)
{
	static const GLfloat black[3] = {0.0, 0.0, 0.0};
	GLboolean pass;
	float x;
	float y;

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

	x = 1 + (BOX_SIZE / 2);
	y = 1 + (BOX_SIZE / 2);
	glBegin(GL_POINTS);
	glVertex2f(x, y);
	glEnd();

	pass = piglit_probe_pixel_rgb(x - (maxSize / 4),
				      y + (maxSize / 4),
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

		x = 1 + (BOX_SIZE / 2) + BOX_SIZE;
		glBegin(GL_POINTS);
		glVertex2f(x, y);
		glEnd();

		pass = piglit_probe_pixel_rgb(x - (maxSize / 4),
					      y + (maxSize / 4),
					      white)
		  && pass;
	}

	glutSwapBuffers();

	return pass ? PIGLIT_SUCCESS : PIGLIT_FAILURE;
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
