/*
 * Copyright © 2011 Intel Corporation
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
 * \file intervening-read.c
 *
 * Verify that transform feedback continues to work correctly if
 * glReadPixels is executed while it is in progress.
 *
 * This test accepts a single command-line argument which determines
 * what aspect of transform feedback is tested:
 *
 * - output: Verifies that correct transform feedback output is generated.
 *
 * - prims_generated: Verifies that the PRIMITIVES_GENERATED counter
 *   is updated correctly.
 *
 * - prims_written: Verifies that the PRIMITIVES_WRITTEN counter is
 *   updated correctly.
 *
 * The test draws two triangles before executing glReadPixels, and two
 * triangles after executing glReadPixels.  It uses a transform
 * feedback buffer that is large enough to accommodate 12 vertices,
 * but it requests that no more than 9 vertices be written to it.
 * This allows us to verify that the intervening glReadPixels call
 * doesn't interfere with overflow checking.
 */

#include "piglit-util-gl-common.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_width = 64;
	config.window_height = 32;
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGB;

PIGLIT_GL_TEST_CONFIG_END

static enum test_mode {
	TEST_MODE_OUTPUT,
	TEST_MODE_PRIMS_GENERATED,
	TEST_MODE_PRIMS_WRITTEN
} test_mode;

static const char *vstext =
	"attribute vec4 in_position;\n"
	"attribute vec4 in_color;\n"
	"varying vec4 out_position;\n"
	"varying vec4 out_color;\n"
	"\n"
	"void main()\n"
	"{\n"
	"  gl_Position = in_position;\n"
	"  out_position = in_position;\n"
	"  out_color = in_color;\n"
	"}\n";

static const char *fstext =
	"varying vec4 out_color;\n"
	"\n"
	"void main()\n"
	"{\n"
	"  gl_FragColor = out_color;\n"
	"}\n";

static const char *varyings[] = { "out_position", "out_color" };

static GLuint xfb_buf;
static GLuint prog;
static GLuint query;

static void
print_usage_and_exit(char *prog_name)
{
	printf("Usage: %s <mode>\n"
	       "  where <mode> is one of:\n"
	       "    output\n"
	       "    prims_generated\n"
	       "    prims_written\n", prog_name);
	exit(1);
}

void
piglit_init(int argc, char **argv)
{
	GLuint vs, fs;

	/* Interpret command line args */
	if (argc != 2)
		print_usage_and_exit(argv[0]);
	if (strcmp(argv[1], "output") == 0)
		test_mode = TEST_MODE_OUTPUT;
	else if (strcmp(argv[1], "prims_generated") == 0)
		test_mode = TEST_MODE_PRIMS_GENERATED;
	else if (strcmp(argv[1], "prims_written") == 0)
		test_mode = TEST_MODE_PRIMS_WRITTEN;
	else
		print_usage_and_exit(argv[0]);

	piglit_require_GLSL();
	piglit_require_transform_feedback();

	vs = piglit_compile_shader_text(GL_VERTEX_SHADER, vstext);
	fs = piglit_compile_shader_text(GL_FRAGMENT_SHADER, fstext);
	prog = piglit_CreateProgram();
	piglit_AttachShader(prog, vs);
	piglit_AttachShader(prog, fs);
	piglit_BindAttribLocation(prog, 0, "in_position");
	piglit_BindAttribLocation(prog, 1, "in_color");
	glTransformFeedbackVaryings(prog, 2, varyings, GL_INTERLEAVED_ATTRIBS);
	piglit_LinkProgram(prog);
	if (!piglit_link_check_status(prog)) {
		piglit_DeleteProgram(prog);
		piglit_report_result(PIGLIT_FAIL);
	}

	glGenBuffers(1, &xfb_buf);
	glGenQueries(1, &query);
}

struct vertex_data {
	float position[4];
	float color[4];
};

void
dump_vertex_data(const struct vertex_data *data)
{
	printf("position=(%f, %f, %f, %f), color=(%f, %f, %f, %f)",
	       data->position[0], data->position[1], data->position[2], data->position[3],
	       data->color[0], data->color[1], data->color[2], data->color[3]);
}

enum piglit_result
piglit_display(void)
{
	int i, j;
	GLboolean pass = GL_TRUE;
	static const struct vertex_data vertex_input[12] = {
		/*  position XYZW             color RGBA */
		{ { -1.0, -1.0, 0.0, 1.0 }, { 1.0, 1.0, 0.0, 1.0 } },
		{ {  0.0, -1.0, 0.0, 1.0 }, { 1.0, 1.0, 0.0, 1.0 } },
		{ { -1.0,  1.0, 0.0, 1.0 }, { 1.0, 1.0, 0.0, 1.0 } },
		{ { -1.0,  1.0, 0.0, 1.0 }, { 1.0, 1.0, 0.0, 1.0 } },
		{ {  0.0, -1.0, 0.0, 1.0 }, { 1.0, 1.0, 0.0, 1.0 } },
		{ {  0.0,  1.0, 0.0, 1.0 }, { 1.0, 1.0, 0.0, 1.0 } },
		{ {  0.0, -1.0, 0.0, 1.0 }, { 0.0, 0.0, 1.0, 1.0 } },
		{ {  1.0, -1.0, 0.0, 1.0 }, { 0.0, 0.0, 1.0, 1.0 } },
		{ {  0.0,  1.0, 0.0, 1.0 }, { 0.0, 0.0, 1.0, 1.0 } },
		{ {  0.0,  1.0, 0.0, 1.0 }, { 0.0, 0.0, 1.0, 1.0 } },
		{ {  1.0, -1.0, 0.0, 1.0 }, { 0.0, 0.0, 1.0, 1.0 } },
		{ {  1.0,  1.0, 0.0, 1.0 }, { 0.0, 0.0, 1.0, 1.0 } }
	};
	static struct vertex_data initial_xfb_data[12];
	const struct vertex_data *readback;
	GLuint query_result;

	glUseProgram(prog);

	/* Setup inputs */
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE,
			      sizeof(struct vertex_data),
			      &vertex_input[0].position);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE,
			      sizeof(struct vertex_data),
			      &vertex_input[0].color);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	/* Setup transform feedback */
	for (i = 0; i < ARRAY_SIZE(initial_xfb_data); ++i) {
		for (j = 0; j < 4; ++j) {
			initial_xfb_data[i].position[j] = 12345.0;
			initial_xfb_data[i].color[j] = 12345.0;
		}
	}
	glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, xfb_buf);
	glBufferData(GL_TRANSFORM_FEEDBACK_BUFFER, sizeof(initial_xfb_data),
		     initial_xfb_data, GL_STREAM_READ);
	glBindBufferRange(GL_TRANSFORM_FEEDBACK_BUFFER, 0, xfb_buf, 0,
			  sizeof(float[9][8]));
	glBeginTransformFeedback(GL_TRIANGLES);
	switch (test_mode) {
	case TEST_MODE_PRIMS_GENERATED:
		glBeginQuery(GL_PRIMITIVES_GENERATED, query);
		break;
	case TEST_MODE_PRIMS_WRITTEN:
		glBeginQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN, query);
		break;
	default:
		break;
	}

	/* First draw call */
	glDrawArrays(GL_TRIANGLES, 0, 6);

	/* Read pixels */
	pass = piglit_probe_rect_rgba(0, 0, piglit_width / 2, piglit_height,
				      vertex_input[0].color) && pass;

	/* Second draw call */
	glDrawArrays(GL_TRIANGLES, 6, 6);

	/* Finish transform feedback and test correct behavior. */
	glEndTransformFeedback();
	switch (test_mode) {
	case TEST_MODE_OUTPUT:
		readback = glMapBuffer(GL_TRANSFORM_FEEDBACK_BUFFER,
				       GL_READ_ONLY);
		for (i = 0; i < 12; ++i) {
			const struct vertex_data *expected = i < 9
				? &vertex_input[i] : &initial_xfb_data[i];
			if (memcmp(&readback[i], expected,
				   sizeof(struct vertex_data)) != 0) {
				printf("Read incorrect data for vertex %i.\n",
				       i);
				printf("Readback: ");
				dump_vertex_data(&readback[i]);
				printf("\nExpected: ");
				dump_vertex_data(expected);
				printf("\n");
				pass = GL_FALSE;
			}
		}
		glUnmapBuffer(GL_TRANSFORM_FEEDBACK_BUFFER);
		break;
	case TEST_MODE_PRIMS_GENERATED:
		glEndQuery(GL_PRIMITIVES_GENERATED);
		glGetQueryObjectuiv(query, GL_QUERY_RESULT, &query_result);
		if (query_result != 4) {
			printf("Expected 4 primitives generated, got %u\n",
			       query_result);
			pass = GL_FALSE;
		}
		break;
	case TEST_MODE_PRIMS_WRITTEN:
		glEndQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN);
		glGetQueryObjectuiv(query, GL_QUERY_RESULT, &query_result);
		if (query_result != 3) {
			printf("Expected 3 primitives written, got %u\n",
			       query_result);
			pass = GL_FALSE;
		}
		break;
	}

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}
