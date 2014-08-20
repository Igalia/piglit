/*
 * Copyright (c) 2014 Intel Corporation
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

#include "piglit-util-gl.h"

/**
 * @file tf-wrong-stream-value.c
 *
 * This test uses geometry shader multiple stream support from
 * GL_ARB_gpu_shader5 and GL_ARB_transform_feedback3 to capture
 * transform feedback from 2 streams into one buffer.
 *
 * This test is expected to fail when linking.
 * From ARB_transform_feedback3 spec:
 *
 * "A program will fail to link if:
 * [...]
 * * the set of varyings to capture to any single binding point
 *   includes varyings from more than one vertex stream."
 */

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 32;
	config.supports_gl_core_version = 32;

PIGLIT_GL_TEST_CONFIG_END

static const char vs_pass_thru_text[] =
	"#version 150\n"
	"void main() {\n"
	"  gl_Position = vec4(0.0);\n"
	"}\n";

static const char gs_text[] =
	"#version 150\n"
	"#extension GL_ARB_gpu_shader5 : enable\n"
	"layout(points) in;\n"
	"layout(points, max_vertices = 1) out;\n"
	"out float stream0_0_out;\n"
	"layout(stream = 1) out vec2 stream1_0_out;\n"
	"void main() {\n"
	"  gl_Position = gl_in[0].gl_Position;\n"

	"  stream0_0_out = 0.0;\n"
	"  stream1_0_out = vec2(1.0, 2.0);\n"
	"  EmitVertex();\n"
	"  EndPrimitive();\n"
	"}";

const char *stream_names[] = { "first", "second", "third", "forth" };
int stream_float_counts[] = { 1, 5, 5, 3 };

#define STREAMS ARRAY_SIZE(stream_names)

static const char *varyings[] = {
	"stream0_0_out", "stream1_0_out"
};

void
piglit_init(int argc, char **argv)
{
	bool pass;
	unsigned primitive_n;
	GLint gs_invocation_n;
	GLuint prog;


	piglit_require_extension("GL_ARB_gpu_shader5");
	piglit_require_extension("GL_ARB_transform_feedback3");

	prog = piglit_build_simple_program_multiple_shaders(
			GL_VERTEX_SHADER, vs_pass_thru_text,
			GL_GEOMETRY_SHADER, gs_text, 0);

	if (!piglit_check_gl_error(GL_NO_ERROR)) {
		piglit_report_result(PIGLIT_FAIL);
		return;
	}

	glTransformFeedbackVaryings(prog, ARRAY_SIZE(varyings), varyings,
			GL_INTERLEAVED_ATTRIBS);

	glLinkProgram(prog);
	if (!piglit_link_check_status(prog))
		piglit_report_result(PIGLIT_PASS);
	else
		piglit_report_result(PIGLIT_FAIL);
}

enum piglit_result
piglit_display(void)
{
	/* Should never be reached */
	return PIGLIT_FAIL;
}
