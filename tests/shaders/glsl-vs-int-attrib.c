/*
 * Copyright (c) 2014 Intel Corporation
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

/** @file glsl-vs-int-attrib.c
 *
 * Tests that we can set an integer vertex attribute to a value that
 * looks like a signalling NaN if it were interpreted as a float. If
 * an implementation passes this through a floating point variable it
 * might incorrectly get corrupted to a quiet NaN.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 20;

	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

/* This value looks like a signalling NaN if it were interpreted as a
 * float */
#define TEST_VALUE 0x7f817f81

#define STR(x) #x
#define STRINGIFY(x) STR(x)

static const char
vertex_source[] =
	"#version 130\n"
	"\n"
	"attribute vec4 piglit_vertex;\n"
	"attribute uint a_value;\n"
	"\n"
	"void\n"
	"main()\n"
	"{\n"
	"  if (a_value == " STRINGIFY(TEST_VALUE) "u)\n"
	"    gl_FrontColor = vec4(0.0, 1.0, 0.0, 1.0);\n"
	"  else\n"
	"    gl_FrontColor = vec4(1.0, 0.0, 0.0, 1.0);\n"
	"  gl_Position = piglit_vertex;\n"
	"}\n";

static const float green[] = { 0.0f, 1.0f, 0.0f };

static bool
run_test(void)
{
	GLuint prog;
	GLint attrib;
	bool pass;

	prog = piglit_build_simple_program(vertex_source,
					   NULL /* fs_source */);
	glUseProgram(prog);

	attrib = glGetAttribLocation(prog, "a_value");
	glVertexAttribI1ui(attrib, TEST_VALUE);

	piglit_draw_rect(-1, -1, 2, 2);
	pass = piglit_probe_pixel_rgb(0, 0, green);

	glUseProgram(0);
	glDeleteProgram(prog);

	return pass;
}

void
piglit_init(int argc, char **argv)
{
	bool pass;

	/* We need to be able to call glVertexAttribI1ui */
	if (piglit_get_gl_version() < 30
	    && !piglit_is_extension_supported("GL_EXT_gpu_shader4")) {
		printf("OpenGL 3.0 or GL_EXT_gpu_shader4 is required.\n");
		piglit_report_result(PIGLIT_SKIP);
	}

	piglit_require_GLSL_version(130);

	pass = run_test();

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}

enum piglit_result
piglit_display(void)
{
	/* unused */
	return PIGLIT_FAIL;
}
