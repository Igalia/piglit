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

static void
Init()
{

        glewInit();
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        glOrtho(0, 400, 0, 300, -1, 1);

        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();

        glEnable(GL_VERTEX_ARRAY);
}

static void
vboMap(GLfloat *vArray, int count)
{


	float *ptr = (float*)glMapBufferARB(GL_ARRAY_BUFFER_ARB, 
						GL_READ_WRITE_ARB);

	int i=0;
	for (i;i<count;++i) {
		*ptr = vArray[i];
		++ptr;
	}

	if(!glUnmapBufferARB(GL_ARRAY_BUFFER_ARB))
		piglit_report_result(PIGLIT_FAILURE);

}

static void
vboInit()
{
        glGenBuffersARB(1, &vBuffer);
        glBindBufferARB(GL_ARRAY_BUFFER_ARB, vBuffer);
	glBufferDataARB(GL_ARRAY_BUFFER_ARB, 12*sizeof(GLfloat), 
			NULL, GL_DYNAMIC_DRAW);

}

static void
display()
{

	GLfloat vArray[12] = {  175, 125, 0,
				175, 175, 0,
				125, 125, 0,
				125, 175, 0};

	glBindBufferARB(GL_ARRAY_BUFFER_ARB, vBuffer);


	glEnableClientState(GL_VERTEX_ARRAY);
        glVertexPointer(3, GL_FLOAT, 0, 0);

	vboMap(vArray, 12);

       	glClear(GL_COLOR_BUFFER_BIT);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);


	GLfloat vArray1[12] = { 275, 125, 0,
				275, 175, 0,
				225, 125, 0,
				225, 175, 0};

	vboMap(vArray1, 12);

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);


	glutSwapBuffers();
	glFlush();


	glDisableClientState(GL_VERTEX_ARRAY);

	GLfloat white[4] = {1.0, 1.0, 1.0, 0.0};
	GLboolean pass;

	if(glGetError() == 0)
		pass = GL_TRUE;
	else
		pass = GL_FALSE;

	pass = pass && piglit_probe_pixel_rgb(250, 150, white);
	pass = pass && piglit_probe_pixel_rgb(150, 150, white);

	if(Automatic)
		piglit_report_result(pass ? PIGLIT_SUCCESS : PIGLIT_FAILURE);

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


int main(int argc, char **argv)
{
	glutInit(&argc, argv);
	if(argc==2 && !strncmp(argv[1],"-auto", 5))
                Automatic = GL_TRUE;

        glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
        glutInitWindowSize(400, 300);
        glutCreateWindow("VBO map remap");
        glutDisplayFunc(display);
	glutKeyboardFunc(Key);

        Init();
        vboInit();

        glutMainLoop();

	glDeleteBuffersARB(1, &vBuffer);
        return 0;

}
