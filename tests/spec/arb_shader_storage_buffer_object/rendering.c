/*
 * Copyright (c) 2015 Intel Corporation
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
 * Test rendering with SSBOs.  We draw four squares with different positions,
 * sizes, rotations and colors where those parameters come from SSBOs.
 *
 * Based on GL_ARB_uniform_buffer_object's rendering.c
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 32;
	config.supports_gl_core_version = 32;
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;
	config.khr_no_error_support = PIGLIT_NO_ERRORS;

PIGLIT_GL_TEST_CONFIG_END

static const char vert_shader_text[] =
	"#version 130\n"
	"#extension GL_ARB_shader_storage_buffer_object : require\n"
	"\n"
	"buffer ssbo_pos_size { vec2 pos; float size; };\n"
	"buffer ssbo_rot {float rotation; };\n"
	"in vec4 piglit_vertex;\n"
	"\n"
	"void main()\n"
	"{\n"
	"   mat2 m;\n"
	"   m[0][0] = m[1][1] = cos(rotation); \n"
	"   m[0][1] = sin(rotation); \n"
	"   m[1][0] = -m[0][1]; \n"
	"   gl_Position.xy = m * piglit_vertex.xy * vec2(size) + pos;\n"
	"   gl_Position.zw = vec2(0, 1);\n"
	"}\n";

static const char vert_shader_no_ssbo_text[] =
	"#version 130\n"
	"#extension GL_ARB_uniform_buffer_object : require\n"
	"\n"
	"layout(std140) uniform;\n"
	"uniform ub_pos_size { vec2 pos; float size; };\n"
	"uniform ub_rot {float rotation; };\n"
	"in vec4 piglit_vertex;\n"
	"\n"
	"void main()\n"
	"{\n"
	"   mat2 m;\n"
	"   m[0][0] = m[1][1] = cos(rotation); \n"
	"   m[0][1] = sin(rotation); \n"
	"   m[1][0] = -m[0][1]; \n"
	"   gl_Position.xy = m * piglit_vertex.xy * vec2(size) + pos;\n"
	"   gl_Position.zw = vec2(0, 1);\n"
	"}\n";

static const char frag_shader_text[] =
	"#version 130\n"
	"#extension GL_ARB_shader_storage_buffer_object : require\n"
	"\n"
	"buffer ssbo_color { vec4 color; float color_scale; };\n"
	"\n"
	"void main()\n"
	"{\n"
	"   gl_FragColor = color * color_scale;\n"
	"}\n";

#define NUM_SQUARES 4
#define NUM_SSBOS 3

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
static GLuint buffers[NUM_SSBOS];
static GLint alignment, ubo_alignment;
static bool test_buffer_offset = false;
static bool vertex_ssbo = false;

static void
setup_ubos(void)
{
	static const char *names[NUM_SSBOS] = {
		"ssbo_pos_size",
		"ssbo_color",
		"ssbo_rot"
	};
	static const char *ubo_names[NUM_SSBOS] = {
		"ub_pos_size",
		"ssbo_color",
		"ub_rot"
	};

	bool vs_ubo[NUM_SSBOS] = {true, false, true};
	static GLubyte zeros[1000] = {0};
	int i;

	glGetIntegerv(GL_SHADER_STORAGE_BUFFER_OFFSET_ALIGNMENT, &alignment);
	printf("GL_SHADER_STORAGE_BUFFER_OFFSET_ALIGNMENT = %d\n", alignment);

	if (!vertex_ssbo) {
		glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &ubo_alignment);
		printf("GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT = %d\n", ubo_alignment);
	}
	if (test_buffer_offset) {
		printf("Testing buffer offset %d\n", alignment);
	}
	else {
		/* we use alignment as the offset */
		alignment = 0;
	}

	glGenBuffers(NUM_SSBOS, buffers);

	for (i = 0; i < NUM_SSBOS; i++) {
		GLint index, size;

		if (!vertex_ssbo && vs_ubo[i]) {
			/* query UBO index */
			index = glGetUniformBlockIndex(prog, ubo_names[i]);

			/* query UBO size */
			glGetActiveUniformBlockiv(prog, index,
						  GL_UNIFORM_BLOCK_DATA_SIZE, &size);
			printf("UBO %s: index = %d, size = %d\n",
			       ubo_names[i], index, size);

			/* Allocate UBO */
			/* XXX for some reason, this test doesn't work at all with
			 * nvidia if we pass NULL instead of zeros here.  The UBO data
			 * is set/overwritten in the piglit_display() function so this
			 * really shouldn't matter.
			 */
			glBindBuffer(GL_UNIFORM_BUFFER, buffers[i]);
			glBufferData(GL_UNIFORM_BUFFER, size + alignment,
				     zeros, GL_DYNAMIC_DRAW);

			/* Attach UBO */
			glBindBufferRange(GL_UNIFORM_BUFFER, i, buffers[i],
					  alignment,  /* offset */
					  size);
			glUniformBlockBinding(prog, index, i);
		} else {
			/* query SSBO index */
			index = glGetProgramResourceIndex(prog,
							  GL_SHADER_STORAGE_BLOCK,
							  names[i]);

			GLenum prop = GL_BUFFER_DATA_SIZE;
			/* query SSBO size */
			glGetProgramResourceiv(prog, GL_SHADER_STORAGE_BLOCK, index,
					       1, &prop, 1, NULL, &size);
			printf("SSBO %s: index = %d, size = %d\n",
			       names[i], index, size);

			/* Allocate SSBO */
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, buffers[i]);
			glBufferData(GL_SHADER_STORAGE_BUFFER, size + alignment,
				     zeros, GL_DYNAMIC_DRAW);

			/* Attach SSBO */
			glBindBufferRange(GL_SHADER_STORAGE_BUFFER, i, buffers[i],
					  alignment,  /* offset */
					  size);
			glShaderStorageBlockBinding(prog, index, i);
		}

		if (!piglit_check_gl_error(GL_NO_ERROR))
			piglit_report_result(PIGLIT_FAIL);
	}
}


void
piglit_init(int argc, char **argv)
{
	int num_vertex_ssbo = 0;

	piglit_require_extension("GL_ARB_shader_storage_buffer_object");
	piglit_require_extension("GL_ARB_program_interface_query");

	if (argc > 1 && strcmp(argv[1], "offset") == 0) {
		test_buffer_offset = true;
	}

	glGetIntegerv(GL_MAX_VERTEX_SHADER_STORAGE_BLOCKS, &num_vertex_ssbo);

	if (num_vertex_ssbo == 0)
		vertex_ssbo = false;

	glViewport(0, 0, piglit_width, piglit_height);

	prog = piglit_build_simple_program(vertex_ssbo ? vert_shader_text : vert_shader_no_ssbo_text, frag_shader_text);
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
		/* Load UBO data, at offset=alignment */
		if (!vertex_ssbo) {
			glBindBuffer(GL_UNIFORM_BUFFER, buffers[0]);
			glBufferSubData(GL_UNIFORM_BUFFER, alignment, sizeof(pos_size[0]),
					pos_size[i]);
		} else {
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, buffers[0]);
			glBufferSubData(GL_SHADER_STORAGE_BUFFER, alignment,
					sizeof(pos_size[0]), pos_size[i]);
		}
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, buffers[1]);
		glBufferSubData(GL_SHADER_STORAGE_BUFFER, alignment,
				sizeof(color[0]), color[i]);
		if (!vertex_ssbo) {
			glBindBuffer(GL_UNIFORM_BUFFER, buffers[2]);
			glBufferSubData(GL_UNIFORM_BUFFER, alignment, sizeof(rotation[0]),
					&rotation[i]);
		} else {
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, buffers[2]);
			glBufferSubData(GL_SHADER_STORAGE_BUFFER, alignment,
					sizeof(rotation[0]), &rotation[i]);
		}

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
