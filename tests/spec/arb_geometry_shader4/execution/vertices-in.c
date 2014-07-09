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

/** @file vertices-in.c
 *
 * Check that the built-in geometry shader constant gl_VerticesIn has
 * the correct value for all input primitive types.
 *
 * The test uses transform feedback to extract the value of
 * gl_VerticesIn out of the shader.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 30;
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGB;

PIGLIT_GL_TEST_CONFIG_END

static const char *vs_text =
	"#version 130\n"
	"void main()\n"
	"{\n"
	"  gl_Position = vec4(0.0);\n"
	"}\n";

static const char *gs_text =
	"#version 130\n"
	"#extension GL_ARB_geometry_shader4: require\n"
	"out int vertices_in;\n"
	"void main()\n"
	"{\n"
	"  vertices_in = gl_VerticesIn;\n"
	"  EmitVertex();\n"
	"}\n";

static const char *varyings[] = { "vertices_in" };

struct test_vector {
	const char *name;
	GLenum prim_type;
	GLint vertices_in;
} test_vectors[] = {
	{ "GL_POINTS",              GL_POINTS,              1},
	{ "GL_LINES",               GL_LINES,               2},
	{ "GL_LINES_ADJACENCY",     GL_LINES_ADJACENCY,     4},
	{ "GL_TRIANGLES",           GL_TRIANGLES,           3},
	{ "GL_TRIANGLES_ADJACENCY", GL_TRIANGLES_ADJACENCY, 6}
};

void
piglit_init(int argc, char **argv)
{
	GLuint vs, gs, prog, buf;
	int i;
	GLint *ptr;
	bool pass = true;

	/* Requirements */
	piglit_require_GLSL_version(130);
	piglit_require_extension("GL_ARB_geometry_shader4");
	piglit_require_extension("GL_EXT_transform_feedback");

	/* Compile shaders, and prepare for linking.  We don't link
	 * yet because we're going to need to change the input
	 * primitive type inside the "for" loop below.
	 */
	prog = glCreateProgram();
	vs = piglit_compile_shader_text(GL_VERTEX_SHADER, vs_text);
	glAttachShader(prog, vs);
	gs = piglit_compile_shader_text(GL_GEOMETRY_SHADER, gs_text);
	glAttachShader(prog, gs);
	glProgramParameteriARB(prog, GL_GEOMETRY_OUTPUT_TYPE_ARB, GL_POINTS);
	glProgramParameteriARB(prog, GL_GEOMETRY_VERTICES_OUT_ARB, 1);
	glTransformFeedbackVaryings(prog, 1, varyings,
				    GL_INTERLEAVED_ATTRIBS_EXT);

	/* Set up the transform feedback buffer. */
	glGenBuffers(1, &buf);
	glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER_EXT, 0, buf);
	glBufferData(GL_TRANSFORM_FEEDBACK_BUFFER_EXT, sizeof(GLint), NULL,
		     GL_STREAM_READ);

	/* Use GL_RASTERIZER_DISCARD, since we are going to use
	 * transform feedback for this test.
	 */
	glEnable(GL_RASTERIZER_DISCARD);

	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	for (i = 0; i < ARRAY_SIZE(test_vectors); i++) {
		printf("Testing %s:\n", test_vectors[i].name);
		glProgramParameteriARB(prog, GL_GEOMETRY_INPUT_TYPE_ARB,
				       test_vectors[i].prim_type);
		glLinkProgram(prog);
		if (!piglit_link_check_status(prog))
			piglit_report_result(PIGLIT_FAIL);
		if (!piglit_check_gl_error(GL_NO_ERROR))
			piglit_report_result(PIGLIT_FAIL);
		glUseProgram(prog);
		glBeginTransformFeedback(GL_POINTS);
		glDrawArrays(test_vectors[i].prim_type, 0,
			     test_vectors[i].vertices_in);
		glEndTransformFeedback();
		if (!piglit_check_gl_error(GL_NO_ERROR))
			piglit_report_result(PIGLIT_FAIL);
		ptr = glMapBuffer(GL_TRANSFORM_FEEDBACK_BUFFER_EXT,
				  GL_READ_ONLY);
		printf("  Expected gl_VerticesIn = %d, got %d\n",
		       test_vectors[i].vertices_in, *ptr);
		if (test_vectors[i].vertices_in != *ptr)
			pass = false;
		glUnmapBuffer(GL_TRANSFORM_FEEDBACK_BUFFER_EXT);
		if (!piglit_check_gl_error(GL_NO_ERROR))
			piglit_report_result(PIGLIT_FAIL);
	}

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}


enum piglit_result
piglit_display(void)
{
	/* Should never be reached */
	return PIGLIT_FAIL;
}
