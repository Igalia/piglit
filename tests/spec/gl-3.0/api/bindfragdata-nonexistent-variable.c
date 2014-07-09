/* Copyright © 2011 Intel Corporation
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

/**
 * \file bindfragdata-nonexistent-variable.c
 * Test the behavior of glBindFragDataLocation on a non-existent variable
 *
 * \author Ian Romanick
 */
#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;
	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

static const char *vs_text =
	"#version 130\n"
	"in vec4 vertex;\n"
	"void main() { gl_Position = vertex; }\n"
	;

static const char *fs_text =
	"#version 130\n"
	"out vec4 v;\n"
	"void main() {\n"
	"    v = vec4(0.0);\n"
	"}\n"
	;

enum piglit_result
piglit_display(void)
{
	return PIGLIT_FAIL;
}

void piglit_init(int argc, char **argv)
{
	GLint max_draw_buffers;
	GLuint prog;
	GLuint vs;
	GLuint fs;
	GLint loc;

	piglit_require_gl_version(30);

	/* This test needs some number of draw buffers, so make sure the
	 * implementation isn't broken.  This enables the test to generate a
	 * useful failure message.
	 */
	glGetIntegerv(GL_MAX_DRAW_BUFFERS, &max_draw_buffers);
	if (max_draw_buffers < 8) {
		fprintf(stderr,
			"OpenGL 3.0 requires GL_MAX_DRAW_BUFFERS >= 8.  "
			"Only got %d!\n",
			max_draw_buffers);
		piglit_report_result(PIGLIT_FAIL);
	}

	prog = glCreateProgram();
	vs = piglit_compile_shader_text(GL_VERTEX_SHADER, vs_text);
	fs = piglit_compile_shader_text(GL_FRAGMENT_SHADER, fs_text);
	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	/* First, verify that the program will link without making any
	 * location assignments through the API.
	 */
	printf("Basic test...\n");

	glAttachShader(prog, vs);
	glAttachShader(prog, fs);
	glLinkProgram(prog);
	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	if (!piglit_link_check_status(prog)) {
		piglit_report_result(PIGLIT_FAIL);
	}

	/* Page 237 (page 253 of the PDF) of the OpenGL 3.0 spec says:
	 *
	 *     "Assigned bindings for variables that do not exist are
	 *     ignored."
	 */
	printf("Binding `unicorn' to a non-conflicting location...\n");

	glBindFragDataLocation(prog, 0, "v");
	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);
	glBindFragDataLocation(prog, 1, "unicorn");
	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	glLinkProgram(prog);
	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	if (!piglit_link_check_status(prog)) {
		fprintf(stderr,
			"Linking failed when it should have been "
			"successful.\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	loc = glGetFragDataLocation(prog, "unicorn");
	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	if (loc != -1) {
		fprintf(stderr, "Expected location = -1, got %d\n", loc);
		piglit_report_result(PIGLIT_FAIL);
	}

	printf("Binding `unicorn' to a conflicting location...\n");

	glBindFragDataLocation(prog, 0, "v");
	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);
	glBindFragDataLocation(prog, 0, "unicorn");
	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	glLinkProgram(prog);
	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	if (!piglit_link_check_status(prog)) {
		fprintf(stderr,
			"Linking failed when it should have been "
			"successful.\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	loc = glGetFragDataLocation(prog, "unicorn");
	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	if (loc != -1) {
		fprintf(stderr, "Expected location = -1, got %d\n", loc);
		piglit_report_result(PIGLIT_FAIL);
	}

	piglit_report_result(PIGLIT_PASS);
}
