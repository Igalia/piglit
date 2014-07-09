/*
 * Copyright © 2013 Intel Corporation
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
 * \file attrib-assignments.c
 * Verify that vertex shader attributes are assigned in order starting with 0.
 *
 * THIS IS NOT REQUIRED BY ANY VERSION OF THE OpenGL SPECIFICATION!
 *
 * However, almost every OpenGL implementation happens to behave this way when
 * there is a single vertex shader compilation unit linked into the program.
 * As a result, some programs accidentally rely on this behavior.  If the
 * application was never tested on a implementation that behaves any other
 * way, there's a reasonable chance it has bugs without its developers even
 * knowing.
 */
#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;
	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

static const char *vs_code =
	"attribute vec4 i00;\n"
	"attribute vec4 i01;\n"
	"attribute vec4 i02;\n"
	"attribute vec4 i03;\n"
	"attribute vec4 i04;\n"
	"attribute vec4 i05;\n"
	"attribute vec4 i06;\n"
	"attribute vec4 i07;\n"
	"attribute vec4 i08;\n"
	"attribute vec4 i09;\n"
	"attribute vec4 i10;\n"
	"attribute vec4 i11;\n"
	"attribute vec4 i12;\n"
	"attribute vec4 i13;\n"
	"attribute vec4 i14;\n"
	"attribute vec4 i15;\n"
	"varying vec4 a;\n"
	"void main()\n"
	"{\n"
	"    gl_Position = i00;\n"
	"    a = i01 + i02 + i03 + i04 + i05\n"
	"        + i06 + i07 + i08 + i09 + i10\n" 
	"        + i11 + i12 + i13 + i14 + i15;\n" 
	"}"
	;

static const char *fs_code =
	"varying vec4 a;\n"
	"void main() { gl_FragColor = a; }"
	;

enum piglit_result
piglit_display(void)
{
	return PIGLIT_FAIL;
}

void piglit_init(int argc, char **argv)
{
	GLint prog;
	GLint loc;
	int i;

	piglit_require_vertex_shader();
	piglit_require_fragment_shader();

	prog = piglit_build_simple_program(vs_code, fs_code);

	for (i = 0; i < 16; i++) {
		char name[4];

		snprintf(name, sizeof(name), "i%02d", i);
		loc = glGetAttribLocation(prog, name);

		if (loc != i) {
			fprintf(stderr,
				"Attribute \"%s\" has location %d, "
				"expected %d.\n",
				name, loc, i);
			piglit_report_result(PIGLIT_FAIL);
		}
	}

	piglit_report_result(PIGLIT_PASS);
}
