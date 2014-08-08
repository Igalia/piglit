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
 * \file tri-strip-ordering-with-prim-restart.c
 *
 * Check a subtle corner case that affects the i965/gen7 mesa driver:
 * when the primitive type is either GL_TRIANGLE_STRIP or
 * GL_TRIANGLE_STRIP_ADJACENCY, the hardware delivers the vertices of
 * odd numbered triangles to the geometry shader in the wrong order,
 * so the driver must emit workaround code to re-order them.  This
 * test verifies that the workaround code functions correctly in the
 * presence of primitive restart, since the presence of primitive
 * restart can make a triangle "odd numbered" in relation to the
 * current strip even if it is "even numbered" as measured by
 * gl_PrimitiveIDIn.
 *
 * This test works by issuing a single draw call and using primitive
 * restart to split it into a pair of 3-triangle strips (this ensures
 * that triangles in the first strip have the same parity in relation
 * to the strip as they have when measured by gl_PrimitiveIDIn;
 * triangles in the second strip hav opposite parity in relation to
 * the strip from what they have when measured by gl_PrimitiveIDIn).
 * The vertex IDs of all vertices are collected using transform
 * feedback, and checked in C to make sure it matches the expected
 * sequence of vertices.
 *
 * Note: some generations of Intel hardware require primitive restart
 * to be emulated in software when either:
 *
 * - certain primitive types are used, or
 *
 * - the primitive restart index is not all 0xff's.
 *
 * To make sure that both the hardware and software primitive restart
 * codepaths are tested, this test accepts an additional command line
 * option to control whether the primitive restart index should be all
 * 0xff's.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 32;
	config.supports_gl_core_version = 32;

PIGLIT_GL_TEST_CONFIG_END


/**
 * 5 triangles per strip, 6 vertices per triangle in
 * GL_TRIANGLE_STRIP_ADJACENCY mode.
 */
#define MAX_OUTPUT_VERTICES_PER_STRIP (5*6)


static const char *vs_text =
	"#version 150\n"
	"\n"
	"out int vertex_id;\n"
	"\n"
	"void main()\n"
	"{\n"
	"  vertex_id = gl_VertexID;\n"
	"}\n";

static const char *gs_template =
	"#version 150\n"
	"#define INPUT_LAYOUT %s\n"
	"#define VERTICES_PER_PRIM %d\n"
	"layout(INPUT_LAYOUT) in;\n"
	"layout(points, max_vertices = VERTICES_PER_PRIM) out;\n"
	"\n"
	"in int vertex_id[VERTICES_PER_PRIM];\n"
	"out int vertex_out[VERTICES_PER_PRIM];\n"
	"\n"
	"void main()\n"
	"{\n"
	"  for (int i = 0; i < VERTICES_PER_PRIM; i++) {\n"
	"    vertex_out[i] = vertex_id[i] + 1;\n"
	"  }\n"
	"  EmitVertex();\n"
	"}\n";


static const char *varyings[] = {
	"vertex_out[0]",
	"vertex_out[1]",
	"vertex_out[2]",
	"vertex_out[3]",
	"vertex_out[4]",
	"vertex_out[5]",
};


static const struct test_vector
{
	const char *name;
	GLenum prim_type;
	const char *input_layout;
	unsigned vertices_per_prim;

	/**
	 * Number of vertices to send down the pipeline for a single
	 * 3-triangle strip
	 */
	unsigned input_vertices_per_strip;

	/**
	 * Number of output vertices that are expected for a single
	 * 3-triangle strip
	 */
	unsigned output_vertices_per_strip;

	/**
	 * Vertices that each GS invocation is expected to see for a
	 * single 3-triangle strip.
	 */
	GLint expected_results[MAX_OUTPUT_VERTICES_PER_STRIP];
} tests[] = {
	{ "GL_TRIANGLE_STRIP", GL_TRIANGLE_STRIP, "triangles", 3, 5, 9,
	  { 1, 2, 3,
	    3, 2, 4,
	    3, 4, 5 } },
	/* See primitive-types.c for how this vertex ordering was
	 * determined.
	 */
	{ "GL_TRIANGLE_STRIP_ADJACENCY", GL_TRIANGLE_STRIP_ADJACENCY,
	  "triangles_adjacency", 6, 10, 18,
	  { 1, 2, 3, 7, 5, 4,
	    5, 1, 3, 6, 7, 9,
	    5, 3, 7, 10, 9, 8 } },
};


static void
print_usage_and_exit(const char *prog_name)
{
	int i;
	printf("Usage: %s <primitive> <restart-index>\n"
	       "  where <primitive> is one of the following:\n", prog_name);
	for(i = 0; i < ARRAY_SIZE(tests); i++)
		printf("    %s\n", tests[i].name);
	printf("  and <restart-index> is one of the following:\n"
	       "    ffs - use a primitive restart index that is all 0xffs\n"
	       "    other - use a different primitive restart index\n");
	piglit_report_result(PIGLIT_FAIL);
}


void
piglit_init(int argc, char **argv)
{
	int i, j;
	const struct test_vector *test = NULL;
	GLubyte prim_restart_index;
	GLuint prog, vs, gs, vao, xfb_buf, generated_query, element_buf,
		primitives_generated;
	char *gs_text;
	GLsizei num_input_elements;
	GLubyte *elements;
	bool pass = true;
	unsigned expected_output_points_per_strip, actual_output_points;
	GLuint *readback;

	/* Parse params */
	if (argc != 3)
		print_usage_and_exit(argv[0]);
	for (i = 0; i < ARRAY_SIZE(tests); i++) {
		if (strcmp(argv[1], tests[i].name) == 0) {
			test = &tests[i];
			break;
		}
	}
	if (test == NULL)
		print_usage_and_exit(argv[0]);
	if (strcmp(argv[2], "ffs") == 0)
		prim_restart_index = 0xff;
	else if (strcmp(argv[2], "other") == 0)
		prim_restart_index = 0x80;
	else
		print_usage_and_exit(argv[0]);

	/* Compile shaders */
	prog = glCreateProgram();
	vs = piglit_compile_shader_text(GL_VERTEX_SHADER, vs_text);
	glAttachShader(prog, vs);
	asprintf(&gs_text, gs_template, test->input_layout,
		 test->vertices_per_prim);
	gs = piglit_compile_shader_text(GL_GEOMETRY_SHADER, gs_text);
	free(gs_text);
	glAttachShader(prog, gs);
	glTransformFeedbackVaryings(prog, test->vertices_per_prim, varyings,
				    GL_INTERLEAVED_ATTRIBS);
	glLinkProgram(prog);
	if (!piglit_link_check_status(prog))
		piglit_report_result(PIGLIT_FAIL);
	glUseProgram(prog);

	/* Set up other GL state */
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glGenBuffers(1, &xfb_buf);
	glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, xfb_buf);
	glBufferData(GL_TRANSFORM_FEEDBACK_BUFFER,
		     MAX_OUTPUT_VERTICES_PER_STRIP * 2 * sizeof(GLint), NULL,
		     GL_STREAM_READ);
	glGenQueries(1, &generated_query);
	glEnable(GL_RASTERIZER_DISCARD);
	glEnable(GL_PRIMITIVE_RESTART);
	glPrimitiveRestartIndex(prim_restart_index);

	/* Set up element buffer containing:
	 *
	 * [0, 1, ..., input_vertices_per_strip-1, prim_restart_index,
	 *  0, 1, ..., input_vertices_per_strip-1]
	 */
	num_input_elements = test->input_vertices_per_strip * 2 + 1;
	glGenBuffers(1, &element_buf);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_buf);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER,
		     sizeof(GLubyte) * num_input_elements, NULL,
		     GL_STATIC_DRAW);
	elements = glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_READ_WRITE);
	for (i = 0; i < test->input_vertices_per_strip; i++) {
		elements[i] = i;
		elements[i + test->input_vertices_per_strip + 1] = i;
	}
	elements[test->input_vertices_per_strip] = prim_restart_index;
	glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);

	/* Run vertices through the pipeline */
	glBeginQuery(GL_PRIMITIVES_GENERATED, generated_query);
	glBeginTransformFeedback(GL_POINTS);
	glDrawElements(test->prim_type, num_input_elements, GL_UNSIGNED_BYTE,
		       NULL);
	glEndTransformFeedback();
	glEndQuery(GL_PRIMITIVES_GENERATED);

	/* Check that the GS got invoked the right number of times */
	glGetQueryObjectuiv(generated_query, GL_QUERY_RESULT,
			    &primitives_generated);
	if (primitives_generated != 6) {
		printf("Expected 6 GS invocations, got %d\n",
		       primitives_generated);
		pass = false;
	}
	expected_output_points_per_strip = 3 * test->vertices_per_prim;
	actual_output_points = primitives_generated * test->vertices_per_prim;

	/* Check the data output by the GS.  The expected output is
	 * two exact copies of test->expected_results, one for each
	 * strip.
	 */
	readback = glMapBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, GL_READ_ONLY);
	if (memcmp(readback, test->expected_results,
		   expected_output_points_per_strip * sizeof(GLint)) != 0) {
		pass = false;
	}
	if (memcmp(readback + expected_output_points_per_strip,
		   test->expected_results,
		   expected_output_points_per_strip * sizeof(GLint)) != 0) {
		pass = false;
	}

	/* Output details if the result was wrong */
	if (!pass) {
		printf("Expected vertex IDs:");
		for (i = 0; i < 2; i++)
			for (j = 0; j < expected_output_points_per_strip; j++)
				printf(" %d", test->expected_results[j]);
		printf("\n");
		printf("Actual vertex IDs:");
		for (i = 0; i < actual_output_points; i++)
			printf(" %d", readback[i]);
		printf("\n");
	}

	glUnmapBuffer(GL_TRANSFORM_FEEDBACK_BUFFER);

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}


enum piglit_result
piglit_display(void)
{
	/* Should never be reached */
	return PIGLIT_FAIL;
}
