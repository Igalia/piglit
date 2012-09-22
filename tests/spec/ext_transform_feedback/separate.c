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
 * Test writing separate vertex attribs into a buffer object.
 */

#include "piglit-util-gl-common.h"

PIGLIT_GL_TEST_MAIN(
    64 /*window_width*/,
    32 /*window_height*/,
    PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_ALPHA)

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

static const char *varyings[] = {"v3", "gl_FrontColor", "v2", "gl_TexCoord[1]"};
GLuint buf[4];
GLuint prog;

#define NUM_OUT_VERTICES 6

void piglit_init(int argc, char **argv)
{
	GLuint vs;
	GLint maxcomps, maxattrs;
	unsigned i;

	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);

	/* Check the driver. */
	piglit_require_gl_version(15);
	piglit_require_GLSL();
	piglit_require_transform_feedback();

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
	prog = piglit_CreateProgram();
	piglit_AttachShader(prog, vs);
	glTransformFeedbackVaryings(prog, sizeof(varyings)/sizeof(varyings[0]),
				    varyings, GL_SEPARATE_ATTRIBS_EXT);
	piglit_LinkProgram(prog);
	if (!piglit_link_check_status(prog)) {
		piglit_DeleteProgram(prog);
		piglit_report_result(PIGLIT_FAIL);
	}

	/* Set up the transform feedback buffer. */
	glGenBuffers(4, buf);
	for (i = 0; i < 4; i++) {
		unsigned j;
		float *ptr;
		glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER_EXT, buf[i]);
		glBufferData(GL_TRANSFORM_FEEDBACK_BUFFER_EXT,
			     NUM_OUT_VERTICES*4*sizeof(float), NULL, GL_STREAM_READ);
		ptr = glMapBuffer(GL_TRANSFORM_FEEDBACK_BUFFER_EXT, GL_WRITE_ONLY);
		for (j = 0; j < NUM_OUT_VERTICES*4; j++) {
			ptr[j] = 0.123456;
		}
		glUnmapBuffer(GL_TRANSFORM_FEEDBACK_BUFFER_EXT);
		glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER_EXT, i, buf[i]);
	}

	assert(glGetError() == 0);

	glClearColor(0.2, 0.2, 0.2, 1.0);
	glEnableClientState(GL_VERTEX_ARRAY);
}

static GLboolean probe_buffer(GLuint buf, int bufindex, unsigned comps, const float *expected)
{
	float *ptr;
	unsigned i;
	GLboolean status = GL_TRUE;

	glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER_EXT, buf);
	ptr = glMapBuffer(GL_TRANSFORM_FEEDBACK_BUFFER_EXT, GL_READ_ONLY);
	for (i = 0; i < NUM_OUT_VERTICES*comps; i++) {
		if (fabs(ptr[i] - expected[i % comps]) > 0.01) {
			printf("Buffer[%i][%i]: %f,  Expected: %f\n", bufindex, i, ptr[i], expected[i % comps]);
			status = GL_FALSE;
		} else {
			printf("Buffer[%i][%i]: %f,  Expected: %f -- OK\n", bufindex, i, ptr[i], expected[i % comps]);

		}
	}
	glUnmapBuffer(GL_TRANSFORM_FEEDBACK_BUFFER_EXT);
	return status;
}

enum piglit_result piglit_display(void)
{
	GLboolean pass = GL_TRUE;
	static const float verts[] = {
		10, 10,
		10, 20,
		20, 20,
		20, 10
	};
	static const float v3[] = {0.55, 0.66, 0.77};
	static const float frontcolor[] = {1.0, 0.9, 0.8, 0.7};
	static const float v2[] = {0.2, 0.7};
	static const float texcoord1[] = {0.6, 0.0, 0.1, 0.6};

	glClear(GL_COLOR_BUFFER_BIT);

	/* Render into TFBO. */
	glLoadIdentity();
	piglit_UseProgram(prog);
	glEnable(GL_RASTERIZER_DISCARD);
	glBeginTransformFeedback(GL_TRIANGLES);
	glVertexPointer(2, GL_FLOAT, 0, verts);
	glDrawArrays(GL_QUADS, 0, 4);
	glEndTransformFeedback();
	glDisable(GL_RASTERIZER_DISCARD);

	assert(glGetError() == 0);

	pass = probe_buffer(buf[0], 0, 3, v3) && pass;
	pass = probe_buffer(buf[1], 1, 4, frontcolor) && pass;
	pass = probe_buffer(buf[2], 2, 2, v2) && pass;
	pass = probe_buffer(buf[3], 3, 4, texcoord1) && pass;

	assert(glGetError() == 0);

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}
