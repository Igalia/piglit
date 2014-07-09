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
 *
 * Authors:
 *    Pierre-Eric Pelloux-Prayer <pelloux@gmail.com>
 */

/**
 * Test whether LIT behaves correctly with src == dst
 * (Heavily based on fp-lit-mask.c)
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_width = 200;
	config.window_height = 200;
	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_DEPTH;

PIGLIT_GL_TEST_CONFIG_END

static GLuint FragProg[15];

static const char fragProgramTemplate[] =
	"!!ARBfp1.0\n"
	"PARAM values = { 0.65, 0.9, 0.0, 8.0 }; \n"
	"PARAM bogus = { 0.8, 0.8, 0.8, 0.8 }; \n"
	"TEMP _values; \n"
	"MOV _values, values; \n"
	"MOV result.color, bogus; \n"
	"LIT _values, _values; \n"
	"MOV result.color.%s, _values; \n"
	"END\n";
static float LitExpected[4] = { 1.0, 0.65, 0.433, 1.0 };

static void DoFrame(void)
{
	int mask;

	glClearColor(0.0, 0.0, 0.0, 0.0);
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	glEnable(GL_FRAGMENT_PROGRAM_ARB);

	for(mask = 1; mask < 16; ++mask) {
		glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, FragProg[mask-1]);
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
}

static bool
DoTest(void)
{
	int mask;
 	bool pass = true;

	for(mask = 1; mask < 16; ++mask) {
		float expected[4];
		int i;

		for(i = 0; i < 4; ++i) {
			if (mask & (1 << i))
				expected[i] = LitExpected[i];
			else
				expected[i] = 0.8;
		}

		pass = piglit_probe_pixel_rgba(piglit_width * (2*(mask%4)+1)/8,
					       piglit_height * (2*(mask/4)+1)/8,
					       expected) && pass;
	}

	return pass;
}

enum piglit_result
piglit_display(void)
{
	int pass;

	DoFrame();
	pass = DoTest();

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void piglit_init(int argc, char **argv)
{
	int mask;

	printf("GL_RENDERER = %s\n", (char *) glGetString(GL_RENDERER));

	piglit_require_fragment_program();

	/*
	 * Fragment programs
	 */
	for(mask = 1; mask < 16; ++mask) {
		char programText[1024];
		char maskstring[5];

		maskstring[0] = 0;
		if (mask & 1) strcat(maskstring, "x");
		if (mask & 2) strcat(maskstring, "y");
		if (mask & 4) strcat(maskstring, "z");
		if (mask & 8) strcat(maskstring, "w");
		sprintf(programText, fragProgramTemplate, maskstring);

		FragProg[mask-1] = piglit_compile_program(GL_FRAGMENT_PROGRAM_ARB, programText);
	}

	piglit_ortho_projection(4.0, 4.0, GL_FALSE);
}
