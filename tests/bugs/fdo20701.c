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
 *    Ian Romanick <ian.d.romanick@intel.com>
 *
 */

/**
 * \file fdo20701.c
 * Test case from fd.o bug #20701
 *
 * Configure an FBO for rendering to a color texture with border.  Call
 * glFinish while that FBO is bound.  If it doesn't segfault, then the test
 * passes.
 */

#include "piglit-util.h"

static int Automatic = 0;
static int Width = 128, Height = 128;
static GLuint fb;
static GLuint tex;

static void Display(void)
{
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fb);
	glClearColor(1.0, 0.0, 0.0, 1.0);

	glClear(GL_COLOR_BUFFER_BIT);
	glFinish();

	if (Automatic) {
		printf("PIGLIT: {'result': 'pass' }\n");
		exit(0);
	}
}


static void Reshape(int width, int height)
{
	(void) width;
	(void) height;
}


static void Key(unsigned char key, int x, int y)
{
	(void) x;
	(void) y;
	switch (key) {
	case 27:
		exit(0);
		break;
	}
	glutPostRedisplay();
}


static void
init(void)
{
	GLenum status;

	glewInit();
	
	piglit_require_extension("GL_EXT_framebuffer_object");

	glGenFramebuffersEXT(1, &fb);
	glGenTextures(1, &tex);
	
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 66, 66, 1, GL_RGBA,
		     GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fb);
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
				  GL_COLOR_ATTACHMENT0_EXT,
				  GL_TEXTURE_2D, tex, 0);

	status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
	if (status != GL_FRAMEBUFFER_COMPLETE_EXT) {
		printf("%s:%u: framebuffer status = 0x%04x\n",
			   __FUNCTION__, __LINE__, status);
		printf("PIGLIT: {'result': 'fail' }\n");
		exit(1);
	}
}


int main(int argc, char**argv)
{
	glutInit(&argc, argv);
	if (argc == 2 && !strcmp(argv[1], "-auto"))
		Automatic = 1;
	glutInitDisplayMode(GLUT_RGB);
	glutInitWindowSize(Width, Height);
	glutCreateWindow("FD.O bug #20701 test");
	glutReshapeFunc(Reshape);
	glutKeyboardFunc(Key);
	glutDisplayFunc(Display);
	if (!Automatic)
		printf("If the test doesn't crash, then it passes.\n");
	init();
	glutMainLoop();
	return 0;
}
