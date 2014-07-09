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
 *
 * The optional argument "use_gs" causes the test to use a geometry
 * shader.  When this argument is given, the number of vertices output
 * by the geometry shader is in general different from the number of
 * vertices sent down the pipeline by the glDrawArrays() command.
 * Thus, the test verifies that the implementation does overflow
 * checking based on the post-geometry-shader vertex count.
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

#define XFB_BUFFER_SIZE 12
#define MAX_VERTICES 9

/**
 * Vertex shader used when use_gs is false.
 */
static const char *vstext_nogs =
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

/**
 * Vertex shader used when use_gs is true.
 */
static const char *vstext_gs =
	"#version 150\n"
	"\n"
	"void main()\n"
	"{\n"
	"}\n";

/**
 * Geometry shader used when use_gs is true.
 */
static const char *gstext_gs =
	"#version 150\n"
	"layout(points) in;\n"
	"layout(%s, max_vertices=9) out;\n"
	"uniform int num_primitives;\n"
	"uniform int vertices_per_prim;\n"
	"out float varying1;\n"
	"out float varying2;\n"
	"\n"
	"void main()\n"
	"{\n"
	"  int vertex_num = 0;\n"
	"  for (int i = 0; i < num_primitives; i++) {\n"
	"    for (int j = 0; j < vertices_per_prim; j++) {\n"
	"      varying1 = 100.0 + float(vertex_num);\n"
	"      varying2 = 200.0 + float(vertex_num);\n"
	"      vertex_num++;\n"
	"      EmitVertex();\n"
	"    }\n"
	"    EndPrimitive();\n"
	"  }\n"
	"}\n";


static const char *varyings[] = { "varying1", "varying2" };

static GLuint xfb_buf, vao, array_buf;
static GLuint progs[3][2]; /* indexed by (mode, num_varyings - 1) */
static GLuint query_prims_generated;
static GLuint query_prims_written;

static GLenum modes[] = { GL_POINTS, GL_LINES, GL_TRIANGLES };
static const char *mode_names[] = {
	"GL_POINTS", "GL_LINES", "GL_TRIANGLES"
};
static const char *mode_gs_out_primtypes[] = {
	"points", "line_strip", "triangle_strip"
};

void
piglit_init(int argc, char **argv)
{
	GLuint vs, gs;
	int num_varyings;
	int mode;

	piglit_require_GLSL();
	piglit_require_transform_feedback();

	for (mode = 0; mode < ARRAY_SIZE(modes); mode++) {
		if (use_gs) {
			char *gstext;
			vs = piglit_compile_shader_text(GL_VERTEX_SHADER,
							vstext_gs);
			asprintf(&gstext, gstext_gs,
				 mode_gs_out_primtypes[mode]);
			gs = piglit_compile_shader_text(GL_GEOMETRY_SHADER,
							gstext);
		} else {
			vs = piglit_compile_shader_text(GL_VERTEX_SHADER,
							vstext_nogs);
		}
		for (num_varyings = 1; num_varyings <= 2; ++num_varyings) {
			GLuint prog = glCreateProgram();
			glAttachShader(prog, vs);
			if (use_gs)
				glAttachShader(prog, gs);
			else
				glBindAttribLocation(prog, 0, "vertex_num");
			glTransformFeedbackVaryings(prog, num_varyings,
						    varyings,
						    GL_INTERLEAVED_ATTRIBS);
			glLinkProgram(prog);
			if (!piglit_link_check_status(prog)) {
				glDeleteProgram(prog);
				piglit_report_result(PIGLIT_FAIL);
			}
			progs[mode][num_varyings - 1] = prog;
		}
	}

	glGenBuffers(1, &xfb_buf);
	glGenBuffers(1, &array_buf);
	glGenQueries(1, &query_prims_generated);
	glGenQueries(1, &query_prims_written);
	if (piglit_is_extension_supported("GL_ARB_vertex_array_object") ||
	    piglit_get_gl_version() >= 30) {
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);
	}
}

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
	GLuint prog;

	printf("size=%d, num_varyings=%d, num_primitives=%d, mode=%s: ",
	       bind_size, num_varyings, num_primitives,
	       mode_names[mode_index]);

	/* Setup program and initial buffer contents */
	prog = progs[mode_index][num_varyings - 1];
	glUseProgram(prog);
	if (use_gs) {
		glUniform1i(glGetUniformLocation(prog, "num_primitives"),
			    num_primitives);
		glUniform1i(glGetUniformLocation(prog, "vertices_per_prim"),
			    vertices_per_prim);
	} else {
		for (i = 0; i < MAX_VERTICES; ++i)
			vertex_data[i] = i;
		glBindBuffer(GL_ARRAY_BUFFER, array_buf);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_data),
			     &vertex_data, GL_STATIC_DRAW);
		glVertexAttribPointer(0, 1, GL_FLOAT, GL_FALSE, sizeof(float), NULL);
		glEnableVertexAttribArray(0);
	}
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
	if (use_gs) {
		glDrawArrays(GL_POINTS, 0, 1);
	} else {
		glDrawArrays(modes[mode_index], 0,
			     num_primitives * vertices_per_prim);
	}

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
