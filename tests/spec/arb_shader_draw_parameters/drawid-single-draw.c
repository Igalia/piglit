/*
 * Copyright © 2015, 2020 Intel Corporation
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
 * \file drawid-single-draw.c
 *
 * The GL_ARB_shader_draw_parameters spec says:
 *
 *    In non-MultiDraw* commands, the value of <gl_DrawIDARB> is always zero.
 *
 * This test contains two variations.  The first variation attempts a bunch of
 * non-MultiDraw* commands and verifies that gl_DrawIDARB is always zero.  The
 * second variation does the same thing using compatibility profile display
 * lists.  The display lists are constructed in a way that an implementation
 * may coalesce the draws into a single operation that resembles a MultDraw*
 * command.
 */

#include "piglit-util-gl.h"

static bool use_dlist = false;

PIGLIT_GL_TEST_CONFIG_BEGIN

	for (unsigned i = 1; i < argc; i++) {
		if (strcmp(argv[i], "dlist") == 0) {
			printf("Using display lists.\n");
			use_dlist = true;
		}
	}

	if (use_dlist) {
		config.supports_gl_compat_version = 31;
		config.supports_gl_core_version = 0;
	} else {
		config.supports_gl_compat_version = 0;
		config.supports_gl_core_version = 31;
	}
	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

static const char vs_text[] =
	"#version 140\n"
	"#extension GL_ARB_shader_draw_parameters: require\n"
	"\n"
	"in vec4 piglit_vertex;\n"
	"out vec4 color;\n"
	"\n"
	"void main()\n"
	"{\n"
	"  gl_Position = piglit_vertex;\n"
	"  color = gl_DrawIDARB == 0 ? vec4(0, 1, 0, 1) : vec4(1, 0, 0, 1);\n"
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

void
piglit_init(int argc, char **argv)
{
	GLuint prog;

	piglit_require_extension("GL_ARB_shader_draw_parameters");
	piglit_require_extension("GL_ARB_base_instance");

	prog = piglit_build_simple_program(vs_text, fs_text);

	glUseProgram(prog);
}

enum piglit_result
piglit_display()
{
	static GLint dlist = 0;
	bool pass;

	struct {
		float vertex_array[16];
		int indices[6];
	} geometry = {
		.vertex_array = {
			-1, -1,
			0, -1,
			0, 1,
			-1, 1,

			0, -1,
			1, -1,
			1, 1,
			0, 1,
		},
	};

	const int indices[12] = {
		0, 1, 2,
		0, 2, 3,

		4, 5, 6,
		4, 6, 7,
	};

	float green[] = {0, 1, 0, 1};

	glClear(GL_COLOR_BUFFER_BIT);

	if (dlist == 0) {
		if (use_dlist)
			dlist = 6;

		GLuint vao, vbo;
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);

		glGenBuffers(1, &vbo);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(geometry), &geometry,
			     GL_STATIC_DRAW);

		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE,
				      2 * sizeof(GLfloat),
				      (void *) ((char *) &geometry.vertex_array -
						(char *) &geometry));

		/* Enable the attributes */
		glEnableVertexAttribArray(0);

		if (dlist != 0)
			glNewList(dlist, GL_COMPILE);

		glDrawElements(GL_TRIANGLES,
			       6,
			       GL_UNSIGNED_INT,
			       &indices[0]);

		glDrawElements(GL_TRIANGLES,
			       6,
			       GL_UNSIGNED_INT,
			       &indices[6]);

		if (dlist != 0)
			glEndList();
	}

	if (dlist != 0)
		glCallList(dlist);

	pass = piglit_probe_rect_rgba(0, 0, piglit_width, piglit_height,
				      green);

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}
