/**
 * @file texrect-many.c
 *
 * Tests whether the driver can support a full set of rectangle textures.
 *
 * (Prompted by a bug in R300 where the driver ran out of indirections).
 */

#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define GL_GLEXT_PROTOTYPES
#include <GL/glew.h>
#if defined(__APPLE__)
#include <GLUT/glut.h>
#else
#include "GL/glut.h"
#endif

#include "piglit-util.h"


static int Width = 16*16, Height = 11*16;
static int Automatic = 0;
static int NumTextures = 16;
static GLuint Textures[16];

static const GLubyte colors[7][4] = {
	{ 0, 0, 0, 255 },
	{ 255, 0, 0, 255 },
	{ 0, 255, 0, 255 },
	{ 0, 0, 255, 255 },
	{ 128, 0, 0, 128 },
	{ 0, 128, 0, 128 },
	{ 0, 0, 128, 128 }
};

static void ActiveTexture(int i)
{
	glActiveTexture(GL_TEXTURE0+i);
	glClientActiveTexture(GL_TEXTURE0+i);
}

static void DoFrame(void)
{
	int i;

	glClearColor(0.5, 0.5, 0.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glColor4f(1,1,1,1);
	glBegin(GL_QUADS);
		for(i = 0; i < NumTextures; ++i)
			glMultiTexCoord2f(GL_TEXTURE0+i, 0, 0);
		glVertex2f(0, 0);
		for(i = 0; i < NumTextures; ++i)
			glMultiTexCoord2f(GL_TEXTURE0+i, 16, 0);
		glVertex2f(1, 0);
		for(i = 0; i < NumTextures; ++i)
			glMultiTexCoord2f(GL_TEXTURE0+i, 16, 11);
		glVertex2f(1, 1);
		for(i = 0; i < NumTextures; ++i)
			glMultiTexCoord2f(GL_TEXTURE0+i, 0, 11);
		glVertex2f(0, 1);
	glEnd();

	glutSwapBuffers();
}

static int DoTest(void)
{
	GLfloat dmax;
	int x, y;

	glReadBuffer(GL_FRONT);
	dmax = 0;

	for(x = 0; x < NumTextures; ++x) {
		for(y = 0; y < 11; ++y) {
			GLfloat probe[4];
			GLfloat delta[4];
			int i;
			int clr;

			glReadPixels((2*x+1)*Width/32, (2*y+1)*Height/22,
				1, 1, GL_RGBA, GL_FLOAT, probe);

			printf("   %i/%i: %f,%f,%f,%f", x, y,
				probe[0], probe[1], probe[2], probe[3]);

			clr = (x+y)%7;
			for(i = 0; i < 4; ++i) {
				delta[i] = probe[i] - colors[clr][i]/255.0;

				if (delta[i] > dmax) dmax = delta[i];
				else if (-delta[i] > dmax) dmax = -delta[i];
			}

			printf("   Delta: %f,%f,%f,%f\n", delta[0], delta[1], delta[2], delta[3]);
		}
	}

	printf("Max delta: %f\n", dmax);

	if (dmax >= 0.02)
		return 0;
	else
		return 1;
}


static void Display(void)
{
	int succ;

	DoFrame();
	succ = DoTest();

	if (Automatic) {
		printf("\nPIGLIT: { 'result': '%s' }\n", succ ? "pass" : "fail");
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
	glOrtho(0.0, 1.0, 0.0, 1.0, -1.0, 1.0);
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
	int maxtextures;

	piglit_require_extension("GL_ARB_texture_rectangle");

	glGetIntegerv(GL_MAX_TEXTURE_UNITS, &maxtextures);
	if (maxtextures < NumTextures)
		NumTextures = maxtextures;

	glGenTextures(NumTextures, Textures);
	for(i = 0; i < NumTextures; ++i) {
		GLubyte tex[11*16*4];
		GLubyte* p;
		int x, y;

		ActiveTexture(i);
		glEnable(GL_TEXTURE_RECTANGLE_ARB);
		glBindTexture(GL_TEXTURE_RECTANGLE_ARB, Textures[i]);

		p = tex;
		for(y = 0; y < 11; ++y) {
			for(x = 0; x < 16; ++x, p += 4) {
				if (x != i) {
					p[0] = p[1] = p[2] = p[3] = 255;
				} else {
					int clr = (x+y)%7;
					p[0] = colors[clr][0];
					p[1] = colors[clr][1];
					p[2] = colors[clr][2];
					p[3] = colors[clr][3];
				}
			}
		}

		glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, GL_RGBA, 16, 11, 0,
			     GL_RGBA, GL_UNSIGNED_BYTE, tex);
		glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_RECTANGLE_ARB, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	}

	Reshape(Width,Height);
}


int main(int argc, char**argv)
{
	glutInit(&argc, argv);
	if (argc == 2 && !strcmp(argv[1], "-auto"))
		Automatic = 1;
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_ALPHA);
	glutInitWindowSize(Width, Height);
	glutCreateWindow(argv[0]);
	glutReshapeFunc(Reshape);
	glutKeyboardFunc(Key);
	glutDisplayFunc(Display);
	glewInit();
	init();
	glutMainLoop();
	return 0;
}
