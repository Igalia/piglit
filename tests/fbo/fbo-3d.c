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

/** @file fbo-3d.c
 *
 * Tests that drawing to each depth of a 3D texture FBO and then drawing views
 * of those inidivual depths to the window system framebuffer succeeds.
 */

#define GL_GLEXT_PROTOTYPES
#include "GL/glut.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "piglit-util.h"

#define BUF_WIDTH 32
#define BUF_HEIGHT 32
#define WIN_WIDTH 200
#define WIN_HEIGHT 100

static GLboolean Automatic = GL_FALSE;

#define NUM_DEPTHS	6
float depth_color[NUM_DEPTHS][4] = {
	{1.0, 0.0, 0.0, 0.0},
	{0.0, 1.0, 0.0, 0.0},
	{0.0, 0.0, 1.0, 0.0},
	{1.0, 0.0, 1.0, 0.0},
	{1.0, 1.0, 0.0, 0.0},
	{0.0, 1.0, 1.0, 0.0},
};

static void rect(int x1, int y1, int x2, int y2)
{
	glBegin(GL_POLYGON);
	glVertex2f(x1, y1);
	glVertex2f(x1, y2);
	glVertex2f(x2, y2);
	glVertex2f(x2, y1);
	glEnd();
}

static void
report_fail(char *name, char *method, int x, int y,
	    GLfloat *results, GLfloat *expected)
{
	printf("%s vs %s: expected at (%d,%d): %f,%f,%f\n",
	       name, method, x, y, expected[0], expected[1], expected[2]);
	printf("%s vs %s: results at (%d,%d): %f,%f,%f\n",
	       name, method, x, y,
	       results[0], results[1], results[2]);
}

static int
create_3d_fbo(void)
{
	GLuint tex, fb, rb;
	GLenum status;
	GLboolean pass = GL_TRUE;
	int subrect_w = BUF_WIDTH / 5;
	int subrect_h = BUF_HEIGHT / 5;
	int x, y;
	int rbits, gbits, bbits, abits;
	int depth;

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_3D, tex);

	glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA,
		     BUF_WIDTH, BUF_HEIGHT, NUM_DEPTHS,
		     0,
		     GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	assert(glGetError() == 0);

	glGenFramebuffersEXT(1, &fb);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fb);

	for (depth = 0; depth < NUM_DEPTHS; depth++) {
		glFramebufferTexture3DEXT(GL_FRAMEBUFFER_EXT,
					  GL_COLOR_ATTACHMENT0_EXT,
					  GL_TEXTURE_3D,
					  tex,
					  0,
					  depth);

		assert(glGetError() == 0);

		status = glCheckFramebufferStatusEXT (GL_FRAMEBUFFER_EXT);
		if (status != GL_FRAMEBUFFER_COMPLETE_EXT) {
			fprintf(stderr, "FBO incomplete\n");
			goto done;
		}

		glViewport(0, 0, BUF_WIDTH, BUF_HEIGHT);
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(0, BUF_WIDTH, 0, BUF_HEIGHT, -1, 1);

		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		glColor4fv(depth_color[depth]);
		rect(-2, -2, BUF_WIDTH + 2, BUF_HEIGHT + 2);
	}


done:
	glDeleteFramebuffersEXT(1, &fb);

	return tex;
}

static GLboolean
draw_depth(int x, int y, int depth)
{
	float depth_coord = (float)depth / (NUM_DEPTHS - 1);

	glViewport(0, 0, WIN_WIDTH, WIN_HEIGHT);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, WIN_WIDTH, 0, WIN_HEIGHT, -1, 1);

	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

	glEnable(GL_TEXTURE_3D);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	glBegin(GL_QUADS);

	glTexCoord3f(0, 0, depth_coord);
	glVertex2f(x, y);

	glTexCoord3f(1, 0, depth_coord);
	glVertex2f(x + BUF_WIDTH, y);

	glTexCoord3f(1, 1, depth_coord);
	glVertex2f(x + BUF_WIDTH, y + BUF_HEIGHT);

	glTexCoord3f(0, 1, depth_coord);
	glVertex2f(x, y + BUF_HEIGHT);

	glEnd();
}

static GLboolean test_depth_drawing(int start_x, int start_y, float *expected)
{
	GLboolean pass = GL_TRUE;
	int x, y;

	for (y = start_y; y < start_y + BUF_HEIGHT; y++) {
		for (x = start_x; x < start_x + BUF_WIDTH; x++) {
			pass &= piglit_probe_pixel_rgb(x, y, expected);
		}
	}
}

static void
display()
{
	GLboolean pass = GL_TRUE;
	int depth, tex;

	glClearColor(1.0, 1.0, 1.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	tex = create_3d_fbo();

	for (depth = 0; depth < NUM_DEPTHS; depth++) {
		int x = 1 + depth * (BUF_WIDTH + 1);
		int y = 1;
		pass &= draw_depth(x, y, depth);
	}

	for (depth = 0; depth < NUM_DEPTHS; depth++) {
		int x = 1 + depth * (BUF_WIDTH + 1);
		int y = 1;
		pass &= test_depth_drawing(x, y, depth_color[depth]);
	}

	glDeleteTextures(1, &tex);

	glutSwapBuffers();

	if (Automatic) {
		printf("PIGLIT: {'result': '%s' }\n",
		       pass ? "pass" : "fail");
		exit(pass ? 0 : 1);
	}
}

int main(int argc, char**argv)
{
	glutInit(&argc, argv);
	if (argc == 2 && !strcmp(argv[1], "-auto"))
		Automatic = 1;
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize(WIN_WIDTH, WIN_HEIGHT);
	glutCreateWindow("buffer_sync");
	glutDisplayFunc(display);

	piglit_require_extension("GL_EXT_framebuffer_object");

	glutMainLoop();
}
