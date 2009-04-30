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
 * Test the GL_EXT_texture_lod_bias extension.
 *
 * Only test LOD bias with a granularity of 1.0 with a nearest mip filter.
 * This leaves room for somewhat inaccurate hardware implementations.
 * The point of this test is that the implementation has to get the big
 * picture issues right:
 *
 *  1. LOD bias is per texture stage, not per texture object.
 *  2. LOD bias is applied *before* clamping.
 *  3. The supported bias range must be reported correctly.
 *
 * @todo
 * Check per-texture object LOD bias (support for this was added in OpenGL 1.4).
 * In particular, check interaction of per-texture and per-TexUnit bias.
 * Check clamping behaviour.
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

#define SquareSize 32

static int Width = 3*SquareSize, Height = 3*SquareSize;
static int Automatic = 0;
static int CurrentTest = 0;
static int CurrentBias = 0;
static int CurrentBias2 = 0;
static int MaxTextureLodBias;
static GLuint Textures[2];

#define NrTests 2

/**
 * The test uses two 4x4 clamped, mipmapped textures (i.e. 3 mip levels)
 * with the following RGB colors on each level.
 *
 * Note: Black is used as a background color, so don't use black for the textures.
 */
static GLfloat TextureData[2][3][3] = {
	{ { 0.5, 0.5, 0.5 }, { 0.5, 0.0, 0.0 }, { 0.0, 0.5, 0.0 } },
	{ { 0.0, 0.0, 0.5 }, { 0.5, 0.5, 0.0 }, { 0.0, 0.5, 0.5 } }
};

static void probe_cell(const char* testname, int cellx, int celly, const float* expected)
{
	int x, y;

	for(y = 0; y < 4; ++y) {
		for(x = 0; x < 4; ++x) {
			int pixx = (5*cellx+x+1)*SquareSize/5;
			int pixy = (5*celly+y+1)*SquareSize/5;
			if (!piglit_probe_pixel_rgb(pixx, pixy, expected)) {
				fprintf(stderr, "%s: %i,%i failed\n", testname, cellx, celly);
				if (Automatic)
					piglit_report_result(PIGLIT_FAILURE);
			}
		}
	}
}

static float scale_for_miplevel(int bias, int level)
{
	float base = SquareSize/(4>>level);
	if (bias >= 0)
		return base/(float)(1<<bias);
	else
		return base*(float)(1<<(-bias));
}

static void test_simple_texture(int tex, int bias)
{
	int level;

	glBindTexture(GL_TEXTURE_2D, Textures[tex]);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glTexEnvf(GL_TEXTURE_FILTER_CONTROL_EXT, GL_TEXTURE_LOD_BIAS_EXT, bias);

	for(level = 0; level < 3; ++level) {
		float scale = scale_for_miplevel(bias, level);

		glPushMatrix();
		glScalef(SquareSize, SquareSize, 1.0);
		glTranslatef(level, tex, 0.0);
		glBegin(GL_QUADS);
			glTexCoord2f(0.0,   0.0);   glVertex2f(0, 0);
			glTexCoord2f(scale, 0.0);   glVertex2f(1, 0);
			glTexCoord2f(scale, scale); glVertex2f(1, 1);
			glTexCoord2f(0.0,   scale); glVertex2f(0, 1);
		glEnd();
		glPopMatrix();

		probe_cell("test_simple", level, tex, TextureData[tex][level]);
	}
}

/**
 * Simple test: Attempt to draw all LOD levels of both textures
 * at the given LOD bias.
 */
static void test_simple(int bias1, int bias2)
{
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	glEnable(GL_TEXTURE_2D);
	test_simple_texture(0, bias1);
	test_simple_texture(1, bias2);
	glDisable(GL_TEXTURE_2D);

	glutSwapBuffers();
}


static void test_multitex_combo(int bias1, int level1, int bias2, int level2)
{
	float scale1 = scale_for_miplevel(bias1, level1);
	float scale2 = scale_for_miplevel(bias2, level2);
	GLfloat expected[3];
	int i;

	for(i = 0; i < 3; ++i)
		expected[i] = TextureData[0][level1][i] + TextureData[1][level2][i];

	glPushMatrix();
	glScalef(SquareSize, SquareSize, 1.0);
	glTranslatef(level1, level2, 0.0);
	glBegin(GL_QUADS);
		glMultiTexCoord2f(GL_TEXTURE0, 0.0,    0.0);
		glMultiTexCoord2f(GL_TEXTURE1, 0.0,    0.0);
		glVertex2f(0, 0);

		glMultiTexCoord2f(GL_TEXTURE0, scale1, 0.0);
		glMultiTexCoord2f(GL_TEXTURE1, scale2, 0.0);
		glVertex2f(1, 0);

		glMultiTexCoord2f(GL_TEXTURE0, scale1, scale1);
		glMultiTexCoord2f(GL_TEXTURE1, scale2, scale2);
		glVertex2f(1, 1);

		glMultiTexCoord2f(GL_TEXTURE0, 0.0,    scale1);
		glMultiTexCoord2f(GL_TEXTURE1, 0.0,    scale2);
		glVertex2f(0, 1);
	glEnd();
	glPopMatrix();

	probe_cell("multitex", level1, level2, expected);
}


/**
 * Test combinations of LOD bias when multitexturing.
 */
static void test_multitex(int bias1, int bias2)
{
	int x, y;

	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, Textures[0]);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glTexEnvf(GL_TEXTURE_FILTER_CONTROL_EXT, GL_TEXTURE_LOD_BIAS_EXT, bias1);

	glActiveTexture(GL_TEXTURE1);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, Textures[1]);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_ADD);
	glTexEnvf(GL_TEXTURE_FILTER_CONTROL_EXT, GL_TEXTURE_LOD_BIAS_EXT, bias2);

	for(y = 0; y < 3; ++y) {
		for(x = 0; x < 3; ++x)
			test_multitex_combo(bias1, x, bias2, y);
	}

	glDisable(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE0);
	glDisable(GL_TEXTURE_2D);

	glutSwapBuffers();
}



static void Redisplay(void)
{
	if (Automatic) {
		int bias1, bias2;
		for(bias1 = -MaxTextureLodBias; bias1 <= MaxTextureLodBias; bias1++) {
			for(bias2 = -MaxTextureLodBias; bias2 <= MaxTextureLodBias; bias2++) {
				test_simple(bias1, bias2);
				test_multitex(bias1, bias2);
			}
		}

		piglit_report_result(PIGLIT_SUCCESS);
	} else {
		if (CurrentTest == 0) {
			test_simple(CurrentBias, CurrentBias2);
		} else {
			test_multitex(CurrentBias, CurrentBias2);
		}
	}
}


static void Reshape(int width, int height)
{
	glViewport(0, 0, Width, Height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0, Width, 0.0, Height, -1.0, 1.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}


static void Init(void)
{
	int i;

	piglit_require_extension("GL_EXT_texture_lod_bias");

	glGetIntegerv(GL_MAX_TEXTURE_LOD_BIAS_EXT, &MaxTextureLodBias);
	if (!Automatic)
		printf("MAX_TEXTURE_LOD_BIAS_EXT = %i\n", MaxTextureLodBias);

	glGenTextures(2, Textures);

	for(i = 0; i < 2; ++i) {
		int level;

		glBindTexture(GL_TEXTURE_2D, Textures[i]);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		for(level = 0; level < 3; ++level) {
			GLfloat texdata[16][3];
			int dim = 4 >> level;
			int j;

			for(j = 0; j < dim*dim; ++j) {
				texdata[j][0] = TextureData[i][level][0];
				texdata[j][1] = TextureData[i][level][1];
				texdata[j][2] = TextureData[i][level][2];
			}

			glTexImage2D(GL_TEXTURE_2D, level, GL_RGB, dim, dim, 0, GL_RGB, GL_FLOAT, texdata);
		}
	}

	glReadBuffer(GL_BACK);
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
		printf("Test: %s\n", CurrentTest ? "multitexturing" : "simple");
		break;
	case 'b':
		CurrentBias--;
		if (CurrentBias < -MaxTextureLodBias)
			CurrentBias = -MaxTextureLodBias;
		break;
	case 'B':
		CurrentBias++;
		if (CurrentBias > MaxTextureLodBias)
			CurrentBias = MaxTextureLodBias;
		break;
	case 'n':
		CurrentBias2--;
		if (CurrentBias2 < -MaxTextureLodBias)
			CurrentBias2 = -MaxTextureLodBias;
		break;
	case 'N':
		CurrentBias2++;
		if (CurrentBias2 > MaxTextureLodBias)
			CurrentBias2 = MaxTextureLodBias;
		break;
	case 27:
		exit(0);
		break;
	}
	printf("Current LOD bias: 1st tex: %i  2nd tex: %i\n", CurrentBias, CurrentBias2);
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
		printf(
			"Press 't' to switch tests\n"
			"Press 'b'/'B' to change primary LOD bias\n"
			"Press 'n'/'N' to change secondary LOD bias\n"
			"Press 'Escape' to quit\n");
		glutKeyboardFunc(Key);
	}
	Init();
	glutMainLoop();
	return 0;
}

