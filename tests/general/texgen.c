/*
 * Copyright (c) The Piglit project 2008
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
 * @file
 * Test a number of basic TexGen functions.
 */

#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#if defined(__APPLE__)
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#include "piglit-util.h"


static int Width = 128, Height = 128;
static int Automatic = 0;
static int CurrentTest = 0;
static int UseFragmentProgram = 0;

/**
 * The test uses a 4x4 clamped, nearest-filtered texture with the following
 * RGB colors. The pattern matches what @ref TextureFP produces and is filled
 * in in @ref Init.
 */
static GLfloat TextureData[4][4][3];

/**
 * Implement the inner part of the above texture in a fragment program.
 */
static const char TextureFP[] =
"!!ARBfp1.0\n"
"TEMP r0;\n"
"MUL r0, fragment.texcoord, 4;\n"
"FLR r0, r0;\n"
"MUL result.color, r0, 0.25;\n"
"END\n";

static void probe_cell(const char* testname, int x, int y, const float* expected)
{
	if (!piglit_probe_pixel_rgb((2*x+1)*Width/8, (2*y+1)*Height/8, expected)) {
		fprintf(stderr, "%s: %i,%i failed\n", testname, x, y);
		if (Automatic)
			piglit_report_result(PIGLIT_FAILURE);
	}
}

/**
 * Sanity test whether the texture is rendered correctly at all.
 */
static void test_sanity(void)
{
	int x, y;

	glClearColor(0.5, 0.5, 0.5, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

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
	glReadBuffer(GL_FRONT);

	for(y = 0; y < 4; ++y) {
		for(x = 0; x < 4; ++x)
			probe_cell("test_sanity", x, y, TextureData[y][x]);
	}
}

static void do_test_texgen_eye(const char* testname)
{
	static GLfloat sPlane1[4] = { 1.0, 0.0, 0.0, 0.25 };
	static GLfloat sPlane2[4] = { 1.0, 0.0, 0.0, -0.25 };
	static GLfloat sPlane3[4] = { -1.0, 0.0, 0.0, 1.25 };
	int x, y;

	glClearColor(0.5, 0.5, 0.5, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	// Note: Modelview matrix is identity
	glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
	glTexGenfv(GL_S, GL_EYE_PLANE, sPlane1);
	glEnable(GL_TEXTURE_GEN_S);

	// Draw lower left quad
	glBegin(GL_QUADS);
		glTexCoord2f(0, 0.25); glVertex2f(0.0, 0.0);
		glTexCoord2f(0, 0.25); glVertex2f(0.5, 0.0);
		glTexCoord2f(0, 0.75); glVertex2f(0.5, 0.5);
		glTexCoord2f(0, 0.75); glVertex2f(0.0, 0.5);
	glEnd();

	// Draw lower right quad
	glTexGenfv(GL_S, GL_EYE_PLANE, sPlane2);
	glPushMatrix();
	glTranslatef(0.5, -0.5, 0.0);
	glScalef(2.0, 1.0, 1.0);
	glBegin(GL_QUADS);
		glTexCoord2f(0, 0.25); glVertex2f(0.0,  0.5);
		glTexCoord2f(0, 0.25); glVertex2f(0.25, 0.5);
		glTexCoord2f(0, 0.75); glVertex2f(0.25, 1.0);
		glTexCoord2f(0, 0.75); glVertex2f(0.0,  1.0);
	glEnd();
	glPopMatrix();

	// Draw upper left quad
	glPushMatrix();
	glTranslatef(1.0, 0.5, 0.0);
	glScalef(-1.0, 1.0, 1.0);
	glTexGenfv(GL_S, GL_EYE_PLANE, sPlane3);
	glBegin(GL_QUADS);
		glTexCoord2f(0, 0.25); glVertex2f(1.0, 0.0);
		glTexCoord2f(0, 0.25); glVertex2f(0.5, 0.0);
		glTexCoord2f(0, 0.75); glVertex2f(0.5, 0.5);
		glTexCoord2f(0, 0.75); glVertex2f(1.0, 0.5);
	glEnd();
	glPopMatrix();

	glDisable(GL_TEXTURE_GEN_S);

	glutSwapBuffers();
	glReadBuffer(GL_FRONT);

	for(y = 0; y < 2; ++y) {
		for(x = 0; x < 2; ++x)
			probe_cell(testname, x, y, TextureData[y+1][x+1]);
	}
}

static void test_texgen_eye(void)
{
	do_test_texgen_eye("test_texgen_eye");
}

static void test_texgen_eye_fp(void)
{
	if (UseFragmentProgram) {
		glEnable(GL_FRAGMENT_PROGRAM_ARB);
		do_test_texgen_eye("test_texgen_eye_fp");
		glDisable(GL_FRAGMENT_PROGRAM_ARB);
	}
}

static struct {
	const char* name;
	void (*function)(void);
} Tests[] = {
	{ "sanity", &test_sanity },
	{ "texgen_eye", &test_texgen_eye },
	{ "texgen_eye_fp", &test_texgen_eye_fp }
};
#define NrTests (ARRAY_SIZE(Tests))

static void Redisplay(void)
{
	if (Automatic) {
		int i;
		for(i = 0; i < NrTests; ++i)
			Tests[i].function();

		piglit_report_result(PIGLIT_SUCCESS);
	} else {
		Tests[CurrentTest].function();
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
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}


static void Init(void)
{
	int x, y;

	if (piglit_use_fragment_program()) {
		UseFragmentProgram = 1;
		pglBindProgramARB(GL_FRAGMENT_PROGRAM_ARB,
			piglit_compile_program(GL_FRAGMENT_PROGRAM_ARB, TextureFP));
	}

	for(y = 0; y < 4; ++y) {
		for(x = 0; x < 4; ++x) {
			TextureData[y][x][0] = x * 0.25;
			TextureData[y][x][1] = y * 0.25;
			TextureData[y][x][2] = 0.0;
		}
	}

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 4, 4, 0, GL_RGB, GL_FLOAT, TextureData);
	glEnable(GL_TEXTURE_2D);

	Reshape(Width,Height);
}

static void Key(unsigned char key, int x, int y)
{
	(void) x;
	(void) y;
	switch (key) {
	case 't':
		CurrentTest++;
		if (CurrentTest >= NrTests)
			CurrentTest = 0;
		printf("Test: %s\n", Tests[CurrentTest].name);
		break;
	case 27:
		exit(0);
		break;
	}
	glutPostRedisplay();
}

int main(int argc, char *argv[])
{
	glutInit(&argc, argv);
	if (argc == 2 && !strcmp(argv[1], "-auto"))
		Automatic = 1;
	glutInitWindowPosition(0, 0);
	glutInitWindowSize(Width, Height);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
	glutCreateWindow(argv[0]);
	glutReshapeFunc(Reshape);
	glutDisplayFunc(Redisplay);
	if (!Automatic) {
		printf("Press 't' to switch tests; Escape to quit\n");
		glutKeyboardFunc(Key);
	}
	Init();
	glutMainLoop();
	return 0;
}

