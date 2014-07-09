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
 * \file primitive-types.c
 *
 * Verify that the geometry shader is invoked the proper number of
 * times, and input vertices are delivered in the proper order, for
 * all input primitive types.
 *
 * This test uses a simple geometry shader that copies the gl_VertexID
 * + 1 from each of its inputs to an output array, and then captures
 * the result using transform feedback (gl_VertexID + 1 is used
 * because this corresponds to the 1-based numbering used in the
 * OpenGL spec: see section 2.6.1 (Primitive Types) of the GL 3.2 core
 * spec).  The resulting data is checked in C to make sure it matches
 * the expected sequence of vertices.
 *
 * As an incidental side effect, this test verifies that the
 * implementation assigns the correct input array size for each input
 * primitive type (since geometry shader compilation would fail if it
 * didn't).
 */


#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 32;
	config.supports_gl_core_version = 32;

PIGLIT_GL_TEST_CONFIG_END


#define MAX_OUTPUT_VERTICES 24


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


struct test_vector
{
	/** Number of vertices to send down the pipeline */
	unsigned num_input_vertices;

	/** Number of GS invocations expected */
	unsigned expected_gs_invocations;

	/**
	 * Vertices that each GS invocation is expected to see.
	 */
	GLint expected_results[MAX_OUTPUT_VERTICES];
};

static const struct test_vector points_tests[] = {
	{ 0, 0, { 0 } },
	{ 1, 1, { 1 } },
	{ 2, 2, { 1, 2 } },
};

static const struct test_vector line_loop_tests[] = {
	{ 1, 0, { 0 } },
	{ 2, 2, { 1, 2,
		  2, 1 } },
	{ 3, 3, { 1, 2,
		  2, 3,
		  3, 1 } },
	{ 4, 4, { 1, 2,
		  2, 3,
		  3, 4,
		  4, 1 } },
};

static const struct test_vector line_strip_tests[] = {
	{ 1, 0, { 0 } },
	{ 2, 1, { 1, 2 } },
	{ 3, 2, { 1, 2,
		  2, 3 } },
	{ 4, 3, { 1, 2,
		  2, 3,
		  3, 4 } },
};

static const struct test_vector lines_tests[] = {
	{ 1, 0, { 0 } },
	{ 2, 1, { 1, 2 } },
	{ 3, 1, { 1, 2 } },
	{ 4, 2, { 1, 2,
		  3, 4 } },
};

static const struct test_vector triangles_tests[] = {
	{ 2, 0, { 0 } },
	{ 3, 1, { 1, 2, 3 } },
	{ 5, 1, { 1, 2, 3 } },
	{ 6, 2, { 1, 2, 3,
		  4, 5, 6 } },
};

static const struct test_vector triangle_strip_tests[] = {
	{ 2, 0, { 0 } },
	{ 3, 1, { 1, 2, 3 } },
	{ 4, 2, { 1, 2, 3,
		  3, 2, 4 } },
	{ 5, 3, { 1, 2, 3,
		  3, 2, 4,
		  3, 4, 5 } },
};

static const struct test_vector triangle_fan_tests[] = {
	{ 2, 0, { 0 } },
	{ 3, 1, { 1, 2, 3 } },
	{ 4, 2, { 1, 2, 3,
		  1, 3, 4 } },
	{ 5, 3, { 1, 2, 3,
		  1, 3, 4,
		  1, 4, 5 } },
};

static const struct test_vector lines_adjacency_tests[] = {
	{ 3, 0, { 0 } },
	{ 4, 1, { 1, 2, 3, 4 } },
	{ 7, 1, { 1, 2, 3, 4 } },
	{ 8, 2, { 1, 2, 3, 4,
		  5, 6, 7, 8 } },
};

static const struct test_vector line_strip_adjacency_tests[] = {
	{ 3, 0, { 0 } },
	{ 4, 1, { 1, 2, 3, 4 } },
	{ 5, 2, { 1, 2, 3, 4,
		  2, 3, 4, 5 } },
	{ 6, 3, { 1, 2, 3, 4,
		  2, 3, 4, 5,
		  3, 4, 5, 6 } },
};

static const struct test_vector triangles_adjacency_tests[] = {
	{ 5, 0, { 0 } },
	{ 6, 1, { 1, 2, 3, 4, 5, 6 } },
	{ 11, 1, { 1, 2, 3, 4, 5, 6 } },
	{ 12, 2, { 1, 2, 3, 4, 5, 6,
		   7, 8, 9, 10, 11, 12 } },
};

/* Note: the required vertex order is surprisingly non-obvious for
 * GL_TRIANGLE_STRIP_ADJACENCY.
 *
 * Table 2.4 in the GL 3.2 core spec (Triangles generated by triangle
 * strips with adjacency) defines how the vertices in the triangle
 * strip are to be interpreted:
 *
 *                               Primitive Vertices  Adjacent Vertices
 *     Primitive                 1st   2nd   3rd     1/2   2/3   3/1
 *     only (i = 0, n = 1)        1     3     5       2     6     4
 *     first (i = 0)              1     3     5       2     7     4
 *     middle (i odd)            2i+3  2i+1  2i+5    2i-1  2i+4  2i+7
 *     middle (i even)           2i+1  2i+3  2i+5    2i-1  2i+7  2i+4
 *     last (i = n - 1, i odd)   2i+3  2i+1  2i+5    2i-1  2i+4  2i+6
 *     last (i = n - 1, i even)  2i+1  2i+3  2i+5    2i-1  2i+6  2i+4
 *
 * But it does not define the order in which these vertices should be
 * delivered to the geometry shader.  That's defined in section 2.12.1
 * of the GL 3.2 core spec (Geometry Shader Input Primitives):
 *
 *     Geometry shaders that operate on triangles with adjacent
 *     vertices are valid for the TRIANGLES_ADJACENCY and
 *     TRIANGLE_STRIP_ADJACENCY primitive types. There are six
 *     vertices available for each program invocation. The first,
 *     third and fifth vertices refer to attributes of the first,
 *     second and third vertex of the triangle, respectively. The
 *     second, fourth and sixth vertices refer to attributes of the
 *     vertices adjacent to the edges from the first to the second
 *     vertex, from the second to the third vertex, and from the third
 *     to the first vertex, respectively.
 *
 * Therefore the order in which the columns of table 2.4 should be
 * read is 1st, 1/2, 2nd, 2/3, 3rd, 3/1.
 *
 * So, for example, in the case where there is just a single triangle
 * delivered to the pipeline, we consult the first row of table 2.4 to
 * find:
 *
 *     Primitive Vertices  Adjacent Vertices
 *     1st   2nd   3rd     1/2   2/3   3/1
 *      1     3     5       2     6     4
 *
 * Rearranging into the order that should be delivered to the geometry
 * shader, we get:
 *
 *     1st   1/2   2nd   2/3   3rd   3/1
 *      1     2     3     6     5     4
 */
static const struct test_vector triangle_strip_adjacency_tests[] = {
	{ 5, 0, { 0 } },
	{ 6, 1, { 1, 2, 3, 6, 5, 4 } },
	{ 7, 1, { 1, 2, 3, 6, 5, 4 } },
	{ 8, 2, { 1, 2, 3, 7, 5, 4,
		  5, 1, 3, 6, 7, 8 } },
	{ 9, 2, { 1, 2, 3, 7, 5, 4,
		  5, 1, 3, 6, 7, 8 } },
	{ 10, 3, { 1, 2, 3, 7, 5, 4,
		   5, 1, 3, 6, 7, 9,
		   5, 3, 7, 10, 9, 8 } },
	{ 11, 3, { 1, 2, 3, 7, 5, 4,
		   5, 1, 3, 6, 7, 9,
		   5, 3, 7, 10, 9, 8 } },
	{ 12, 4, { 1, 2, 3, 7, 5, 4,
		   5, 1, 3, 6, 7, 9,
		   5, 3, 7, 11, 9, 8,
		   9, 5, 7, 10, 11, 12 } },
};


static const struct test_set
{
	const char *name;
	GLenum prim_type;
	const char *input_layout;
	unsigned vertices_per_prim;
	unsigned num_test_vectors;
	const struct test_vector *test_vectors;
} tests[] = {
#define TEST(prim_type, input_layout, vertices_per_prim, test_array) \
	{ #prim_type, prim_type, input_layout, vertices_per_prim, \
	  ARRAY_SIZE(test_array), test_array }
	TEST(GL_POINTS, "points", 1, points_tests),
	TEST(GL_LINE_LOOP, "lines", 2, line_loop_tests),
	TEST(GL_LINE_STRIP, "lines", 2, line_strip_tests),
	TEST(GL_LINES, "lines", 2, lines_tests),
	TEST(GL_TRIANGLES, "triangles", 3, triangles_tests),
	TEST(GL_TRIANGLE_STRIP, "triangles", 3, triangle_strip_tests),
	TEST(GL_TRIANGLE_FAN, "triangles", 3, triangle_fan_tests),
	TEST(GL_LINES_ADJACENCY, "lines_adjacency", 4, lines_adjacency_tests),
	TEST(GL_LINE_STRIP_ADJACENCY, "lines_adjacency", 4,
	     line_strip_adjacency_tests),
	TEST(GL_TRIANGLES_ADJACENCY, "triangles_adjacency", 6,
	     triangles_adjacency_tests),
	TEST(GL_TRIANGLE_STRIP_ADJACENCY, "triangles_adjacency", 6,
	     triangle_strip_adjacency_tests),
#undef TEST
};


static GLuint generated_query;


static void
print_usage_and_exit(const char *prog_name)
{
	int i;
	printf("Usage: %s <primitive>\n"
	       "  where <primitive> is one of the following:\n", prog_name);
	for (i = 0; i < ARRAY_SIZE(tests); i++)
		printf("    %s\n", tests[i].name);
	piglit_report_result(PIGLIT_FAIL);
}


static bool
do_test_vector(const struct test_set *test, const struct test_vector *vector)
{
	GLuint primitives_generated;
	int i;
	const GLint *readback;
	unsigned expected_output_points;
	unsigned actual_output_points;
	bool pass = true;

	printf("Testing %s(%d vertices)\n", test->name,
	       vector->num_input_vertices);

	/* Run vertices through the pipeline */
	glBeginQuery(GL_PRIMITIVES_GENERATED, generated_query);
	glBeginTransformFeedback(GL_POINTS);
	glDrawArrays(test->prim_type, 0, vector->num_input_vertices);
	glEndTransformFeedback();
	glEndQuery(GL_PRIMITIVES_GENERATED);

	/* Check that the GS got invoked the right number of times */
	glGetQueryObjectuiv(generated_query, GL_QUERY_RESULT,
			    &primitives_generated);
	if (primitives_generated != vector->expected_gs_invocations) {
		printf("  Expected %d GS invocations, got %d\n",
		       vector->expected_gs_invocations, primitives_generated);
		pass = false;
	}
	expected_output_points =
		vector->expected_gs_invocations * test->vertices_per_prim;
	actual_output_points = primitives_generated * test->vertices_per_prim;

	/* Check the data output by the GS */
	readback = glMapBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, GL_READ_ONLY);
	if (memcmp(readback, vector->expected_results,
		   expected_output_points * sizeof(GLint)) != 0) {
		pass = false;
	}

	/* Output details if the result was wrong */
	if (!pass) {
		printf("  Expected vertex IDs:");
		for (i = 0; i < expected_output_points; i++)
			printf(" %d", vector->expected_results[i]);
		printf("\n");
		printf("  Actual vertex IDs:  ");
		for (i = 0; i < actual_output_points; i++)
			printf(" %d", readback[i]);
		printf("\n");
	}

	glUnmapBuffer(GL_TRANSFORM_FEEDBACK_BUFFER);

	return pass;
}


void
piglit_init(int argc, char **argv)
{
	int i;
	const struct test_set *test = NULL;
	GLuint prog, vs, gs, vao, xfb_buf;
	char *gs_text;
	bool pass = true;

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
		     MAX_OUTPUT_VERTICES * sizeof(GLint), NULL,
		     GL_STREAM_READ);
	glGenQueries(1, &generated_query);
	glEnable(GL_RASTERIZER_DISCARD);

	for (i = 0; i < test->num_test_vectors; i++) {
		pass = do_test_vector(test, &test->test_vectors[i]) && pass;
	}

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}


enum piglit_result
piglit_display(void)
{
	/* Should never be reached */
	return PIGLIT_FAIL;
}
