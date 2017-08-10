/*
 * Copyright © 2011 Marek Olšák <maraeo@gmail.com>
 * Copyright © 2015 Red Hat
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
 *
 * Authors:
 *   Dave Airlie
 */

/**
 * ARB_gpu_shader_fp64 + EXT_transform_feedback test.
 *
 * Test writing separate double vertex attribs into a buffer object.
 * this is based on Marek's separate.c test for EXT_transform_feedback
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

        config.supports_gl_core_version = 32;
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;
	config.khr_no_error_support = PIGLIT_NO_ERRORS;

PIGLIT_GL_TEST_CONFIG_END

static const char *vstext = {
	"#version 150\n"
	"#extension GL_ARB_gpu_shader_fp64 : require\n"
	"in vec4 vertex;"
	"out vec4 color;"
	"out vec4 texcoord[2];"
	"out dvec3 v3;"
	"out dvec2 v2;"
	"void main() {"
	"  gl_Position = vertex;"
	"  color = vec4(1.0, 0.9, 0.8, 0.7);"
	"  texcoord[0] = vec4(0.5);"
	"  texcoord[1] = vec4(0.6, 0.0, 0.1, 0.6);"
	"  v2 = dvec2(0.2lf, 0.7lf);"
	"  v3 = dvec3(0.55lf, 0.66lf, 0.77lf);"
	"}"
};

static const char *varyings[] = {"v3", "color", "v2", "texcoord[1]"};
GLuint buf[4];
GLuint prog;
GLuint vbo, vao;

#define NUM_OUT_VERTICES 6

void piglit_init(int argc, char **argv)
{
	GLuint vs;
	GLint maxcomps, maxattrs;
	unsigned i;

	/* Check the driver. */
	piglit_require_transform_feedback();
	piglit_require_extension("GL_ARB_gpu_shader_fp64");

	glGetIntegerv(GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_ATTRIBS_EXT, &maxattrs);
	if (maxattrs < 4) {
		fprintf(stderr, "Not enough separate attribs supported by transform feedback.\n");
		piglit_report_result(PIGLIT_SKIP);
	}
	glGetIntegerv(GL_MAX_TRANSFORM_FEEDBACK_SEPARATE_COMPONENTS_EXT, &maxcomps);
	if (maxcomps < 4) {
		fprintf(stderr, "Not enough separate components supported by transform feedback.\n");
		piglit_report_result(PIGLIT_SKIP);
	}

	/* Create shaders. */
	vs = piglit_compile_shader_text(GL_VERTEX_SHADER, vstext);
	prog = glCreateProgram();
	glAttachShader(prog, vs);
	glTransformFeedbackVaryings(prog, sizeof(varyings)/sizeof(varyings[0]),
				    varyings, GL_SEPARATE_ATTRIBS_EXT);
	glLinkProgram(prog);
	if (!piglit_link_check_status(prog)) {
		glDeleteProgram(prog);
		piglit_report_result(PIGLIT_FAIL);
	}

	/* Set up the transform feedback buffer. */
	glGenBuffers(4, buf);
	for (i = 0; i < 4; i++) {
		unsigned j;
		float *ptr;
		glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER_EXT, buf[i]);
		glBufferData(GL_TRANSFORM_FEEDBACK_BUFFER_EXT,
			     NUM_OUT_VERTICES*8*sizeof(float), NULL, GL_STREAM_READ);
		ptr = glMapBuffer(GL_TRANSFORM_FEEDBACK_BUFFER_EXT, GL_WRITE_ONLY);
		for (j = 0; j < NUM_OUT_VERTICES*4; j++) {
			ptr[j] = 0.123456;
		}
		glUnmapBuffer(GL_TRANSFORM_FEEDBACK_BUFFER_EXT);
		glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER_EXT, i, buf[i]);
	}

	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	glClearColor(0.2, 0.2, 0.2, 1.0);
}

enum piglit_result piglit_display(void)
{
	bool pass = true;
	static const float verts[] = {
		10, 10,
		10, 20,
		20, 20,
		20, 10
	};
	static const double v3[] = {0.55, 0.66, 0.77};
	static const float frontcolor[] = {1.0, 0.9, 0.8, 0.7};
	static const double v2[] = {0.2, 0.7};
	static const float texcoord1[] = {0.6, 0.0, 0.1, 0.6};

        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid const *)0);

	glClear(GL_COLOR_BUFFER_BIT);

	/* Render into TFBO. */
	glUseProgram(prog);
	glEnable(GL_RASTERIZER_DISCARD);
	glBeginTransformFeedback(GL_TRIANGLES);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glEndTransformFeedback();
	glDisable(GL_RASTERIZER_DISCARD);

	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	pass = piglit_probe_buffer_doubles(buf[0], GL_TRANSFORM_FEEDBACK_BUFFER_EXT,
			"Buffer[0]", NUM_OUT_VERTICES, 3, v3) && pass;
	pass = piglit_probe_buffer(buf[1], GL_TRANSFORM_FEEDBACK_BUFFER_EXT,
			"Buffer[1]", NUM_OUT_VERTICES, 4, frontcolor)&& pass;
	pass = piglit_probe_buffer_doubles(buf[2], GL_TRANSFORM_FEEDBACK_BUFFER_EXT,
			"Buffer[2]", NUM_OUT_VERTICES, 2, v2) && pass;
	pass = piglit_probe_buffer(buf[3], GL_TRANSFORM_FEEDBACK_BUFFER_EXT,
			"Buffer[3]", NUM_OUT_VERTICES, 4, texcoord1) && pass;

	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}
