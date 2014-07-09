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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

/**
 * \file array-elements.c
 *
 * Tests that array elements get sequential locations.
 *
 * The GL_ARB_explicit_uniform_location spec says:
 *     "Individual elements of a uniform array are assigned consecutive
 *     locations with the first element taking location <location>."
 */
#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_core_version = 33;
	config.window_visual = PIGLIT_GL_VISUAL_RGB;

PIGLIT_GL_TEST_CONFIG_END

enum piglit_result
piglit_display(void)
{
	return PIGLIT_FAIL;
}

static const char vs_text[] =
	"vec4 vertex;\n"
	"void main() {\n"
		"gl_Position = vertex;\n"
	"}";

static const char fs_text[] =
	"#version 330\n"
	"#extension GL_ARB_explicit_uniform_location: require\n"
	"#define ARRAY_SIZE 16\n"
	"layout(location = 1) uniform float r;\n"
	"layout(location = 2) uniform float g;\n"
	"layout(location = 3) uniform float a[ARRAY_SIZE];\n"
	"layout(location = 19) uniform float b;\n"
	"void main() {\n"
		"gl_FragColor = vec4(r, g, b, a[ARRAY_SIZE - 1]);\n"
	"}";

void
piglit_init(int argc, char **argv)
{
	GLuint prog, i;

	piglit_require_extension("GL_ARB_explicit_uniform_location");

	prog = piglit_build_simple_program(vs_text, fs_text);

	/* verify that locations are sequential */
	for (i = 0; i < 16; i++) {
		char *element;
		if (asprintf(&element, "a[%d]", i) == -1)
			piglit_report_result(PIGLIT_FAIL);

		if (glGetUniformLocation(prog, element) != 3 + i)
			piglit_report_result(PIGLIT_FAIL);

		free(element);
	}

	glDeleteProgram(prog);
	piglit_report_result(PIGLIT_PASS);
}
