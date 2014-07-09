/* Copyright © 2012 Intel Corporation
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

/** @file ranges.c
 *
 * Test drawing with various ranges and sizes for glTexBufferRange.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

        config.supports_gl_compat_version = 10;
        config.supports_gl_core_version = 31;

        config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;

PIGLIT_GL_TEST_CONFIG_END

GLuint prog;
GLuint tbo;
GLuint tex;

uint8_t *data;

GLint vertex_location;

#define TBO_WIDTH 1024
#define TBO_SIZE  TBO_WIDTH

/* NOTE: must adjust shader when changing WIN_WIDTH */
#define WIN_WIDTH 32
#define WIN_HEIGHT (TBO_WIDTH / WIN_WIDTH)

enum piglit_result
test_range(GLint offset, GLint size)
{
	const float green[4] = { 0, 1, 0, 0 };

	glUseProgram(prog);

	glBindTexture(GL_TEXTURE_BUFFER, tex);
	glTexBufferRange(GL_TEXTURE_BUFFER, GL_R8UI, tbo, offset, size);

	glUniform1i(glGetUniformLocation(prog, "buf"), 0);
	glUniform1i(glGetUniformLocation(prog, "offset"), offset);
	glUniform1i(glGetUniformLocation(prog, "size"), size);

	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	if (!piglit_probe_rect_rgba(0, 0, WIN_WIDTH, WIN_HEIGHT, green))
		return PIGLIT_FAIL;
	return PIGLIT_PASS;
}

enum piglit_result
piglit_display(void)
{
	enum piglit_result result = PIGLIT_SKIP;
	GLint i, j, n;
	GLuint vao, vbo;
	GLint incr;
	const float verts[] =
		{ -1.0f, -1.0f,  -1.0f, 1.0f,  1.0f, 1.0f,  1.0f, -1.0f };

	glClearColor(0.5, 0.5, 0.5, 0.5);
	glClear(GL_COLOR_BUFFER_BIT);

	/* For GL core, we need to have a vertex array object bound.
	 * Otherwise, we don't particularly have to.  Always use a
	 * vertex buffer object, though.
	 */
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER_ARB, vbo);
	if (piglit_get_gl_version() >= 31) {
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);
	}
	glVertexAttribPointer(vertex_location, 2, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(vertex_location);

	glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);

	glGetIntegerv(GL_TEXTURE_BUFFER_OFFSET_ALIGNMENT, &incr);
	if (!incr)
		return PIGLIT_FAIL;
	for (n = 0, i = 0; i < TBO_SIZE && result != PIGLIT_FAIL; i += incr) {
		for (j = 1; j <= 4 && result != PIGLIT_FAIL; ++j, ++n) {
			GLint size = (TBO_SIZE - i) / j;
			if (!size)
				break;
			result = test_range(i, (TBO_SIZE - i) / j);
		}
		if (n > 128) { /* takes too long otherwise */
			n = 0;
			incr *= 2;
		}
	}

	glDeleteBuffers(1, &vbo);
	if (piglit_get_gl_version() >= 31)
		glDeleteVertexArrays(1, &vao);

	piglit_present_results();

	return result;
}

static char *vs_source =
	"#version 140\n"
	"in vec4 vertex;\n"
	"void main()\n"
	"{\n"
	"	gl_Position = vertex;\n"
	"}\n";

static char *fs_source =
	"#version 140\n"
	"#define WIN_WIDTH 32\n"
	"uniform isamplerBuffer buf;\n"
	"uniform int offset;\n"
	"uniform int size;\n"
	"\n"
	"void main()\n"
	"{\n"
	"  int pos = int(gl_FragCoord.x) + int(gl_FragCoord.y) * WIN_WIDTH;\n"
	"  int expected = ((pos + offset) | 1) & 0xff;\n"
	"  if (pos >= size)\n"
	"    expected = 0;\n"
	"  float ok = float(texelFetch(buf, pos).r == expected);\n"
	"  gl_FragColor = vec4(1.0 - ok, ok, 0.0, 0.0);\n"
	"}\n";

static void
init_program()
{
	prog = piglit_build_simple_program(vs_source, fs_source);

	vertex_location = glGetAttribLocation(prog, "vertex");
}

static void
init_tbo()
{
	int i;

	data = malloc(TBO_SIZE);
	if (!data) {
		fprintf(stderr, "malloc failed\n");
		piglit_report_result(PIGLIT_SKIP);
	}

	/* always non-zero to distinguish from out-of-bounds access */
	for (i = 0; i < TBO_WIDTH; ++i)
		data[i] = (i | 1) & 0xff;

	glGenBuffers(1, &tbo);
	glBindBuffer(GL_TEXTURE_BUFFER, tbo);
	glBufferData(GL_TEXTURE_BUFFER, TBO_SIZE, data, GL_STATIC_DRAW);

	glGenTextures(1, &tex);

	free(data);
}

void
piglit_init(int argc, char **argv)
{
	piglit_require_GLSL_version(140);
	piglit_require_extension("GL_ARB_texture_buffer_range");

	init_program();
	init_tbo();
}
