/*
 * Copyright (c) The Piglit project 2007
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * on the rights to use, copy, modify, merge, publish, distribute, sub
 * license, and/or sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.  IN NO EVENT SHALL
 * VA LINUX SYSTEM, IBM AND/OR THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/**
 * Test whether LIT honours the output mask.
 */

#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <GL/glut.h>


static GLuint FragProg[15];

static const char fragProgramTemplate[] =
	"!!ARBfp1.0\n"
	"PARAM values = { 0.65, 0.9, 0.0, 8.0 }; \n"
	"PARAM bogus = { 0.8, 0.8, 0.8, 0.8 }; \n"
	"MOV result.color, bogus; \n"
	"LIT result.color.%s, values; \n"
	"END\n";
static float LitExpected[4] = { 1.0, 0.65, 0.433, 1.0 };

static int Automatic = 0;

static int Width = 200, Height = 200;

static PFNGLPROGRAMLOCALPARAMETER4FVARBPROC pglProgramLocalParameter4fvARB;
static PFNGLPROGRAMLOCALPARAMETER4DARBPROC pglProgramLocalParameter4dARB;
static PFNGLGETPROGRAMLOCALPARAMETERDVARBPROC pglGetProgramLocalParameterdvARB;
static PFNGLGENPROGRAMSARBPROC pglGenProgramsARB;
static PFNGLPROGRAMSTRINGARBPROC pglProgramStringARB;
static PFNGLBINDPROGRAMARBPROC pglBindProgramARB;
static PFNGLISPROGRAMARBPROC pglIsProgramARB;
static PFNGLDELETEPROGRAMSARBPROC pglDeleteProgramsARB;


static void DoFrame(void)
{
	int mask;
	
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	
	glEnable(GL_FRAGMENT_PROGRAM_ARB);

	for(mask = 1; mask < 16; ++mask) {
		pglBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, FragProg[mask-1]);
		glPushMatrix();
		glTranslatef((mask % 4), (mask / 4), 0.0);

		glBegin(GL_QUADS);
			glVertex2f(0, 0);
			glVertex2f(1, 0);
			glVertex2f(1, 1);
			glVertex2f(0, 1);
		glEnd();

		glPopMatrix();
	}

	glutSwapBuffers();
}

static int DoTest( void )
{
	int mask;
	GLfloat dmax;
	
	glReadBuffer( GL_FRONT );
	dmax = 0;
	
	for(mask = 1; mask < 16; ++mask) {
		GLfloat probe[4];
		GLfloat delta[4];
		int i;

		glReadPixels(Width*(2*(mask%4)+1)/8, Height*(2*(mask/4)+1)/8, 1, 1,
				GL_RGBA, GL_FLOAT, probe);
		
		printf("Probe %i: %f,%f,%f,%f\n", mask, probe[0], probe[1], probe[2], probe[3]);
		
		for(i = 0; i < 4; ++i) {
			if (mask & (1 << i))
				delta[i] = probe[i] - LitExpected[i];
			else
				delta[i] = probe[i] - 0.8;
			
			if (delta[i] > dmax) dmax = delta[i];
			else if (-delta[i] > dmax) dmax = -delta[i];
		}
	
		printf("   Delta: %f,%f,%f,%f\n", delta[0], delta[1], delta[2], delta[3]);
	}

	printf("Max delta: %f\n", dmax);

	if (dmax >= 0.02)
		return 0;
	else
		return 1;
}


static void Redisplay(void)
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
	glOrtho(0.0, 4.0, 0.0, 4.0, -1.0, 1.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}


static void Key(unsigned char key, int x, int y)
{
	(void) x;
	(void) y;
	switch (key) {
		case 27:
			pglDeleteProgramsARB(15, FragProg);
			exit(0);
			break;
	}
	glutPostRedisplay();
}


static void Init(void)
{
	int mask;
	
	printf("GL_RENDERER = %s\n", (char *) glGetString(GL_RENDERER));
	
	if (!glutExtensionSupported("GL_ARB_fragment_program")) {
		fprintf(stderr, "Sorry, this demo requires GL_ARB_fragment_program\n");
		if (Automatic)
			printf("PIGLIT: {'result': 'fail' }\n");
		exit(1);
	}

	/*
	 * Get extension function pointers.
	 */
	pglProgramLocalParameter4fvARB = (PFNGLPROGRAMLOCALPARAMETER4FVARBPROC) glutGetProcAddress("glProgramLocalParameter4fvARB");
	assert(pglProgramLocalParameter4fvARB);
	
	pglProgramLocalParameter4dARB = (PFNGLPROGRAMLOCALPARAMETER4DARBPROC) glutGetProcAddress("glProgramLocalParameter4dARB");
	assert(pglProgramLocalParameter4dARB);
	
	pglGetProgramLocalParameterdvARB = (PFNGLGETPROGRAMLOCALPARAMETERDVARBPROC) glutGetProcAddress("glGetProgramLocalParameterdvARB");
	assert(pglGetProgramLocalParameterdvARB);
	
	pglGenProgramsARB = (PFNGLGENPROGRAMSARBPROC) glutGetProcAddress("glGenProgramsARB");
	assert(pglGenProgramsARB);
	
	pglProgramStringARB = (PFNGLPROGRAMSTRINGARBPROC) glutGetProcAddress("glProgramStringARB");
	assert(pglProgramStringARB);
	
	pglBindProgramARB = (PFNGLBINDPROGRAMARBPROC) glutGetProcAddress("glBindProgramARB");
	assert(pglBindProgramARB);
	
	pglIsProgramARB = (PFNGLISPROGRAMARBPROC) glutGetProcAddress("glIsProgramARB");
	assert(pglIsProgramARB);
	
	pglDeleteProgramsARB = (PFNGLDELETEPROGRAMSARBPROC) glutGetProcAddress("glDeleteProgramsARB");
	assert(pglDeleteProgramsARB);
	
	/*
	 * Fragment programs
	 */
	pglGenProgramsARB(15, FragProg);
	
	for(mask = 1; mask < 16; ++mask) {
		GLint errorPos;
		char programText[1024];
		char maskstring[5];
		
		maskstring[0] = 0;
		if (mask & 1) strcat(maskstring, "x");
		if (mask & 2) strcat(maskstring, "y");
		if (mask & 4) strcat(maskstring, "z");
		if (mask & 8) strcat(maskstring, "w");
		sprintf(programText, fragProgramTemplate, maskstring);
		
		assert(FragProg[mask-1] > 0);
		pglBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, FragProg[mask-1]);
		pglProgramStringARB(
				GL_FRAGMENT_PROGRAM_ARB,
				GL_PROGRAM_FORMAT_ASCII_ARB,
				strlen(programText),
				(const GLubyte *)programText);
		glGetIntegerv(GL_PROGRAM_ERROR_POSITION_ARB, &errorPos);
		if (glGetError() != GL_NO_ERROR || errorPos != -1) {
			int l = FindLine(programText, errorPos);
			fprintf(stderr, "Fragment Program Error (pos=%d line=%d): %s\n", errorPos, l,
					(char *) glGetString(GL_PROGRAM_ERROR_STRING_ARB));
			if (Automatic)
				printf("PIGLIT: {'result': 'fail' }\n");
			exit(1);
		}
		if (!pglIsProgramARB(FragProg[mask-1])) {
			fprintf(stderr, "pglIsProgramARB failed\n");
			if (Automatic)
				printf("PIGLIT: {'result': 'fail' }\n");
			exit(1);
		}
	}
	
	Reshape(Width,Height);
}


int main(int argc, char *argv[])
{
	glutInit(&argc, argv);
	if (argc == 2 && !strcmp(argv[1], "-auto"))
		Automatic = 1;
	glutInitWindowPosition(0, 0);
	glutInitWindowSize(Width, Height);
	glutInitDisplayMode(GLUT_RGB | GLUT_ALPHA | GLUT_DOUBLE | GLUT_DEPTH);
	glutCreateWindow(argv[0]);
	glutReshapeFunc(Reshape);
	glutKeyboardFunc(Key);
	glutDisplayFunc(Redisplay);
	Init();
	glutMainLoop();
	return 0;
}

