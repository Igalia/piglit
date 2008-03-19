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
#include <unistd.h>
#include <stdlib.h>
#include <math.h>
#include "piglit-util.h"

static GLboolean Automatic = GL_FALSE;

#define MAX_SIZE	64
#define PAD		5

#define WIN_WIDTH	(MAX_SIZE * 6 + PAD * 7)
#define WIN_HEIGHT	(10 * PAD + MAX_SIZE * 2)

static const GLenum face_targets[6] = {
	GL_TEXTURE_CUBE_MAP_POSITIVE_X,
	GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
	GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
	GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
	GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
	GL_TEXTURE_CUBE_MAP_NEGATIVE_Z,
};

static const char *face_names[6] = {
	"POSITIVE_X",
	"POSITIVE_Y",
	"POSITIVE_Z",
	"NEGATIVE_X",
	"NEGATIVE_Y",
	"NEGATIVE_Z",
};

/* These texture coordinates should have 1 or -1 in the major axis selecting
 * the face, and a nearly-1-or-negative-1 value in the other two coordinates
 * which will be used to produce the s,t values used to sample that face's
 * image.
 */
static GLfloat face_texcoords[6][4][3] = {
	{ /* GL_TEXTURE_CUBE_MAP_POSITIVE_X */
		{1.0,  0.99,  0.99},
		{1.0,  0.99, -0.99},
		{1.0, -0.99, -0.99},
		{1.0, -0.99,  0.99},
	},
	{ /* GL_TEXTURE_CUBE_MAP_POSITIVE_Y */
		{-0.99, 1.0, -0.99},
		{ 0.99, 1.0, -0.99},
		{ 0.99, 1.0,  0.99},
		{-0.99, 1.0,  0.99},
	},
	{ /* GL_TEXTURE_CUBE_MAP_POSITIVE_Z */
		{-0.99,  0.99, 1.0},
		{-0.99, -0.99, 1.0},
		{ 0.99, -0.99, 1.0},
		{ 0.99,  0.99, 1.0},
	},
	{ /* GL_TEXTURE_CUBE_MAP_NEGATIVE_X */
		{-1.0,  0.99, -0.99},
		{-1.0,  0.99,  0.99},
		{-1.0, -0.99,  0.99},
		{-1.0, -0.99, -0.99},
	},
	{ /* GL_TEXTURE_CUBE_MAP_NEGATIVE_Y */
		{-0.99, -1.0,  0.99},
		{-0.99, -1.0, -0.99},
		{ 0.99, -1.0, -0.99},
		{ 0.99, -1.0,  0.99},
	},
	{ /* GL_TEXTURE_CUBE_MAP_NEGATIVE_Z */
		{ 0.99,  0.99, -1.0},
		{-0.99,  0.99, -1.0},
		{-0.99, -0.99, -1.0},
		{ 0.99, -0.99, -1.0},
	},
};

static GLfloat colors[][3] = {
	{1.0, 1.0, 1.0},
	{1.0, 1.0, 0.0},
	{1.0, 0.0, 0.0},
	{1.0, 0.0, 1.0},
	{0.0, 0.0, 1.0},
	{0.0, 1.0, 1.0},
	{0.0, 1.0, 0.0},
};

static void
set_face_image(int level, GLenum face, int size, int color)
{
	GLfloat *color1 = colors[color];
	GLfloat *color2 = colors[(color + 1) % ARRAY_SIZE(colors)];
	GLfloat *tex;
	int x, y;

	tex = malloc(size * size * 3 * sizeof(GLfloat));

	/* Set the texture for this face to one corner being color2 and the
	 * rest color1.  If the texture is 1x1, then it's all color1.
	 */
	for (y = 0; y < size; y++) {
		for (x = 0; x < size; x++) {
			GLfloat *chosen_color;

			if (y >= (size / 2) || x >= (size / 2))
				chosen_color = color1;
			else
				chosen_color = color2;

			tex[(y * size + x) * 3 + 0] = chosen_color[0];
			tex[(y * size + x) * 3 + 1] = chosen_color[1];
			tex[(y * size + x) * 3 + 2] = chosen_color[2];
		}
	}

	glTexImage2D(face, level, GL_RGB, size, size, 0, GL_RGB, GL_FLOAT, tex);

	free(tex);
}

/**
 * Returns whether the pixel at the coordinates matches the referenced color.
 *
 * Only the RGB channels are considered.
 */
static GLboolean
probe_pixel(int x, int y, GLfloat *color)
{
	GLfloat probe[4], delta[3];
	GLfloat dmax = 0;
	int i;

	glReadPixels(x, y, 1, 1, GL_RGBA, GL_FLOAT, probe);
	for (i = 0; i < 3; i++) {
		delta[i] = probe[i] - color[i];

		if (dmax < fabs(delta[i]))
			dmax = fabs(delta[i]);
	}

	if (dmax > .02) {
		printf("Expected at (%d,%d): %f,%f,%f\n",
		       x, y, color[0], color[1], color[2]);
		printf("Probed at   (%d,%d): %f,%f,%f\n",
		       x, y, probe[0], probe[1], probe[2]);
		return GL_FALSE;
	}

	return GL_TRUE;
}

/**
 * Tests that the mipmap drawn at (x,y)-(x+size,y+size) has the majority color,
 * with color+1 in bottom left.
 */
static GLboolean
test_results(int x, int y, int size, int level, int face, GLboolean mipmapped,
	     int color)
{
	GLfloat pix[4];
	GLfloat probe[4];
	GLfloat *color1 = colors[color];
	GLfloat *color2 = colors[(color + 1) % ARRAY_SIZE(colors)];
	GLboolean pass = GL_TRUE;
	int x1 = x + size / 4, x2 = x + size * 3 / 4;
	int y1 = y + size / 4, y2 = y + size * 3 / 4;

	pass = pass && probe_pixel(x1, y1, color2);
	pass = pass && probe_pixel(x2, y1, color1);
	pass = pass && probe_pixel(x2, y2, color1);
	pass = pass && probe_pixel(x1, y2, color1);

	if (!pass) {
		printf("Cube map failed at size %dx%d, level %d, face %s%s\n",
		       size, size, level, face_names[face],
		       mipmapped ? ", mipmapped" : "");
	}

	return pass;
}

static GLboolean
draw_at_size(int size, GLboolean mipmapped)
{
	GLfloat row_y = PAD;
	int dim, face;
	int color = 0, level = 0;
	GLint texname;
	GLboolean pass = GL_TRUE;

	/* Create the texture. */
	glGenTextures(1, &texname);
	glBindTexture(GL_TEXTURE_CUBE_MAP_ARB, texname);

	/* For each face drawing, we want to only see that face's contents at
	 * that mipmap level.
	 */
	if (mipmapped) {
		glTexParameteri(GL_TEXTURE_CUBE_MAP_ARB,
				GL_TEXTURE_MIN_FILTER,
				GL_NEAREST_MIPMAP_NEAREST);
	} else {
		glTexParameteri(GL_TEXTURE_CUBE_MAP_ARB,
				GL_TEXTURE_MIN_FILTER,
				GL_NEAREST);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP_ARB,
			GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP_ARB,
			GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP_ARB,
			GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	/* Fill in faces on each level */
	for (dim = size; dim > 0; dim /= 2) {
		for (face = 0; face < 6; face++) {
			set_face_image(level, face_targets[face], dim, color);
			color = (color + 1) % ARRAY_SIZE(colors);
		}
		if (!mipmapped)
			break;

		level++;
	}

	glEnable(GL_TEXTURE_CUBE_MAP_ARB);

	/* Clear background to gray */
	glClearColor(0.5, 0.5, 0.5, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	color = 0;
	level = 0;
	for (dim = size; dim > 0; dim /= 2) {
		GLfloat row_x = PAD;

		for (face = 0; face < 6; face++) {
			GLfloat base_x = row_x + face * (MAX_SIZE + PAD);
			GLfloat base_y = row_y;

			glBegin(GL_QUADS);

			glTexCoord3fv(face_texcoords[face][0]);
			glVertex2f(base_x, base_y);

			glTexCoord3fv(face_texcoords[face][1]);
			glVertex2f(base_x + dim, base_y);

			glTexCoord3fv(face_texcoords[face][2]);
			glVertex2f(base_x + dim, base_y + dim);

			glTexCoord3fv(face_texcoords[face][3]);
			glVertex2f(base_x, base_y + dim);

			glEnd();

			if (Automatic) {
				pass = test_results(base_x, base_y,
						    dim, level, face,
						    mipmapped,
						    color) && pass;
			}

			color = (color + 1) % ARRAY_SIZE(colors);
		}

		if (!mipmapped)
			break;

		row_y += dim + PAD;
		level++;
	}

	glutSwapBuffers();

	glDeleteTextures(1, &texname);

	return pass;
}


static void display()
{
	if (Automatic) {
		int dim;
		GLboolean pass = GL_TRUE;

		/* First, do each size from MAX_SIZExMAX_SIZE to 1x1 as a
		 * single texture level.
		 */
		for (dim = MAX_SIZE; dim > 0; dim /= 2) {
			pass = draw_at_size(dim, GL_FALSE) && pass;
		}

		/* Next, do each size with mipmaps from MAX_SIZExMAX_SIZE
		 * to 1x1.
		 */
		for (dim = MAX_SIZE; dim > 0; dim /= 2) {
			pass = draw_at_size(dim, GL_TRUE) && pass;
		}

		if (Automatic)
			printf("PIGLIT: {'result': '%s' }\n",
			       pass ? "pass" : "fail");

		exit(pass ? 0 : 1);
	} else {
		/* Demo mode: MAX_SIZE and mipmapped. */
		draw_at_size(MAX_SIZE, GL_FALSE);
	}
}

static void init()
{
	if (!glutExtensionSupported("GL_ARB_texture_cube_map")) {
		fprintf(stderr,
			"Sorry, this demo requires GL_ARB_texture_cube_map\n");
		if (Automatic)
			printf("PIGLIT: {'result': 'fail' }\n");
		exit(1);
	}

	/* Set up projection matrix so we can just draw using window
	 * coordinates.
	 */
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
