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
 * \file overflow-edge-cases.c
 *
 * Verify edge cases of transform feedback overflow checking.
 *
 * This test exercises all possible combinations of the following four
 * variables:
 *
 * - Size passed to glBindBufferRange (1-6 floats)
 * - Number of transform feedback varying components (1 or 2)
 * - Number of primitives drawn (1-3)
 * - Primitive mode (GL_POINTS, GL_LINES, or GL_TRIANGLES)
 *
 * In all cases, it verifies that:
 *
 * - The proper values were written to the transform feedback buffer.
 * - GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN is set correctly.
 * - GL_PRIMITIVES_GENERATED is set correctly.
 */

#include "piglit-util-gl-common.h"

PIGLIT_GL_TEST_MAIN(
    16 /*window_width*/,
    16 /*window_height*/,
    GLUT_DOUBLE | GLUT_RGB)

#define XFB_BUFFER_SIZE 12
#define MAX_VERTICES 9

static const char *vstext =
	"attribute float vertex_num;\n"
	"varying float varying1;\n"
	"varying float varying2;\n"
	"\n"
	"void main()\n"
	"{\n"
	"  gl_Position = vec4(vertex_num);\n"
	"  varying1 = 100.0 + vertex_num;\n"
	"  varying2 = 200.0 + vertex_num;\n"
	"}\n";

static const char *varyings[] = { "varying1", "varying2" };

static GLuint xfb_buf;
static GLuint progs[2]; /* indexed by num_varyings - 1 */
static GLuint query_prims_generated;
static GLuint query_prims_written;

void
piglit_init(int argc, char **argv)
{
	GLuint vs;
	int num_varyings;

	piglit_require_GLSL();
	piglit_require_transform_feedback();

	vs = piglit_compile_shader_text(GL_VERTEX_SHADER, vstext);
	for (num_varyings = 1; num_varyings <= 2; ++num_varyings) {
		GLuint prog = piglit_CreateProgram();
		piglit_AttachShader(prog, vs);
		piglit_BindAttribLocation(prog, 0, "vertex_num");
		glTransformFeedbackVaryings(prog, num_varyings, varyings,
					    GL_INTERLEAVED_ATTRIBS);
		piglit_LinkProgram(prog);
		if (!piglit_link_check_status(prog)) {
			piglit_DeleteProgram(prog);
			piglit_report_result(PIGLIT_FAIL);
		}
		progs[num_varyings - 1] = prog;
	}

	glGenBuffers(1, &xfb_buf);
	glGenQueries(1, &query_prims_generated);
	glGenQueries(1, &query_prims_written);
}

static GLenum modes[] = { GL_POINTS, GL_LINES, GL_TRIANGLES };
static const char *mode_names[] = {
	"GL_POINTS", "GL_LINES", "GL_TRIANGLES"
};
static int mode_vertices_per_prim[] = { 1, 2, 3 };

static GLboolean
test(int bind_size, int num_varyings, int num_primitives, int mode_index)
{
	float initial_xfb_buf[XFB_BUFFER_SIZE];
	float vertex_data[MAX_VERTICES];
	int i, j;
	int vertices_per_prim = mode_vertices_per_prim[mode_index];
	int expected_primitives_written =
		MIN2(num_primitives,
		     bind_size / num_varyings / vertices_per_prim);
	int expected_vertices_written =
		expected_primitives_written * vertices_per_prim;
	GLuint query_result;
	GLboolean pass = GL_TRUE;
	float expected_xfb_results[XFB_BUFFER_SIZE];
	float *readback;

	printf("size=%d, num_varyings=%d, num_primitives=%d, mode=%s: ",
	       bind_size, num_varyings, num_primitives,
	       mode_names[mode_index]);

	/* Setup program and initial buffer contents */
	piglit_UseProgram(progs[num_varyings - 1]);
	for (i = 0; i < MAX_VERTICES; ++i)
		vertex_data[i] = i;
	glVertexAttribPointer(0, 1, GL_FLOAT, GL_FALSE, sizeof(float),
			      &vertex_data);
	glEnableVertexAttribArray(0);
	for (i = 0; i < XFB_BUFFER_SIZE; ++i)
		initial_xfb_buf[i] = 0.0;
	glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, xfb_buf);
	glBufferData(GL_TRANSFORM_FEEDBACK_BUFFER, sizeof(initial_xfb_buf),
		     initial_xfb_buf, GL_STREAM_READ);
	glBindBufferRange(GL_TRANSFORM_FEEDBACK_BUFFER, 0, xfb_buf, 0,
			  sizeof(float) * bind_size);

	/* Start queries and XFB */
	glBeginQuery(GL_PRIMITIVES_GENERATED, query_prims_generated);
	glBeginQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN,
		     query_prims_written);
	glBeginTransformFeedback(modes[mode_index]);

	/* Draw */
	glDrawArrays(modes[mode_index], 0, num_primitives * vertices_per_prim);

	/* Stop XFB and check queries */
	glEndTransformFeedback();
	glEndQuery(GL_PRIMITIVES_GENERATED);
	glGetQueryObjectuiv(query_prims_generated, GL_QUERY_RESULT,
			    &query_result);
	if (query_result != num_primitives) {
		printf("\n  Expected %u primitives generated, got %u\n",
		       (unsigned) num_primitives, query_result);
		pass = GL_FALSE;
	}
	glEndQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN);
	glGetQueryObjectuiv(query_prims_written, GL_QUERY_RESULT,
			    &query_result);
	if (query_result != expected_primitives_written) {
		printf("\n  Expected %u primitives written, got %u",
		       (unsigned) expected_primitives_written, query_result);
		pass = GL_FALSE;
	}

	/* Check transform feedback buffer */
	memcpy(expected_xfb_results, initial_xfb_buf, sizeof(initial_xfb_buf));
	for (i = 0; i < expected_vertices_written; ++i) {
		for (j = 0; j < num_varyings; ++j) {
			expected_xfb_results[i * num_varyings + j] =
				100.0 * (j + 1) + i;
		}
	}
	readback = glMapBuffer(GL_TRANSFORM_FEEDBACK_BUFFER,
			       GL_READ_ONLY);
	for (i = 0; i < XFB_BUFFER_SIZE; ++i) {
		if (expected_xfb_results[i] != readback[i]) {
			printf("\n  Expected buf[%i] = %f, got %f",
			       i, expected_xfb_results[i], readback[i]);
			pass = GL_FALSE;
		}
	}

	if (pass)
		printf("PASS\n");
	else
		printf("  FAIL\n");

	return pass;
}

enum piglit_result
piglit_display(void)
{
	GLboolean pass = GL_TRUE;
	int bind_size, num_varyings, num_primitives, mode_index;

	for (bind_size = 1; bind_size <= 6; ++bind_size) {
		for (num_varyings = 1; num_varyings <= 2; ++num_varyings) {
			for (num_primitives = 1; num_primitives <= 3; ++num_primitives) {
				for (mode_index = 0; mode_index < 3; ++mode_index) {
					pass = test(bind_size,
						    num_varyings,
						    num_primitives,
						    mode_index) && pass;
				}
			}
		}
	}

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}
