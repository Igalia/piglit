/*
 * Copyright © 2015 Intel Corporation
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
 * \file drawid.c
 *
 * Basic test for gl_DrawIDARB.
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
	"/* This is floating point so we can use immediate mode */\n"
	"layout(location = 0) in vec2 pos;\n"
	"layout(location = 1) in ivec2 ref;\n"
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

void
piglit_init(int argc, char **argv)
{
	GLuint prog;
	char *vs_text;

	if (strcmp(argv[1], "drawid") == 0) {
		asprintf(&vs_text, vs_template,
			 "ref.x == gl_DrawIDARB");
	} else if (strcmp(argv[1], "vertexid") == 0) {
		asprintf(&vs_text, vs_template,
			 "ref.x == gl_DrawIDARB && ref.y == gl_VertexID");
	} else {
                printf("Unknown subtest: %s\n", argv[1]);
		piglit_report_result(PIGLIT_FAIL);
	}

	piglit_require_GLSL_version(130);

	piglit_require_extension("GL_ARB_shader_draw_parameters");
	piglit_require_extension("GL_ARB_base_instance");

	prog = piglit_build_simple_program(vs_text, fs_text);

	glUseProgram(prog);
}

enum piglit_result
piglit_display()
{
	bool pass;

	struct {
		float vertex_array[16];
		int reference_array[16];
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
		.reference_array = {
			0, 0,
			0, 1,
			0, 2,
			0, 3,

			1, 4,
			1, 5,
			1, 6,
			1, 7,
		}
	};

	const int indices[12] = {
		0, 1, 2,
		0, 2, 3,

		4, 5, 6,
		4, 6, 7,
	};

	float green[] = {0, 1, 0, 1};

	GLuint vao, vbo;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(geometry), &geometry, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE,
			      2 * sizeof(GLfloat),
			      (void *) ((char *) &geometry.vertex_array - (char *) &geometry));

	glVertexAttribIPointer(1, 2, GL_UNSIGNED_INT,
			       2 * sizeof(int),
			       (void *) ((char *) &geometry.reference_array - (char *) &geometry));

	/* Enable the attributes */
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	GLsizei counts[2] = { 6, 6 };
	const int *indices_array[2] = { &indices[0], &indices[6] };

	glMultiDrawElements(GL_TRIANGLES,
			    counts,
			    GL_UNSIGNED_INT,
			    (const void **) indices_array,
			    2);

	pass = piglit_probe_rect_rgba(0, 0, piglit_width, piglit_height,
				      green);

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}
