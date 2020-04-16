/*
 * Copyright © 2020 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

/**
 * @file unused-attributes.c
 * Tests that unused attributes in GL_ARB_vertex_program don't affect
 * attributes that are actually used.
 * See https://gitlab.freedesktop.org/mesa/mesa/issues/2758
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;
	config.khr_no_error_support = PIGLIT_NO_ERRORS;

PIGLIT_GL_TEST_CONFIG_END

static GLfloat pos[4][3] =
{
	{  1.0,  -1.0, 0.0 },
	{  1.0,   1.0, 0.0 },
	{  -1.0,  1.0, 0.0 },
	{  -1.0, -1.0, 0.0 }
};

static GLfloat norms[4][3] =
{
	{ 1, 0, 0},
	{ 1, 0, 0},
	{ 1, 0, 0},
	{ 1, 0, 0}
};

static GLfloat colors[4][4] =
{
	{ 1, 0, 0, 1},
	{ 1, 0, 0, 1},
	{ 1, 0, 0, 1},
	{ 1, 0, 0, 1}
};

static GLfloat texcoords[4][4] =
{
	{ 0, 1, 0, 1},
	{ 0, 1, 0, 1},
	{ 0, 1, 0, 1},
	{ 0, 1, 0, 1}
};

static enum piglit_result
test_conventional_attribs(void *data)
{
	static const char *vertProgramText =
		"!!ARBvp1.0 \n"
		"TEMP temp1, temp2;\n"
		"MOV temp1, vertex.normal;\n"
		"MOV temp2, vertex.color;\n"
		"MOV result.position, vertex.position;\n"
		"MOV result.color, vertex.texcoord;\n"
		"END";

	static const char *fragProgramText =
		"!!ARBfp1.0 \n"
		"MOV result.color, fragment.color;\n"
		"END";

	static const GLfloat expected[4] = {0.0, 1.0, 0.0, 1.0};
	bool pass = true;
	GLuint vertProg, fragProg;

	glEnable(GL_FRAGMENT_PROGRAM_ARB);
	glEnable(GL_VERTEX_PROGRAM_ARB);

	fragProg = piglit_compile_program(GL_FRAGMENT_PROGRAM_ARB, fragProgramText);
	glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, fragProg);

	vertProg = piglit_compile_program(GL_VERTEX_PROGRAM_ARB, vertProgramText);
	glBindProgramARB(GL_VERTEX_PROGRAM_ARB, vertProg);

	glVertexPointer(3, GL_FLOAT, 0, pos);
	glNormalPointer(GL_FLOAT, 0, norms);
	glColorPointer(4, GL_FLOAT, 0, colors);
	glTexCoordPointer(4, GL_FLOAT, 0, texcoords);

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	glClearColor(0.0, 0.0, 0.0, 1);
	glClear(GL_COLOR_BUFFER_BIT);

	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	pass = piglit_probe_pixel_rgba(piglit_width - 1, piglit_height - 1, expected);
	piglit_present_results();

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

	glDeleteProgramsARB(1, &vertProg);
	glDeleteProgramsARB(1, &fragProg);

	glDisable(GL_FRAGMENT_PROGRAM_ARB);
	glDisable(GL_VERTEX_PROGRAM_ARB);

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

static enum piglit_result
test_generic_attribs(void *data)
{
	static const char *vertProgramText =
		"!!ARBvp1.0 \n"
		"TEMP temp1, temp2;\n"
		"MOV temp1, vertex.attrib[1];\n"
		"MOV temp2, vertex.attrib[2];\n"
		"MOV result.position, vertex.attrib[0];\n"
		"MOV result.color, vertex.attrib[7];\n"
		"END";

	static const char *fragProgramText =
		"!!ARBfp1.0 \n"
		"MOV result.color, fragment.color;\n"
		"END";

	static const GLfloat expected[4] = {0.0, 1.0, 0.0, 1.0};
	bool pass = true;
	GLuint vertProg, fragProg;

	glEnable(GL_FRAGMENT_PROGRAM_ARB);
	glEnable(GL_VERTEX_PROGRAM_ARB);

	fragProg = piglit_compile_program(GL_FRAGMENT_PROGRAM_ARB, fragProgramText);
	glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, fragProg);

	vertProg = piglit_compile_program(GL_VERTEX_PROGRAM_ARB, vertProgramText);
	glBindProgramARB(GL_VERTEX_PROGRAM_ARB, vertProg);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,
					3 * sizeof(GLfloat), pos);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,
					3 * sizeof(GLfloat), norms);
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE,
					4 * sizeof(GLfloat), colors);
	glVertexAttribPointer(7, 4, GL_FLOAT, GL_FALSE,
					4 * sizeof(GLfloat), texcoords);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glEnableVertexAttribArray(7);

	glClearColor(0.0, 0.0, 0.0, 1);
	glClear(GL_COLOR_BUFFER_BIT);

	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	pass = piglit_probe_pixel_rgba(piglit_width - 1, piglit_height - 1, expected);
	piglit_present_results();

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);
	glDisableVertexAttribArray(7);

	glDeleteProgramsARB(1, &vertProg);
	glDeleteProgramsARB(1, &fragProg);

	glDisable(GL_FRAGMENT_PROGRAM_ARB);
	glDisable(GL_VERTEX_PROGRAM_ARB);

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

enum piglit_result
piglit_display(void)
{
	return PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	piglit_require_extension("GL_ARB_vertex_program");
	piglit_require_extension("GL_ARB_fragment_program");

	struct piglit_subtest tests[] = {
		{
			"Unused conventional attributes",
			"conventional-attribs",
			&test_conventional_attribs,
			NULL
		},
		{
			"Unused generic attributes",
			"generic-attribs",
			&test_generic_attribs,
			NULL
		},
		{
			NULL
		}
	};

	enum piglit_result result =
		piglit_run_selected_subtests(tests, NULL, 0, PIGLIT_PASS);

	piglit_report_result(result);
}
