/*
 * Copyright © 2015, 2017 Intel Corporation
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

/**
 * \file basevertex.c
 *
 * Test that gl_BaseVertexARB has the correct values.
 *
 * The framebuffer is filled with three quads like this:
 *
 * #########
 * #   #   #
 * #   # B #
 * # A #####
 * #   # C #
 * #   #   #
 * #########
 *
 * Quad A is rendered using a non-base-vertex draw call to verify that
 * gl_BaseVertexARB is zero in that case.
 *
 * Quad B is rendered with baseVertex as 4.
 *
 * Quad C is rendered using a non-indexed draw call with a non-zero
 * ‘first’ parameter. This shouldn’t affect gl_BaseVertex but it
 * should affect gl_VertexID.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_core_version = 31;
	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

static const char vs_template[] =
	"#version 330\n"
	"#extension GL_ARB_shader_draw_parameters: require\n"
	"\n"
	"layout(location = 0) in vec2 pos;\n"
	"layout(location = 1) in ivec4 ref;\n"
	"out vec4 color;\n"
	"\n"
	"void main()\n"
	"{\n"
	"  gl_Position = vec4(pos, 0.0, 1.0);\n"
	"  if (%s)\n"
	"    color = vec4(0, 1, 0, 1);\n"
	"  else\n"
	"    color = vec4(1, 0, 0, 1);\n"
	"}\n";

static const char fs_text[] =
	"#version 130\n"
	"\n"
	"in vec4 color;\n"
	"\n"
	"void main()\n"
	"{\n"
	"  gl_FragColor = color;\n"
	"}\n";

static bool opt_draw_indirect = false;

void
piglit_init(int argc, char **argv)
{
	GLuint prog;
	char *vs_text;

	if (strcmp(argv[1], "basevertex") == 0) {
		(void)!asprintf(&vs_text, vs_template,
			 "ref.x == gl_BaseVertexARB");
	} else if (strcmp(argv[1], "baseinstance") == 0) {
		(void)!asprintf(&vs_text, vs_template,
			 "ref.y == gl_BaseInstanceARB");
	} else if (strcmp(argv[1], "basevertex-baseinstance") == 0) {
		(void)!asprintf(&vs_text, vs_template,
			 "ref.xy == ivec2(gl_BaseVertexARB, gl_BaseInstanceARB)");
	} else if (strcmp(argv[1], "vertexid-zerobased") == 0) {
		(void)!asprintf(&vs_text, vs_template,
			 "ref.z == gl_VertexID - gl_BaseVertexARB");
	} else {
                printf("Unknown subtest: %s\n", argv[1]);
		piglit_report_result(PIGLIT_FAIL);
	}

	if (argc > 2) {
		if (strcmp(argv[2], "indirect") == 0) {
			opt_draw_indirect = true;
		} else {
			printf("Unknown second argument: %s\n", argv[2]);
			piglit_report_result(PIGLIT_FAIL);
		}
	}

	piglit_require_GLSL_version(330);

	piglit_require_extension("GL_ARB_shader_draw_parameters");
	piglit_require_extension("GL_ARB_base_instance");
	if (opt_draw_indirect)
		piglit_require_extension("GL_ARB_draw_indirect");

	prog = piglit_build_simple_program(vs_text, fs_text);

	glUseProgram(prog);
}

static void
draw_direct(void)
{
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);

	/* We use this monster to draw the right half of the
	 * window. Base vertex so that we can reuse the indices to
	 * draw with vertices and colors 4-7, base instance so that we
	 * can verify that the value presented in the shader is
	 * correct. We only draw one instance so the only effect of
	 * instancing is that gl_BaseInstanceARB is 7.
	 */
	glDrawElementsInstancedBaseVertexBaseInstance(GL_TRIANGLES, 6,
						      GL_UNSIGNED_INT,
						      NULL, 1,
						      4, /* basevertex */
						      7 /* baseinstance */);

	/* Test using glDrawArrays with a non-zero ‘first’ parameter.
	 * This value should be included in gl_VertexID but not in
	 * gl_BaseVertex.
	 */
	glDrawArrays(GL_TRIANGLE_STRIP,
		     8, /* first */
		     4 /* count */);
}

static void
draw_indirect(void)
{
	GLuint params_bo;

	static const GLuint draw_params[] = {
		6, /* count */
		1, /* prim count */
		0, /* firstIndex */
		0, /* baseVertex */
		0, /* baseInstance */

		6, /* count */
		1, /* prim count */
		0, /* firstIndex */
		4, /* baseVertex */
		7, /* baseInstance */

		4, /* count */
		1, /* prim count */
		8, /* first */
		0, /* baseInstance */
	};

	glGenBuffers(1, &params_bo);
	glBindBuffer(GL_DRAW_INDIRECT_BUFFER, params_bo);
	glBufferData(GL_DRAW_INDIRECT_BUFFER,
		     sizeof draw_params,
		     draw_params,
		     GL_STATIC_DRAW);

	/* The draw commands are all equivalent to those in draw_direct. */

	glDrawElementsIndirect(GL_TRIANGLES,
			       GL_UNSIGNED_INT,
			       (void *) 0);

	glDrawElementsIndirect(GL_TRIANGLES,
			       GL_UNSIGNED_INT,
			       (void *) (5 * sizeof draw_params[0]));

	glDrawArraysIndirect(GL_TRIANGLE_STRIP,
			       (void *) (10 * sizeof draw_params[0]));

	glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);
	glDeleteBuffers(1, &params_bo);
}

enum piglit_result
piglit_display()
{
	bool pass;

	static const float vertex_array[24] = {
		/* Left half of the screen */
		-1, -1,
		0, -1,
		0, 1,
		-1, 1,

		/* Top-right quarter of the screen */
		0, 0,
		1, 0,
		1, 1,
		0, 1,

		/* Bottom-right quarter of the screen */
		0, -1,
		1, -1,
		0, 0,
		1, 0,
	};

	static const int reference_array[48] = {
		0, 0, 0, 0,
		0, 0, 1, 0,
		0, 0, 2, 0,
		0, 0, 3, 0,
		4, 7, 0, 0,
		4, 7, 1, 0,
		4, 7, 2, 0,
		4, 7, 3, 0,
		0, 0, 8, 0,
		0, 0, 9, 0,
		0, 0, 10, 0,
		0, 0, 11, 0,
	};

	const int indices[6] = {
		0, 1, 2,
		0, 2, 3,
	};

	float green[] = { 0, 1, 0, 1 };

	GLuint vao, vbo, ibo;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, 2048, NULL, GL_STATIC_DRAW);

	glGenBuffers(1, &ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof indices, indices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), 0);
	glVertexAttribIPointer(1, 4, GL_UNSIGNED_INT, 4 * sizeof(int), (void *) 1024);

	/* Enable the attributes */
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertex_array), vertex_array);
	glBufferSubData(GL_ARRAY_BUFFER, 1024, sizeof(reference_array), reference_array);

	if (opt_draw_indirect)
		draw_indirect();
	else
		draw_direct();

	pass = piglit_probe_rect_rgba(0, 0, piglit_width, piglit_height,
				      green);

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}
