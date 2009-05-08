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

/** @file fbo-generatemipmap.c
 *
 * Tests that glGenerateMipmapEXT works correctly on a 2D texture.
 */

#define GL_GLEXT_PROTOTYPES
#include <GL/glew.h>
#if defined(__APPLE__)
#include <GLUT/glut.h>
#else
#include "GL/glut.h"
#endif
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "piglit-util.h"

#define TEX_WIDTH 256
#define TEX_HEIGHT 256
#define WIN_WIDTH 700
#define WIN_HEIGHT 300

static GLboolean Automatic = GL_FALSE;

static const float red[] =   {1, 0, 0, 0};
static const float green[] = {0, 1, 0, 0};
static const float blue[] =  {0, 0, 1, 0};
static const float white[] = {1, 1, 1, 1};

static int
create_fbo(void)
{
	GLuint tex, fb;
	GLenum status;
	int i, dim;

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);

	for (i = 0, dim = TEX_WIDTH; dim >0; i++, dim /= 2) {
		glTexImage2D(GL_TEXTURE_2D, i, GL_RGBA,
			     dim, dim,
			     0,
			     GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	}
	assert(glGetError() == 0);

	glGenFramebuffersEXT(1, &fb);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fb);
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
				  GL_COLOR_ATTACHMENT0_EXT,
				  GL_TEXTURE_2D,
				  tex,
				  0);
	assert(glGetError() == 0);

	status = glCheckFramebufferStatusEXT (GL_FRAMEBUFFER_EXT);
	if (status != GL_FRAMEBUFFER_COMPLETE_EXT) {
		fprintf(stderr, "FBO incomplete\n");
		goto done;
	}

	glViewport(0, 0, TEX_WIDTH, TEX_HEIGHT);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, TEX_WIDTH, 0, TEX_HEIGHT, -1, 1);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glColor4fv(red);
	piglit_draw_rect(0, 0, TEX_WIDTH / 2, TEX_HEIGHT / 2);
	glColor4fv(green);
	piglit_draw_rect(TEX_WIDTH / 2, 0, TEX_WIDTH, TEX_HEIGHT / 2);
	glColor4fv(blue);
	piglit_draw_rect(0, TEX_HEIGHT / 2, TEX_WIDTH/2, TEX_HEIGHT);
	glColor4fv(white);
	piglit_draw_rect(TEX_WIDTH / 2, TEX_HEIGHT / 2, TEX_WIDTH, TEX_HEIGHT);

	glGenerateMipmapEXT(GL_TEXTURE_2D);
done:
	glDeleteFramebuffersEXT(1, &fb);

	return tex;
}

static void
draw_mipmap(int x, int y, int dim)
{
	glViewport(0, 0, WIN_WIDTH, WIN_HEIGHT);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, WIN_WIDTH, 0, WIN_HEIGHT, -1, 1);

	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

	glEnable(GL_TEXTURE_2D);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glBegin(GL_QUADS);

	glTexCoord2f(0, 0);
	glVertex2f(x, y);

	glTexCoord2f(1, 0);
	glVertex2f(x + dim, y);

	glTexCoord2f(1, 1);
	glVertex2f(x + dim, y + dim);

	glTexCoord2f(0, 1);
	glVertex2f(x, y + dim);

	glEnd();

	glDisable(GL_TEXTURE_2D);
}

static GLboolean
test_mipmap_drawing(int start_x, int start_y, int dim)
{
	GLboolean pass = GL_TRUE;
	int x, y;

	for (y = 0; y < dim; y++) {
		for (x = 0; x < dim; x++) {
			const float *expected;

			if (x < dim / 2 && y < dim / 2)
				expected = red;
			else if (y < dim / 2)
				expected = green;
			else if (x < dim / 2)
				expected = blue;
			else
				expected = white;

			pass &= piglit_probe_pixel_rgb(start_x + x,
						       start_y + y,
						       expected);
		}
	}

	return pass;
}

static void
display()
{
	GLboolean pass = GL_TRUE;
	int dim;
	GLuint tex;
	int x;

	glClearColor(0.5, 0.5, 0.5, 0.5);
	glClear(GL_COLOR_BUFFER_BIT);

	tex = create_fbo();

	x = 1;
	for (dim = TEX_WIDTH; dim > 1; dim /= 2) {
		draw_mipmap(x, 1, dim);
		x += dim + 1;
	}

	x = 1;
	for (dim = TEX_WIDTH; dim > 1; dim /= 2) {
		pass &= test_mipmap_drawing(x, 1, dim);
		x += dim + 1;
	}

	glDeleteTextures(1, &tex);

	glutSwapBuffers();

	if (Automatic)
		piglit_report_result(pass ? PIGLIT_SUCCESS : PIGLIT_FAILURE);
}

int main(int argc, char**argv)
{
	glutInit(&argc, argv);
	if (argc == 2 && !strcmp(argv[1], "-auto"))
		Automatic = 1;
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize(WIN_WIDTH, WIN_HEIGHT);
	glutCreateWindow("fbo-generatemipmap");
	glutDisplayFunc(display);

	glewInit();

	piglit_require_extension("GL_EXT_framebuffer_object");

	glutMainLoop();

	return 0;
}
