/*
 * Copyright © 2017 Fabian Bieler
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

/** @file transform-feedback-uniform-buffer-object.c
 *
 * Some tiling renderers reorder draw-calls for performance reasons.
 * This could interfere with the integrity of resources written to by draws
 * calls and read from by subsequent draw calls.
 *
 * This test uses a buffer object, written by transform feedback and read as a
 * uniform buffer object.
 *
 * With blending enabled, draw 3 identical passes:
 * Each pass consists of two sub-passes:
 * First sub-pass: Draw a grid of 8x8 transparent quads (each 32x32 pixels in
 * size). Write the index of the current pass with transform feedback in a
 * buffer object.
 * Second sub-pass: Draw another grid of 8x8 quads. If the buffer object
 * contains the index of the current pass output transparent, white otherwise.
 *
 * Check that the framebuffer is unaltered.
 */

#include "piglit-util-gl.h"

#define TILESIZE 32
#define X_TILES 8
#define Y_TILES 8
#define BO_SIZE (X_TILES * Y_TILES * 6 * 4 * sizeof(float))

PIGLIT_GL_TEST_CONFIG_BEGIN
	config.supports_gl_es_version = 30;
	config.window_visual = PIGLIT_GL_VISUAL_RGB;
	config.window_width = TILESIZE * X_TILES;
	config.window_height = TILESIZE * Y_TILES;
	config.khr_no_error_support = PIGLIT_NO_ERRORS;
PIGLIT_GL_TEST_CONFIG_END

static const char *vs_tf_src =
	"#version 300 es\n"
	"uniform int tf_pass;\n"
	"in vec4 piglit_vertex;\n"
	"flat out int out_pass;\n"
	"void main()\n"
	"{\n"
	"	out_pass = tf_pass;\n"
	"	gl_Position = piglit_vertex;\n"
	"}\n";

static const char *fs_tf_src =
	"#version 300 es\n"
	"precision highp float;\n"
	"out vec4 frag_color;\n"
	"void main()\n"
	"{\n"
	"	 frag_color = vec4(0.0);\n"
	"}\n";

static const char *vs_ubo_src =
	"#version 300 es\n"
	"uniform int ubo_pass;\n"
	"uniform int tile;\n"
	"uniform ubo {\n"
	"	int tf_pass[8 * 8 * 6];\n"
	"};\n"
	"in vec4 piglit_vertex;\n"
	"flat out vec4 color;\n"
	"void main()\n"
	"{\n"
	"	color = vec4(0.0);\n"
	"	for (int i = 0; i < tf_pass.length(); ++i)\n"
	"		if (ubo_pass != tf_pass[i])\n"
	"			color[ubo_pass] = 1.0;\n"
	"	gl_Position = piglit_vertex;\n"
	"}\n";

static const char *fs_ubo_src =
	"#version 300 es\n"
	"precision highp float;\n"
	"flat in vec4 color;\n"
	"out vec4 frag_color;\n"
	"void main()\n"
	"{\n"
	"	 frag_color = color;\n"
	"}\n";

static int tf_prog, ubo_prog;
static unsigned buffer_object;

void
piglit_init(int argc, char **argv)
{
	tf_prog = piglit_build_simple_program_unlinked(vs_tf_src, fs_tf_src);
	ubo_prog = piglit_build_simple_program(vs_ubo_src, fs_ubo_src);

	const char *name = "out_pass";
	glTransformFeedbackVaryings(tf_prog, 1, &name, GL_INTERLEAVED_ATTRIBS);
	glLinkProgram(tf_prog);

	glGenBuffers(1, &buffer_object);
	glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER, buffer_object);
	glBufferData(GL_TRANSFORM_FEEDBACK_BUFFER, BO_SIZE, NULL,
		     GL_STREAM_COPY);

	glBlendFunc(GL_ONE, GL_ONE);
	glEnable(GL_BLEND);
}

enum piglit_result
piglit_display(void)
{
	bool pass = true;
	const int num_passes = 3;
	const float black[] = {0, 0, 0};

	int tf_pass_loc = glGetUniformLocation(tf_prog, "tf_pass");
	int tile_loc = glGetUniformLocation(ubo_prog, "tile");
	int ubo_pass_loc = glGetUniformLocation(ubo_prog, "ubo_pass");
	int ubo_idx = glGetUniformBlockIndex(ubo_prog, "ubo");

	glClear(GL_COLOR_BUFFER_BIT);

	for (int i = 0; i < num_passes; ++i) {
		glUseProgram(tf_prog);
		glUniform1i(tf_pass_loc, i);
		glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0,
				 buffer_object);

		/* Don't enable GL_RASTERIZER_DISCARD to dare the driver
		 * to reorder those draw calls.
		 */
		glBeginTransformFeedback(GL_TRIANGLES);
		for (int y = 0; y < Y_TILES; ++y) {
			for (int x = 0; x < X_TILES; ++x) {
				const float w = 2.0 / X_TILES;
				const float h = 2.0 / Y_TILES;
				piglit_draw_rect(-1 + x * w, -1 + y * h,
						 w, h);
			}
		}
		glEndTransformFeedback();

		glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, 0);

		glUseProgram(ubo_prog);
		glUniform1i(ubo_pass_loc, i);
		glBindBufferBase(GL_UNIFORM_BUFFER, 0, buffer_object);
		glUniformBlockBinding(ubo_prog, ubo_idx, 0);

		for (int y = 0; y < Y_TILES; ++y) {
			for (int x = 0; x < X_TILES; ++x) {
				const float w = 2.0 / X_TILES;
				const float h = 2.0 / Y_TILES;
				glUniform1i(tile_loc, y * X_TILES + x);
				piglit_draw_rect(-1 + x * w, -1 + y * h,
						 w, h);
			}
		}

		glBindBufferBase(GL_UNIFORM_BUFFER, 0, 0);
	}

	/* Check result */
	pass = piglit_probe_rect_rgb(0, 0, piglit_width,
				     piglit_height, black) && pass;

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

