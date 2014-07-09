/*
 * Copyright (c) 2014 VMware, Inc.
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

/** @file rendering.c
 *
 * Test rendering with UBOs.  We draw four squares with different positions,
 * sizes, rotations and colors where those parameters come from UBOs.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 20;
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;

PIGLIT_GL_TEST_CONFIG_END

static const char vert_shader_text[] =
	"#extension GL_ARB_uniform_buffer_object : require\n"
	"\n"
	"layout(std140) uniform;\n"
	"uniform ub_pos_size { vec2 pos; float size; };\n"
	"uniform ub_color { vec4 color; float color_scale; };\n"
	"uniform ub_rot {float rotation; };\n"
	"\n"
	"void main()\n"
	"{\n"
	"   mat2 m;\n"
	"   m[0][0] = m[1][1] = cos(rotation); \n"
	"   m[0][1] = sin(rotation); \n"
	"   m[1][0] = -m[0][1]; \n"
	"   gl_Position.xy = m * gl_Vertex.xy * vec2(size) + pos;\n"
	"   gl_Position.zw = vec2(0, 1);\n"
	"   gl_FrontColor = color * color_scale;\n"
	"}\n";

static const char frag_shader_text[] =
	"#extension GL_ARB_uniform_buffer_object : require\n"
	"\n"
	"void main()\n"
	"{\n"
	"	gl_FragColor = gl_Color;\n"
	"}\n";

#define NUM_SQUARES 4
#define NUM_UBOS 3

/* Square positions and sizes */
static const float pos_size[NUM_SQUARES][3] = {
	{ -0.5, -0.5, 0.1 },
	{  0.5, -0.5, 0.2 },
	{ -0.5, 0.5, 0.3 },
	{  0.5, 0.5, 0.4 }
};

/* Square color and color_scales */
static const float color[NUM_SQUARES][8] = {
	{ 2.0, 0.0, 0.0, 1.0,   0.50, 0.0, 0.0, 0.0 },
	{ 0.0, 4.0, 0.0, 1.0,   0.25, 0.0, 0.0, 0.0 },
	{ 0.0, 0.0, 5.0, 1.0,   0.20, 0.0, 0.0, 0.0 },
	{ 0.2, 0.2, 0.2, 0.2,   5.00, 0.0, 0.0, 0.0 }
};

/* Square rotations */
static const float rotation[NUM_SQUARES] = {
	0.0,
	0.1,
	0.2,
	0.3
};

static GLuint prog;
static GLuint buffers[NUM_UBOS];


static void
setup_ubos(void)
{
	static const char *names[NUM_UBOS] = {
		"ub_pos_size",
		"ub_color",
		"ub_rot"
	};
	static GLubyte zeros[1000] = {0};
	int i;

	glGenBuffers(NUM_UBOS, buffers);

	for (i = 0; i < NUM_UBOS; i++) {
		GLint index, size;

		/* query UBO index */
		index = glGetUniformBlockIndex(prog, names[i]);

		/* query UBO size */
		glGetActiveUniformBlockiv(prog, index,
					  GL_UNIFORM_BLOCK_DATA_SIZE, &size);

		printf("UBO %s: index = %d, size = %d\n",
		       names[i], index, size);

		/* Allocate UBO */
		/* XXX for some reason, this test doesn't work at all with
		 * nvidia if we pass NULL instead of zeros here.  The UBO data
		 * is set/overwritten in the piglit_display() function so this
		 * really shouldn't matter.
		 */
		glBindBuffer(GL_UNIFORM_BUFFER, buffers[i]);
		glBufferData(GL_UNIFORM_BUFFER, size, zeros, GL_DYNAMIC_DRAW);

		/* Attach UBO */
		glBindBufferBase(GL_UNIFORM_BUFFER, i, buffers[i]);
		glUniformBlockBinding(prog, index, i);

		if (!piglit_check_gl_error(GL_NO_ERROR))
			piglit_report_result(PIGLIT_FAIL);
	}
}


void
piglit_init(int argc, char **argv)
{
	piglit_require_extension("GL_ARB_uniform_buffer_object");

	prog = piglit_build_simple_program(vert_shader_text, frag_shader_text);
	assert(prog);
	glUseProgram(prog);

	setup_ubos();

	glClearColor(0.2, 0.2, 0.2, 0.2);
}


static bool
probe(int x, int y, int color_index)
{
	float expected[4];

	/* mul color by color_scale */
	expected[0] = color[color_index][0] * color[color_index][4];
	expected[1] = color[color_index][1] * color[color_index][4];
	expected[2] = color[color_index][2] * color[color_index][4];
	expected[3] = color[color_index][3] * color[color_index][4];

	return piglit_probe_pixel_rgba(x, y, expected);
}


enum piglit_result
piglit_display(void)
{
	bool pass = true;
	int x0 = piglit_width / 4;
	int x1 = piglit_width * 3 / 4;
	int y0 = piglit_height / 4;
	int y1 = piglit_height * 3 / 4;
	int i;

	glViewport(0, 0, piglit_width, piglit_height);

	glClear(GL_COLOR_BUFFER_BIT);

	for (i = 0; i < NUM_SQUARES; i++) {
		/* Load UBO data */
		glBindBuffer(GL_UNIFORM_BUFFER, buffers[0]);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(pos_size[0]),
				pos_size[i]);
		glBindBuffer(GL_UNIFORM_BUFFER, buffers[1]);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(color[0]),
				color[i]);
		glBindBuffer(GL_UNIFORM_BUFFER, buffers[2]);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(rotation[0]),
				&rotation[i]);

		if (!piglit_check_gl_error(GL_NO_ERROR))
			return PIGLIT_FAIL;

		piglit_draw_rect(-1, -1, 2, 2);
	}

	pass = probe(x0, y0, 0) && pass;
	pass = probe(x1, y0, 1) && pass;
	pass = probe(x0, y1, 2) && pass;
	pass = probe(x1, y1, 3) && pass;

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}
