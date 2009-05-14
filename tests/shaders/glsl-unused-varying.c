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

/** @file glsl-unused-varying.c
 *
 * Tests that a vertex/fragment program combination with a varying that's
 * unused gets the right varying contents for the one that is used.
 *
 * This reveals a bug in the 965 brw_wm_glsl.c code.  Note that the
 * conditional in the fragment shader is required to trigger brw_wm_glsl.c.
 */

#define GL_GLEXT_PROTOTYPES
#include <GL/glew.h>
#include "GL/glut.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/stat.h>

#include "piglit-util.h"

#define WIN_WIDTH 100
#define WIN_HEIGHT 100

static int do_red_location;
static int red_location;
static int green_location;
static GLint prog;
static GLboolean Automatic;

static void
display(void)
{
	static const float red[] = {1.0, 0.0, 0.0, 0.0};
	static const float green[] = {0.0, 1.0, 0.0, 0.0};
	GLboolean pass = GL_TRUE;

	glClearColor(0.5, 0.5, 0.5, 0.5);
	glClear(GL_COLOR_BUFFER_BIT);

	glUniform4fv(red_location, 1, red);
	glUniform4fv(green_location, 1, green);

	glUniform1i(do_red_location, 1);
	piglit_draw_rect(10, 10, 10, 10);

	glUniform1i(do_red_location, 0);
	piglit_draw_rect(10, 30, 10, 10);

	pass &= piglit_probe_pixel_rgb(15, 15, red);
	pass &= piglit_probe_pixel_rgb(15, 35, green);

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


static void init()
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
				   SOURCE_DIR "tests/shaders/glsl-unused-varying.vert");
	fs = piglit_compile_shader(GL_FRAGMENT_SHADER,
				   SOURCE_DIR "tests/shaders/glsl-unused-varying.frag");

	prog = piglit_link_simple_program(vs, fs);

	glUseProgram(prog);

	red_location = glGetUniformLocation(prog, "red");
	green_location = glGetUniformLocation(prog, "green");
	do_red_location = glGetUniformLocation(prog, "do_red");
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
	glutCreateWindow("glsl-unused-varying");
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
