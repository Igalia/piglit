/*
 * Copyright © 2011 Marek Olšák <maraeo@gmail.com>
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
 * EXT_transform_feedback test.
 *
 * Test writing interleaved vertex attribs into a buffer object.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;

PIGLIT_GL_TEST_CONFIG_END

static const char *vstext = {
	"varying vec3 v3;"
	"varying vec2 v2;"
	"void main() {"
	"  gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;"
	"  gl_FrontColor = vec4(1.0, 0.9, 0.8, 0.7);"
	"  gl_TexCoord[0] = vec4(0.5);"
	"  gl_TexCoord[1] = vec4(0.6, 0.0, 0.1, 0.6);"
	"  v2 = vec2(0.2, 0.7);"
	"  v3 = vec3(0.55, 0.66, 0.77);"
	"}"
};

static const char *varyings[] = {"v3", "gl_FrontColor", "v2", "gl_Position", "gl_TexCoord[1]"};
GLuint buf;
GLuint prog;

#define BUF_FLOATS (17*6)

void piglit_init(int argc, char **argv)
{
	GLuint vs, i;
	GLint maxcomps;
	float *ptr;

	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);

	/* Check the driver. */
	piglit_require_gl_version(15);
	piglit_require_GLSL();
	piglit_require_transform_feedback();

	glGetIntegerv(GL_MAX_TRANSFORM_FEEDBACK_INTERLEAVED_COMPONENTS_EXT, &maxcomps);
	if (maxcomps < 17) {
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
		     BUF_FLOATS*sizeof(float), NULL, GL_STREAM_READ);
	ptr = glMapBuffer(GL_TRANSFORM_FEEDBACK_BUFFER_EXT, GL_WRITE_ONLY);
	for (i = 0; i < BUF_FLOATS; i++) {
		ptr[i] = 0.123456;
	}
	glUnmapBuffer(GL_TRANSFORM_FEEDBACK_BUFFER_EXT);
	glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER_EXT, 0, buf);

	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	glClearColor(0.2, 0.2, 0.2, 1.0);
	glEnableClientState(GL_VERTEX_ARRAY);
}

enum piglit_result piglit_display(void)
{
	GLboolean pass = GL_TRUE;
	float *ptr;
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
		0.550000, 0.660000, 0.770000,
		1.000000, 0.900000, 0.800000, 0.700000,
		0.200000, 0.700000,
		-0.687500, -0.375000, 0.000000, 1.000000,
		0.600000, 0.000000, 0.100000, 0.600000,

		0.550000, 0.660000, 0.770000,
		1.000000, 0.900000, 0.800000, 0.700000,
		0.200000, 0.700000,
		-0.687500, 0.250000, 0.000000, 1.000000,
		0.600000, 0.000000, 0.100000, 0.600000,

		0.550000, 0.660000, 0.770000,
		1.000000, 0.900000, 0.800000, 0.700000,
		0.200000, 0.700000,
		-0.375000, -0.375000, 0.000000, 1.000000,
		0.600000, 0.000000, 0.100000, 0.600000,

		0.550000, 0.660000, 0.770000,
		1.000000, 0.900000, 0.800000, 0.700000,
		0.200000, 0.700000,
		-0.687500, 0.250000, 0.000000, 1.000000,
		0.600000, 0.000000, 0.100000, 0.600000,

		0.550000, 0.660000, 0.770000,
		1.000000, 0.900000, 0.800000, 0.700000,
		0.200000, 0.700000,
		-0.375000, 0.250000, 0.000000, 1.000000,
		0.600000, 0.000000, 0.100000, 0.600000,

		0.550000, 0.660000, 0.770000,
		1.000000, 0.900000, 0.800000, 0.700000,
		0.200000, 0.700000,
		-0.375000, -0.375000, 0.000000, 1.000000,
		0.600000, 0.000000, 0.100000, 0.600000
	};

	glClear(GL_COLOR_BUFFER_BIT);

	/* Setup projection for a 64 x 32 window region.  That's what
	 * the expected coords above assume.
	 * XXX it would be better if the position coords in the above
	 * array were compute instead of fixed.
	 */
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, 64, 0, 32, -1, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	/* Render into TFBO. */
	glUseProgram(prog);
	glEnable(GL_RASTERIZER_DISCARD);
	glBeginTransformFeedback(GL_TRIANGLES);
	glVertexPointer(2, GL_FLOAT, 0, verts);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, indices);
	glEndTransformFeedback();
	glDisable(GL_RASTERIZER_DISCARD);

	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	ptr = glMapBuffer(GL_TRANSFORM_FEEDBACK_BUFFER_EXT, GL_READ_ONLY);
	for (i = 0; i < BUF_FLOATS; i++) {
		//printf("%f, ", ptr[i]); continue;
		if (fabs(ptr[i] - expected[i]) > 0.01) {
			printf("Buffer[%i]: %f,  Expected: %f\n", i, ptr[i], expected[i]);
			pass = GL_FALSE;
		} else {
			printf("Buffer[%i]: %f,  Expected: %f -- OK\n", i, ptr[i], expected[i]);
		}
	}
	//puts("");
	glUnmapBuffer(GL_TRANSFORM_FEEDBACK_BUFFER_EXT);

	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}
