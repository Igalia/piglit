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

/** @file fbo-readpixels.c
 *
 * Tests that various formats of color renderbuffer get correct results from
 * glReadPixels() versus glClear and immediate mode rendering.
 */

#include "piglit-util.h"

#define BUF_WIDTH 32
#define BUF_HEIGHT 32
#define WIN_WIDTH 100
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

static GLboolean
test_with_format(GLenum internal_format, GLenum format,
		 float results_x, float results_y)
{
	GLuint tex, fb;
	GLenum status;
	GLboolean pass = GL_TRUE;
	int subrect_w = BUF_WIDTH / 5;
	int subrect_h = BUF_HEIGHT / 5;
	int x, y;
	int rbits, gbits, bbits, abits;

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexImage2D(GL_TEXTURE_2D, 0, internal_format,
		     BUF_WIDTH, BUF_HEIGHT, 0,
		     format, GL_UNSIGNED_BYTE, NULL);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_RED_SIZE,
				 &rbits);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_GREEN_SIZE,
				 &gbits);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_BLUE_SIZE,
				 &bbits);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_ALPHA_SIZE,
				 &abits);

	printf("testing with format 0x%04x, 0x%04x "
	       "(%d,%d,%d,%d rgba)\n",
	       internal_format, format,
	       rbits, gbits, bbits, abits);

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
		fprintf(stderr, "texture for internalformat 0x%04x. "
			"format 0x%04x is framebuffer "
			"incomplete (status = 0x%04x)\n",
			internal_format, format, status);
		goto done;
	}

	/* Set matrices */
	glViewport(0, 0, BUF_WIDTH, BUF_HEIGHT);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, BUF_WIDTH, 0, BUF_HEIGHT, -1, 1);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glClearColor(1.0, 0.0, 1.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT);

	glColor4f(1.0, 0.0, 0.0, 0.0);
	rect(subrect_w * 1, subrect_h * 1,
	     subrect_w * 2, subrect_h * 2);

	glColor4f(0.0, 1.0, 0.0, 0.0);
	rect(subrect_w * 3, subrect_h * 1,
	     subrect_w * 4, subrect_h * 2);

	glColor4f(0.0, 0.0, 1.0, 0.0);
	rect(subrect_w * 1, subrect_h * 3,
	     subrect_w * 2, subrect_h * 4);

	glColor4f(0.0, 0.0, 0.0, 1.0);
	rect(subrect_w * 3, subrect_h * 3,
	     subrect_w * 4, subrect_h * 4);

	for (y = 0; y < BUF_HEIGHT; y++) {
		for (x = 0; x < BUF_WIDTH; x++) {
			float expected[4];

			if (x >= subrect_w * 1 && x < subrect_w * 2 &&
			    y >= subrect_h * 1 && y < subrect_h * 2) {
				expected[0] = 1.0;
				expected[1] = 0.0;
				expected[2] = 0.0;
				expected[3] = 0.0;
			} else if (x >= subrect_w * 3 && x < subrect_w * 4 &&
				   y >= subrect_h * 1 && y < subrect_h * 2) {
				expected[0] = 0.0;
				expected[1] = 1.0;
				expected[2] = 0.0;
				expected[3] = 0.0;
			} else if (x >= subrect_w * 1 && x < subrect_w * 2 &&
				   y >= subrect_h * 3 && y < subrect_h * 4) {
				expected[0] = 0.0;
				expected[1] = 0.0;
				expected[2] = 1.0;
				expected[3] = 0.0;
			} else if (x >= subrect_w * 3 && x < subrect_w * 4 &&
				   y >= subrect_h * 3 && y < subrect_h * 4) {
				expected[0] = 0.0;
				expected[1] = 0.0;
				expected[2] = 0.0;
				expected[3] = 1.0;
			} else {
				expected[0] = 1.0;
				expected[1] = 0.0;
				expected[2] = 1.0;
				expected[3] = 0.0;
			}
			pass &= piglit_probe_pixel_rgb(x, y, expected);
		}
	}

	glViewport(0, 0, WIN_WIDTH, WIN_HEIGHT);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, WIN_WIDTH, 0, WIN_HEIGHT, -1, 1);

	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
	glEnable(GL_TEXTURE_2D);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

	glBegin(GL_TRIANGLE_FAN);
	glTexCoord2f(0.0, 0.0);
	glVertex2f(results_x, results_y);
	glTexCoord2f(1.0, 0.0);
	glVertex2f(results_x + BUF_WIDTH, results_y);
	glTexCoord2f(1.0, 1.0);
	glVertex2f(results_x + BUF_WIDTH, results_y + BUF_HEIGHT);
	glTexCoord2f(0.0, 1.0);
	glVertex2f(results_x, results_y + BUF_HEIGHT);
	glEnd();
	glDisable(GL_TEXTURE_2D);

done:
	glDeleteFramebuffersEXT(1, &fb);
	glDeleteTextures(1, &tex);
	return pass;
}

static void
display(void)
{
	GLboolean pass = GL_TRUE;

	glClearColor(1.0, 1.0, 1.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	pass &= test_with_format(GL_RGBA8, GL_BGRA,
				 0, 0);
	pass &= test_with_format(GL_RGB5, GL_RGB,
				 0, BUF_HEIGHT + 1);
	pass &= test_with_format(GL_RGBA4, GL_BGRA,
				 0, (BUF_HEIGHT + 1) * 2);
	pass &= test_with_format(GL_RGB5_A1, GL_BGRA,
				 0, (BUF_HEIGHT + 1) * 3);
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
	glutCreateWindow("fbo-readpixels");
	glutDisplayFunc(display);
	glutKeyboardFunc(piglit_escape_exit_key);

	glewInit();

	piglit_require_extension("GL_EXT_framebuffer_object");

	glutMainLoop();

	return 0;
}
