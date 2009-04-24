/**
 * @file getteximage-simple.c
 *
 * Extremely basic test to check whether image data can be retrieved.
 * 
 * Note that the texture is used in a full frame of rendering before
 * the readback, to ensure that buffer manager related code for uploading
 * texture images is executed before the readback.
 *
 * This used to crash for R300+bufmgr.
 */

#if defined(__APPLE__)
#include <GLUT/glut.h>
#else
#include "GL/glut.h"
#endif
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <math.h>

#include "piglit-util.h"


static int Automatic = 0;
static int Width = 100, Height = 100;
static GLubyte data[4096]; /* 64*16*4 */

static int test_getteximage()
{
	GLubyte compare[4096];
	int i;

	glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, compare);

	for(i = 0; i < 4096; ++i) {
		if (data[i] != compare[i]) {
			printf("GetTexImage() returns incorrect data in byte %i\n", i);
			printf("    corresponding to (%i,%i) channel %i\n", i / 64, (i / 4) % 16, i % 4);
			printf("    expected: %i\n", data[i]);
			printf("    got: %i\n", compare[i]);
			return 0;
		}
	}

	return 1;
}
	

static void Display(void)
{
	int succ;
	
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_TEXTURE_2D);
	glBegin(GL_QUADS);
	glTexCoord2f(0, 0);
	glVertex2f(0, 0);
	glTexCoord2f(1, 0);
	glVertex2f(1, 0);
	glTexCoord2f(1, 1);
	glVertex2f(1, 1);
	glTexCoord2f(0, 1);
	glVertex2f(0, 1);
	glEnd();

	glutSwapBuffers();
	
	succ = test_getteximage();
	if (Automatic)
		piglit_report_result(succ ? PIGLIT_SUCCESS : PIGLIT_FAILURE);
}

static void Reshape(int width, int height)
{
	Width = width;
	Height = height;
	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0, 1.0, 0.0, 1.0, -2.0, 6.0);
	glScalef(1.0, 1.0, -1.0); // flip z-axis
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
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



static void init()
{
	int i;

	for(i = 0; i < 4096; ++i)
		data[i] = rand() & 0xff;

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 64, 16, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

	Reshape(Width, Height);
}


int main(int argc, char**argv)
{
	glutInit(&argc, argv);
	if (argc == 2 && !strcmp(argv[1], "-auto"))
		Automatic = 1;
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowSize(Width, Height);
	glutCreateWindow(argv[0]);
	glutReshapeFunc(Reshape);
	glutKeyboardFunc(Key);
	glutDisplayFunc(Display);
	init();
	glutMainLoop();
	return 0;
}
