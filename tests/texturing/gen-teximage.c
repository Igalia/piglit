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
 *    Chris Lord <chris@openedhand.com>
 *    Eric Anholt <eric@anholt.net>
 *
 */

/** @file gen-teximage.c
 *
 * Tests that:
 * - The full mipmap tree is generated when level 0 is set in a new
 *   texture object.
 * - Changing GL_GENERATE_MIPMAP state flushes previous vertices.
 * - The full mipmap tree is regenerated when level 0 is updated in an
 *   existing texture.
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#if defined(__APPLE__)
#include <GLUT/glut.h>
#else
#include "GL/glut.h"
#endif
#include "piglit-util.h"

#define WIN_WIDTH 512
#define WIN_HEIGHT 512
#define SIZE 128

static GLboolean Automatic = GL_FALSE;

static void display_mipmaps(int start_x, int start_y)
{
	int i;

	/* Disply all the mipmap levels */
	for (i = SIZE; i > 0; i /= 2) {
		glBegin(GL_QUADS);
		glTexCoord2f(0.0, 0.0); glVertex2f(start_x + 0, start_y + 0);
		glTexCoord2f(1.0, 0.0); glVertex2f(start_x + i, start_y + 0);
		glTexCoord2f(1.0, 1.0); glVertex2f(start_x + i, start_y + i);
		glTexCoord2f(0.0, 1.0); glVertex2f(start_x + 0, start_y + i);
		glEnd();

		start_x += i;
	}
}

static void fill_level(int level, const GLfloat *color)
{
	GLfloat *data;
	int size = SIZE / (1 << level);
	int i;

	/* Update a square inside the texture to red */
	data = malloc(size * size * 4 * sizeof(GLfloat));
	for (i = 0; i < 4 * size * size; i += 4) {
		data[i + 0] = color[0];
		data[i + 1] = color[1];
		data[i + 2] = color[2];
		data[i + 3] = color[3];
	}
        glTexImage2D(GL_TEXTURE_2D, level, GL_RGBA, size, size, 0,
		     GL_RGBA, GL_FLOAT, data);
	free(data);
}

static GLboolean check_resulting_mipmaps(int x, int y, const GLfloat *color)
{
	GLboolean pass = GL_TRUE;
	int i;

	for (i = SIZE; i > 4; i /= 2) {
		pass = pass && piglit_probe_pixel_rgb(x + i / 2, y + i / 2,
						      color);
		x += i;
	}

	return pass;
}

static void display(void)
{
	const GLfloat red[4] = {1.0, 0.0, 0.0, 0.0};
	const GLfloat blue[4] = {0.0, 0.0, 1.0, 0.0};
	GLuint texture;
	int i;

	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT);

	/* Set up a texture object with mipmap generation */
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
			GL_LINEAR_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);

	/* Set the first level of the new texture to red and display. */
	fill_level(0, red);
	display_mipmaps(0, 0);

	glDeleteTextures(1, &texture);

	/* Set up texture object without mipmap generation */
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
			GL_LINEAR_MIPMAP_NEAREST);

	/* Paint normal blue mipmap set */
	for (i = 0; SIZE / (1 << i) > 0; i++)
		fill_level(i, blue);

	display_mipmaps(0, SIZE);

	/* Enable GENERATE_MIPMAP and set the first (and thus all) levels to
	 * red.
	 */
	glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
	fill_level(0, red);
	display_mipmaps(0, SIZE * 2);

	glDeleteTextures(1, &texture);

	glutSwapBuffers();
	glFlush();

	if (Automatic) {
		GLboolean pass = GL_TRUE;

		pass = pass && check_resulting_mipmaps(0, 0, red);
		pass = pass && check_resulting_mipmaps(0, SIZE, blue);
		pass = pass && check_resulting_mipmaps(0, SIZE * 2, red);

		if (Automatic)
			printf("PIGLIT: {'result': '%s' }\n",
			       pass ? "pass" : "fail");
		exit(pass ? 0 : 1);
	}
}

static void init(void)
{
	piglit_require_extension("GL_SGIS_generate_mipmap");

	/* Set up projection matrix so we can just draw using window
	 * coordinates.
	 */
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0, WIN_WIDTH, 0, WIN_HEIGHT, -1, 1);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glEnable(GL_TEXTURE_2D);
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
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize(WIN_WIDTH, WIN_HEIGHT);
	glutInitWindowPosition(100, 100);
	glutCreateWindow("gen-teximage");
	init();
	glutDisplayFunc(display);
	glutMainLoop();

	return 0;
}
