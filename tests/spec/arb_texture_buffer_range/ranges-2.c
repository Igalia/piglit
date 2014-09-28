/* Copyright © 2014 Intel Corporation
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

/** @file ranges-2.c
 *
 * Test that *just* changing the bound range of a TexBO (without changing
 * anything else) works. This is to demonstrate a bug in Mesa's dirty state
 * flagging.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

        config.supports_gl_core_version = 31;

        config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;

PIGLIT_GL_TEST_CONFIG_END

GLuint prog;
GLuint vao;
GLuint tbo;
GLuint tex;

char const *vs_source =
"#version 140\n"
"uniform samplerBuffer s;\n"
"out vec4 color;\n"
"void main() {\n"
"	vec4 x = texelFetch(s, gl_VertexID);\n"
"	gl_Position = vec4(x.xy, 0, 1);\n"
"	color = vec4(x.zw, 0, 1);\n"
"}\n";

char const *fs_source =
"#version 140\n"
"in vec4 color;\n"
"out vec4 frag_color;\n"
"void main() {\n"
"	frag_color = color;\n"
"}\n";

float data[] = {
	-1, -1,		0, 1,
	0, -1,		0, 1,
	0, 0,		0, 1,
	-1, -1,		0, 1,
	0, 0,		0, 1,
	-1, 0,		0, 1,

	-1, 0,		0, 0.5,
	0, 0,		0, 0.5,
	0, 1,		0, 0.5,
	-1, 0,		0, 0.5,
	0, 1,		0, 0.5,
	-1, 1,		0, 0.5,

	0, 0,		1, 0,
	1, 0,		1, 0,
	1, 1,		1, 0,
	0, 0,		1, 0,
	1, 1,		1, 0,
	0, 1,		1, 0,

	0, -1,		0.5, 0,
	1, -1,		0.5, 0,
	1, 0,		0.5, 0,
	0, -1,		0.5, 0,
	1, 0,		0.5, 0,
	0, 0,		0.5, 0,
};

enum piglit_result
piglit_display(void) {
	int i;
	int chunk_size = 24 * sizeof(float);
	bool pass = true;

	glClearColor(0.2, 0.2, 0.2, 0.2);
	glClear(GL_COLOR_BUFFER_BIT);

	for (i = 0; i < sizeof(data) / chunk_size; i++) {
		glTexBufferRange(GL_TEXTURE_BUFFER, GL_RGBA32F,
				 tbo, i * chunk_size, chunk_size);
		glDrawArrays(GL_TRIANGLES, 0, 6);
	}

	for (i = 0; i < sizeof(data) / chunk_size; i++) {
		float c[4] = {
			data[i * 24 + 2],
			data[i * 24 + 3],
			0,
			1
		};

		pass = piglit_probe_rect_rgba(
			piglit_width * 0.5 * (1 + data[i * 24 + 0]),
			piglit_height * 0.5 * (1 + data[i * 24 + 1]),
			piglit_width/2,
			piglit_height/2, c) && pass;
	}

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv) {
	prog = piglit_build_simple_program(vs_source, fs_source);
	glUseProgram(prog);

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glGenBuffers(1, &tbo);
	glBindBuffer(GL_ARRAY_BUFFER, tbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(data), data, GL_STATIC_DRAW);

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_BUFFER, tex);
}
