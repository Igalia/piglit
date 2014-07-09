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

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_width = 200;
	config.window_height = 200;
	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_DEPTH;

PIGLIT_GL_TEST_CONFIG_END

extern float piglit_tolerance[4];

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
	glClearColor(0.3, 0.3, 0.3, 0.3);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_FRAGMENT_PROGRAM_ARB);

	glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, FragProg[0]);

	glBegin(GL_QUADS);
		glVertex3f(0, 0, 0);
		glVertex3f(1, 0, 1);
		glVertex3f(1, 1, 2);
		glVertex3f(0, 1, 1);
	glEnd();

	glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, FragProg[1]);

	glBegin(GL_QUADS);
		glVertex2f(0, 1);
		glVertex2f(1, 1);
		glVertex2f(1, 2);
		glVertex2f(0, 2);
	glEnd();

	glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, FragProg[2]);

	glBegin(GL_QUADS);
		glVertex2f(1, 0);
		glVertex2f(2, 0);
		glVertex2f(2, 1);
		glVertex2f(1, 1);
	glEnd();

	glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, FragProg[3]);

	glBegin(GL_QUADS);
		glVertex2f(1, 1);
		glVertex2f(2, 1);
		glVertex2f(2, 2);
		glVertex2f(1, 2);
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
};

static bool
DoTest(void)
{
	int i;
	bool pass = true;

	for (i = 0; i < ARRAY_SIZE(Probes); i++) {
		printf("Testing: %s\n", Probes[i].name);
		pass = piglit_probe_pixel_rgba(Probes[i].x * piglit_width / 2,
					       Probes[i].y * piglit_height / 2,
					       Probes[i].expected) && pass;
	}

	return pass;
}


enum piglit_result
piglit_display(void)
{
	int pass;

	DoFrame();
	pass = DoTest();

	if (!piglit_automatic)
		piglit_present_results();
	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
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
	GLubyte rectangle[200][200][4];
	GLubyte tex[256*256][4];

	/* Need GL 1.4 for GL_GENERATE_MIPMAP tex param */
	piglit_require_gl_version(14);

	printf("GL_RENDERER = %s\n", (char *) glGetString(GL_RENDERER));

	piglit_require_fragment_program();

	piglit_tolerance[0] = 0.02;
	piglit_tolerance[1] = 0.02;
	piglit_tolerance[2] = 0.02;
	piglit_tolerance[3] = 0.02;

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
	glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 256, 256, 0,
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

	Reshape(piglit_width, piglit_height);
}
