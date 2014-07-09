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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

/**
 * \file change-objects-while-paused.c
 * Verify behavior of changing XFB objects while XFB is paused.
 *
 * The test methodology is:
 *
 * - Bind an XFB object, start XFB, draw something, pause XFB.
 *
 * - Bind a different XFB object, start XFB, draw someting, pause XFB.
 *
 * - Rebind the first XFB object, resume XFB, draw something, end XFB.
 *
 * - Rebind the second XFB object, resume XFB, draw something, end XFB.
 *
 * - Verify that all the expected data has landed in the expected places.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

#ifdef PIGLIT_USE_OPENGL
	config.supports_gl_compat_version = 10;
#elif defined PIGLIT_USE_OPENGL_ES3
	config.supports_gl_es_version = 30;
#else
#error "Cannot build this."
#endif
	config.window_visual = PIGLIT_GL_VISUAL_RGB;

PIGLIT_GL_TEST_CONFIG_END

enum piglit_result
piglit_display(void)
{
	return PIGLIT_FAIL;
}

static const float data[] = {
	1.0,
	2.0,
	3.0,
	4.0,
	5.0,
	6.0,
	7.0,
	8.0,
	9.0,
	10.0,
	11.0,
	12.0,
};

static const char vstext[] =
#if defined PIGLIT_USE_OPENGL_ES3
	"#version 300 es\n"
#else
	"#version 130\n"
#endif
	"in vec4 piglit_vertex;\n"
	"out float x;\n"
	"\n"
	"void main()\n"
	"{\n"
	"    gl_Position = piglit_vertex;\n"
	"    x = piglit_vertex.x;\n"
	"}\n"
	;

static const char fstext[] =
#if defined PIGLIT_USE_OPENGL_ES3
	"#version 300 es\n"
#else
	"#version 130\n"
#endif
	"out highp vec4 color;\n"
	"void main() { color = vec4(0); }\n"
	;

bool
check_results(unsigned test, unsigned expect_written, const float *expect_data,
	      GLuint q0, GLuint q1)
{
	float *data;
	bool pass = true;
	GLuint written[2];
	GLuint total;

	glGetQueryObjectuiv(q0, GL_QUERY_RESULT, &written[0]);
	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

	glGetQueryObjectuiv(q0, GL_QUERY_RESULT, &written[1]);
	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

	total = written[0] + written[1];
	if (total != expect_written) {
		fprintf(stderr,
			"XFB %d GL_PRIMITIVES_WRITTEN: "
			"Expected %d, got %d\n",
			test, expect_written, total);
		pass = false;
	} 

	data = glMapBufferRange(GL_ARRAY_BUFFER, 0, 512, GL_MAP_READ_BIT);
	if (!piglit_check_gl_error(GL_NO_ERROR) || data == NULL) {
		fprintf(stderr,	"XFB %d: Could not map results buffer.\n",
			test);
		pass = false;
	} else {
		unsigned i;

		for (i = 0; i < expect_written; i++) {
			if (data[i] != expect_data[i]) {
				fprintf(stderr,
					"XFB %d data %d: "
					"Expected %f, got %f\n",
					test, i, expect_data[i], data[i]);
				pass = false;
			}
		}
		
		glUnmapBuffer(GL_ARRAY_BUFFER);
	}

	return pass;
}

void piglit_init(int argc, char **argv)
{
	static const char *varyings[] = {"x"};
	GLuint buffers[3];
	GLuint vao;
	GLuint prog;
	GLuint queries[4];
	GLuint xfb[2];
	bool pass = true;

#ifdef PIGLIT_USE_OPENGL
	piglit_require_transform_feedback();
	piglit_require_GLSL_version(130);
	piglit_require_extension("GL_ARB_vertex_array_object");
	piglit_require_extension("GL_ARB_transform_feedback2");
#endif

	/* This is all just the boot-strap work for the test.
	 */
	glGenTransformFeedbacks(ARRAY_SIZE(xfb), xfb);
	glGenBuffers(ARRAY_SIZE(buffers), buffers);

	glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, buffers[0]);
	glBufferData(GL_TRANSFORM_FEEDBACK_BUFFER, 1024, NULL, GL_STREAM_READ);

	glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, buffers[1]);
	glBufferData(GL_TRANSFORM_FEEDBACK_BUFFER, 1024, NULL, GL_STREAM_READ);

	glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, 0);

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, buffers[2]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(data), data, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 1, GL_FLOAT, GL_FALSE, 1 * sizeof(GLfloat), 0);
	glEnableVertexAttribArray(0);

	glGenQueries(ARRAY_SIZE(queries), queries);

	prog = piglit_build_simple_program_unlinked(vstext, fstext);

	glTransformFeedbackVaryings(prog, 1, varyings, GL_INTERLEAVED_ATTRIBS);
	glLinkProgram(prog);
	if (!piglit_link_check_status(prog)) {
		pass = false;
		goto done;
	}

	glUseProgram(prog);
	glEnable(GL_RASTERIZER_DISCARD);

	/* Here's the actual test.
	 */
	glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, xfb[0]);
	glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, buffers[0]);
	glBeginTransformFeedback(GL_POINTS);

	glBeginQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN, queries[0]);
	glDrawArrays(GL_POINTS, 0, 4);
	glEndQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN);

	glPauseTransformFeedback();

	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

	glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, xfb[1]);
	glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, buffers[1]);
	glBeginTransformFeedback(GL_POINTS);

	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

	glBeginQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN, queries[1]);
	glDrawArrays(GL_POINTS, 4, 2);
	glEndQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN);

	glPauseTransformFeedback();

	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

	glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, xfb[0]);
	glResumeTransformFeedback();

	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

	glBeginQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN, queries[2]);
	glDrawArrays(GL_POINTS, 6, 4);
	glEndQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN);

	glEndTransformFeedback();

	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

	glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, xfb[1]);
	glResumeTransformFeedback();

	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

	glBeginQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN, queries[3]);
	glDrawArrays(GL_POINTS, 10, 2);
	glEndQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN);

	glEndTransformFeedback();

	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

	glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, 0);
	glBindVertexArray(0);

	/* The first XFB should have 8 primitives generated, and the buffer
	 * object should contain the values {1.0, 2.0, 3.0, 4.0, 7.0, 8.0,
	 * 9.0, 10.0}.
	 */
	{
		static const float expected_xfb_data[] = {
			1.0, 2.0, 3.0, 4.0, 7.0, 8.0, 9.0, 10.0
		};

		glBindBuffer(GL_ARRAY_BUFFER, buffers[0]);
		pass = check_results(1, ARRAY_SIZE(expected_xfb_data),
				     expected_xfb_data,
				     queries[0], queries[2])
			&& pass;
	}

	/* The second XFB should have 4 primitives generated, and the buffer
	 * object should contain the values {5.0, 6.0, 11.0, 12.0}.
	 */
	{
		static const float expected_xfb_data[] = {
			5.0, 6.0, 11.0, 12.0
		};

		glBindBuffer(GL_ARRAY_BUFFER, buffers[1]);
		pass = check_results(2, ARRAY_SIZE(expected_xfb_data),
				     expected_xfb_data,
				     queries[1], queries[3])
			&& pass;
	}

	glBindBuffer(GL_ARRAY_BUFFER, 0);

done:
	glBindVertexArray(0);
	glDeleteVertexArrays(1, &vao);
	glDeleteBuffers(ARRAY_SIZE(buffers), buffers);
	glDeleteQueries(ARRAY_SIZE(queries), queries);
	glDeleteTransformFeedbacks(ARRAY_SIZE(xfb), xfb);

	glUseProgram(0);
	glDeleteProgram(prog);

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}
