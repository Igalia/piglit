/*
 * Copyright © 2010 Intel Corporation
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
 *
 * Authors:
 *    Eric Anholt <eric@anholt.net>
 *    Neil Roberts <neil@linux.intel.com>
 *
 */

/** @file useprogram-refcount-1.c
 *
 * test_delete_active: tests that a metaops call (glDrawPixels()) doesn't lose the last
 *                     reference on an active, deleted shader prorgam (Bug #31194)
 *
 * test_delete_duplicate: tests for shader cache errors (issue #2596) by creating 2
 *                        identical program, deleting one of them, and using the other
 *                        to draw something.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

static enum {
	TEST_DELETE_ACTIVE,
	TEST_DELETE_DUPLICATE
} test_mode;

static GLint
build_program(void)
{
	GLint vs, fs;
	const char *vs_source =
		"void main()\n"
		"{\n"
		"	gl_Position = gl_Vertex;\n"
		"}\n";
	const char *fs_source =
		"void main()\n"
		"{\n"
		"	gl_FragColor = vec4(0.0, 1.0, 0.0, 0.0);\n"
		"}\n";

	piglit_require_gl_version(20);

	vs = piglit_compile_shader_text(GL_VERTEX_SHADER, vs_source);
	fs = piglit_compile_shader_text(GL_FRAGMENT_SHADER, fs_source);
	return piglit_link_simple_program(vs, fs);
}

enum piglit_result
test_delete_active(void)
{
	bool pass = true;
	float green[4] = {0.0, 1.0, 0.0, 0.0};
	uint8_t pixel[4] = {0x00, 0xff, 0x00, 0xff};

	GLint prog = build_program();
	glUseProgram(prog);
	glDeleteProgram(prog);

	/* Set up fixed function to draw red if we lose our shader. */
	glColor4f(1.0, 0.0, 0.0, 0.0);

	/* Tiny drawpixels */
	glDrawPixels(1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixel);

	/* Draw over the whole screen with the shader. */
	piglit_draw_rect(-1, -1, 2, 2);

	pass &= piglit_probe_rect_rgba(0, 0, piglit_width, piglit_height,
				       green);

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

enum piglit_result
test_delete_duplicate(void)
{
	bool pass = true;
	float green[4] = {0.0, 1.0, 0.0, 0.0};
	uint8_t pixel[4] = {0x00, 0xff, 0x00, 0xff};

	GLint prog = build_program();
	/* Build a second program, using the same shaders */
	GLint prog_dup = build_program();

	/* Set up fixed function to draw red if we lose our shader. */
	glColor4f(1.0, 0.0, 0.0, 0.0);

	glDrawPixels(1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixel);

	glUseProgram(prog);
	piglit_draw_rect(-1, -1, 2, 2);

	/* Delete the duplicate */
	glDeleteProgram(prog_dup);

	/* Re-draw over the whole screen with the shader. */
	piglit_draw_rect(-1, -1, 2, 2);

	pass &= piglit_probe_rect_rgba(0, 0, piglit_width, piglit_height,
				       green);

	piglit_present_results();

	glDeleteProgram(prog);

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

enum piglit_result
piglit_display(void)
{
	switch (test_mode) {
		case TEST_DELETE_DUPLICATE:
			return test_delete_duplicate();
		case TEST_DELETE_ACTIVE:
		default:
			return test_delete_active();
	}
}

void
piglit_init(int argc, char **argv)
{
	test_mode = TEST_DELETE_ACTIVE;
	if (argc == 2 && strcmp(argv[1], "delete_dup") == 0)
		test_mode = TEST_DELETE_DUPLICATE;
	piglit_require_gl_version(20);
}
