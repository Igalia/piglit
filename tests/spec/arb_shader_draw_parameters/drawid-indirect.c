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
 * Test for gl_DrawIDARB with indirect multi-draw. For mesa, this is
 * interesting because this is the only mode where we actually get
 * multiple _mesa_prim into the backend in one call. This tests that
 * we properly reemit state to update gl_DrawIDARB between rendering,
 * which on i965 involves reemitting vertex buffer state.
 *
 * Also, on i965, we source the vertex and instance ID from an
 * internal vertex buffer for direct rdraw, but point the vertex
 * buffer the parameter buffer for indirect draws. The baseinstance
 * subtest verifies that this all works right. Conversely, the
 * vertexid subtest doesn't reference gl_DrawIDARB and is useful for
 * validating that we don't reemit vertex buffer state between multi
 * draw calls. We can't test for that with this test, of course, but
 * we can inspect the generate command stream from the driver.
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

void
piglit_init(int argc, char **argv)
{
	GLuint prog;
	char *vs_text;

	if (strcmp(argv[1], "drawid") == 0) {
		(void)!asprintf(&vs_text, vs_template,
			 "ref.x == gl_DrawIDARB");
	} else if (strcmp(argv[1], "basevertex") == 0) {
		(void)!asprintf(&vs_text, vs_template,
			 "ref.xy == ivec2(gl_DrawIDARB, gl_BaseVertexARB)");
	} else if (strcmp(argv[1], "baseinstance") == 0) {
		(void)!asprintf(&vs_text, vs_template,
			 "ref.xz == ivec2(gl_DrawIDARB, gl_BaseInstanceARB)");
	} else if (strcmp(argv[1], "vertexid") == 0) {
		(void)!asprintf(&vs_text, vs_template,
			 "ref.w == gl_VertexID");
	} else {
                printf("Unknown subtest: %s\n", argv[1]);
		piglit_report_result(PIGLIT_FAIL);
	}

	piglit_require_GLSL_version(330);

	piglit_require_extension("GL_ARB_shader_draw_parameters");
	piglit_require_extension("GL_ARB_base_instance");

	prog = piglit_build_simple_program(vs_text, fs_text);

	glUseProgram(prog);
}

struct cmd {
	GLuint  count;
	GLuint  instanceCount;
	GLuint  firstIndex;
	GLuint  baseVertex;
	GLuint  baseInstance;
};

enum piglit_result
piglit_display()
{
	bool pass;

	struct {
		float vertex_array[16];
		int reference_array[32];
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
			0, 0, 0, 0,
			0, 0, 0, 1,
			0, 0, 0, 2,
			0, 0, 0, 3,

			1, 4, 7, 4,
			1, 4, 7, 5,
			1, 4, 7, 6,
			1, 4, 7, 7,
		}
	};

	const int indices[12] = {
		0, 1, 2,
		0, 2, 3,
	};

	float green[] = {0, 1, 0, 1};

	GLuint vao, vbo, ibo, dbo;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(geometry), &geometry, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE,
			      2 * sizeof(GLfloat),
			      (void *) ((char *) &geometry.vertex_array - (char *) &geometry));

	glVertexAttribIPointer(1, 4, GL_UNSIGNED_INT,
			       4 * sizeof(int),
			       (void *) ((char *) &geometry.reference_array - (char *) &geometry));

	/* Enable the attributes */
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glGenBuffers(1, &ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	struct cmd cmds[] = {
		{
			.count = 6,
			.instanceCount = 1,
			.firstIndex = 0,
			.baseVertex = 0,
			.baseInstance = 0
		},
		{
			.count = 6,
			.instanceCount = 1,
			.firstIndex = 0,
			.baseVertex = 4,
			.baseInstance = 7
		}
	};

	glGenBuffers(1, &dbo);
	glBindBuffer(GL_DRAW_INDIRECT_BUFFER, dbo);
	glBufferData(GL_DRAW_INDIRECT_BUFFER, sizeof(cmds), cmds, GL_STATIC_DRAW);

	glMultiDrawElementsIndirect(GL_TRIANGLES,
				    GL_UNSIGNED_INT,
				    0, 2, 0);

	pass = piglit_probe_rect_rgba(0, 0, piglit_width, piglit_height,
				      green);

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}
