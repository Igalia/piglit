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

/*
* draws two triangles using different colors for each vert(1st-red, 2nd-green,
* 3rd-blue). first tri drawn using glProvokingVertexEXT set to
* GL_FIRST_VERTEX_CONVENTION_EXT.
* Second tri using GL_LAST_VERTEX_CONVENTION_EXT.
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#if defined(__APPLE__)
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#if defined(FREEGLUT)
#include <GL/freeglut_ext.h>
#endif
#endif
#include "piglit-util.h"


#ifndef APIENTRY
#define APIENTRY
#endif
#ifndef APIENTRYP
#define APIENTRYP APIENTRY *
#endif
#ifndef GL_EXT_provoking_vertex
#define GL_EXT_provoking_vertex
#define GL_QUADS_FOLLOW_PROVOKING_VERTEX_CONVENTION_EXT 0x8E4C
#define GL_FIRST_VERTEX_CONVENTION_EXT    0x8E4D
#define GL_LAST_VERTEX_CONVENTION_EXT     0x8E4E
#define GL_PROVOKING_VERTEX_EXT           0x8E4F
typedef void (APIENTRYP PFNGLPROVOKINGVERTEXEXTPROC) (GLenum mode);
#endif

static PFNGLPROVOKINGVERTEXEXTPROC pglProvokingVertexEXT = 0;


static GLboolean Automatic = GL_FALSE;

static void
Init(void)
{

	piglit_require_extension("GL_EXT_provoking_vertex");
	pglProvokingVertexEXT = (PFNGLPROVOKINGVERTEXEXTPROC)
#if defined(_MSC_VER)
		wglGetProcAddress("glProvokingVertexEXT");
#else
		glutGetProcAddress("glProvokingVertexEXT");
#endif
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0, 400, 0, 300, -1, 1);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glShadeModel(GL_FLAT);

	glClearColor(0.2, 0.2, 0.2, 1.0);

}

static void
display(void)
{
	float red[3] = {1.0, 0.0, 0.0};
	float blue[3] = {0.0, 0.0, 1.0};
	GLboolean pass = GL_TRUE;

	glClear(GL_COLOR_BUFFER_BIT);
	pglProvokingVertexEXT(GL_FIRST_VERTEX_CONVENTION_EXT);
	glBegin(GL_TRIANGLES);
		glColor3f(1.0, 0.0, 0.0);
		glVertex3i(125, 125, 0);
		glColor3f(0.0, 1.0, 0.0);
		glVertex3i(175, 125, 0);
		glColor3f(0.0, 0.0, 1.0);
		glVertex3i(150, 150, 0);
	glEnd();

	pglProvokingVertexEXT(GL_LAST_VERTEX_CONVENTION_EXT);
	glBegin(GL_TRIANGLES);
		glColor3f(1.0, 0.0, 0.0);
		glVertex3i(200, 125, 0);
		glColor3f(0.0, 1.0, 0.0);
		glVertex3i(250, 125, 0);
		glColor3f(0.0, 0.0, 1.0);
		glVertex3i(225, 150, 0);
	glEnd();

	pass = pass && piglit_probe_pixel_rgb(150, 130, red);
	pass = pass && piglit_probe_pixel_rgb(225, 130, blue);
	if (Automatic)
		piglit_report_result(pass ? PIGLIT_SUCCESS : PIGLIT_FAILURE);

	glFinish();
	glutSwapBuffers();

}

int main(int argc, char **argv)
{
	glutInit(&argc, argv);
	if (argc==2 && !strncmp(argv[1], "-auto", 5))
		Automatic=GL_TRUE;
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize(400, 300);
	glutCreateWindow("provoking vertex");
	glutDisplayFunc(display);
	glutKeyboardFunc(piglit_escape_exit_key);

	Init();

	glutMainLoop();

	return 0;

}
