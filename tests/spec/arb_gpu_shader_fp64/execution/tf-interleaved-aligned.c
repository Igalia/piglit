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
 */

/**
 * ARB_gpu_shader_fp64 + ARB_transform_feedback3  test.
 *
 * Test writing interleaved vertex attribs into a buffer object.
 * Writing unaligned doubles is undefined so if we want to have a float
 * follow a double we need to use ARB_tf3
 *
 * "If capturing a mix of single- and double-precision components, it might
 *  be necessary to use the "gl_SkipComponents1" variable from
 *  ARB_transform_feedback3 to force proper alignment."
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
	"out float tf;"
	"out dvec3 v3;"
	"out dvec2 v2;"
	"void main() {"
	"  gl_Position = vertex;"
	"  color = vec4(1.0, 0.9, 0.8, 0.7);"
	"  tf = 0.5;"
	"  v2 = dvec2(0.2lf, 0.7lf);"
	"  v3 = dvec3(0.55lf, 0.66lf, 0.77lf);"
	"}"
};

static const char *varyings[] = {"tf", "gl_SkipComponents1", "v3", "color", "v2"};
GLuint buf;
GLuint prog;
GLuint vao, vbo;

#define NUM_COMPONENTS 16
#define TOTAL_BUF_COMPONENTS (NUM_COMPONENTS*6)

static uint32_t dbl_components = (0x3f << 2) | (0xf << 12);
void piglit_init(int argc, char **argv)
{
	GLuint vs, i;
	GLint maxcomps;
	float *ptr;

	/* Check the driver. */
	piglit_require_transform_feedback();
	piglit_require_extension("GL_ARB_gpu_shader_fp64");
	piglit_require_extension("GL_ARB_transform_feedback3");

	glGetIntegerv(GL_MAX_TRANSFORM_FEEDBACK_INTERLEAVED_COMPONENTS_EXT, &maxcomps);
	if (maxcomps < 18) {
		fprintf(stderr, "Not enough interleaved components supported by transform feedback.\n");
		piglit_report_result(PIGLIT_SKIP);
	}

	/* Create shaders. */
	vs = piglit_compile_shader_text(GL_VERTEX_SHADER, vstext);
	prog = glCreateProgram();
	glAttachShader(prog, vs);
	glTransformFeedbackVaryings(prog, sizeof(varyings)/sizeof(varyings[0]),
				    varyings, GL_INTERLEAVED_ATTRIBS_EXT);
	glLinkProgram(prog);
	if (!piglit_link_check_status(prog)) {
		glDeleteProgram(prog);
		piglit_report_result(PIGLIT_FAIL);
	}

	/* Set up the transform feedback buffer. */
	glGenBuffers(1, &buf);
	glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER_EXT, buf);
	glBufferData(GL_TRANSFORM_FEEDBACK_BUFFER_EXT,
		     TOTAL_BUF_COMPONENTS*sizeof(float), NULL, GL_STREAM_READ);
	ptr = glMapBuffer(GL_TRANSFORM_FEEDBACK_BUFFER_EXT, GL_WRITE_ONLY);
	for (i = 0; i < TOTAL_BUF_COMPONENTS; i++) {
		ptr[i] = 0.123456;
	}
	glUnmapBuffer(GL_TRANSFORM_FEEDBACK_BUFFER_EXT);
	glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER_EXT, 0, buf);

	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	glClearColor(0.2, 0.2, 0.2, 1.0);
}

enum piglit_result piglit_display(void)
{
	GLboolean pass = GL_TRUE;
	float *ptr;
	double *dptr;
	unsigned i;
	static const float verts[] = {
		10, 10,
		10, 20,
		20, 20,
		20, 10
	};
	static const unsigned indices[] = {
		0, 1, 3, 1, 2, 3
	};
	static const float expected[] = {
		0.5, 0.123456,
		0.550000, 0.0, 0.660000, 0.0, 0.770000, 0.0,
		1.000000, 0.900000, 0.800000, 0.700000,
		0.200000, 0.0, 0.700000, 0.0,
	};

	glClear(GL_COLOR_BUFFER_BIT);

        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid const *)0);

	/* Render into TFBO. */
	glUseProgram(prog);
	glEnable(GL_RASTERIZER_DISCARD);
	glBeginTransformFeedback(GL_TRIANGLES);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, indices);
	glEndTransformFeedback();
	glDisable(GL_RASTERIZER_DISCARD);

	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	ptr = glMapBuffer(GL_TRANSFORM_FEEDBACK_BUFFER_EXT, GL_READ_ONLY);
	dptr = (double *)ptr;
	for (i = 0; i < TOTAL_BUF_COMPONENTS; i++) {
		float expect = expected[i % NUM_COMPONENTS];
		if ((dbl_components) & (1 << (i % NUM_COMPONENTS))) {
			if ((i % 2) == 0) {
				if (fabs(dptr[i/2] - expect) > 0.01) {
					printf("Bufferd[%i]: %f,  Expected: %f\n", i, dptr[i/2], expect);
					pass = GL_FALSE;
				} else {
					printf("Bufferd[%i]: %f,  Expected: %f -- OK\n", i, dptr[i/2], expect);
				}
			}
		} else {
			//printf("%f, ", ptr[i]); continue;
			if (fabs(ptr[i] - expect) > 0.01) {
				printf("Buffer[%i]: %f,  Expected: %f\n", i, ptr[i], expect);
				pass = GL_FALSE;
			} else {
				printf("Buffer[%i]: %f,  Expected: %f -- OK\n", i, ptr[i], expect);
			}
		}
	}
	//puts("");
	glUnmapBuffer(GL_TRANSFORM_FEEDBACK_BUFFER_EXT);

	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}
