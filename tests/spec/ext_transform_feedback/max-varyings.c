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

/** @file glsl-max-varyings.c
 *
 * Test that all varyings can be captured using transform feedback, up
 * to the maximum allowed by
 * GL_MAX_TRANSFORM_FEEDBACK_INTERLEAVED_COMPONENTS.  Also verify that
 * the varyings are passed correctly to the fragment shader.  This
 * test uses vec4 varyings, so it does not rely on the implementation
 * packing varyings correctly.
 *
 * The test uses a vertex shader that generates an array of
 * (GL_MAX_VARYING_FLOATS / 4) vec4's, and a fragment shader that
 * checks the values of all of those vec4's.  It uses transform
 * feedback to capture contiguous subsets of that array, with all
 * possible lengths and offsets.
 */

#include "piglit-util-gl.h"

#define MAX_VARYING 32

/* 10x10 rectangles with 2 pixels of pad.  Deal with up to 32 varyings. */

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_width = (2+MAX_VARYING*12);
	config.window_height = (2+MAX_VARYING*12);
	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

static const char *xfb_varying_array[MAX_VARYING];
static GLuint xfb_buf;

/* Generate a VS that writes to a varying vec4[num_varyings] called
 * "v".  The values written are v[0] = (0.0, 1.0, 2.0, 3.0), v[1] =
 * (4.0, 5.0, 6.0, 7.0), and so on.
 */
static GLint
get_vs(int num_varyings)
{
	GLuint shader;
	char vstext[2048];

	sprintf(vstext,
		"#version 120\n"
		"varying vec4[%d] v;\n"
		"\n"
		"void main()\n"
		"{\n"
		"  int i;\n"
		"  gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;\n"
		"  for (i = 0; i < v.length(); ++i) {\n"
		"    v[i] = 4.0*i + vec4(0.0, 1.0, 2.0, 3.0);\n"
		"  }\n"
		"}\n", num_varyings);

	shader = piglit_compile_shader_text(GL_VERTEX_SHADER, vstext);

	return shader;
}

/* Generate a FS that checks all the varyings written by the VS, and
 * outputs green if they are all correct.
 */
static GLint
get_fs(int num_varyings)
{
	GLuint shader;
	char fstext[2048];

	sprintf(fstext,
		"#version 120\n"
		"varying vec4[%d] v;\n"
		"\n"
		"void main()\n"
		"{\n"
		"  int i;\n"
		"  gl_FragColor = vec4(0.0, 1.0, 0.0, 1.0);\n"
		"  for(i = 0; i < v.length(); ++i) {\n"
		"    if (v[i] != 4.0*i + vec4(0.0, 1.0, 2.0, 3.0)) {\n"
		"      gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);\n"
		"    }\n"
		"  }\n"
		"}\n", num_varyings);

	shader = piglit_compile_shader_text(GL_FRAGMENT_SHADER, fstext);

	return shader;
}

/**
 * Initialize xfb_varying_array to contain the names of the varyings
 * used by get_vs and get_fs.
 */
static void
init_xfb_varying_array()
{
	int i;
	for (i = 0; i < MAX_VARYING; ++i) {
		char *buf = malloc(6);
		sprintf(buf, "v[%d]", i);
		xfb_varying_array[i] = buf;
	}
}

static int
coord_from_index(int index)
{
	return 2 + 12 * index;
}

static GLboolean
check_xfb_output(int max_varyings, int num_xfb_varyings, int offset)
{
	GLboolean pass = GL_TRUE;
	int vertex, varying, i;
	float (*buffer)[4] = glMapBuffer(GL_TRANSFORM_FEEDBACK_BUFFER,
					 GL_READ_ONLY);

	for (vertex = 0; vertex < 6; ++vertex) {
		for (varying = 0; varying < num_xfb_varyings; ++varying) {
			float expected[4];
			float *actual;
			for (i = 0; i < 4; ++i) {
				expected[i] = (varying + offset) * 4.0 + i;
			}
			actual = buffer[vertex * num_xfb_varyings + varying];
			if (memcmp(expected, actual, 4 * sizeof(float)) != 0) {
				printf("When recording %i varyings\n",
				       num_xfb_varyings);
				printf("Out of a total of %i\n", max_varyings);
				printf("With an offset of %i\n", offset);
				printf("Got incorrect transform feedback data "
				       "for vertex %i, varying %i\n", vertex,
				       varying);
				printf("Expected (%f, %f, %f, %f)\n",
				       expected[0], expected[1], expected[2],
				       expected[3]);
				printf("Actual (%f, %f, %f, %f)\n",
				       actual[0], actual[1], actual[2],
				       actual[3]);
				pass = GL_FALSE;
			}
		}
	}

	glUnmapBuffer(GL_TRANSFORM_FEEDBACK_BUFFER);

	return pass;
}

static GLboolean
draw(GLuint vs, GLuint fs, int num_xfb_varyings, int max_varyings)
{
	GLboolean pass = GL_TRUE;
	int offset;

	for (offset = 0; offset + num_xfb_varyings <= max_varyings; ++offset) {
		GLuint prog;
		float initial_buffer[MAX_VARYING * 6][4];

		prog = glCreateProgram();
		glAttachShader(prog, vs);
		glAttachShader(prog, fs);

		glTransformFeedbackVaryings(prog, num_xfb_varyings,
					    xfb_varying_array + offset,
					    GL_INTERLEAVED_ATTRIBS);

		glLinkProgram(prog);
		if (!piglit_link_check_status(prog))
			piglit_report_result(PIGLIT_FAIL);

		glUseProgram(prog);

		glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, xfb_buf);
		memset(initial_buffer, 0, sizeof(initial_buffer));
		glBufferData(GL_TRANSFORM_FEEDBACK_BUFFER,
			     sizeof(initial_buffer), initial_buffer,
			     GL_STREAM_READ);
		glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, xfb_buf);
		glBeginTransformFeedback(GL_TRIANGLES);

		piglit_draw_rect(coord_from_index(offset),
				 coord_from_index(num_xfb_varyings - 1),
				 10,
				 10);

		glEndTransformFeedback();
		pass = check_xfb_output(max_varyings, num_xfb_varyings, offset)
			&& pass;

		glDeleteProgram(prog);
	}

	return pass;
}

enum piglit_result
piglit_display(void)
{
	GLint max_components;
	int max_varyings, row, col;
	int max_xfb_varyings = 0;
	GLint max_xfb_components;
	GLboolean pass = GL_TRUE, warned = GL_FALSE;
	GLuint vs, fs;

	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);

	glGetIntegerv(GL_MAX_VARYING_FLOATS, &max_components);
	max_varyings = max_components / 4;

	printf("GL_MAX_VARYING_FLOATS = %i\n", max_components);

	if (max_varyings > MAX_VARYING) {
		printf("test not designed to handle >%d varying vec4s.\n"
		       "(implementation reports %d components)\n",
		       max_components, MAX_VARYING);
		max_varyings = MAX_VARYING;
		warned = GL_TRUE;
	}

	glGetIntegerv(GL_MAX_TRANSFORM_FEEDBACK_INTERLEAVED_COMPONENTS,
		      &max_xfb_components);
	max_xfb_varyings = MIN2(max_xfb_components / 4, max_varyings);

	printf("GL_MAX_TRANSFORM_FEEDBACK_INTERLEAVED_COMPONENTS = %i\n",
	       max_xfb_components);

	vs = get_vs(max_varyings);
	fs = get_fs(max_varyings);

	glClearColor(0.5, 0.5, 0.5, 0.5);
	glClear(GL_COLOR_BUFFER_BIT);

	for (row = 0; row < max_xfb_varyings; row++) {
		pass = draw(vs, fs, row + 1, max_varyings) && pass;
	}

	for (row = 0; row < max_xfb_varyings; row++) {
		for (col = 0; col < max_varyings - row; col++) {
			GLboolean ok;
			float green[3] = {0.0, 1.0, 0.0};

			ok = piglit_probe_rect_rgb(coord_from_index(col),
						   coord_from_index(row),
						   10, 10,
						   green);
			if (!ok) {
				printf("  Failure with %d vec4 varyings"
				       " captured and offset %d\n",
				       row + 1, col);
				pass = GL_FALSE;
				break;
			}
		}
	}

	piglit_present_results();

	if (!pass)
		return PIGLIT_FAIL;
	if (warned)
		return PIGLIT_WARN;
	else
		return PIGLIT_PASS;
}

void piglit_init(int argc, char **argv)
{
	piglit_require_gl_version(20);
	piglit_require_GLSL_version(120);
	piglit_require_transform_feedback();
	init_xfb_varying_array();
	glGenBuffers(1, &xfb_buf);

	printf("Vertical axis: Increasing numbers of varyings captured by "
	       "transform feedback.\n");
	printf("Horizontal axis: Offset of first varying captured.\n");
}

