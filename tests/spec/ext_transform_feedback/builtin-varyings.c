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
 * \file builtin-varyings.c
 *
 * Verify that transform feedback can be used with built-in varying
 * variables, such as gl_FrontColor, gl_BackColor, etc.
 *
 * Note: gl_FrontColor and gl_BackColor are tested at the same time,
 * in order to verify that the implementation is able to distinguish
 * them.  Same for gl_FrontSecondaryColor and gl_BackSecondaryColor.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGB;

PIGLIT_GL_TEST_CONFIG_END

#define MAX_EXPECTED_OUTPUT_COMPONENTS 8

/**
 * All possible tests to run.  Note that in each test, the input will
 * consist of 6 vertices, with attribute vertex_num running from 0
 * through 5, and attribute vertex_pos tracing out one front-facing
 * triangle and one back-facing triangle.
 *
 * The expected output of each test is the sequence of floating point
 * values (0.0, 1.0/256.0, 2.0/256.0, 3.0/256.0, ...).
 */
struct test_desc {
	const char *name;
	int version;
	const char *vs;
	unsigned num_varyings;
	const char *varyings[16];
	unsigned expected_num_output_components;
	GLenum expected_type;
	GLsizei expected_size;
} tests[] = {
	{
		"gl_Color", /* name */
		110, /* version */

		"#version 110\n" /* vs */
		"attribute vec4 vertex_pos;\n"
		"attribute float vertex_num;\n"
		"void main() {\n"
		"  gl_Position = vertex_pos;\n"
		"  vec4 offset = vec4(0.0, 1.0, 2.0, 3.0);\n"
		"  float scale = 1.0/256.0;\n"
		"  gl_FrontColor = (offset + 8.0 * vertex_num) * scale;\n"
		"  gl_BackColor = (offset + 4.0 + 8.0 * vertex_num) * scale;\n"
		"}\n",

		2, /* num_varyings */
		{"gl_FrontColor", "gl_BackColor"}, /* varyings */

		8, /* expected_num_output_components */
		GL_FLOAT_VEC4, /* expected_type */
		1, /* expected_size */
	},
	{
		"gl_SecondaryColor", /* name */
		110, /* version */

		"#version 110\n" /* vs */
		"attribute vec4 vertex_pos;\n"
		"attribute float vertex_num;\n"
		"void main() {\n"
		"  gl_Position = vertex_pos;\n"
		"  vec4 offset = vec4(0.0, 1.0, 2.0, 3.0);\n"
		"  float scale = 1.0/256.0;\n"
		"  gl_FrontSecondaryColor = (offset + 8.0 * vertex_num) * scale;\n"
		"  gl_BackSecondaryColor = (offset + 4.0 + 8.0 * vertex_num) * scale;\n"
		"}\n",

		2, /* num_varyings */
		{"gl_FrontSecondaryColor", "gl_BackSecondaryColor"}, /* varyings */

		8, /* expected_num_output_components */
		GL_FLOAT_VEC4, /* expected_type */
		1, /* expected_size */
	},
	{
		"gl_TexCoord", /* name */
		110, /* version */

		"#version 110\n" /* vs */
		"attribute vec4 vertex_pos;\n"
		"attribute float vertex_num;\n"
		"void main() {\n"
		"  gl_Position = vertex_pos;\n"
		"  vec4 offset = vec4(0.0, 1.0, 2.0, 3.0);\n"
		"  float scale = 1.0/256.0;\n"
		"  gl_TexCoord[0] = (offset + 8.0 * vertex_num) * scale;\n"
		"  gl_TexCoord[1] = (offset + 4.0 + 8.0 * vertex_num) * scale;\n"
		"}\n",

		2, /* num_varyings */
		{"gl_TexCoord[0]", "gl_TexCoord[1]"}, /* varyings */

		8, /* expected_num_output_components */
		GL_FLOAT_VEC4, /* expected_type */
		1, /* expected_size */
	},
	{
		"gl_FogFragCoord", /* name */
		110, /* version */

		"#version 110\n" /* vs */
		"attribute vec4 vertex_pos;\n"
		"attribute float vertex_num;\n"
		"void main() {\n"
		"  gl_Position = vertex_pos;\n"
		"  gl_FogFragCoord = vertex_num / 256.0;\n"
		"}\n",

		1, /* num_varyings */
		{"gl_FogFragCoord"}, /* varyings */

		1, /* expected_num_output_components */
		GL_FLOAT, /* expected_type */
		1, /* expected_size */
	},
	{
		"gl_Position", /* name */
		110, /* version */

		"#version 110\n" /* vs */
		"attribute vec4 vertex_pos;\n"
		"attribute float vertex_num;\n"
		"void main() {\n"
		"  vec4 offset = vec4(0.0, 1.0, 2.0, 3.0);\n"
		"  float scale = 1.0/256.0;\n"
		"  gl_Position = (offset + 4.0 * vertex_num) * scale;\n"
		"}\n",

		1, /* num_varyings */
		{"gl_Position"}, /* varyings */

		4, /* expected_num_output_components */
		GL_FLOAT_VEC4, /* expected_type */
		1, /* expected_size */
	},
	{
		"gl_PointSize", /* name */
		110, /* version */

		"#version 110\n" /* vs */
		"attribute vec4 vertex_pos;\n"
		"attribute float vertex_num;\n"
		"void main() {\n"
		"  gl_Position = vertex_pos;\n"
		"  gl_PointSize = vertex_num / 256.0;\n"
		"}\n",

		1, /* num_varyings */
		{"gl_PointSize"}, /* varyings */

		1, /* expected_num_output_components */
		GL_FLOAT, /* expected_type */
		1, /* expected_size */
	},
	{
		"gl_ClipVertex", /* name */
		110, /* version */

		"#version 110\n" /* vs */
		"attribute vec4 vertex_pos;\n"
		"attribute float vertex_num;\n"
		"void main() {\n"
		"  gl_Position = vertex_pos;\n"
		"  vec4 offset = vec4(0.0, 1.0, 2.0, 3.0);\n"
		"  float scale = 1.0/256.0;\n"
		"  gl_ClipVertex = (offset + 4.0 * vertex_num) * scale;\n"
		"}\n",

		1, /* num_varyings */
		{"gl_ClipVertex"}, /* varyings */

		4, /* expected_num_output_components */
		GL_FLOAT_VEC4, /* expected_type */
		1, /* expected_size */
	},
	{
		"gl_ClipDistance", /* name */
		130, /* version */

		"#version 130\n" /* vs */
		"in vec4 vertex_pos;\n"
		"in float vertex_num;\n"
		"out float gl_ClipDistance[8];\n"
		"void main() {\n"
		"  gl_Position = vertex_pos;\n"
		"  float scale = 1.0/256.0;\n"
		"  for(int i = 0; i < 8; ++i)\n"
		"    gl_ClipDistance[i] = (float(i) + 8.0 * vertex_num) * scale;\n"
		"}\n",

		8, /* num_varyings */
		{"gl_ClipDistance[0]", "gl_ClipDistance[1]", /* varyings */
		 "gl_ClipDistance[2]", "gl_ClipDistance[3]",
		 "gl_ClipDistance[4]", "gl_ClipDistance[5]",
		 "gl_ClipDistance[6]", "gl_ClipDistance[7]"},

		8, /* expected_num_output_components */
		GL_FLOAT, /* expected_type */
		1, /* expected_size */
	},
	{
		"gl_ClipDistance[1]-no-subscript", /* name */
		130, /* version */

		"#version 130\n" /* vs */
		"in vec4 vertex_pos;\n"
		"in float vertex_num;\n"
		"out float gl_ClipDistance[1];\n"
		"void main() {\n"
		"  gl_Position = vertex_pos;\n"
		"  float scale = 1.0/256.0;\n"
		"  for(int i = 0; i < 1; ++i)\n"
		"    gl_ClipDistance[i] = (float(i) + 1.0 * vertex_num) * scale;\n"
		"}\n",

		1, /* num_varyings */
		{"gl_ClipDistance"}, /* varyings */

		1, /* expected_num_output_components */
		GL_FLOAT, /* expected_type */
		1, /* expected_size */
	},
	{
		"gl_ClipDistance[2]-no-subscript", /* name */
		130, /* version */

		"#version 130\n" /* vs */
		"in vec4 vertex_pos;\n"
		"in float vertex_num;\n"
		"out float gl_ClipDistance[2];\n"
		"void main() {\n"
		"  gl_Position = vertex_pos;\n"
		"  float scale = 1.0/256.0;\n"
		"  for(int i = 0; i < 2; ++i)\n"
		"    gl_ClipDistance[i] = (float(i) + 2.0 * vertex_num) * scale;\n"
		"}\n",

		1, /* num_varyings */
		{"gl_ClipDistance"}, /* varyings */

		2, /* expected_num_output_components */
		GL_FLOAT, /* expected_type */
		2, /* expected_size */
	},
	{
		"gl_ClipDistance[3]-no-subscript", /* name */
		130, /* version */

		"#version 130\n" /* vs */
		"in vec4 vertex_pos;\n"
		"in float vertex_num;\n"
		"out float gl_ClipDistance[3];\n"
		"void main() {\n"
		"  gl_Position = vertex_pos;\n"
		"  float scale = 1.0/256.0;\n"
		"  for(int i = 0; i < 3; ++i)\n"
		"    gl_ClipDistance[i] = (float(i) + 3.0 * vertex_num) * scale;\n"
		"}\n",

		1, /* num_varyings */
		{"gl_ClipDistance"}, /* varyings */

		3, /* expected_num_output_components */
		GL_FLOAT, /* expected_type */
		3, /* expected_size */
	},
	{
		"gl_ClipDistance[4]-no-subscript", /* name */
		130, /* version */

		"#version 130\n" /* vs */
		"in vec4 vertex_pos;\n"
		"in float vertex_num;\n"
		"out float gl_ClipDistance[4];\n"
		"void main() {\n"
		"  gl_Position = vertex_pos;\n"
		"  float scale = 1.0/256.0;\n"
		"  for(int i = 0; i < 4; ++i)\n"
		"    gl_ClipDistance[i] = (float(i) + 4.0 * vertex_num) * scale;\n"
		"}\n",

		1, /* num_varyings */
		{"gl_ClipDistance"}, /* varyings */

		4, /* expected_num_output_components */
		GL_FLOAT, /* expected_type */
		4, /* expected_size */
	},
	{
		"gl_ClipDistance[5]-no-subscript", /* name */
		130, /* version */

		"#version 130\n" /* vs */
		"in vec4 vertex_pos;\n"
		"in float vertex_num;\n"
		"out float gl_ClipDistance[5];\n"
		"void main() {\n"
		"  gl_Position = vertex_pos;\n"
		"  float scale = 1.0/256.0;\n"
		"  for(int i = 0; i < 5; ++i)\n"
		"    gl_ClipDistance[i] = (float(i) + 5.0 * vertex_num) * scale;\n"
		"}\n",

		1, /* num_varyings */
		{"gl_ClipDistance"}, /* varyings */

		5, /* expected_num_output_components */
		GL_FLOAT, /* expected_type */
		5, /* expected_size */
	},
	{
		"gl_ClipDistance[6]-no-subscript", /* name */
		130, /* version */

		"#version 130\n" /* vs */
		"in vec4 vertex_pos;\n"
		"in float vertex_num;\n"
		"out float gl_ClipDistance[6];\n"
		"void main() {\n"
		"  gl_Position = vertex_pos;\n"
		"  float scale = 1.0/256.0;\n"
		"  for(int i = 0; i < 6; ++i)\n"
		"    gl_ClipDistance[i] = (float(i) + 6.0 * vertex_num) * scale;\n"
		"}\n",

		1, /* num_varyings */
		{"gl_ClipDistance"}, /* varyings */

		6, /* expected_num_output_components */
		GL_FLOAT, /* expected_type */
		6, /* expected_size */
	},
	{
		"gl_ClipDistance[7]-no-subscript", /* name */
		130, /* version */

		"#version 130\n" /* vs */
		"in vec4 vertex_pos;\n"
		"in float vertex_num;\n"
		"out float gl_ClipDistance[7];\n"
		"void main() {\n"
		"  gl_Position = vertex_pos;\n"
		"  float scale = 1.0/256.0;\n"
		"  for(int i = 0; i < 7; ++i)\n"
		"    gl_ClipDistance[i] = (float(i) + 7.0 * vertex_num) * scale;\n"
		"}\n",

		1, /* num_varyings */
		{"gl_ClipDistance"}, /* varyings */

		7, /* expected_num_output_components */
		GL_FLOAT, /* expected_type */
		7, /* expected_size */
	},
	{
		"gl_ClipDistance[8]-no-subscript", /* name */
		130, /* version */

		"#version 130\n" /* vs */
		"in vec4 vertex_pos;\n"
		"in float vertex_num;\n"
		"out float gl_ClipDistance[8];\n"
		"void main() {\n"
		"  gl_Position = vertex_pos;\n"
		"  float scale = 1.0/256.0;\n"
		"  for(int i = 0; i < 8; ++i)\n"
		"    gl_ClipDistance[i] = (float(i) + 8.0 * vertex_num) * scale;\n"
		"}\n",

		1, /* num_varyings */
		{"gl_ClipDistance"}, /* varyings */

		8, /* expected_num_output_components */
		GL_FLOAT, /* expected_type */
		8, /* expected_size */
	},
};

const struct test_desc *test_to_run;

static GLuint xfb_buf;
static GLuint prog;
static GLuint query;
static GLboolean size_and_type_ok = GL_TRUE;

void
print_usage_and_exit(const char *prog_name)
{
	int i;

	printf("Usage: %s <test_name>\n"
	       "  where <test_name> is one of:\n", prog_name);
	for (i = 0; i < ARRAY_SIZE(tests); ++i)
		printf("    %s\n", tests[i].name);
	exit(0);
}

const struct test_desc *
find_matching_test(const char *prog_name, const char *test_name)
{
	int i;
	for (i = 0; i < ARRAY_SIZE(tests); ++i) {
		if (strcmp(tests[i].name, test_name) == 0)
			return &tests[i];
	}
	print_usage_and_exit(prog_name);
	return NULL; /* won't actually be reached */
}

void
piglit_init(int argc, char **argv)
{
	GLuint vs;
	int i;

	/* Parse params. */
	if (argc != 2)
		print_usage_and_exit(argv[0]);
	test_to_run = find_matching_test(argv[0], argv[1]);

	/* Set up test */
	piglit_require_GLSL_version(test_to_run->version);
	piglit_require_transform_feedback();
	vs = piglit_compile_shader_text(GL_VERTEX_SHADER, test_to_run->vs);
	prog = glCreateProgram();
	glAttachShader(prog, vs);
	glBindAttribLocation(prog, 0, "vertex_pos");
	glBindAttribLocation(prog, 1, "vertex_num");
	glTransformFeedbackVaryings(prog, test_to_run->num_varyings,
				    (const char **) test_to_run->varyings,
				    GL_INTERLEAVED_ATTRIBS_EXT);
	glLinkProgram(prog);
	if (!piglit_link_check_status(prog)) {
		glDeleteProgram(prog);
		piglit_report_result(PIGLIT_FAIL);
	}

	/* Test that GetTransformFeedbackVarying reports the correct
	 * size and type for all of the varyings.
	 */
	for (i = 0; i < test_to_run->num_varyings; ++i) {
		GLsizei size;
		GLenum type;
		glGetTransformFeedbackVarying(prog, i, 0, NULL, &size,
					      &type, NULL);
		if (size != test_to_run->expected_size) {
			printf("For varying %i, expected size %i, got %i\n",
			       i, test_to_run->expected_size, size);
			size_and_type_ok = GL_FALSE;
		}
		if (type != test_to_run->expected_type) {
			printf("For varying %i, expected type %i, got %i\n",
			       i, test_to_run->expected_type, type);
			size_and_type_ok = GL_FALSE;
		}
	}

	glGenBuffers(1, &xfb_buf);
	glGenQueries(1, &query);
	glEnable(GL_VERTEX_PROGRAM_TWO_SIDE);
}

struct vertex_data {
	float vertex_pos[4];
	float vertex_num;
};

enum piglit_result
piglit_display(void)
{
	static const struct vertex_data vertex_input[6] = {
		/*  vertex_pos XYZW         vertex_num */
		{ { -1.0, -1.0, 0.0, 1.0 }, 0.0 },
		{ { -1.0,  1.0, 0.0, 1.0 }, 1.0 },
		{ {  1.0, -1.0, 0.0, 1.0 }, 2.0 },
		{ { -1.0,  1.0, 0.0, 1.0 }, 3.0 },
		{ {  1.0, -1.0, 0.0, 1.0 }, 4.0 },
		{ {  1.0,  1.0, 0.0, 1.0 }, 5.0 }
	};
	static float initial_xfb_data[MAX_EXPECTED_OUTPUT_COMPONENTS * 6];
	GLboolean pass = size_and_type_ok;
	int i;
	float *readback;
	GLuint query_result;

	glUseProgram(prog);

	/* Setup inputs */
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE,
			      sizeof(struct vertex_data),
			      &vertex_input[0].vertex_pos);
	glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE,
			      sizeof(struct vertex_data),
			      &vertex_input[0].vertex_num);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	/* Setup transform feedback */
	for (i = 0; i < ARRAY_SIZE(initial_xfb_data); ++i)
		initial_xfb_data[i] = 12345.0;
	glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, xfb_buf);
	glBufferData(GL_TRANSFORM_FEEDBACK_BUFFER, sizeof(initial_xfb_data),
		     initial_xfb_data, GL_STREAM_READ);
	glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, xfb_buf);
	glBeginTransformFeedback(GL_TRIANGLES);
	glBeginQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN, query);

	/* Draw */
	glDrawArrays(GL_TRIANGLES, 0, 6);

	/* Check that there was room in the buffer to write all
	 * transform feedback outputs.
	 */
	glEndQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN);
	glGetQueryObjectuiv(query, GL_QUERY_RESULT, &query_result);
	if (query_result != 2) {
		printf("Expected 2 primitives written, got %u\n",
		       query_result);
		pass = GL_FALSE;
	}

	/* Check transform feedback output */
	glEndTransformFeedback();
	readback = glMapBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, GL_READ_ONLY);
	for (i = 0; i < ARRAY_SIZE(initial_xfb_data); ++i) {
		float expected =
			i < 6 * test_to_run->expected_num_output_components
			? i/256.0 : 12345.0;
		if (readback[i] != expected) {
			printf("Buffer[%i]=%f, expected=%f\n", i, readback[i],
			       expected);
			pass = GL_FALSE;
		}
	}
	glUnmapBuffer(GL_TRANSFORM_FEEDBACK_BUFFER);

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}
