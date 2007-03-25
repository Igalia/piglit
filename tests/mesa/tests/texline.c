/* $Id: texline.c,v 1.5 2004/01/28 10:07:48 keithw Exp $ */

/*
 * Test textured lines.
 *
 * Brian Paul
 * September 2000
 */


#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <GL/glut.h>
#include "../util/readtex.c"   /* I know, this is a hack. */

#define TEXTURE_FILE "./tests/data/girl.rgb"

static int Width = 400, Height = 300;


static void DoStar(int texture)
{
	int l;

	glPushMatrix();
	glScalef(0.5, 0.5, 1.0);
	glTranslatef(1.0, 1.0, 0.0);

	glBegin(GL_LINES);
		for(l = 0; l < 30; ++l) {
			double rad = l*M_PI/15.0;
			double dx = cos(rad);
			double dy = sin(rad);

			if (texture >= 1)
				glTexCoord2f(l/30.0, 0.0);
			if (texture >= 2)
			glMultiTexCoord2fARB(GL_TEXTURE1_ARB, 0.0, l/30.0);
			if (texture == 0)
				glColor3f(0, 1, 0);
			glVertex2f(dx*0.2, dy*0.2);

			if (texture >= 1)
				glTexCoord2f(l/30.0, 1.0);
			if (texture >= 2)
			glMultiTexCoord2fARB(GL_TEXTURE1_ARB, 1.0, l/30.0);
			if (texture == 0)
				glColor3f(1, 0, 1);
			glVertex2f(dx*0.8, dy*0.8);
		}
	glEnd();
	glColor3f(1,1,1);

	glPopMatrix();
}

static void Display(void)
{
	int texture;

	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	for(texture = 0; texture < 3; ++texture) {
		glPushMatrix();
		glTranslatef(0, texture, 0);

		if (texture == 0) {
			glActiveTextureARB(GL_TEXTURE0_ARB);
			glDisable(GL_TEXTURE_2D);
			glActiveTextureARB(GL_TEXTURE1_ARB);
			glDisable(GL_TEXTURE_2D);
		}
		else if (texture == 1) {
			glActiveTextureARB(GL_TEXTURE0_ARB);
			glEnable(GL_TEXTURE_2D);
			glActiveTextureARB(GL_TEXTURE1_ARB);
			glDisable(GL_TEXTURE_2D);
		}
		else {
			glActiveTextureARB(GL_TEXTURE0_ARB);
			glEnable(GL_TEXTURE_2D);
			glActiveTextureARB(GL_TEXTURE1_ARB);
			glEnable(GL_TEXTURE_2D);
		}

		glDisable(GL_LINE_SMOOTH);
		glDisable(GL_BLEND);
		glDisable(GL_LINE_STIPPLE);
		DoStar(texture);

		glTranslatef(1, 0, 0);
		glEnable(GL_LINE_STIPPLE);
		DoStar(texture);

		glTranslatef(1, 0, 0);
		glEnable(GL_LINE_SMOOTH);
		glEnable(GL_POINT_SMOOTH);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glDisable(GL_LINE_STIPPLE);
		DoStar(texture);

		glTranslatef(1, 0, 0);
		glEnable(GL_LINE_STIPPLE);
		DoStar(texture);

		glPopMatrix();
	}

	glutSwapBuffers();
}


static void Reshape(int width, int height)
{
	Width = width;
	Height = height;
	glViewport(0, 0, Width, Height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0, 4.0, 0.0, 3.0, -1.0, 1.0);
	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();
}


static void Key( unsigned char key, int x, int y )
{
	(void) x;
	(void) y;
	switch (key) {
		case 27:
			exit(0);
			break;
	}
}


static void Init( int argc, char *argv[] )
{
	GLuint u;
	for (u = 0; u < 2; u++) {
		glActiveTextureARB(GL_TEXTURE0_ARB + u);
		glBindTexture(GL_TEXTURE_2D, 10+u);
		if (u == 0)
			glEnable(GL_TEXTURE_2D);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		if (u == 0)
			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
		else
			glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_ADD);

		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		if (!LoadRGBMipmaps(TEXTURE_FILE, GL_RGB)) {
			printf("Error: couldn't load texture image\n");
			exit(1);
		}
	}

	glLineStipple(1, 0xff);

	printf("GL_RENDERER   = %s\n", (char *) glGetString(GL_RENDERER));
	printf("GL_VERSION    = %s\n", (char *) glGetString(GL_VERSION));
	printf("GL_VENDOR     = %s\n", (char *) glGetString(GL_VENDOR));
	printf("GL_EXTENSIONS = %s\n", (char *) glGetString(GL_EXTENSIONS));

	Reshape(Width, Height);
}


int main( int argc, char *argv[] )
{
	glutInit( &argc, argv );
	glutInitWindowPosition(0, 0);
	glutInitWindowSize(Width, Height);

	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);

	glutCreateWindow("texline");

	Init(argc, argv);

	glutReshapeFunc(Reshape);
	glutKeyboardFunc(Key);
	glutDisplayFunc(Display);

	glutMainLoop();
	return 0;
}
