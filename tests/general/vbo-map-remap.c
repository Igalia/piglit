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
 *
 * Authors:
 *    Ben Holmes <shranzel@hotmail.com>
 */

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
static GLuint vbo;

static void
init()
{

	glewInit();
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0, 400, 0, 300, -1, 1);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glGenBuffersARB(1, &vbo);
	glBindBufferARB(GL_ARRAY_BUFFER_ARB, vbo);
	glBufferDataARB(GL_ARRAY_BUFFER_ARB, 12 * sizeof(GLfloat),
			NULL, GL_DYNAMIC_DRAW);
}

static void
vbo_write_floats_mapped(const float *varray, size_t count)
{
	float *ptr = glMapBufferARB(GL_ARRAY_BUFFER_ARB, GL_WRITE_ONLY_ARB);

	if (ptr == NULL)
		piglit_report_result(PIGLIT_FAILURE);

	memcpy(ptr, varray, count * sizeof(GLfloat));

	if (!glUnmapBufferARB(GL_ARRAY_BUFFER_ARB))
		piglit_report_result(PIGLIT_FAILURE);
}

static void
display()
{
	GLfloat white[4] = {1.0, 1.0, 1.0, 0.0};
	GLboolean pass = GL_TRUE;
	GLfloat varray1[12] = {175, 125, 0,
			       175, 175, 0,
			       125, 125, 0,
			       125, 175, 0};
	GLfloat varray2[12] = {275, 125, 0,
			       275, 175, 0,
			       225, 125, 0,
			       225, 175, 0};
	GLenum err;

	glBindBufferARB(GL_ARRAY_BUFFER_ARB, vbo);

	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, 0);

	vbo_write_floats_mapped(varray1, 12);

       	glClear(GL_COLOR_BUFFER_BIT);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	vbo_write_floats_mapped(varray2, 12);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	if ((err = glGetError()) != 0) {
		printf("gl error: 0x%08x\n", err);
		pass = GL_FALSE;
	}

	pass = pass && piglit_probe_pixel_rgb(250, 150, white);
	pass = pass && piglit_probe_pixel_rgb(150, 150, white);

	glutSwapBuffers();

	glDisableClientState(GL_VERTEX_ARRAY);

	if (Automatic)
		piglit_report_result(pass ? PIGLIT_SUCCESS : PIGLIT_FAILURE);
}

int main(int argc, char **argv)
{
	glutInit(&argc, argv);
	if (argc == 2 && !strcmp(argv[1],"-auto"))
		Automatic = GL_TRUE;

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize(400, 300);
	glutCreateWindow("VBO map remap");
	glutDisplayFunc(display);
	glutKeyboardFunc(piglit_escape_exit_key);

	init();

	piglit_require_extension("GL_ARB_vertex_buffer_object");

	glutMainLoop();

	glDeleteBuffersARB(1, &vbo);

	return 0;
}
