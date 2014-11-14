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
 *
 * The optional argument "use_gs" causes the test to use a geometry
 * shader.  When this argument is given, the number of vertices output
 * by the geometry shader is in general different from the number of
 * vertices sent down the pipeline by the glDrawArrays() command.
 * Thus, the test verifies that the implementation uses the
 * post-geometry-shader vertex count to figure out where to resume
 * transform feedback after the glReadPixels call.
 */

#include "piglit-util-gl.h"

static bool use_gs;

PIGLIT_GL_TEST_CONFIG_BEGIN

	use_gs = PIGLIT_STRIP_ARG("use_gs");
	if (use_gs) {
		config.supports_gl_compat_version = 32;
		config.supports_gl_core_version = 32;
	} else {
		config.supports_gl_compat_version = 10;
		config.supports_gl_core_version = 31;
	}

	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGB;

PIGLIT_GL_TEST_CONFIG_END

static enum test_mode {
	TEST_MODE_OUTPUT,
	TEST_MODE_PRIMS_GENERATED,
	TEST_MODE_PRIMS_WRITTEN
} test_mode;

/**
 * Vertex shader used when use_gs is false.
 */
static const char *vstext_nogs =
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

/**
 * Fragment shader used when use_gs is false.
 */
static const char *fstext_nogs =
	"varying vec4 out_color;\n"
	"\n"
	"void main()\n"
	"{\n"
	"  gl_FragColor = out_color;\n"
	"}\n";

/**
 * Vertex shader used when use_gs is true.
 */
static const char *vstext_gs =
	"#version 150\n"
	"in vec4 in_color;\n"
	"out vec4 color_to_gs;\n"
	"\n"
	"void main()\n"
	"{\n"
	"  color_to_gs = in_color;\n"
	"}\n";

/**
 * Geometry shader used when use_gs is true.
 */
static const char *gstext_gs =
	"#version 150\n"
	"layout(points) in;\n"
	"layout(triangle_strip, max_vertices=6) out;\n"
	"uniform int start_index;\n"
	"in vec4 color_to_gs[1];\n"
	"out vec4 out_position;\n"
	"out vec4 out_color;\n"
	"\n"
	"void main()\n"
	"{\n"
	"  const vec2 positions[12] = vec2[12](\n"
	"    vec2(-1.0, -1.0),\n"
	"    vec2( 0.0, -1.0),\n"
	"    vec2(-1.0,  1.0),\n"
	"    vec2(-1.0,  1.0),\n"
	"    vec2( 0.0, -1.0),\n"
	"    vec2( 0.0,  1.0),\n"
	"    vec2( 0.0, -1.0),\n"
	"    vec2( 1.0, -1.0),\n"
	"    vec2( 0.0,  1.0),\n"
	"    vec2( 0.0,  1.0),\n"
	"    vec2( 1.0, -1.0),\n"
	"    vec2( 1.0,  1.0)\n"
	"  );\n"
	"  int index = start_index;\n"
	"  for (int i = 0; i < 2; i++) {\n"
	"    for (int j = 0; j < 3; j++) {\n"
	"      vec4 position = vec4(positions[index], 0.0, 1.0);\n"
	"      gl_Position = position;\n"
	"      out_position = position;\n"
	"      out_color = color_to_gs[0];\n"
	"      EmitVertex();\n"
	"      index++;\n"
	"    }\n"
	"    EndPrimitive();\n"
	"  }\n"
	"}\n";

/**
 * Fragment shader used when use_gs is false.
 */
static const char *fstext_gs =
	"#version 150\n"
	"in vec4 out_color;\n"
	"\n"
	"void main()\n"
	"{\n"
	"  gl_FragColor = out_color;\n"
	"}\n";

static const char *varyings[] = { "out_position", "out_color" };

static GLuint xfb_buf, vao, array_buf;
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
	GLuint vs, gs, fs;

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

	if (use_gs) {
		vs = piglit_compile_shader_text(GL_VERTEX_SHADER, vstext_gs);
		gs = piglit_compile_shader_text(GL_GEOMETRY_SHADER, gstext_gs);
		fs = piglit_compile_shader_text(GL_FRAGMENT_SHADER, fstext_gs);
	} else {
		vs = piglit_compile_shader_text(GL_VERTEX_SHADER, vstext_nogs);
		gs = 0;
		fs = piglit_compile_shader_text(GL_FRAGMENT_SHADER,
						fstext_nogs);
	}
	prog = glCreateProgram();
	glAttachShader(prog, vs);
	if (use_gs)
		glAttachShader(prog, gs);
	glAttachShader(prog, fs);
	if (!use_gs)
		glBindAttribLocation(prog, 0, "in_position");
	glBindAttribLocation(prog, 1, "in_color");
	glTransformFeedbackVaryings(prog, 2, varyings, GL_INTERLEAVED_ATTRIBS);
	glLinkProgram(prog);
	if (!piglit_link_check_status(prog)) {
		glDeleteProgram(prog);
		piglit_report_result(PIGLIT_FAIL);
	}

	glGenBuffers(1, &xfb_buf);
	glGenBuffers(1, &array_buf);
	glGenQueries(1, &query);
	if (piglit_is_extension_supported("GL_ARB_vertex_array_object") ||
	    piglit_get_gl_version() >= 30) {
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);
	}
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
	glBindBuffer(GL_ARRAY_BUFFER, array_buf);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_input), &vertex_input,
		     GL_STATIC_DRAW);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE,
			      sizeof(struct vertex_data),
			      (void *) offsetof(struct vertex_data, position));
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE,
			      sizeof(struct vertex_data),
			      (void *) offsetof(struct vertex_data, color));
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
	if (use_gs) {
		glUniform1i(glGetUniformLocation(prog, "start_index"), 0);
		glDrawArrays(GL_POINTS, 0, 1);
	} else {
		glDrawArrays(GL_TRIANGLES, 0, 6);
	}

	/* Read pixels */
	pass = piglit_probe_rect_rgba(0, 0, piglit_width / 2, piglit_height,
				      vertex_input[0].color) && pass;

	/* Second draw call */
	if (use_gs) {
		glUniform1i(glGetUniformLocation(prog, "start_index"), 6);
		glDrawArrays(GL_POINTS, 6, 1);
	} else {
		glDrawArrays(GL_TRIANGLES, 6, 6);
	}

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
