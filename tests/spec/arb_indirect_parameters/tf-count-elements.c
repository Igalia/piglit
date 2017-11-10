/*
 * Copyright (C) 2016 Ilia Mirkin
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

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_core_version = 31;
	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;
	config.khr_no_error_support = PIGLIT_NO_ERRORS;

PIGLIT_GL_TEST_CONFIG_END

static const char *vs_tf =
	"#version 140\n"
	"out int tf;\n"
	"uniform int tf_val;\n"
	"void main() { gl_Position = vec4(0); tf = tf_val; }\n";

static const char *vs_draw =
	"#version 140\n"
	"out vec4 color;\n"
	"in vec4 vtx, in_color;\n"
	"void main() { gl_Position = vtx; color = in_color; }\n";

static const char *fs_draw =
	"#version 140\n"
	"out vec4 c;\n"
	"in vec4 color;\n"
	"void main() { c = color; }\n";

static GLint tf_prog, draw_prog;
static GLint tf_val;
static GLuint tf_vao, draw_vao;

void
piglit_init(int argc, char **argv)
{
	static const char *varying = "tf";
	static const unsigned cmds[] = {
		6, 1, 0, 0, 0,
		6, 1, 0, 4, 0,
		6, 1, 0, 8, 0,
	};
	static const struct {
		float vertex_array[12 * 2];
		float colors[12 * 4];
	} geometry = {
		{
			-1, -1,
			0, -1,
			0, 1,
			-1, 1,

			0, -1,
			1, -1,
			1, 1,
			0, 1,

			-1, -1,
			1, -1,
			1, 1,
			-1, 1,
		},
		{
			0, 1, 0, 1,
			0, 1, 0, 1,
			0, 1, 0, 1,
			0, 1, 0, 1,

			0, 1, 1, 1,
			0, 1, 1, 1,
			0, 1, 1, 1,
			0, 1, 1, 1,

			1, 0, 0, 0,
			1, 0, 0, 0,
			1, 0, 0, 0,
			1, 0, 0, 0,
		},
	};

	static const int indices[12] = {
		0, 1, 2,
		0, 2, 3,
	};

	GLuint vbo, ibo, dbo;

	piglit_require_extension("GL_ARB_indirect_parameters");

	tf_prog = piglit_build_simple_program_unlinked(vs_tf, NULL);
	draw_prog = piglit_build_simple_program(vs_draw, fs_draw);

	glTransformFeedbackVaryings(tf_prog, 1, &varying,
				    GL_INTERLEAVED_ATTRIBS);
	glLinkProgram(tf_prog);
	if (!piglit_link_check_status(tf_prog))
		piglit_report_result(PIGLIT_FAIL);
	tf_val = glGetUniformLocation(tf_prog, "tf_val");

	glGenVertexArrays(1, &tf_vao);

	glGenVertexArrays(1, &draw_vao);
	glBindVertexArray(draw_vao);

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(geometry), &geometry, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE,
			      2 * sizeof(GLfloat), NULL);

	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE,
			      4 * sizeof(GLfloat), (void *)(12 * 2 * 4));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glGenBuffers(1, &ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glGenBuffers(1, &dbo);
	glBindBuffer(GL_DRAW_INDIRECT_BUFFER, dbo);
	glBufferData(GL_DRAW_INDIRECT_BUFFER, sizeof(cmds), cmds, GL_STATIC_DRAW);
}

enum piglit_result
piglit_display(void)
{
	GLuint xfb_buf;
	bool pass = true;
	unsigned *map;

	static const float g[] = {0, 1, 0, 1};
	static const float gb[] = {0, 1, 1, 1};

	glClearColor(0.2, 0.2, 0.2, 0.2);
	glClear(GL_COLOR_BUFFER_BIT);

	glGenBuffers(1, &xfb_buf);
	glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, xfb_buf);
	glBufferData(GL_TRANSFORM_FEEDBACK_BUFFER, 8, NULL, GL_STATIC_READ);

	glEnable(GL_RASTERIZER_DISCARD);
	glBindVertexArray(tf_vao);
	glUseProgram(tf_prog);

	/* write a 2 into xfb_buf[0] */
	glBindBufferRange(GL_TRANSFORM_FEEDBACK_BUFFER, 0, xfb_buf, 0, 4);
	glUniform1i(tf_val, 2);
	glBeginTransformFeedback(GL_POINTS);
	glDrawArrays(GL_POINTS, 0, 1);
	glEndTransformFeedback();

	/* write a 0 into xfb_buf[1] */
	glBindBufferRange(GL_TRANSFORM_FEEDBACK_BUFFER, 0, xfb_buf, 4, 4);
	glUniform1i(tf_val, 0);
	glBeginTransformFeedback(GL_POINTS);
	glDrawArrays(GL_POINTS, 0, 1);
	glEndTransformFeedback();

	glDisable(GL_RASTERIZER_DISCARD);
	glBindVertexArray(draw_vao);
	glUseProgram(draw_prog);

	/* Draw the g/gb halves */
	glBindBuffer(GL_PARAMETER_BUFFER_ARB, xfb_buf);
	glMultiDrawElementsIndirectCountARB(
			GL_TRIANGLES, GL_UNSIGNED_INT,
			0, 0, 2, 0);

	pass &= piglit_probe_rect_rgba(0, 0, piglit_width / 2, piglit_height,
				       g);
	pass &= piglit_probe_rect_rgba(piglit_width / 2, 0,
				       piglit_width / 2, piglit_height,
				       gb);
	if (!pass) {
		printf("first draw failed\n");
		goto end;
	}

	/* Overdraw with the red quad, except count = 0 */
	glMultiDrawElementsIndirectCountARB(
			GL_TRIANGLES, GL_UNSIGNED_INT,
			(const void *) (2 * 5 * 4), 4, 1, 0);

	pass &= piglit_probe_rect_rgba(0, 0, piglit_width / 2, piglit_height,
				       g);
	pass &= piglit_probe_rect_rgba(piglit_width / 2, 0,
				       piglit_width / 2, piglit_height,
				       gb);
	if (!pass)
		printf("second draw did something when it shouldn't have.\n");

end:
	piglit_present_results();
	map = glMapBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, GL_READ_ONLY);
	if (map[0] != 2) {
		printf("map[0] != 2\n");
		pass = false;
	}
	if (map[1] != 0) {
		printf("map[1] != 0\n");
		pass = false;
	}
	glUnmapBuffer(GL_TRANSFORM_FEEDBACK_BUFFER);
	glDeleteBuffers(1, &xfb_buf);
	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}
