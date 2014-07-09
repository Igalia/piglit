/*
 * Copyright © 2011 VMware, Inc.
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
 */

/**
 * Tests simple two-sided lighting with separate specular color.
 *
 * One command line option: if "flat" is specified, use flat shading.
 * It shouldn't make any difference though because we only specify one normal
 * vector per quad.
 *
 * Brian Paul
 * Oct 2011
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_width = 256;
	config.window_height = 256;
	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

static const GLfloat red[4] = {1, 0, 0, 0};
static const GLfloat green[4] = {0, 1, 0, 1};
static const GLfloat blue[4] = {0, 0, 1, 1};
static const GLfloat purple[4] = {1, 0, 1, 1};
static const GLfloat yellow[4] = {1, 1, 0, 1};


enum piglit_result
piglit_display(void)
{
	int x0 = piglit_width * 1 / 4;
	int x1 = piglit_width * 3 / 4;
	int y0 = piglit_height * 1 / 4;
	int y1 = piglit_height * 3 / 4;
	GLboolean pass = GL_TRUE;

	glClear(GL_COLOR_BUFFER_BIT);

	glFrontFace(GL_CCW);  /* the default winding */

	glBegin(GL_QUADS);
	/* counter-clockwise / front-facing */
	glNormal3f(0, 0, 1);
	glVertex2f(-1.0, -1.0);
	glVertex2f( 0.0, -1.0);
	glVertex2f( 0.0,  0.0);
	glVertex2f(-1.0,  0.0);

	/* clockwise / back-facing */
	glNormal3f(0, 0, -1);
	glVertex2f( 0.0, -1.0);
	glVertex2f( 0.0,  0.0);
	glVertex2f( 1.0,  0.0);
	glVertex2f( 1.0, -1.0);
	glEnd();

	glFrontFace(GL_CW);  /* reverse winding */

	glBegin(GL_QUADS);
	/* counter-clockwise / back-facing */
	glNormal3f(0, 0, -1);
	glVertex2f(-1.0, 0.0);
	glVertex2f( 0.0, 0.0);
	glVertex2f( 0.0, 1.0);
	glVertex2f(-1.0, 1.0);

	/* clockwise / front-facing */
	glNormal3f(0, 0, 1);
	glVertex2f( 0.0, 0.0);
	glVertex2f( 0.0, 1.0);
	glVertex2f( 1.0, 1.0);
	glVertex2f( 1.0, 0.0);
	glEnd();

	pass = piglit_probe_pixel_rgb(x0, y0, yellow) && pass;
	pass = piglit_probe_pixel_rgb(x1, y0, purple) && pass;
	pass = piglit_probe_pixel_rgb(x0, y1, purple) && pass;
	pass = piglit_probe_pixel_rgb(x1, y1, yellow) && pass;

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}


void
piglit_init(int argc, char **argv)
{
	GLubyte teximage[8][8][4];
	GLuint t;
	int i;

	for (i = 1; i < argc; i++) {
		if (strcmp(argv[i], "flat") == 0) {
			glShadeModel(GL_FLAT);
			break;
		}
	}

	glClearColor(0.5, 0.5, 0.5, 0.0);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-1.1, 1.1, -1.1, 1.1, -1.0, 1.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	/* front diffuse = green */
	/* back diffuse = blue */
	/* front/back specular = red */
	glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, green);
	glMaterialfv(GL_BACK, GL_AMBIENT_AND_DIFFUSE, blue);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, red);
	glLightModeli( GL_LIGHT_MODEL_COLOR_CONTROL, GL_SEPARATE_SPECULAR_COLOR);
	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHTING);

	/* make solid white texture */
	glGenTextures(1, &t);
	glBindTexture(GL_TEXTURE_2D, t);
	memset(teximage, 255, sizeof(teximage));
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 8, 8, 0,
		     GL_RGBA, GL_UNSIGNED_BYTE, teximage);
	glEnable(GL_TEXTURE_2D);
}
