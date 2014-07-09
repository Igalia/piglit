/*
 * Copyright © 2010 Intel Corporation
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
 *
 * Authors:
 *    Eric Anholt <eric@anholt.net>
 *
 */

/** @file glsl-max-varyings.c
 *
 * Tests whether each varying can be used at all numbers of varyings up to
 * GL_MAX_VARYING_FLOATS / 4.
 */

#include "piglit-util-gl.h"

#define MAX_VARYING 256

/* 2x2 rectangles with 2 pixels of pad.  Deal with up to 256 varyings. */

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_width = (2+MAX_VARYING*4);
	config.window_height = (2+MAX_VARYING*4);
	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

static bool exceed_limits = false;
static int max_varyings;

/* Generate a VS that writes to num_varyings vec4s, and put
 * interesting data in data_varying with 0.0 everywhere else.
 */
static GLint get_vs(int num_varyings, int data_varying)
{
	GLuint shader;
	unsigned i;
	char *code = malloc(4096);
	char temp[2048];

	code[0] = 0;
	for (i = 0; i < num_varyings; i++) {
		sprintf(temp, "varying vec4 v%d;\n", i);
		strcat(code, temp);
	}

	sprintf(temp,
		"attribute vec4 vertex;\n"
		"attribute vec4 green;\n"
		"attribute vec4 red;\n"
		"void main()\n"
		"{\n"
		"	gl_Position = (gl_ModelViewProjectionMatrix * \n"
		"			vertex);\n"
		);
	strcat(code, temp);

	for (i = 0; i < num_varyings; i++) {
		if (i == data_varying)
			sprintf(temp, "	v%d = green;\n", i);
		else
			sprintf(temp, "	v%d = red;\n", i);
		strcat(code, temp);
	}

	sprintf(temp,
		"}\n"
		);
	strcat(code, temp);

	shader = piglit_compile_shader_text(GL_VERTEX_SHADER, code);
	/* printf("%s\n", code); */

	free(code);
	return shader;
}

/* Generate a FS that does operations on num_varyings, yet makes only
 * data_varying contribute to the output.
 *
 * We could make a single FS per num_varyings that did this by using a
 * uniform for data_varying and some multiplication by comparisons
 * (see glsl-routing for an example), but since we're linking a new
 * shader each time anyway, this produces a simpler FS to read and
 * verify.
 */
static GLint get_fs(int num_varyings, int data_varying)
{
	GLuint shader;
	unsigned i;
	char *code = malloc(8192);
	char temp[2048];

	code[0] = 0;
	for (i = 0; i < num_varyings; i++) {
		sprintf(temp, "varying vec4 v%d;\n", i);
		strcat(code, temp);
	}

	sprintf(temp,
		"uniform float zero;\n"
		"uniform float one;\n"
		"void main()\n"
		"{\n"
		"	vec4 val = vec4(0.0);\n"
		);
	strcat(code, temp);

	for (i = 0; i < num_varyings; i++) {
		if (i == data_varying)
			sprintf(temp, "	val += one * v%d;\n", i);
		else
			sprintf(temp, "	val += zero * v%d;\n", i);
		strcat(code, temp);
	}

	sprintf(temp,
		"	gl_FragColor = val;\n"
		"}\n"
		);
	strcat(code, temp);

	/* printf("%s\n", code); */
	shader = piglit_compile_shader_text(GL_FRAGMENT_SHADER, code);

	free(code);
	return shader;
}

static int
coord_from_index(int index)
{
	return 2 + 4 * index;
}

static bool
draw(int num_varyings)
{
	int data_varying;
	float vertex[4][4] = { {0.0, 0.0, 0.0, 1.0},
			       {0.0, 0.0, 0.0, 1.0},
			       {0.0, 0.0, 0.0, 1.0},
			       {0.0, 0.0, 0.0, 1.0} };
	float green[4][4] = { {0.0, 1.0, 0.0, 0.0},
			      {0.0, 1.0, 0.0, 0.0},
			      {0.0, 1.0, 0.0, 0.0},
			      {0.0, 1.0, 0.0, 0.0} };
	float red[4][4] = { {1.0, 0.0, 0.0, 0.0},
			    {1.0, 0.0, 0.0, 0.0},
			    {1.0, 0.0, 0.0, 0.0},
			    {1.0, 0.0, 0.0, 0.0} };

	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
			      vertex);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
			      green);
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
			      red);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);

	for (data_varying = 0; data_varying < num_varyings; data_varying++) {
		GLuint prog, vs, fs;
		GLint loc;
		float x, y;

		vs = get_vs(num_varyings, data_varying);
		fs = get_fs(num_varyings, data_varying);

		prog = glCreateProgram();
		glAttachShader(prog, vs);
		glAttachShader(prog, fs);

		glBindAttribLocation(prog, 0, "vertex");
		glBindAttribLocation(prog, 1, "green");
		glBindAttribLocation(prog, 2, "red");

		glLinkProgram(prog);
		if (!piglit_link_check_status_quiet(prog)) {
			if (num_varyings > max_varyings) {
				printf("Failed to link with %d out of %d "
				       "varyings used\n",
				       num_varyings, max_varyings);
				return false;
			} else {
				piglit_report_result(PIGLIT_FAIL);
			}
		}

		glUseProgram(prog);

		loc = glGetUniformLocation(prog, "zero");
		if (loc != -1) /* not used for num_varyings == 1 */
			glUniform1f(loc, 0.0);

		loc = glGetUniformLocation(prog, "one");
		assert(loc != -1); /* should always be used */
		glUniform1f(loc, 1.0);

		x = coord_from_index(data_varying);
		y = coord_from_index(num_varyings - 1);
		vertex[0][0] = x;
		vertex[0][1] = y;
		vertex[1][0] = x + 2;
		vertex[1][1] = y;
		vertex[2][0] = x;
		vertex[2][1] = y + 2;
		vertex[3][0] = x + 2;
		vertex[3][1] = y + 2;
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

		glDeleteShader(vs);
		glDeleteShader(fs);
		glDeleteProgram(prog);
	}

	return true;
}

enum piglit_result
piglit_display(void)
{
	GLint max_components;
	int test_varyings, row, col;
	GLboolean pass = GL_TRUE, warned = GL_FALSE;
	bool drew[MAX_VARYING];

	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);

	glGetIntegerv(GL_MAX_VARYING_FLOATS, &max_components);
	max_varyings = max_components / 4;

	printf("GL_MAX_VARYING_FLOATS = %i\n", max_components);

	test_varyings = max_varyings;
	if (exceed_limits)
		test_varyings++;
	if (test_varyings > MAX_VARYING) {
		printf("test not designed to handle >%d varying vec4s.\n"
		       "(implementation reports %d components)\n",
		       MAX_VARYING, max_varyings);
		test_varyings = MAX_VARYING;
		warned = GL_TRUE;
	}

	glClearColor(0.5, 0.5, 0.5, 0.5);
	glClear(GL_COLOR_BUFFER_BIT);

	for (row = 0; row < test_varyings; row++) {
		drew[row] = draw(row + 1);
	}

	for (row = 0; row < test_varyings; row++) {
		if (!drew[row])
			continue;

		for (col = 0; col <= row; col++) {
			GLboolean ok;
			float green[3] = {0.0, 1.0, 0.0};

			ok = piglit_probe_rect_rgb(coord_from_index(col),
						   coord_from_index(row),
						   2, 2,
						   green);
			if (!ok) {
				printf("  Failure with %d vec4 varyings used "
				       "in varying index %d\n",
				       row + 1, col);
				pass = GL_FALSE;
				break;
			}
		}
	}

	piglit_present_results();

	if (!pass)
		return PIGLIT_FAIL;
	if (warned)
		return PIGLIT_WARN;
	else
		return PIGLIT_PASS;
}

void piglit_init(int argc, char **argv)
{
	int i;

	piglit_require_gl_version(20);

	for (i = 0; i < argc; i++) {
		if (strcmp(argv[i], "--exceed-limits") == 0)
			exceed_limits = true;
	}

	printf("Vertical axis: Increasing numbers of varyings.\n");
	printf("Horizontal axis: Which of the varyings contains the color.\n");
}

