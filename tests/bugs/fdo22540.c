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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

// author: Ben Holmes


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <GL/glew.h>
#if defined(__APPLE__)
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#include "piglit-util.h"

static GLboolean Automatic = GL_FALSE;
static GLuint vBuffer;

extern void *glutGetProcAddress(const GLubyte *);

static void
Init(void)
{
	glewInit();
	piglit_require_extension("GL_ARB_vertex_buffer_object");
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0, 400, 0, 300, -1, 1);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
}

static void
vboInit(void)
{
	static const GLfloat vArray[12] = {
		225, 125, 0,
		225, 175, 0,
		175, 125, 0,
		175, 175, 0
	};
	glGenBuffersARB(1, &vBuffer);
	glBindBufferARB(GL_ARRAY_BUFFER_ARB, vBuffer);
	glBufferDataARB(GL_ARRAY_BUFFER_ARB, sizeof(vArray),
			vArray, GL_STATIC_DRAW_ARB);

}

static GLboolean
vboMap(void)
{
	(void) glMapBufferARB(GL_ARRAY_BUFFER_ARB, GL_READ_WRITE_ARB);
	glUnmapBufferARB(GL_ARRAY_BUFFER_ARB);
	return (glGetError() == 0);
}

static void
display(void)
{
	glBindBufferARB(GL_ARRAY_BUFFER_ARB, vBuffer);
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0 ,0);

	glColor3f(0.5, 0.5, 0.5);
	glClear(GL_COLOR_BUFFER_BIT);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	GLfloat gray[3] = {0.5, 0.5, 0.5};

	GLboolean pass = vboMap();
	pass = pass && piglit_probe_pixel_rgb(200, 150, gray);

	glFinish();
	glutSwapBuffers();

	if (Automatic)
		piglit_report_result(pass ? PIGLIT_SUCCESS : PIGLIT_FAILURE);

}

int main(int argc, char  **argv)
{
	glutInit(&argc, argv);
	if(argc==2 && !strncmp(argv[1],"-auto", 5))
		Automatic = GL_TRUE;

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize(400, 300);
	glutCreateWindow("VBO map");
	glutDisplayFunc(display);
	glutKeyboardFunc(piglit_escape_exit_key);

	Init();
	vboInit();

	glutMainLoop();

	glDeleteBuffersARB(1, &vBuffer);
	return 0;
}
