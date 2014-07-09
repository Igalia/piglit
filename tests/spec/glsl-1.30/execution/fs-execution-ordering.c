/*
 * Copyright © 2012 Intel Corporation
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
 * fs-execution-ordering.c
 *
 * Confirm that fragment shader outputs are written to the color
 * buffer in the correct order, even if some fragments take
 * dramatically longer to execute than others.
 *
 * Since this test is looking for race conditions, it repeats 100
 * times, drawing different primitives sizes, to increase the chances
 * of a race condition occurring.
 */

#include "piglit-util-gl.h"

#define SHIFT_COUNT 64

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;

PIGLIT_GL_TEST_CONFIG_END

#define SMALL_COMPOSITE 4
#define LARGE_PRIME 7919

GLint prog;
GLuint vbo_handle;

static const char *vstext =
	"#version 130\n"
	"in uint num;\n"
	"in vec4 pos;\n"
	"flat out uint number_to_classify;\n"
	"\n"
	"void main()\n"
	"{\n"
	"  gl_Position = pos;\n"
	"  number_to_classify = num;\n"
	"}\n";

/* This fragment shader implements a simple primality test using trial
 * division.  It outputs a color of red if its input is prime, and
 * green if its input is composite.
 *
 * Note: no special effort has been made to use a very fast algorithm,
 * since the purpose of the shader is to have dramatically different
 * execution times based on the input parameter.
 */
static const char *fstext =
	"#version 130\n"
	"flat in uint number_to_classify;\n"
	"\n"
	"void main()\n"
	"{\n"
	"  bool factor_found = false;\n"
	"  for (uint i = 2u; i < number_to_classify; ++i) {\n"
	"    if (number_to_classify % i == 0u)\n"
	"      factor_found = true;\n"
	"  }\n"
	"  gl_FragColor = factor_found ?\n"
	"    vec4(0.0, 1.0, 0.0, 1.0) :\n"
	"    vec4(1.0, 0.0, 0.0, 1.0);\n"
	"}\n";

void
piglit_init(int argc, char **argv)
{
	piglit_require_GLSL_version(130);

	prog = piglit_build_simple_program(vstext, fstext);
	glGenBuffers(1, &vbo_handle);

	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);
}

bool
do_test(unsigned size)
{
	float expected_color[4] = { 0.0, 1.0, 0.0, 1.0 };

	/* Set up the necessary vertex array to draw two squares, each
	 * size x size pixels.  The first square is drawn using a
	 * large prime number for the "num" parameter, so it will be
	 * drawn in red and take a long time to compute.  The second
	 * square is drawn using a small composite number for the
	 * "num" parameter, so it will be drawn in green and take a
	 * short time to compute.
	 *
	 * Even though the second square takes a short time to
	 * compute, it should be written to the color buffer after the
	 * first square, so the result should be a green square.
	 */
	GLfloat xmin = -1.0;
	GLfloat ymin = -1.0;
	GLfloat xmax = 2.0 * size / piglit_width - 1.0;
	GLfloat ymax = 2.0 * size / piglit_height - 1.0;
	struct vertex_attributes {
		GLfloat pos[2];
		GLuint num;
	} vertex_data[] = {
		{ { xmin, ymin }, LARGE_PRIME },
		{ { xmin, ymax }, LARGE_PRIME },
		{ { xmax, ymax }, LARGE_PRIME },
		{ { xmin, ymin }, LARGE_PRIME },
		{ { xmax, ymax }, LARGE_PRIME },
		{ { xmax, ymin }, LARGE_PRIME },
		{ { xmin, ymin }, SMALL_COMPOSITE },
		{ { xmax, ymax }, SMALL_COMPOSITE },
		{ { xmax, ymin }, SMALL_COMPOSITE },
		{ { xmin, ymin }, SMALL_COMPOSITE },
		{ { xmin, ymax }, SMALL_COMPOSITE },
		{ { xmax, ymax }, SMALL_COMPOSITE },
	};

	size_t stride = sizeof(vertex_data[0]);
	GLint pos_index = glGetAttribLocation(prog, "pos");
	GLint num_index = glGetAttribLocation(prog, "num");
	glBindBuffer(GL_ARRAY_BUFFER, vbo_handle);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_data), &vertex_data,
		     GL_STATIC_DRAW);
	glVertexAttribPointer(pos_index, 2, GL_FLOAT, GL_FALSE, stride,
			      (void *) offsetof(struct vertex_attributes,
						pos));
	glVertexAttribIPointer(num_index, 1, GL_UNSIGNED_INT, stride,
			       (void *) offsetof(struct vertex_attributes,
						 num));
	glEnableVertexAttribArray(pos_index);
	glEnableVertexAttribArray(num_index);

	/* Draw the squares and check their color. */
	glDrawArrays(GL_TRIANGLES, 0, ARRAY_SIZE(vertex_data));
	return piglit_probe_rect_rgba(0, 0, size, size, expected_color);
}

enum piglit_result
piglit_display(void)
{
	enum piglit_result result = PIGLIT_PASS;
	unsigned size;

	glUseProgram(prog);
	glClear(GL_COLOR_BUFFER_BIT);

	for (size = 1; size <= 100; ++size) {
		if (!do_test(size)) {
			printf("Failed at rect size %dx%d\n", size, size);
			result = PIGLIT_FAIL;
			break;
		}
	}

	piglit_present_results();

	return result;
}
