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

/** @file glsl-vs-if.c
 *
 * Tests that exp2 produces the expected output in a fragment shader.
 */

#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/stat.h>
#define GL_GLEXT_PROTOTYPES
#include <GL/glew.h>
#if defined(__APPLE__)
#include <GLUT/glut.h>
#else
#include "GL/glut.h"
#endif

#include "piglit-util.h"

#define WIN_WIDTH 100
#define WIN_HEIGHT 100

static GLint prog;
static GLboolean Automatic;

static void
display(void)
{
	GLboolean pass = GL_TRUE;
	static const float red[] = {1.0, 0.0, 0.0, 0.0};
	static const float green[] = {0.0, 1.0, 0.0, 0.0};

	glClearColor(0.5, 0.5, 0.5, 0.5);
	glClear(GL_COLOR_BUFFER_BIT);

	piglit_draw_rect(10, 10, 10, 10);
	piglit_draw_rect(30, 10, 10, 10);

	pass &= piglit_probe_pixel_rgb(15, 15, red);
	pass &= piglit_probe_pixel_rgb(35, 15, green);

	glutSwapBuffers();

	if (Automatic) {
		piglit_report_result (pass ? PIGLIT_SUCCESS : PIGLIT_FAILURE);
	}
}

static void key(unsigned char key, int x, int y)
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


static void init(void)
{
	GLint vs, fs;

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

	vs = piglit_compile_shader(GL_VERTEX_SHADER,
				   SOURCE_DIR "tests/shaders/glsl-vs-if.vert");
	fs = piglit_compile_shader(GL_FRAGMENT_SHADER,
				   SOURCE_DIR "tests/shaders/glsl-vs-if.frag");

	prog = piglit_link_simple_program(vs, fs);

	glUseProgram(prog);
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
	glutCreateWindow("glsl-vs-if");
	glutKeyboardFunc(key);
	glutDisplayFunc(display);
	glewInit();

	if (!GLEW_VERSION_2_0) {
		printf("Requires OpenGL 2.0\n");
		piglit_report_result(PIGLIT_SKIP);
		exit(1);
	}
	init();

	glutMainLoop();

	return 0;
}
