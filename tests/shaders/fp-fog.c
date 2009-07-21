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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

/**
 * \file
 * Test passing fog coordinates into a fragment program.
 *
 * \author Ian Romanick <ian.d.romanick@intel.com>
 */

#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#if defined(__APPLE__)
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#ifdef FREEGLUT
#include <GL/freeglut_ext.h>
#endif
#endif
#include "piglit-util.h"

static GLint prog = 0;

static const char* const program_text =
	"!!ARBfp1.0\n"
	"MOV result.color, fragment.fogcoord;\n"
	"END\n"
	;

static int Automatic = 0;

static int Width = 50, Height = 50;

#if defined(__APPLE__)
static void (*pglFogCoordf)(GLfloat coord) = NULL;
#else
static PFNGLFOGCOORDFPROC pglFogCoordf = NULL;
#endif

static void Redisplay(void)
{
	static const struct {
		float x, y, r;
	}
	probes[4] = {
		{ 0.5, 1.5, 0.3 },
		{ 1.5, 1.5, 0.6 },
		{ 0.5, 0.5, 0.8 },
		{ 1.5, 0.5, 0.4 },
	};
	int pass = 1;
	unsigned i;

	glClear(GL_COLOR_BUFFER_BIT);

	pglFogCoordf(0.3);
	glBegin(GL_QUADS);
	glVertex2f(0, 1);
	glVertex2f(1, 1);
	glVertex2f(1, 2);
	glVertex2f(0, 2);
	glEnd();

	pglFogCoordf(0.6);
	glBegin(GL_QUADS);
	glVertex2f(1, 1);
	glVertex2f(2, 1);
	glVertex2f(2, 2);
	glVertex2f(1, 2);
	glEnd();

	pglFogCoordf(0.8);
	glBegin(GL_QUADS);
	glVertex2f(0, 0);
	glVertex2f(1, 0);
	glVertex2f(1, 1);
	glVertex2f(0, 1);
	glEnd();

	pglFogCoordf(0.4);
	glBegin(GL_QUADS);
	glVertex2f(1, 0);
	glVertex2f(2, 0);
	glVertex2f(2, 1);
	glVertex2f(1, 1);
	glEnd();

	for (i = 0; i < 4; i++) {
		float expected_color[4];

		expected_color[0] = probes[i].r;
		expected_color[1] = 0.0;
		expected_color[2] = 0.0;
		expected_color[3] = 1.0;

		pass &= piglit_probe_pixel_rgba(probes[i].x * Width / 2,
						probes[i].y * Height / 2,
						expected_color);
	}

	glutSwapBuffers();

	if (Automatic) {
		printf("\nPIGLIT: { 'result': '%s' }\n",
		       pass ? "pass" : "fail");
		exit(0);
	}
}


static void Reshape(int width, int height)
{
	Width = width;
	Height = height;
	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0, 2.0, 0.0, 2.0, -2.0, 6.0);
	glScalef(1.0, 1.0, -1.0); // flip z-axis
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}


static void Key(unsigned char key, int x, int y)
{
	(void) x;
	(void) y;

	if (key) {
		exit(0);
	}

	glutPostRedisplay();
}


static void Init(void)
{
	printf("GL_RENDERER = %s\n", (char *) glGetString(GL_RENDERER));

	glClearColor(0.3, 0.3, 0.3, 0.3);

	if (atof((const char *) glGetString(GL_VERSION)) >= 1.4) {
#if defined(__APPLE__)
		pglFogCoordf = &glFogCoordf;
#elif defined(_MSC_VER)
		pglFogCoordf = (PFNGLFOGCOORDFPROC)
			wglGetProcAddress("glFogCoordf");
#else
		pglFogCoordf = (PFNGLFOGCOORDFPROC)
			glutGetProcAddress("glFogCoordf");
#endif
	} else if (glutExtensionSupported("GL_EXT_fog_coord")) {
#if defined(__APPLE__)
		pglFogCoordf = &glFogCoordfEXT;
#elif defined(_MSC_VER)
		pglFogCoordf = (PFNGLFOGCOORDFPROC)
			wglGetProcAddress("glFogCoordfEXT");
#else
		pglFogCoordf = (PFNGLFOGCOORDFPROC)
			glutGetProcAddress("glFogCoordfEXT");
#endif
	} else {
		piglit_report_result(PIGLIT_SKIP);
	}

	piglit_require_fragment_program();
	prog = piglit_compile_program(GL_FRAGMENT_PROGRAM_ARB, program_text);

	glEnable(GL_FRAGMENT_PROGRAM_ARB);
	pglBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, prog);

	glFogi(GL_FOG_COORDINATE_SOURCE_EXT, GL_FOG_COORDINATE_EXT);

	Reshape(Width, Height);
}


int main(int argc, char *argv[])
{
	glutInit(&argc, argv);

	if ((argc > 1) && (strcmp(argv[1], "-auto") == 0)) {
		Automatic = 1;
	}

	glutInitWindowPosition(0, 0);
	glutInitWindowSize(Width, Height);
	glutInitDisplayMode(GLUT_RGBA | GLUT_ALPHA | GLUT_DOUBLE);
	glutCreateWindow(argv[0]);
	glutReshapeFunc(Reshape);
	glutKeyboardFunc(Key);
	glutDisplayFunc(Redisplay);
	Init();
	glutMainLoop();
	return 0;
}
