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
 * \file primitive-id-restart.c
 *
 * Verify that the use of primitive restart does not reset the
 * geometry shader's gl_PrimitiveID counter.
 *
 * From the GL 3.2 core spec, section 2.12.4 (Geometry Shader
 * Execution Environment):
 *
 *     Restarting a primitive topology using the primitive restart
 *     index has no effect on the primitive ID counter.
 *
 * This test uses a simple geometry shader that copies
 * gl_PrimitiveIDIn to a single output, which is captured using
 * transform feedback.
 *
 * The test operates by specifying a sequence of:
 *
 * - One vertex followed by primitive restart
 * - Two vertices followed by primitive restart
 * - Three vertices followed by primitive restart
 *
 * And so on up to twelve.  The resulting transform feedback output is
 * checked to verify that the primitive ID's received by the geometry
 * shaders count upwards from 0, without restarting anywhere.
 *
 * Note: some generations of Intel hardware require primitive restart
 * to be emulated in software when either:
 *
 * - certain primitive types are used, or
 *
 * - the primitive restart index is not all 0xff's.
 *
 * To make sure that both the hardware and software primitive restart
 * codepaths are tested, this test accepts command line options to
 * control (a) which primitive type to use, and (b) whether the
 * primitive restart index should be all 0xff's.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 32;
	config.supports_gl_core_version = 32;

PIGLIT_GL_TEST_CONFIG_END


/**
 * Test up to 12 vertices both before and after primitive restart,
 * since that ensures that there are at least two primitives before
 * primitive restart in all drawing modes.
 */
#define LONGEST_INPUT_SEQUENCE 12

/* Sum of 2, 3, ..., LONGEST_INPUT_SEQUENCE + 1. */
#define NUM_ELEMENTS \
	(LONGEST_INPUT_SEQUENCE * (LONGEST_INPUT_SEQUENCE + 3) / 2)

/* Sum of 1, 2, ..., LONGEST_INPUT_SEQUENCE. */
#define MAX_TOTAL_PRIMS \
	(LONGEST_INPUT_SEQUENCE * (LONGEST_INPUT_SEQUENCE + 1) / 2)


static const char *vs_text =
	"#version 150\n"
	"\n"
	"void main()\n"
	"{\n"
	"}\n";

static const char *gs_template =
	"#version 150\n"
	"layout(%s) in;\n"
	"layout(points, max_vertices = 1) out;\n"
	"\n"
	"out int primitive_id;\n"
	"\n"
	"void main()\n"
	"{\n"
	"  primitive_id = gl_PrimitiveIDIn;\n"
	"  EmitVertex();\n"
	"}\n";

static const char *varyings[] = { "primitive_id" };


struct prim_type_info
{
	const char *name;
	GLenum prim_type;
	const char *input_layout;
} prim_types[] = {
#define PRIM_TYPE(prim_type, input_layout) \
	{ #prim_type, prim_type, input_layout }
	PRIM_TYPE(GL_POINTS, "points"),
	PRIM_TYPE(GL_LINE_LOOP, "lines"),
	PRIM_TYPE(GL_LINE_STRIP, "lines"),
	PRIM_TYPE(GL_LINES, "lines"),
	PRIM_TYPE(GL_TRIANGLES, "triangles"),
	PRIM_TYPE(GL_TRIANGLE_STRIP, "triangles"),
	PRIM_TYPE(GL_TRIANGLE_FAN, "triangles"),
	PRIM_TYPE(GL_LINES_ADJACENCY, "lines_adjacency"),
	PRIM_TYPE(GL_LINE_STRIP_ADJACENCY, "lines_adjacency"),
	PRIM_TYPE(GL_TRIANGLES_ADJACENCY, "triangles_adjacency"),
	PRIM_TYPE(GL_TRIANGLE_STRIP_ADJACENCY, "triangles_adjacency"),
#undef PRIM_TYPE
};


static void
print_usage_and_exit(const char *prog_name)
{
	int i;
	printf("Usage: %s <primitive> <restart-index>\n"
	       "  where <primitive> is one of the following:\n", prog_name);
	for(i = 0; i < ARRAY_SIZE(prim_types); i++)
		printf("    %s\n", prim_types[i].name);
	printf("  and <restart-index> is one of the following:\n"
	       "    ffs - use a primitive restart index that is all 0xffs\n"
	       "    other - use a different primitive restart index\n");
	piglit_report_result(PIGLIT_FAIL);
}


void
piglit_init(int argc, char **argv)
{
	GLenum prim_type = 0;
	const char *input_layout = NULL;
	GLubyte prim_restart_index;
	char *gs_text;
	GLuint prog, vs, gs, vao, xfb_buf, element_buf, generated_query,
		primitives_generated;
	GLubyte *elements;
	int i, j;
	GLsizei num_elements;
	bool pass = true;
	GLuint *readback;

	/* Parse params */
	if (argc != 3)
		print_usage_and_exit(argv[0]);
	for (i = 0; i < ARRAY_SIZE(prim_types); i++) {
		if (strcmp(argv[1], prim_types[i].name) == 0) {
			prim_type = prim_types[i].prim_type;
			input_layout = prim_types[i].input_layout;
			break;
		}
	}
	if (input_layout == NULL)
		print_usage_and_exit(argv[0]);
	if (strcmp(argv[2], "ffs") == 0)
		prim_restart_index = 0xff;
	else if (strcmp(argv[2], "other") == 0)
		prim_restart_index = 1;
	else
		print_usage_and_exit(argv[0]);

	/* Compile shaders */
	prog = glCreateProgram();
	vs = piglit_compile_shader_text(GL_VERTEX_SHADER, vs_text);
	glAttachShader(prog, vs);
	asprintf(&gs_text, gs_template, input_layout);
	gs = piglit_compile_shader_text(GL_GEOMETRY_SHADER, gs_text);
	free(gs_text);
	glAttachShader(prog, gs);
	glTransformFeedbackVaryings(prog, 1, varyings, GL_INTERLEAVED_ATTRIBS);
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
		     MAX_TOTAL_PRIMS * sizeof(GLint), NULL,
		     GL_STREAM_READ);
	glGenQueries(1, &generated_query);
	glEnable(GL_RASTERIZER_DISCARD);
	glEnable(GL_PRIMITIVE_RESTART);
	glPrimitiveRestartIndex(prim_restart_index);

	/* Set up element buffer */
	glGenBuffers(1, &element_buf);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_buf);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER,
		     sizeof(GLubyte) * NUM_ELEMENTS, NULL,
		     GL_STATIC_DRAW);
	elements = glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_READ_WRITE);
	num_elements = 0;
	for (i = 1; i <= LONGEST_INPUT_SEQUENCE; i++) {
		for (j = 0; j < i; j++) {
			/* Every element that isn't the primitive restart index
			 * can have any value as far as it is not the primitive
			 * restart index since we don't care about the actual
			 * vertex data.
			 *
			 * NOTE: repeating the indices for all elements but the
			 * primitive restart index causes a GPU hang in Intel's
			 * Sandy Bridge platform, likely due to a hardware bug,
			 * so make sure that we do not repeat the indices.
			 *
			 * More information:
			 *
			 * http://lists.freedesktop.org/archives/mesa-dev/2014-July/064221.html
			 */
			elements[num_elements++] =
				j != prim_restart_index ? j : j + 1;
		}
		elements[num_elements++] = prim_restart_index;
	}
	glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);

	/* Run vertices through the pipeline */
	glBeginQuery(GL_PRIMITIVES_GENERATED, generated_query);
	glBeginTransformFeedback(GL_POINTS);
	glDrawElements(prim_type, num_elements, GL_UNSIGNED_BYTE, NULL);
	glEndTransformFeedback();
	glEndQuery(GL_PRIMITIVES_GENERATED);

	/* Find out how many times the GS got invoked so we'll know
	 * how many transform feedback outputs to examine.
	 */
	glGetQueryObjectuiv(generated_query, GL_QUERY_RESULT,
			    &primitives_generated);
	if (primitives_generated > MAX_TOTAL_PRIMS) {
		printf("Expected no more than %d primitives, got %d\n",
		       MAX_TOTAL_PRIMS, primitives_generated);
		pass = false;

		/* Set primitives_generated to MAX_TOTAL_PRIMS so that
		 * we don't overflow the buffer in the loop below.
		 */
		primitives_generated = MAX_TOTAL_PRIMS;
	}

	/* Check transform feedback outputs. */
	readback = glMapBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, GL_READ_ONLY);
	for (i = 0; i < primitives_generated; i++) {
		if (readback[i] != i) {
			printf("Expected primitive %d to have gl_PrimitiveIDIn"
			       " = %d, got %d instead\n", i, i, readback[i]);
			pass = false;
		}
	}
	glUnmapBuffer(GL_TRANSFORM_FEEDBACK_BUFFER);

	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}


enum piglit_result
piglit_display(void)
{
	/* Should never be reached */
	return PIGLIT_FAIL;
}
