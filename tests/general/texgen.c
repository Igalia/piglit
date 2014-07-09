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

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

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
	if (!piglit_probe_pixel_rgb((2*x+1)*piglit_width/8, (2*y+1)*piglit_height/8, expected)) {
		fprintf(stderr, "%s: %i,%i failed\n", testname, x, y);
		if (piglit_automatic)
			piglit_report_result(PIGLIT_FAIL);
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

	for(y = 0; y < 4; ++y) {
		for(x = 0; x < 4; ++x)
			probe_cell("test_sanity", x, y, TextureData[y][x]);
	}

	piglit_present_results();
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

	for(y = 0; y < 2; ++y) {
		for(x = 0; x < 2; ++x)
			probe_cell(testname, x, y, TextureData[y+1][x+1]);
	}

	piglit_present_results();
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

enum piglit_result
piglit_display(void)
{
	if (piglit_automatic) {
		int i;
		for(i = 0; i < NrTests; ++i)
			Tests[i].function();
                return PIGLIT_PASS;
	} else {
		Tests[CurrentTest].function();
                return PIGLIT_PASS;
	}
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
	piglit_post_redisplay();
}

void piglit_init(int argc, char *argv[])
{
	int x, y;

	if (!piglit_automatic) {
		printf("Press 't' to switch tests; Escape to quit\n");
		piglit_set_keyboard_func(Key);
	}

	if (piglit_use_fragment_program()) {
		UseFragmentProgram = 1;
		glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB,
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

	piglit_ortho_projection(1.0, 1.0, GL_FALSE);
}

