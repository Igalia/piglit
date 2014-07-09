/*
 * Copyright © 2012 Intel Corporation
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
 * \file pause-counting.c
 * Verify behavior of XFB "counting" queries when pause and resume are used.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;
	config.window_visual = PIGLIT_GL_VISUAL_RGB;

PIGLIT_GL_TEST_CONFIG_END

enum piglit_result
piglit_display(void)
{
	return PIGLIT_FAIL;
}

static const float data[] = {
	-1.0, -1.0,
	 1.0, -1.0,
	 1.0,  1.0,
	-1.0,  1.0,
};

static const char vstext[] =
	"varying vec4 x; void main() { gl_Position = gl_Vertex; x = vec4(0); }";

void piglit_init(int argc, char **argv)
{
	static const char *varyings[] = {"x"};
	GLuint id;
	GLuint buffers[2];
	GLuint prog;
	GLuint vs;
	GLuint queries[2];
	bool pass = true;
	GLuint generated;
	GLuint written;

	piglit_require_transform_feedback();
	piglit_require_GLSL();
	piglit_require_extension("GL_ARB_transform_feedback2");

	/* This is all just the boot-strap work for the test.
	 */
	glGenBuffers(2, buffers);
	glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, buffers[0]);
	glBufferData(GL_TRANSFORM_FEEDBACK_BUFFER, 1024, NULL, GL_STREAM_READ);

	glBindBuffer(GL_ARRAY_BUFFER, buffers[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(data), data, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), 0);
	glEnableVertexAttribArray(0);

	vs = piglit_compile_shader_text(GL_VERTEX_SHADER, vstext);
	prog = glCreateProgram();
	glAttachShader(prog, vs);

	glTransformFeedbackVaryings(prog, 1, varyings, GL_INTERLEAVED_ATTRIBS);
	glLinkProgram(prog);
	if (!piglit_link_check_status(prog)) {
		pass = false;
		goto done;
	}

	glUseProgram(prog);

	glGenTransformFeedbacks(1, &id);

	glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, id);
	glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, buffers[0]);

	glGenQueries(2, queries);

	/* Here's the actual test.  Start both kinds of query.  Pause and
	 * resume transform feedback around some of the drawing.  This should
	 * cause GL_PRIMITIVES_GENERATED to be larger than
	 * GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN.
	 */
	glEnable(GL_RASTERIZER_DISCARD);
	glBeginQuery(GL_PRIMITIVES_GENERATED, queries[0]);
	glBeginQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN, queries[1]);
        glBeginTransformFeedback(GL_TRIANGLES);

	glDrawArrays(GL_TRIANGLES, 0, 4);

	glPauseTransformFeedback();

	glDrawArrays(GL_TRIANGLES, 0, 4);

	glResumeTransformFeedback();

	glDrawArrays(GL_TRIANGLES, 0, 4);

	glEndTransformFeedback();
	glEndQuery(GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN);
	glEndQuery(GL_PRIMITIVES_GENERATED);

	glGetQueryObjectuiv(queries[0], GL_QUERY_RESULT, &generated);
	glGetQueryObjectuiv(queries[1], GL_QUERY_RESULT, &written);

	if (generated != 3) {
		fprintf(stderr,
			"GL_PRIMITIVES_GENERATED: "
			"Expected %d, got %d\n",
			3, generated);
		pass = false;
	}

	if (written != 2) {
		fprintf(stderr,
			"GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN: "
			"Expected %d, got %d\n",
			2, written);
		pass = false;
	}

	glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, 0);

done:
	glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, 0);
	glDeleteBuffers(2, buffers);
	glDeleteQueries(2, queries);
	glDeleteTransformFeedbacks(1, &id);

	glUseProgram(0);
	glDeleteShader(vs);
	glDeleteProgram(prog);

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}
