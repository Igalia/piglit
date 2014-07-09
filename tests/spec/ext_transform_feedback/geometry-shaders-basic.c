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
 * \file geometry-shaders-basic.c
 *
 * Verify basic functionality of transform feedback when a geometry
 * shader is in use.
 *
 * This test checks that:
 *
 * - The number of primitives counted by GL_PRIMITIVES_GENERATED and
 *   GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN is based on the number
 *   of geometry shader output vertices (rather than the number of
 *   primitives sent down the pipeline).
 *
 * - Data output by the geometry shader is properly recorded in the
 *   transform feedback buffer.
 */

#include "piglit-util-gl.h"

#define GEOM_OUT_VERTS 10

PIGLIT_GL_TEST_CONFIG_BEGIN
	config.supports_gl_compat_version = 32;
	config.supports_gl_core_version = 32;
PIGLIT_GL_TEST_CONFIG_END

static const char *vstext =
	"#version 150\n"
	"in int vertex_count;\n"
	"out int vertex_count_to_gs;\n"
	"\n"
	"void main()\n"
	"{\n"
	"  vertex_count_to_gs = vertex_count;\n"
	"}\n";

static const char *gstext =
	"#version 150\n"
	"layout(points) in;\n"
	"layout(points, max_vertices=10) out;\n"
	"in int vertex_count_to_gs[1];\n"
	"out int vertex_id;\n"
	"\n"
	"void main()\n"
	"{\n"
	"  for (int i = 0; i < vertex_count_to_gs[0]; i++) {\n"
	"    vertex_id = i;\n"
	"    EmitVertex();\n"
	"  }\n"
	"}\n";

static const char *varyings[] = { "vertex_id" };

void
piglit_init(int argc, char **argv)
{
	GLint vertex_data[1] = { GEOM_OUT_VERTS };
	bool pass = true;
	GLuint vs, gs, prog, array_buf, query_result, xfb_buf, generated_query,
		written_query, vao;
	GLint vertex_count_loc;
	const GLint *readback;
	int i;

	vs = piglit_compile_shader_text(GL_VERTEX_SHADER, vstext);
	gs = piglit_compile_shader_text(GL_GEOMETRY_SHADER, gstext);
	prog = glCreateProgram();
	glAttachShader(prog, vs);
	glAttachShader(prog, gs);
	glTransformFeedbackVaryings(prog, 1, varyings, GL_INTERLEAVED_ATTRIBS);
	glLinkProgram(prog);
	if (!piglit_link_check_status(prog)) {
		glDeleteProgram(prog);
		piglit_report_result(PIGLIT_FAIL);
	}
	glUseProgram(prog);

	/* Setup inputs */
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glGenBuffers(1, &array_buf);
	glBindBuffer(GL_ARRAY_BUFFER, array_buf);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_data), &vertex_data,
		     GL_STATIC_DRAW);
	vertex_count_loc = glGetAttribLocation(prog, "vertex_count");
	glVertexAttribIPointer(vertex_count_loc, 1, GL_INT, sizeof(GLint), 0);
	glEnableVertexAttribArray(vertex_count_loc);

	/* Setup transform feedback buffer */
	glGenBuffers(1, &xfb_buf);
	glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, xfb_buf);
	glBufferData(GL_TRANSFORM_FEEDBACK_BUFFER,
		     GEOM_OUT_VERTS * sizeof(GLint), NULL, GL_STREAM_READ);
	glBindBufferRange(GL_TRANSFORM_FEEDBACK_BUFFER, 0, xfb_buf, 0,
			  GEOM_OUT_VERTS * sizeof(GLint));

	/* Setup queries */
	glGenQueries(1, &generated_query);
	glGenQueries(1, &written_query);
	glBeginQuery(GL_PRIMITIVES_GENERATED, generated_query);
	glBeginQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN, written_query);

	/* Do drawing */
	glBeginTransformFeedback(GL_POINTS);
	glDrawArrays(GL_POINTS, 0, 1);
	glEndTransformFeedback();

	/* Check query results */
	glEndQuery(GL_PRIMITIVES_GENERATED);
	glEndQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN);
	glGetQueryObjectuiv(generated_query, GL_QUERY_RESULT, &query_result);
	if (query_result != GEOM_OUT_VERTS) {
		printf("GL_PRIMITIVES_GENERATED query failed."
		       "  Expected %d, got %d.\n", GEOM_OUT_VERTS,
		       query_result);
		pass = false;
	}
	glGetQueryObjectuiv(written_query, GL_QUERY_RESULT, &query_result);
	if (query_result != GEOM_OUT_VERTS) {
		printf("GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN query failed."
		       "  Expected %d, got %d.\n", GEOM_OUT_VERTS,
		       query_result);
		pass = false;
	}

	/* Check transform feedback data */
	readback = glMapBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, GL_READ_ONLY);
	for (i = 0; i < GEOM_OUT_VERTS; i++) {
		if (readback[i] != i) {
			printf("Incorrect data for vertex %d."
			       "  Expected %d, got %d.", i, i, readback[i]);
			pass = false;
		}
	}
	glUnmapBuffer(GL_TRANSFORM_FEEDBACK_BUFFER);

	/* Check for errors */
	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}

enum piglit_result
piglit_display(void)
{
	/* Should never be reached */
	return PIGLIT_FAIL;
}
