/*
 * Copyright © 2012 Intel Corporation
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
 * \file coverage.c
 * Attempt quering every enum in the spec.  Sanity check initial values.
 *
 * GL_NUM_PROGRAM_BINARY_FORMATS and GL_PROGRAM_BINARY_FORMATS are not
 * covered by this test because they are heavily covered by the
 * overrun test.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;
	config.window_visual = PIGLIT_GL_VISUAL_RGB;

PIGLIT_GL_TEST_CONFIG_END

static const char *vs_text =
	"#version 110\n"
	"void main() { gl_Position = vec4(0.); }"
	;

static const char *fs_text =
	"#version 110\n"
	"void main() { gl_FragColor = vec4(0.); }"
	;

enum piglit_result
piglit_display(void)
{
	return PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	GLuint vs;
	GLuint fs;
	GLuint prog;
	GLint value;
	bool pass = true;
	bool got_error;

	piglit_require_gl_version(20);
	piglit_require_extension("GL_ARB_get_program_binary");

	vs = piglit_compile_shader_text(GL_VERTEX_SHADER, vs_text);
	fs = piglit_compile_shader_text(GL_FRAGMENT_SHADER, fs_text);

	prog = piglit_link_simple_program(vs, fs);

	/* Check the initial state of GL_PROGRAM_BINARY_RETRIEVABLE_HINT.  The
	 * state table in the extension spec says the initial state is
	 * GL_FALSE.
	 */
	value = 0xDEADBEEF;
	glGetProgramiv(prog, GL_PROGRAM_BINARY_RETRIEVABLE_HINT, &value);
	got_error = piglit_check_gl_error(0);

	if (!got_error) {
		if (value == 0xDEADBEEF) {
			fprintf(stderr,
				"No error generated for "
				"GL_PROGRAM_BINARY_RETRIEVABLE_HINT, but "
				"no value was written either.\n");
			pass = false;
		} else if (value != 0) {
			fprintf(stderr,
				"Initial state of "
				"GL_PROGRAM_BINARY_RETRIEVABLE_HINT "
				"was %d instead of 0.\n",
				value);
			pass = false;
		}
	} else
		pass = false;

	/* The ARB_get_program_binary spec says:
	 *
	 *     "This hint will not take effect until the next time LinkProgram
	 *     or ProgramBinary has been called successfully."
	 *
	 * The GL spec contains similar language for attribute locations and
	 * fragment data locations.  In those cases, the queried value is only
	 * updated after relinking.  We'll assume this text means the same
	 * thing.
	 */

	glProgramParameteri(prog, GL_PROGRAM_BINARY_RETRIEVABLE_HINT, GL_TRUE);
	got_error = piglit_check_gl_error(0);

	glGetProgramiv(prog, GL_PROGRAM_BINARY_RETRIEVABLE_HINT, &value);
	got_error = piglit_check_gl_error(0);

	if (value != 0) {
		fprintf(stderr,
			"State of GL_PROGRAM_BINARY_RETRIEVABLE_HINT "
			"changed without relinking.\n");
		pass = false;
	}

	glLinkProgram(prog);
	got_error = piglit_check_gl_error(0);

	glGetProgramiv(prog, GL_PROGRAM_BINARY_RETRIEVABLE_HINT, &value);
	got_error = piglit_check_gl_error(0);

	if (value != GL_TRUE) {
		fprintf(stderr,
			"State of GL_PROGRAM_BINARY_RETRIEVABLE_HINT "
			"did not change across relinking.\n");
		pass = false;
	}

	glDeleteShader(vs);
	glDeleteShader(fs);
	glDeleteProgram(prog);
	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}
