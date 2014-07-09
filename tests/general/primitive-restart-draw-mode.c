/*
 * Copyright © 2013 Intel Corporation
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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

/**
 * \file primitive-restart-draw-mode.c
 *
 * Test proper functioning of primitive restart in all draw modes.  In
 * particular, verify that dangling vertices are properly discarded.
 *
 * The test operates as follows:
 *
 * - Choose a pattern of 8 vertices which will allow easy visual
 *   inspection of the rendered image.  For some primitive types, we
 *   arrange the 8 vertices in an octagon.  For others, we arrange
 *   them in two rows, with vertices alternating between the two rows.
 *
 * - Construct an index buffer consisting of the values 0 through 7,
 *   interrupted at some location by the primitive restart index, and
 *   draw using the resulting index buffer using glDrawElements().
 *   Seven images are drawn in a vertical array, one for each possible
 *   place where the primitive restart index might interrupt the
 *   indices.
 *
 * - To the right of each of the above images, use a pair of calls to
 *   glDrawArrays() to simulate the expected behaviour of primitive
 *   restart.
 *
 * - Compare the left and right halves of the resulting window to make
 *   sure they match.
 *
 * Note: for easier visual inspection of the result, the image under
 * test is drawn in blue, and the vertices are drawn in white using
 * GL_POINTS.
 */

#include "piglit-util-gl.h"


#define NUM_VERTICES 8
#define NUM_ROWS (NUM_VERTICES - 1)
#define NUM_COLS 2
#define PATTERN_SIZE 75

PIGLIT_GL_TEST_CONFIG_BEGIN
	config.supports_gl_compat_version = 10;
	config.window_width = PATTERN_SIZE * NUM_COLS;
	config.window_height = PATTERN_SIZE * NUM_ROWS;
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;
PIGLIT_GL_TEST_CONFIG_END


enum vertex_pattern
{
	VERTEX_PATTERN_OCTAGON,
	VERTEX_PATTERN_2_ROWS,
	VERTEX_PATTERN_COUNT,
};


static const struct xy_coords
{
	GLfloat x;
	GLfloat y;
} vertex_patterns[VERTEX_PATTERN_COUNT][NUM_VERTICES] = {
	/* VERTEX_PATTERN_OCTAGON */
	{
		{ 27, 69 },
		{ 48, 69 },
		{ 69, 48 },
		{ 69, 27 },
		{ 48,  6 },
		{ 27,  6 },
		{  6, 27 },
		{  6, 48 },
	},
	/* VERTEX_PATTERN_2_ROWS */
	{
		{ 20, 63 },
		{ 55, 63 },
		{ 20, 46 },
		{ 55, 46 },
		{ 20, 29 },
		{ 55, 29 },
		{ 20, 12 },
		{ 55, 12 },
	},
};


static const struct test_desc
{
	const char *name;
	GLenum prim_type;
	enum vertex_pattern pattern;
} tests[] = {
	{ "points",         GL_POINTS,         VERTEX_PATTERN_OCTAGON },
	{ "lines",          GL_LINES,          VERTEX_PATTERN_OCTAGON },
	{ "line_loop",      GL_LINE_LOOP,      VERTEX_PATTERN_OCTAGON },
	{ "line_strip",     GL_LINE_STRIP,     VERTEX_PATTERN_OCTAGON },
	{ "triangles",      GL_TRIANGLES,      VERTEX_PATTERN_2_ROWS  },
	{ "triangle_strip", GL_TRIANGLE_STRIP, VERTEX_PATTERN_2_ROWS  },
	{ "triangle_fan",   GL_TRIANGLE_FAN,   VERTEX_PATTERN_OCTAGON },
	{ "quads",          GL_QUADS,          VERTEX_PATTERN_OCTAGON },
	{ "quad_strip",     GL_QUAD_STRIP,     VERTEX_PATTERN_2_ROWS  },
	{ "polygon",        GL_POLYGON,        VERTEX_PATTERN_OCTAGON },
};

static const struct test_desc *test = NULL;


static const char vs_text[] =
	"#version 110\n"
	"attribute vec2 vertex;\n"
	"uniform vec2 offset;\n"
	"uniform vec2 window_size;\n"
	"uniform vec4 color;\n"
	"void main()\n"
	"{\n"
	"  gl_Position = vec4((vertex + offset) / window_size * 2.0 - 1.0,\n"
	"                     0.0, 1.0);\n"
	"  gl_FrontColor = color;\n"
	"}\n";


static GLuint prog;
static GLint vertex_attr;
static GLint window_size_loc;
static GLint offset_loc;
static GLint color_loc;


static void
print_usage_and_exit(const char *prog_name)
{
	int i;
	printf("Usage: %s <subtest>\n"
	       "  where <subtest> is one of the following:\n", prog_name);
	for (i = 0; i < ARRAY_SIZE(tests); i++)
		printf("    %s\n", tests[i].name);
	piglit_report_result(PIGLIT_FAIL);
}


void
piglit_init(int argc, char **argv)
{
	int i;
	GLuint vs;

	/* Parse params */
	if (argc != 2)
		print_usage_and_exit(argv[0]);
	for (i = 0; i < ARRAY_SIZE(tests); i++) {
		if (strcmp(argv[1], tests[i].name) == 0) {
			test = &tests[i];
			break;
		}
	}
	if (test == NULL)
		print_usage_and_exit(argv[0]);

	piglit_require_GLSL_version(110);
	if (!piglit_is_extension_supported("GL_NV_primitive_restart") &&
	    piglit_get_gl_version() < 31) {
		printf("GL_NV_primitive_restart or GL 3.1 required\n");
		piglit_report_result(PIGLIT_SKIP);
	}

	vs = piglit_compile_shader_text(GL_VERTEX_SHADER, vs_text);
	prog = piglit_link_simple_program(vs, 0);
	if (!prog)
		piglit_report_result(PIGLIT_FAIL);
	glDeleteShader(vs);
	vertex_attr = glGetAttribLocation(prog, "vertex");
	window_size_loc = glGetUniformLocation(prog, "window_size");
	offset_loc = glGetUniformLocation(prog, "offset");
	color_loc = glGetUniformLocation(prog, "color");
}


static void
draw_pattern(int restart_pos, bool use_primitive_restart)
{
	/* Draw test pattern in blue */
	glUniform4f(color_loc, 0.25, 0.25, 1.0, 1.0);
	if (use_primitive_restart) {
		GLubyte index_buffer[NUM_VERTICES + 1];
		int i;

		for (i = 0; i < restart_pos; i++) {
			index_buffer[i] = i;
		}
		index_buffer[restart_pos] = 0xff;
		for (i = restart_pos + 1; i < ARRAY_SIZE(index_buffer); i++) {
			index_buffer[i] = i - 1;
		}
		if (piglit_get_gl_version() >= 31) {
			glEnable(GL_PRIMITIVE_RESTART);
			glPrimitiveRestartIndex(0xff);
		} else {
			glEnableClientState(GL_PRIMITIVE_RESTART_NV);
			glPrimitiveRestartIndexNV(0xff);
		}
		glDrawElements(test->prim_type, ARRAY_SIZE(index_buffer),
			       GL_UNSIGNED_BYTE, index_buffer);
		if (piglit_get_gl_version() >= 31)
			glDisable(GL_PRIMITIVE_RESTART);
		else
			glDisableClientState(GL_PRIMITIVE_RESTART_NV);
	} else {
		glDrawArrays(test->prim_type, 0, restart_pos);
		glDrawArrays(test->prim_type, restart_pos,
			     NUM_VERTICES - restart_pos);
	}

	if (test->prim_type != GL_POINTS) {
		/* Draw vertices in white */
		glUniform4f(color_loc, 1.0, 1.0, 1.0, 1.0);
		glDrawArrays(GL_POINTS, 0, NUM_VERTICES);
	}
}


enum piglit_result
piglit_display(void)
{
	int row, col;
	bool pass = true;

	glClear(GL_COLOR_BUFFER_BIT);
	glUseProgram(prog);
	glUniform2f(window_size_loc, piglit_width/2, piglit_height);
	glVertexAttribPointer(vertex_attr, 2, GL_FLOAT, GL_FALSE,
			      sizeof(vertex_patterns[test->pattern][0]),
			      vertex_patterns[test->pattern]);
	glEnableVertexAttribArray(vertex_attr);

	for (col = 0; col < NUM_COLS; col++) {
		if (col == 0)
			glViewport(0, 0, piglit_width/2, piglit_height);
		else
			glViewport(piglit_width/2, 0, piglit_width/2, piglit_height);

		for (row = 0; row < NUM_ROWS; row++) {
			glUniform2f(offset_loc, 0,
				    (NUM_ROWS - 1 - row) * PATTERN_SIZE);
			draw_pattern(row + 1, col == 0);
		}
	}

	glViewport(0, 0, piglit_width, piglit_height);
	pass = piglit_probe_rect_halves_equal_rgba(0, 0, piglit_width,
						   piglit_height) && pass;
	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;
	piglit_present_results();
	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}
