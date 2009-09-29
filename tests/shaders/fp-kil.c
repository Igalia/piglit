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
 * Test KIL instruction.
 */

#include "piglit-util.h"

int piglit_width = 200, piglit_height = 200;
int piglit_window_mode = GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH;

#define NUM_PROGRAMS 2

static GLuint FragProg[NUM_PROGRAMS];

static const char* const ProgramText[NUM_PROGRAMS] = {
		"!!ARBfp1.0\n"
		"TEMP r0;\n"
		"MOV result.color, fragment.color;\n"
		"KIL fragment.texcoord[0];\n"
		"END",

		"!!ARBfp1.0\n"
		"TEMP r0;\n"
		"TEX r0, fragment.texcoord[0], texture[0], 2D;\n"
		"KIL -r0;\n"
		"MOV result.color, fragment.color;\n"
		"END"
};

static void DoFrame(void)
{
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_FRAGMENT_PROGRAM_ARB);

	pglBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, FragProg[0]);

	glColor3f(0.0, 1.0, 0.0);
	glBegin(GL_QUADS);
		glTexCoord2f(-1, -1);
		glVertex2f(0, 0);
		glTexCoord2f(1, -1);
		glVertex2f(1, 0);
		glTexCoord2f(1, 1);
		glVertex2f(1, 1);
		glTexCoord2f(-1, 1);
		glVertex2f(0, 1);
	glEnd();

	pglBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, FragProg[1]);

	glBegin(GL_QUADS);
		glTexCoord2f(0, 0);
		glVertex2f(0, 1);
		glTexCoord2f(1, 0);
		glVertex2f(1, 1);
		glTexCoord2f(1, 1);
		glVertex2f(1, 2);
		glTexCoord2f(0, 1);
		glVertex2f(0, 2);
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
		{ 0.0, 0.0, 0.0, 1.0 }
	},
	{
		"basic #2",
		0.8, 0.2,
		{ 0.0, 0.0, 0.0, 1.0 }
	},
	{
		"basic #3",
		0.8, 0.8,
		{ 0.0, 1.0, 0.0, 1.0 }
	},
	{
		"basic #4",
		0.2, 0.8,
		{ 0.0, 0.0, 0.0, 1.0 }
	},

	// Program 0
	{
		"texture #1",
		0.125, 1.125,
		{ 0.0, 1.0, 0.0, 1.0 }
	},
	{
		"texture #2",
		0.375, 1.125,
		{ 0.0, 0.0, 0.0, 1.0 }
	},
	{
		"texture #3",
		0.625, 1.125,
		{ 0.0, 0.0, 0.0, 1.0 }
	},
		{
		"texture #4",
		0.875, 1.125,
		{ 0.0, 0.0, 0.0, 1.0 }
	},
	{
		"texture #5",
		0.125, 1.375,
		{ 0.0, 0.0, 0.0, 1.0 }
	},
	{
		"texture #6",
		0.375, 1.375,
		{ 0.0, 0.0, 0.0, 1.0 }
	},
	{
		"texture #7",
		0.625, 1.375,
		{ 0.0, 0.0, 0.0, 1.0 }
	},
	{
		"texture #8",
		0.875, 1.375,
		{ 0.0, 0.0, 0.0, 1.0 }
	},
	{
		"texture #9",
		0.125, 1.625,
		{ 0.0, 0.0, 0.0, 1.0 }
	},
	{
		"texture #10",
		0.375, 1.625,
		{ 0.0, 0.0, 0.0, 1.0 }
	},
	{
		"texture #11",
		0.625, 1.625,
		{ 0.0, 0.0, 0.0, 1.0 }
	},
	{
		"texture #12",
		0.875, 1.625,
		{ 0.0, 0.0, 0.0, 1.0 }
	},
	{
		"texture #13",
		0.125, 1.875,
		{ 0.0, 0.0, 0.0, 1.0 }
	},
	{
		"texture #14",
		0.375, 1.875,
		{ 0.0, 0.0, 0.0, 1.0 }
	},
	{
		"texture #15",
		0.625, 1.875,
		{ 0.0, 0.0, 0.0, 1.0 }
	},
	{
		"texture #16",
		0.875, 1.875,
		{ 0.0, 0.0, 0.0, 1.0 }
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

		glReadPixels((int)(Probes[idx].x*piglit_width/2),
		             (int)(Probes[idx].y*piglit_height/2),
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

	return pass ? PIGLIT_SUCCESS : PIGLIT_FAILURE;
}


static void Reshape(int width, int height)
{
	piglit_width = width;
	piglit_height = height;
	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0, 2.0, 0.0, 2.0, -2.0, 6.0);
	glScalef(1.0, 1.0, -1.0); // flip z-axis
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void
piglit_init(int argc, char **argv)
{
	int i, x, y;
	GLubyte tex[4][4][4];

	glutReshapeFunc(Reshape);

	if (!GLEW_VERSION_1_3) {
		printf("Requires OpenGL 1.3\n");
		piglit_report_result(PIGLIT_SKIP);
	}

	printf("GL_RENDERER = %s\n", (char *) glGetString(GL_RENDERER));

	piglit_require_fragment_program();

	/*
	 * Fragment programs
	 */
	for(i = 0; i < NUM_PROGRAMS; ++i)
		FragProg[i] = piglit_compile_program(GL_FRAGMENT_PROGRAM_ARB, ProgramText[i]);

	/*
	 * Textures
	 */
	for(y = 0; y < 4; ++y) {
		for(x = 0; x < 4; ++x) {
			tex[y][x][0] = (x & 1) ? 255 : 0;
			tex[y][x][1] = (x & 2) ? 255 : 0;
			tex[y][x][2] = (y & 1) ? 255 : 0;
			tex[y][x][3] = (y & 2) ? 255 : 0;
		}
	}

	glActiveTexture(GL_TEXTURE0);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, 1);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 4, 4, 0,
	             GL_RGBA, GL_UNSIGNED_BYTE, tex);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	Reshape(piglit_width, piglit_height);
}
