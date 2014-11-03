/*
 * Copyright © 2014 VMware, Inc.
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
 * Simple transform feedback test drawing GL_POINTS.
 * If argv[1] == "large" draw large points (which may hit a point->quad
 * conversion path.)
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN
	config.supports_gl_compat_version = 10;
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;
PIGLIT_GL_TEST_CONFIG_END


static GLuint prog;
static GLuint xfb_buf, vert_buf;
static const int xfb_buf_size = 500;

static const char *vstext = {
	"void main() {"
	"   gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;"
	"   gl_FrontColor = vec4(0.9, 0.8, 0.7, 0.6);"
	"}"
};

#define NUM_VERTS 3
static const GLfloat verts[NUM_VERTS][3] =
	{
		{-1, 0.2, 0},
		{0, 0.2, 0},
		{1, 0.2, 0}
	};


void
piglit_init(int argc, char **argv)
{
	static const char *varyings[] = { "gl_Position", "gl_FrontColor" };
	GLuint vs;

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glScalef(0.5, 0.5, 1.0);

	/* Check the driver. */
	piglit_require_gl_version(15);
	piglit_require_GLSL();
	piglit_require_transform_feedback();

	/* Create shaders. */
	vs = piglit_compile_shader_text(GL_VERTEX_SHADER, vstext);
	prog = glCreateProgram();
	glAttachShader(prog, vs);
	glTransformFeedbackVaryings(prog, 2, varyings,
				    GL_INTERLEAVED_ATTRIBS_EXT);
	glLinkProgram(prog);
	if (!piglit_link_check_status(prog)) {
		glDeleteProgram(prog);
		piglit_report_result(PIGLIT_FAIL);
	}
	glUseProgram(prog);

	/* Set up the vertex data buffer */
	glGenBuffers(1, &vert_buf);
	glBindBuffer(GL_ARRAY_BUFFER, vert_buf);
	glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);

	/* Set up the transform feedback buffer. */
	glGenBuffers(1, &xfb_buf);
	glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER_EXT, xfb_buf);
	glBufferData(GL_TRANSFORM_FEEDBACK_BUFFER_EXT,
		     xfb_buf_size, NULL, GL_STREAM_READ);
	glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER_EXT, 0, xfb_buf);

	glClearColor(0.2, 0.2, 0.2, 1.0);

	if (argc > 1 && strcmp(argv[1], "large") == 0) {
		GLint range[2];
		glGetIntegerv(GL_ALIASED_POINT_SIZE_RANGE, range);
		if (range[1] == 1.0) {
			printf("Max point size is %d, can't test large\n",
			       range[1]);
			piglit_report_result(PIGLIT_WARN);
		}
		printf("Testing large points\n");
		glPointSize(10.0);
	}
}


static bool
equal(float a, float b)
{
	return fabsf(a - b) < 0.0001;
}


enum piglit_result
piglit_display(void)
{
	bool pass = true;
	GLfloat *v;
	int i;
	GLuint q, num_prims;

	glClear(GL_COLOR_BUFFER_BIT);

	glGenQueries(1, &q);
	glBeginQuery(GL_PRIMITIVES_GENERATED_EXT, q);

	glBeginTransformFeedback(GL_POINTS);
	glBindBuffer(GL_ARRAY_BUFFER, vert_buf);
	glVertexPointer(3, GL_FLOAT, 0, 0);
	glEnable(GL_VERTEX_ARRAY);
	glDrawArrays(GL_POINTS, 0, 3);
	glEndTransformFeedback();

	glEndQuery(GL_PRIMITIVES_GENERATED);

	glGetQueryObjectuiv(q, GL_QUERY_RESULT, &num_prims);
	glDeleteQueries(1, &q);
	printf("%u primitives generated:\n", num_prims);

	if (num_prims != NUM_VERTS) {
		printf("Incorrect number of prims generated.\n");
		printf("Found %u, expected %u\n", num_prims, NUM_VERTS);
		pass = false;
	}

	glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER_EXT, xfb_buf);
	v = glMapBuffer(GL_TRANSFORM_FEEDBACK_BUFFER_EXT, GL_READ_ONLY);
	for (i = 0; i < num_prims; i++) {
		printf("vertex %2d: pos %5.2g, %5.2g, %5.2g, %5.2g   "
		       "color %5.2g, %5.2g, %5.2g, %5.2g\n", i,
		       v[i*8+0], v[i*8+1], v[i*8+2], v[i*8+3],
		       v[i*8+4], v[i*8+5], v[i*8+6], v[i*8+7]);
		/* spot-check results */
		if (!equal(v[i*8+1], 0.1)) {
			printf("Incorrect Y coord for point %d: %f\n",
			       i, v[i*8+1]);
			pass = false;
		}
		if (!equal(v[i*8+4], 0.9)) {
			printf("Incorrect red value for point %d: %f\n",
			       i, v[i*8+4]);
			pass = false;
		}
	}
	glUnmapBuffer(GL_TRANSFORM_FEEDBACK_BUFFER_EXT);

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}
