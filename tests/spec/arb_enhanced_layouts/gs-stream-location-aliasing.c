/*
 * Copyright (c) 2016 Advanced Micro Devices, Inc.
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
 * \file gs-stream-location-aliasing.c
 *
 * Tests two vertex streams produced by a geometry shader, with location
 * aliasing between the outputs of the different streams.
 *
 * Loosely based on transform_feedback_layout_qualifiers.c
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 32;
	config.supports_gl_core_version = 32;

PIGLIT_GL_TEST_CONFIG_END

static const char vs_text[] =
	"#version 150\n"
	"out int vertexid;\n"
	"void main() {\n"
	"  vertexid = gl_VertexID;\n"
	"}\n";

static const char gs_text[] =
	"#version 150\n"
	"#extension GL_ARB_gpu_shader5: require\n"
	"#extension GL_ARB_separate_shader_objects: require\n"
	"#extension GL_ARB_enhanced_layouts: require\n"
	"\n"
	"layout(points, invocations = 2) in;\n"
	"layout(points, max_vertices = 2) out;\n"
	"\n"
	"in int vertexid[];\n"
	"\n"
	"layout(xfb_offset = 0, xfb_buffer = 0, location = 0, component = 0, stream = 0) out float x1_out;\n"
	"layout(xfb_offset = 0, xfb_buffer = 1, location = 0, component = 1, stream = 1) out float x2_out;\n"
	"\n"
	"void main() {\n"
	"  x1_out = 100 + 10 * vertexid[0] + gl_InvocationID;\n"
	"  EmitStreamVertex(0);\n"
	"  EndStreamPrimitive(0);\n"
	"  x2_out = 200 + 10 * vertexid[0] + gl_InvocationID;\n"
	"  EmitStreamVertex(1);\n"
	"  EndStreamPrimitive(1);\n"
	"}";

static const int num_in_vertices = 2;
static const int num_xfb_results_per_stream = 2 * 2;
static const int num_total_xfb_results = 2 * 2 * 2;

static void
build_and_use_program()
{
	GLuint prog;

	prog = piglit_build_simple_program_multiple_shaders(
			GL_VERTEX_SHADER, vs_text,
			GL_GEOMETRY_SHADER, gs_text, 0);

	glLinkProgram(prog);
	if (!piglit_link_check_status(prog))
		piglit_report_result(PIGLIT_FAIL);
	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	glUseProgram(prog);
}

static bool
probe_buffers(const GLuint *xfb)
{
	bool pass = true;
	float *first;
	float *second;

	first = malloc(num_xfb_results_per_stream * sizeof(float));
	second = malloc(num_xfb_results_per_stream * sizeof(float));

	unsigned i = 0;
	for (unsigned vertex = 0; vertex < 2; ++vertex) {
		for (unsigned invocation = 0; invocation < 2; ++invocation) {
			first[i] = 100 + 10 * vertex + invocation;
			second[i] = 200 + 10 * vertex + invocation;
			i++;
		}
	}
	assert(i == num_xfb_results_per_stream);

	pass = piglit_probe_buffer(xfb[0], GL_TRANSFORM_FEEDBACK_BUFFER,
			"first", 1, num_xfb_results_per_stream, first) && pass;
	pass = piglit_probe_buffer(xfb[1], GL_TRANSFORM_FEEDBACK_BUFFER,
			"second", 1, num_xfb_results_per_stream, second) &&
			pass;

	free(first);
	free(second);

	return pass;
}

void
piglit_init(int argc, char **argv)
{
	bool pass = true;
	GLuint xfb[2];
	GLuint vao;

	piglit_require_GLSL_version(150);
	piglit_require_extension("GL_ARB_transform_feedback3");
	piglit_require_extension("GL_ARB_enhanced_layouts");

	build_and_use_program();
	glEnable(GL_RASTERIZER_DISCARD);

	/* Set up the transform feedback buffers. */
	glGenBuffers(2, xfb);
	glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, xfb[0]);
	glBufferData(GL_TRANSFORM_FEEDBACK_BUFFER,
		     num_xfb_results_per_stream * 4, NULL, GL_STREAM_READ);
	glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 1, xfb[1]);
	glBufferData(GL_TRANSFORM_FEEDBACK_BUFFER,
		     num_xfb_results_per_stream * 4, NULL, GL_STREAM_READ);

	/* Test is run under desktop OpenGL 3.2 -> use of VAOs is required */
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	/* Draw and record */
	glBeginTransformFeedback(GL_POINTS);
	glDrawArrays(GL_POINTS, 0, num_in_vertices);
	glEndTransformFeedback();
	glDeleteVertexArrays(1, &vao);

	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	pass = probe_buffers(xfb);

	glDeleteBuffers(2, xfb);

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}

enum piglit_result
piglit_display(void)
{
	/* Should never be reached */
	return PIGLIT_FAIL;
}
