/*
 * Copyright © 2014 Intel Corporation
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
 * \file binding-layout.c
 * Try some shaders with UBOs that use layout(binding=N).  Verify that the API
 * reports back the correct binding, and verify that the correct thing is used
 * for rendering.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_core_version = 31;

	config.window_width = 100;
	config.window_height = 100;
	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

static const char vert140_source[] =
	"#version 140\n"
	"#extension GL_ARB_explicit_attrib_location: require\n"
	"\n"
	"layout(location=0) in vec4 piglit_vertex;\n"
	"void main() { gl_Position = piglit_vertex; }\n"
	;

static const char *frag140_source =
	"#version 140\n"
	"#extension GL_ARB_shading_language_420pack: require\n"
	"#extension GL_ARB_explicit_attrib_location: require\n"
	"#extension GL_ARB_uniform_buffer_object: require\n"
	"\n"
	"layout(location=0) out vec4 o;\n"
	"layout(binding=2, std140) uniform U { vec4 a; };\n"
	"void main() { o = a; }\n"
	;

static const char vert150_source[] =
	"#version 150 core\n"
	"#extension GL_ARB_explicit_attrib_location: require\n"
	"\n"
	"layout(location=0) in vec4 piglit_vertex;\n"
	"void main() { gl_Position = piglit_vertex; }\n"
	;

static const char *frag150_source =
	"#version 150 core\n"
	"#extension GL_ARB_shading_language_420pack: require\n"
	"#extension GL_ARB_explicit_attrib_location: require\n"
	"\n"
	"layout(location=0) out vec4 o;\n"
	"layout(binding=3, std140) uniform U { vec4 a; } u[2];\n"
	"void main() { o = (u[0].a + u[1].a) / 5.0; }\n"
	;

static GLuint prog140 = 0;
static GLuint prog150 = 0;

static bool
try_140_test()
{
	bool pass = true;
	GLint idx;
	GLint binding;

	prog140 = piglit_build_simple_program(vert140_source, frag140_source);

	idx = glGetUniformBlockIndex(prog140, "U");
	if (idx == -1) {
		printf("Failed to get index for \"U\"\n");
		pass = false;
	}

	glGetActiveUniformBlockiv(prog140, idx, GL_UNIFORM_BLOCK_BINDING,
				  &binding);
	if (binding != 2) {
		printf("Expected block binding = 2, got %d\n", binding);
		pass = false;
	}

	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

	return pass;
}

static bool
try_150_test()
{
	bool pass = true;
	GLint idx;
	GLint binding;
	unsigned i;

	prog150 = piglit_build_simple_program(vert150_source, frag150_source);

	for (i = 0; i < 2; i++) {
		char name[5] = "U[0]";

		name[2] = '0' + i;

		idx = glGetUniformBlockIndex(prog150, name);
		if (idx == -1) {
			printf("Failed to get index for \"%s\"\n", name);
			pass = false;
		}

		glGetActiveUniformBlockiv(prog150, idx,
					  GL_UNIFORM_BLOCK_BINDING, &binding);
		if (binding != (3 + i)) {
			printf("Expected block binding = %d, got %d\n",
			       3 + i, binding);
			pass = false;
		}
	}

	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

	return pass;
}

void
piglit_init(int argc, char **argv)
{
	static const float data[] = {
		0.0, 1.0, 0.0, 1.0,
		0.0, 2.0, 0.0, 1.0,
		0.0, 3.0, 0.0, 0.0,
	};
	bool pass = true;
	GLuint bo;

	piglit_require_extension("GL_ARB_shading_language_420pack");
	piglit_require_extension("GL_ARB_explicit_attrib_location");

	pass = try_140_test() && pass;

	if (piglit_get_gl_version() >= 32)
		pass = try_150_test() && pass;

	/* If the set-up tests failed, don't even bother trying to render.
	 * That can only lead to more failure.  We don't need to rub it in.
	 */
	if (!pass)
		piglit_report_result(PIGLIT_FAIL);

	glGenBuffers(1, &bo);
	glBindBuffer(GL_UNIFORM_BUFFER, bo);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(data), data, GL_STATIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	glBindBufferRange(GL_UNIFORM_BUFFER, 2, bo,  0, 16);
	glBindBufferRange(GL_UNIFORM_BUFFER, 3, bo, 16, 16);
	glBindBufferRange(GL_UNIFORM_BUFFER, 4, bo, 32, 16);

	glClearColor(0.5, 0.5, 0.5, 1.0);
}

enum piglit_result piglit_display(void)
{
	static const float green[] = { 0.0, 1.0, 0.0, 1.0 };

	glClear(GL_COLOR_BUFFER_BIT);

	glUseProgram(prog140);
	piglit_draw_rect(-1.0f, -1.0f, 1.0f, 2.0f);
	piglit_probe_rect_rgb(0, 0,
			      piglit_width / 2, piglit_height,
			      green);

	if (prog150 != 0) {
		glUseProgram(prog150);
		piglit_draw_rect(0.0f, -1.0f, 1.0f, 2.0f);
		piglit_probe_rect_rgb(piglit_width / 2, 0,
				      piglit_width / 2, piglit_height,
				      green);
	}

	piglit_present_results();

	return PIGLIT_PASS;
}
