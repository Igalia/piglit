/*
 * Copyright (c) 2016 VMware, Inc.
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

/** @file rendering-array.c
 *
 * Test rendering with a UBO containing an array of structs.
 * We draw four squares with different positions, sizes, rotations and colors
 * where those parameters come from an array in a UBO.  Each draw command
 * indexes into a different element of that array.
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
	"uniform ub_info { \n"
	"   struct { \n"
	"      vec2 pos; \n"
	"      float size; \n"
	"      float rotation; \n"
	"      vec4 color; \n"
	"   } info [4];\n"
	"};\n"
	"\n"
	"uniform int j; \n"
	"varying vec4 color; \n"
	"\n"
	"void main()\n"
	"{\n"
	"   int i; \n"
	"   mat2 m;\n"
	"   for (i = 0; i < 4; i++) { \n"
	"      if (i == j) { \n"
	"         m[0][0] = m[1][1] = cos(info[i].rotation); \n"
	"         m[0][1] = sin(info[i].rotation); \n"
	"         m[1][0] = -m[0][1]; \n"
	"         gl_Position.xy = m * gl_Vertex.xy * vec2(info[i].size) + info[i].pos;\n"
	"         gl_Position.zw = vec2(0, 1);\n"
	"         color = info[i].color; \n"
	"      } \n"
	"   } \n"
	"}\n";

static const char frag_shader_text[] =
	"#extension GL_ARB_uniform_buffer_object : require\n"
	"\n"
	"varying vec4 color; \n"
	"\n"
	"layout(std140) uniform;\n"
	"\n"
	"void main()\n"
	"{\n"
	"   gl_FragColor = color;\n"
	"}\n";

#define NUM_SQUARES 4

static GLuint prog;
static GLuint ubo_buffer;
static GLint alignment;
static bool test_buffer_offset = false;
static int uniform_j;

struct object_info {
	float pos[2];
	float size, rotation;
	float color[4];
};

/* This data are copied into the UBO */
static const struct object_info obj_info[NUM_SQUARES] = {
	{ {-0.5, -0.5}, 0.1, 0.0, {1.0, 0.0, 0.0, 1.0} },
	{ { 0.5, -0.5}, 0.2, 0.1, {0.0, 1.0, 0.0, 1.0} },
	{ {-0.5,  0.5}, 0.3, 0.2, {0.0, 0.0, 1.0, 1.0} },
	{ { 0.5,  0.5}, 0.4, 0.3, {1.0, 1.0, 1.0, 1.0} }
};


static void
setup_ubos(void)
{
	static const char *ubo_name = "ub_info";
	GLint ubo_index, ubo_size;

	glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &alignment);
	printf("GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT = %d\n", alignment);

	if (test_buffer_offset) {
		printf("Testing buffer offset %d\n", alignment);
	}
	else {
		/* we use alignment as the offset */
		alignment = 0;
	}

	glGenBuffers(1, &ubo_buffer);

	/* query UBO index */
	ubo_index = glGetUniformBlockIndex(prog, ubo_name);

	/* query UBO size */
	glGetActiveUniformBlockiv(prog, ubo_index,
				  GL_UNIFORM_BLOCK_DATA_SIZE, &ubo_size);

	printf("UBO %s: index = %d, size = %d\n",
	       ubo_name, ubo_index, ubo_size);

	assert(ubo_size == sizeof(obj_info));

	/* Allocate UBO and put object info into it */
	glBindBuffer(GL_UNIFORM_BUFFER, ubo_buffer);
	glBufferData(GL_UNIFORM_BUFFER, alignment + sizeof(obj_info),
		     NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_UNIFORM_BUFFER, alignment,
			sizeof(obj_info), obj_info);

	/* Attach UBO */
	glBindBufferRange(GL_UNIFORM_BUFFER, 0, ubo_buffer,
			  alignment,  /* offset */
			  ubo_size);
	glUniformBlockBinding(prog, ubo_index, 0);

	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);
}


void
piglit_init(int argc, char **argv)
{
	piglit_require_extension("GL_ARB_uniform_buffer_object");

	if (argc > 1 && strcmp(argv[1], "offset") == 0) {
		test_buffer_offset = true;
	}

	prog = piglit_build_simple_program(vert_shader_text, frag_shader_text);
	assert(prog);
	glUseProgram(prog);

	uniform_j = glGetUniformLocation(prog, "j");

	setup_ubos();

	glClearColor(0.2, 0.2, 0.2, 0.2);
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
		/* Take object parameters from array position [i] */
		glUniform1i(uniform_j, i);

		if (!piglit_check_gl_error(GL_NO_ERROR))
			return PIGLIT_FAIL;

		piglit_draw_rect(-1, -1, 2, 2);
	}

	pass = piglit_probe_pixel_rgba(x0, y0, obj_info[0].color) && pass;
	pass = piglit_probe_pixel_rgba(x1, y0, obj_info[1].color) && pass;
	pass = piglit_probe_pixel_rgba(x0, y1, obj_info[2].color) && pass;
	pass = piglit_probe_pixel_rgba(x1, y1, obj_info[3].color) && pass;

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}
