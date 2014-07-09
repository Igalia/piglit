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

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_width = 200;
	config.window_height = 200;
	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_DEPTH;

PIGLIT_GL_TEST_CONFIG_END

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

	glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, FragProg[0]);

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

	glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, FragProg[1]);

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

	// Program 1
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

static bool DoTest( void )
{
	int idx = 0;
	bool pass = true;

	while(Probes[idx].name) {
		pass = piglit_probe_pixel_rgba(
			(int)(Probes[idx].x*piglit_width/2),
			(int)(Probes[idx].y*piglit_height/2),
			Probes[idx].expected) && pass;
		idx++;
	}

	return pass;
}

enum piglit_result
piglit_display(void)
{
	bool pass;

	piglit_gen_ortho_projection(0.0, 2.0, 0.0, 2.0, -2.0, 6.0, GL_FALSE);

	DoFrame();
	pass = DoTest();
	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	int i, x, y;
	GLuint texname;
	GLubyte tex[4][4][4];

	piglit_require_gl_version(13);

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

	glGenTextures(1, &texname);
	glActiveTexture(GL_TEXTURE0);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texname);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 4, 4, 0,
	             GL_RGBA, GL_UNSIGNED_BYTE, tex);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}
