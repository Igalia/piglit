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
 * \file loc-boundaries.c
 *
 * Tests the boundary values for uniform locations, this is a positive test
 * for the locations, every check here is expected to pass.
 *
 * The GL_ARB_explicit_uniform_location spec says:
 *     "The explicitly defined locations and the generated locations must be
 *     in the range of 0 to MAX_UNIFORM_LOCATIONS minus one."
 *
 *     "Valid locations for default-block uniform variable locations are in
 *     the range of 0 to the implementation-defined maximum number of
 *     uniform locations."
 *
 * This test tests 0, MAX - 1 and a single value in between, shader contains
 * also uniform without explicit location to see that it does not affect
 * getting the wanted locations.
 */
#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 30;
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

static const char fs_template[] =
	"#version 130\n"
	"#extension GL_ARB_explicit_attrib_location: require\n"
	"#extension GL_ARB_explicit_uniform_location: require\n"
	"uniform float a;\n"
	"layout(location = %d) uniform float r;\n"
	"layout(location = %d) uniform float g;\n"
	"layout(location = %d) uniform float b;\n"
	"void main() {\n"
		"gl_FragColor = vec4(r, g, b, a);\n"
	"}";

void
piglit_init(int argc, char **argv)
{
	int maxloc;
	GLuint prog;
	char *f_sha;

	piglit_require_extension("GL_ARB_explicit_attrib_location");
	piglit_require_extension("GL_ARB_explicit_uniform_location");

	glGetIntegerv(GL_MAX_UNIFORM_LOCATIONS, &maxloc);

	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	/* test GL_MAX_UNIFORM_LOCATIONS - 1, 0, and a loc in between (1) */
	if (asprintf(&f_sha, fs_template, maxloc - 1, 0, 1) == -1)
		piglit_report_result(PIGLIT_FAIL);

	prog = piglit_build_simple_program(vs_text, f_sha);

	free(f_sha);

	if (glGetUniformLocation(prog, "r") != maxloc - 1)
		piglit_report_result(PIGLIT_FAIL);
	if (glGetUniformLocation(prog, "g") != 0)
		piglit_report_result(PIGLIT_FAIL);
	if (glGetUniformLocation(prog, "b") != 1)
		piglit_report_result(PIGLIT_FAIL);

	glDeleteProgram(prog);
	piglit_report_result(PIGLIT_PASS);
}
