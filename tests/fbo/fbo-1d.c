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

/** @file fbo-1d.c
 *
 * Tests that rendering to a 1D texture and then drawing it to the framebuffer
 * succeeds.
 */

#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define GL_GLEXT_PROTOTYPES
#include <GL/glew.h>
#if defined(__APPLE__)
#include <GLUT/glut.h>
#else
#include "GL/glut.h"
#endif

#include "piglit-util.h"

#define BUF_WIDTH 32
#define WIN_WIDTH 50
#define WIN_HEIGHT 20

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

static int
create_1d_fbo(void)
{
	GLuint tex, fb;
	GLenum status;

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_1D, tex);

	glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA,
		     BUF_WIDTH,
		     0,
		     GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	assert(glGetError() == 0);

	glGenFramebuffersEXT(1, &fb);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fb);

	glFramebufferTexture1DEXT(GL_FRAMEBUFFER_EXT,
				  GL_COLOR_ATTACHMENT0_EXT,
				  GL_TEXTURE_1D,
				  tex,
				  0);

	assert(glGetError() == 0);

	status = glCheckFramebufferStatusEXT (GL_FRAMEBUFFER_EXT);
	if (status != GL_FRAMEBUFFER_COMPLETE_EXT) {
		fprintf(stderr, "FBO incomplete\n");
		goto done;
	}

	glViewport(0, 0, BUF_WIDTH, 1);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, BUF_WIDTH, 0, 1, -1, 1);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	/* left side: red */
	glColor4f(1.0, 0.0, 0.0, 0.0);
	rect(0, 0,
	     BUF_WIDTH / 2, 1);

	/* right side: green */
	glColor4f(0.0, 1.0, 0.0, 0.0);
	rect(BUF_WIDTH / 2, 0,
	     BUF_WIDTH, 1);

done:
	glDeleteFramebuffersEXT(1, &fb);

	return tex;
}

static void
draw_fbo_1d(int x, int y)
{
	glViewport(0, 0, WIN_WIDTH, WIN_HEIGHT);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, WIN_WIDTH, 0, WIN_HEIGHT, -1, 1);

	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

	glEnable(GL_TEXTURE_1D);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	glBegin(GL_QUADS);

	glTexCoord2f(0, 0);
	glVertex2f(x, y);

	glTexCoord2f(1, 0);
	glVertex2f(x + BUF_WIDTH, y);

	glTexCoord2f(1, 1);
	glVertex2f(x + BUF_WIDTH, y + 1);

	glTexCoord2f(0, 1);
	glVertex2f(x, y + 1);

	glEnd();
}

static void
display(void)
{
	GLboolean pass = GL_TRUE;
	float red[] = {1,0,0,0};
	float green[] = {0,1,0,0};
	float *expected;
	int x;
	GLuint tex;

	glClearColor(1.0, 1.0, 1.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	tex = create_1d_fbo();

	draw_fbo_1d(10, 10);

	for (x = 0; x < BUF_WIDTH; x++) {
		if (x < BUF_WIDTH / 2)
			expected = red;
		else
			expected = green;

		pass &= piglit_probe_pixel_rgb(10 + x, 10, expected);
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
	glutCreateWindow("fbo-1d");
	glutDisplayFunc(display);
	glutKeyboardFunc(piglit_escape_exit_key);

	glewInit();

	piglit_require_extension("GL_EXT_framebuffer_object");

	glutMainLoop();

	return 0;
}
