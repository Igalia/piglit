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
 * According to the ARB_fragment_program spec, section 3.11.6,
 * sampling an incomplete texture image yields (0,0,0,1).
 */

#include "piglit-util.h"

int piglit_width = 100, piglit_height = 100;
int piglit_window_mode = GLUT_RGBA | GLUT_ALPHA | GLUT_DOUBLE | GLUT_DEPTH;

#define NUM_PROGRAMS 5

static GLuint FragProg[NUM_PROGRAMS];

static const char* const ProgramText[NUM_PROGRAMS] = {
		"!!ARBfp1.0\n"
		"TEX result.color, fragment.color, texture[0], 2D;\n"
		"END",

		"!!ARBfp1.0\n"
		"TEX result.color, fragment.color, texture[0], 3D;\n"
		"END",

		"!!ARBfp1.0\n"
		"TEX result.color, fragment.color, texture[0], 1D;\n"
		"END",

		"!!ARBfp1.0\n"
		"TEX result.color, fragment.color, texture[0], CUBE;\n"
		"END",

		"!!ARBfp1.0\n"
		"TEX result.color, fragment.color, texture[0], RECT;\n"
		"END"
};

static void DoFrame(void)
{
	int i;

	glClearColor(0.3, 0.3, 0.3, 0.3);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_FRAGMENT_PROGRAM_ARB);

	for(i = 0; i < NUM_PROGRAMS; ++i) {
		glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, FragProg[i]);

		glPushMatrix();
		glTranslatef(i/2, i%2, 0);
		glBegin(GL_QUADS);
			glVertex2f(0, 0);
			glVertex2f(1, 0);
			glVertex2f(1, 1);
			glVertex2f(0, 1);
		glEnd();
		glPopMatrix();
	}
}

static const struct {
	const char* name;
	float x, y;
	float expected[4];
} Probes[] = {
	{
		"incomplete 2D",
		0.5, 0.5,
		{ 0,0,0,1 }
	},
	{
		"incomplete 3D",
		0.5, 1.5,
		{ 0,0,0,1 }
	},
	{
		"incomplete 1D",
		1.5, 0.5,
		{ 0,0,0,1 }
	},
	{
		"incomplete CUBE",
		1.5, 1.5,
		{ 0,0,0,1 }
	},
	{
		"incomplete RECT",
		2.5, 0.5,
		{ 0,0,0,1 }
	},

	{
		"sanity",
		2.5, 1.5,
		{ 0.3,0.3,0.3,0.3 }
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

	dmax = 0;

	idx = 0;
	while(Probes[idx].name) {
		GLfloat probe[4];
		GLfloat delta[4];
		int i;

		glReadPixels((int)(Probes[idx].x * piglit_width / 3),
			     (int)(Probes[idx].y * piglit_height / 2),
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


enum piglit_result
piglit_display(void)
{
	int pass;

	DoFrame();
	pass = DoTest();
	glutSwapBuffers();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}


static void Reshape(int width, int height)
{
	piglit_width = width;
	piglit_height = height;
	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0, 3.0, 0.0, 2.0, -2.0, 6.0);
	glScalef(1.0, 1.0, -1.0); // flip z-axis
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void
piglit_init(int argc, char **argv)
{
	int i;

	printf("GL_RENDERER = %s\n", (char *) glGetString(GL_RENDERER));

	glutReshapeFunc(Reshape);

	piglit_require_fragment_program();

	for(i = 0; i < NUM_PROGRAMS; ++i)
		FragProg[i] = piglit_compile_program(GL_FRAGMENT_PROGRAM_ARB, ProgramText[i]);

	Reshape(piglit_width, piglit_height);
}

