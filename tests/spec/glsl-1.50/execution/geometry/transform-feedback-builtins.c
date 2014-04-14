/*
 * Copyright © 2014 Intel Corporation
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

/** \file transform-feedback-builtins.c
 *
 * Intel's i965 driver has a special internal handling for 'gl_Layer' and
 * 'gl_ViewportIndex' builtin variables. This test verifies that transform
 * feedback works fine for these builtin variables.
 *
 * Test creates a geometry shader which outputs the following builtin variables:
 * - gl_Layer
 * - gl_ViewportIndex
 *
 * Then it verifies that data captured by transform feedback is consistent with
 * the assignments in geometry shader.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN
	config.supports_gl_compat_version = 32;
	config.supports_gl_core_version = 32;
PIGLIT_GL_TEST_CONFIG_END

#define NUM_OUTPUT_INTS 3
static const char *vstext =
	"#version 150\n"
	"void main()\n"
	"{\n"
        "  gl_Position = vec4(0.0);\n"
	"}\n";

static const char *gs_template =
	"#version 150\n"
	"%s"
	"layout(triangles) in;\n"
	"layout(triangle_strip, max_vertices = 3) out;\n"
	"void main()\n"
	"{\n"
	"  for (int i = 0; i < 3; i++) {\n"
	"    gl_Layer = 2 * i + 1;\n"
	"%s"
	"    EmitVertex();\n"
	"  }\n"
	"  EndPrimitive();\n"
	"}\n";

/**
 * When below set of varyings are captured from the geometry shader
 * above, the output should be a sequence of integers defined in this
 * array.
 */
static const int expected[2][3] = {{1, 3, 5}, /* gl_Layer */
				   {1, 2, 3}}; /* gl_ViewportIndex */

static const char *varyings[] = {"gl_Layer", "gl_ViewportIndex"};

void
piglit_init(int argc, char **argv)
{
	int i, j, num_xfb_buffers;
	GLuint prog, vao, xfb_buf[2];
	const GLint *readback;
	char *gstext;
	bool pass = true;
	bool test_gl_ViewportIndex =
		piglit_is_extension_supported("GL_ARB_viewport_array");

	num_xfb_buffers = ARRAY_SIZE(xfb_buf);

	if (test_gl_ViewportIndex) {
		asprintf(&gstext, gs_template,
			 "#extension GL_ARB_viewport_array : require\n",
			 "    gl_ViewportIndex = i + 1;\n");
	} else {
		printf("Skip testing 'gl_ViewportIndex'\n");
		asprintf(&gstext, gs_template, "", "");
		num_xfb_buffers -= 1;
	}

	prog = piglit_build_simple_program_unlinked_multiple_shaders(
		GL_VERTEX_SHADER, vstext,
		GL_GEOMETRY_SHADER, gstext,
		0, NULL);

	glTransformFeedbackVaryings(prog, num_xfb_buffers, varyings,
				    GL_SEPARATE_ATTRIBS);

	glLinkProgram(prog);
	if (!piglit_link_check_status(prog)) {
		glDeleteProgram(prog);
		piglit_report_result(PIGLIT_FAIL);
	}
	glUseProgram(prog);

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	/* Setup transform feedback buffers */
	glGenBuffers(num_xfb_buffers, xfb_buf);
	for (i = 0; i < num_xfb_buffers; i++) {
		glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, i, xfb_buf[i]);
		glBufferData(GL_TRANSFORM_FEEDBACK_BUFFER,
			     NUM_OUTPUT_INTS * sizeof(GLint), NULL,
			     GL_STATIC_READ);
	}

	glEnable(GL_RASTERIZER_DISCARD);

	/* Do drawing */
	glBeginTransformFeedback(GL_TRIANGLES);
	glDrawArrays(GL_TRIANGLES, 0, 3);
	glEndTransformFeedback();

	/* Check if the correct data was written into the transform feedback
	 * buffers.
	 */
	for (i = 0; i < num_xfb_buffers; i++) {
		glBindBufferRange(GL_TRANSFORM_FEEDBACK_BUFFER, 0, xfb_buf[i], 0,
				  NUM_OUTPUT_INTS * sizeof(GLint));
		readback = glMapBufferRange(GL_TRANSFORM_FEEDBACK_BUFFER, 0,
					    NUM_OUTPUT_INTS * sizeof(GLint),
					    GL_MAP_READ_BIT);
		for (j = 0; j < NUM_OUTPUT_INTS; j++) {
			if (readback[j] != expected[i][j]) {
				printf("Incorrect data for '%s' output %d."
				       "  Expected %d, got %d.\n", varyings[i],
				       j, expected[i][j], readback[j]);
				pass = false;
			}
		}
		glUnmapBuffer(GL_TRANSFORM_FEEDBACK_BUFFER);
	}

	glDeleteBuffers(num_xfb_buffers, xfb_buf);

	/* Check for the errors */
	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;
	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}

enum piglit_result
piglit_display(void)
{
	/* Unreached */
	return PIGLIT_FAIL;
}
