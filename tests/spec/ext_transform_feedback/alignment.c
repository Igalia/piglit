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
 * \file alignment.c
 *
 * Verify that transform feedback outputs are generated correctly
 * regardless of how the buffers (and the data) are aligned in memory.
 *
 * The test requires a single integer argument, which specifies the
 * number of bytes of offset that should be specified when calling
 * glBindBufferRange().  This value may be 0, 4, 8, or 12.
 */

#include "piglit-util-gl.h"

#define BUFFER_SIZE 0x40

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;

PIGLIT_GL_TEST_CONFIG_END

/* Test parameters */
static unsigned long additional_offset;

/* Other globals */
static GLuint prog;
static GLuint xfb_buf;

/**
 * Input data for the vertex shader.
 */
static GLuint verts[4] = { 0, 1, 2, 3 };

/**
 * Vertex shader.  This is designed so that its transform feedback
 * outputs appear at all possible alignments, and so that the correct
 * output will consist of the following pattern of uints:
 *
 * 0x00010203
 * 0x04050607
 * 0x08090a0b
 *  ...
 * 0xacadaeaf
 *
 * (a total of 44 uints)
 */
static const char *vstext =
	"#version 130\n"
	"in uint input_uint;\n"
	"flat out uint  out_a;\n"
	"flat out uvec2 out_b;\n"
	"flat out uvec3 out_c;\n"
	"flat out uvec4 out_d;\n"
	"flat out uint  out_e;\n"
	"\n"
	"void main()\n"
	"{\n"
	"  gl_Position = vec4(0.0);\n"
	"  uint offset = input_uint * 0x2c2c2c2cu;\n"
	"  out_a = 0x00010203u + offset;\n"
	"  out_b = uvec2(0x04050607, 0x08090a0b) + offset;\n"
	"  out_c = uvec3(0x0c0d0e0f, 0x10111213, 0x14151617) + offset;\n"
	"  out_d = uvec4(0x18191a1b, 0x1c1d1e1f, 0x20212223, 0x24252627) + offset;\n"
	"  out_e = 0x28292a2bu + offset;\n"
	"}\n";

#define EXPECTED_NUM_OUTPUTS 44

static const char *varyings[] = {
	"out_a", "out_b", "out_c", "out_d", "out_e"
};

static const char *fstext =
	"#version 130\n"
	"void main()\n"
	"{\n"
	"  gl_FragColor = vec4(0.0);\n"
	"}\n";

static void
print_usage_and_exit(char *prog_name)
{
	printf("Usage: %s <additional_offset>\n"
	       "  where <additional_offset> is one of the values\n"
	       "  0, 4, 8, or 12.\n", prog_name);
	exit(1);
}

void piglit_init(int argc, char **argv)
{
	char *endptr;
	GLuint vs, fs;

	/* Interpret command line args */
	if (argc != 2)
		print_usage_and_exit(argv[0]);
	endptr = argv[1];
	additional_offset = strtoul(argv[1], &endptr, 0);
	if (*endptr != '\0')
		print_usage_and_exit(argv[0]);
	if (additional_offset > 12 || additional_offset % 4 != 0)
		print_usage_and_exit(argv[0]);

	piglit_require_GLSL_version(130);
	piglit_require_gl_version(30);
	piglit_require_transform_feedback();
	vs = piglit_compile_shader_text(GL_VERTEX_SHADER, vstext);
	fs = piglit_compile_shader_text(GL_FRAGMENT_SHADER, fstext);
	prog = glCreateProgram();
	glAttachShader(prog, vs);
	glAttachShader(prog, fs);
	glTransformFeedbackVaryings(prog, 5, varyings, GL_INTERLEAVED_ATTRIBS);
	glLinkProgram(prog);
	if (!piglit_link_check_status(prog))
		piglit_report_result(PIGLIT_FAIL);
	glGenBuffers(1, &xfb_buf);
	if (!piglit_check_gl_error(0))
		piglit_report_result(PIGLIT_FAIL);
}

enum piglit_result piglit_display(void)
{
	GLint input_index = glGetAttribLocation(prog, "input_uint");
	GLuint *readback;
	GLuint buffer[BUFFER_SIZE];
	GLuint expected[BUFFER_SIZE];
	unsigned int i;
	GLboolean pass = GL_TRUE;

	glUseProgram(prog);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glVertexAttribIPointer(input_index, 1, GL_UNSIGNED_INT,
			       sizeof(GLuint), &verts);
	glEnableVertexAttribArray(input_index);
	pass = piglit_check_gl_error(0) && pass;

	glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, xfb_buf);
	memset(buffer, 0xffffffff, sizeof(buffer));
	glBufferData(GL_TRANSFORM_FEEDBACK_BUFFER, sizeof(buffer), buffer,
		     GL_STREAM_READ);
	glBindBufferRange(GL_TRANSFORM_FEEDBACK_BUFFER, 0, xfb_buf,
			  additional_offset,
			  sizeof(buffer) - additional_offset);
	glBeginTransformFeedback(GL_POINTS);
	glDrawArrays(GL_POINTS, 0, 4);
	glEndTransformFeedback();
	pass = piglit_check_gl_error(0) && pass;

	readback = glMapBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, GL_READ_ONLY);
	pass = piglit_check_gl_error(0) && pass;

	/* Figure out expected output */
	memset(expected, 0xffffffff, sizeof(expected));
	for (i = 0; i < EXPECTED_NUM_OUTPUTS; ++i) {
		expected[i + additional_offset / 4] =
			0x00010203u + 0x04040404u * i;
	}

	/* Check output */
	for (i = 0; i < BUFFER_SIZE; ++i) {
		if (expected[i] != readback[i]) {
			printf("readback[%u]: %u, expected: %u\n", i,
			       readback[i], expected[i]);
			pass = GL_FALSE;
		}
	}

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}
