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
 * Test fragment.position.
 */

#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <GL/glew.h>
#if defined(__APPLE__)
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif
#include "piglit-util.h"

#define NUM_PROGRAMS 4

static GLuint FragProg[NUM_PROGRAMS];

static const char* const ProgramText[NUM_PROGRAMS] = {
		/* Color = fragment pos * scale factor */
		"!!ARBfp1.0\n"
		"PARAM factor = { 0.01, 0.01, 1.0, 0.2 };\n"
		"MUL result.color, fragment.position, factor;\n"
		"END",

		/* Color = dependent 2D texture read */
		"!!ARBfp1.0\n"
		"TEMP r0;\n"
		"ALIAS scaled = r0;\n"
		"MUL r0.xy, fragment.position, 0.01;\n"
		"TEX result.color, scaled, texture[1], 2D;\n"
		"END",

		/* Color = RECT texture color at fragment pos */
		"!!ARBfp1.0\n"
		"TEX result.color, fragment.position, texture[0], RECT;\n"
		"END",

		/* Color = 2D texture color at fragment pos */
		"!!ARBfp1.0\n"
		"PARAM scale = { 0.01, 0.01, 1.0, 1.0 };\n"
		"TEMP tc;\n"
		"MUL tc, fragment.position, scale;\n"
		"TEX result.color, tc, texture[1], 2D;\n"
		"MOV result.color.w, 0.5;\n"
		"END",
};

static int Automatic = 0;

static int Width = 200, Height = 200;


/**
 * Draw four quadrilaterals, one for each fragment program:
 *  +--------+--------+
 *  |        |        |
 *  | Prog 1 | Prog 3 |
 *  |        |        |
 *  +--------+--------+
 *  |        |        |
 *  | Prog 0 | Prog 2 |
 *  |        |        |
 *  +--------+--------+
 * Each quad is about 100x100 pixels in size.
 */
static void DoFrame(void)
{
	printf("rgba: %i %i %i %i\n",
	       glutGet(GLUT_WINDOW_RED_SIZE),
	       glutGet(GLUT_WINDOW_GREEN_SIZE),
	       glutGet(GLUT_WINDOW_BLUE_SIZE),
	       glutGet(GLUT_WINDOW_ALPHA_SIZE));

	glClearColor(0.3, 0.3, 0.3, 0.3);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_FRAGMENT_PROGRAM_ARB);

	pglBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, FragProg[0]);

	glBegin(GL_QUADS);
		glVertex3f(0, 0, 0);
		glVertex3f(1, 0, 1);
		glVertex3f(1, 1, 2);
		glVertex3f(0, 1, 1);
	glEnd();

	pglBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, FragProg[1]);

	glBegin(GL_QUADS);
		glVertex2f(0, 1);
		glVertex2f(1, 1);
		glVertex2f(1, 2);
		glVertex2f(0, 2);
	glEnd();

	pglBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, FragProg[2]);

	glBegin(GL_QUADS);
		glVertex2f(1, 0);
		glVertex2f(2, 0);
		glVertex2f(2, 1);
		glVertex2f(1, 1);
	glEnd();

	pglBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, FragProg[3]);

	glBegin(GL_QUADS);
		glVertex2f(1, 1);
		glVertex2f(2, 1);
		glVertex2f(2, 2);
		glVertex2f(1, 2);
	glEnd();

	glutSwapBuffers();
}

static const struct {
	const char* name;
	float x, y;
	float expected[4];
} Probes[] = {
	// Program 0
	{
		"basic #1",
		0.2, 0.2,
		{ 0.2, 0.2, (0.4+2)/8, 0.2 }
	},
	{
		"basic #2",
		0.8, 0.2,
		{ 0.8, 0.2, (1.0+2)/8, 0.2 }
	},
	{
		"basic #3",
		0.8, 0.8,
		{ 0.8, 0.8, (1.6+2)/8, 0.2 }
	},
	{
		"basic #4",
		0.2, 0.8,
		{ 0.2, 0.8, (1.0+2)/8, 0.2 }
	},

	// Program 1
	{
		"tex2d scaled #1",
		0.2, 1.2,
		{ 0.8, 0.2, 0.2, 0.2 }
	},
	{
		"tex2d scaled #2",
		0.8, 1.2,
		{ 0.2, 0.2, 0.8, 0.5 }
	},
	{
		"tex2d scaled #3",
		0.8, 1.8,
		{ 0.2, 0.8, 0.8, 0.8 }
	},
	{
		"tex2d scaled #4",
		0.2, 1.8,
		{ 0.8, 0.8, 0.2, 0.5 }
	},

	// Program 2
	{
		"texrect #1",
		1.2, 0.2,
		{ 0.53, 0.47, 0.08, 0.27 }
	},
	{
		"texrect #2",
		1.8, 0.2,
		{ 0.29, 0.70, 0.08, 0.40 }
	},
	{
		"texrect #1",
		1.8, 0.8,
		{ 0.29, 0.70, 0.31, 0.51 }
	},
	{
		"texrect #1",
		1.2, 0.8,
		{ 0.53, 0.47, 0.31, 0.39 }
	},

	// Program 3
	{
		"tex2d unscaled #1",
		1.2, 1.2,
		{ 0.8, 0.2, 0.2, 0.5 }
	},
	{
		"tex2d unscaled #2",
		1.8, 1.2,
		{ 0.2, 0.2, 0.8, 0.5 }
	},
	{
		"tex2d unscaled #3",
		1.8, 1.8,
		{ 0.2, 0.8, 0.8, 0.5 }
	},
	{
		"tex2d unscaled #4",
		1.2, 1.8,
		{ 0.8, 0.8, 0.2, 0.5 }
	},

	// Sentinel!
	{
		0,
		0, 0,
		{ 0, 0, 0, 0 }
	}
};

static int DoTest( void )
{
	int idx;
	GLfloat dmax;

	glReadBuffer( GL_FRONT );
	dmax = 0;

	idx = 0;
	while(Probes[idx].name) {
		GLfloat probe[4];
		GLfloat delta[4];
		int i;

		/*
                printf("ReadPixels at %d, %d\n",
                       (int)(Probes[idx].x*Width/2),
                       (int)(Probes[idx].y*Height/2));
		*/

		glReadPixels((int)(Probes[idx].x*Width/2),
		             (int)(Probes[idx].y*Height/2),
		             1, 1,
		             GL_RGBA, GL_FLOAT, probe);

		printf("%20s (%3.1f,%3.1f): %f,%f,%f,%f",
		       Probes[idx].name,
		       Probes[idx].x, Probes[idx].y,
		       probe[0], probe[1], probe[2], probe[3]);

		for(i = 0; i < 4; ++i) {
			delta[i] = probe[i] - Probes[idx].expected[i];

			if (delta[i] > dmax) dmax = delta[i];
			else if (-delta[i] > dmax) dmax = -delta[i];
		}

		printf("   Delta: %f,%f,%f,%f\n", delta[0], delta[1], delta[2], delta[3]);

		idx++;
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
	glOrtho(0.0, 2.0, 0.0, 2.0, -2.0, 6.0);
	glScalef(1.0, 1.0, -1.0); // flip z-axis
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

static void Init(void)
{
	int i, x, y;
	GLubyte rectangle[200][200][4];
	GLubyte tex[256*256][4];

	printf("GL_RENDERER = %s\n", (char *) glGetString(GL_RENDERER));

	piglit_require_fragment_program();

	for(i = 0; i < NUM_PROGRAMS; ++i)
		FragProg[i] = piglit_compile_program(GL_FRAGMENT_PROGRAM_ARB, ProgramText[i]);

	/*
	 * Texture unit 0: 200x200 RECTANGLE texture
	 */
	for(y = 0; y < 200; ++y) {
		for(x = 0; x < 200; ++x) {
			rectangle[y][x][0] = 255-x;
			rectangle[y][x][1] = x;
			rectangle[y][x][2] = y;
			rectangle[y][x][3] = (x+y)/2;
		}
	}

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_RECTANGLE_ARB, 1);
	glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, GL_RGBA, 200, 200, 0,
	             GL_RGBA, GL_UNSIGNED_BYTE, rectangle);

	/*
	 * Texture unit 1: 256x256 2D texture
	 */
	for(y = 0; y < 256; ++y) {
		for(x = 0; x < 256; ++x) {
			tex[256*y+x][0] = 255-x;
			tex[256*y+x][1] = y;
			tex[256*y+x][2] = x;
			tex[256*y+x][3] = (x+y)/2;
		}
	}

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, 2);
	gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGBA, 256, 256,
	                  GL_RGBA, GL_UNSIGNED_BYTE, tex);

	// Overwrite higher mipmap levels
	for(x = 0; x < 4; ++x) {
		tex[x][0] = 255;
		tex[x][1] = 128;
		tex[x][2] = 128;
		tex[x][3] = 255;
	}

	glTexImage2D(GL_TEXTURE_2D, 7, GL_RGBA, 2, 2, 0,
	             GL_RGBA, GL_UNSIGNED_BYTE, tex);
	glTexImage2D(GL_TEXTURE_2D, 8, GL_RGBA, 1, 1, 0,
	             GL_RGBA, GL_UNSIGNED_BYTE, tex);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	Reshape(Width,Height);
}

int main(int argc, char *argv[])
{
	glutInit(&argc, argv);
	if (argc == 2 && !strcmp(argv[1], "-auto"))
		Automatic = 1;
	glutInitWindowPosition(0, 0);
	glutInitWindowSize(Width, Height);
	glutInitDisplayMode(GLUT_RGBA | GLUT_ALPHA | GLUT_DOUBLE | GLUT_DEPTH);
	glutCreateWindow(argv[0]);
	glutReshapeFunc(Reshape);
	glutKeyboardFunc(piglit_escape_exit_key);
	glutDisplayFunc(Redisplay);
	glewInit();

	if (!GLEW_VERSION_1_3) {
		printf("Requires OpenGL 1.3\n");
		piglit_report_result(PIGLIT_SKIP);
		exit(1);
	}

	Init();
	glutMainLoop();
	return 0;
}


