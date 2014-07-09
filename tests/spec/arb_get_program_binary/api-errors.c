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
	bool pass = true;
	GLenum binaryFormat;
	GLsizei length;
	GLint value;

	/* Make "huge" buffer for storing program binaries.
	 */
	size_t buffer_size = 16 * 1024 * 1024;
	void *buffer = malloc(buffer_size);

	piglit_require_gl_version(20);
	piglit_require_extension("GL_ARB_get_program_binary");

	vs = piglit_compile_shader_text(GL_VERTEX_SHADER, vs_text);
	fs = piglit_compile_shader_text(GL_FRAGMENT_SHADER, fs_text);

	prog = glCreateProgram();
	glAttachShader(prog, vs);
	glAttachShader(prog, fs);

	/* The ARB_get_program_binary spec says:
	 *
	 *     "An INVALID_ENUM error is generated if the <pname> argument to
	 *     ProgramParameteri is not PROGRAM_BINARY_RETRIEVABLE_HINT."
	 *
	 * The ARB_geometry_shader4 extension also adds some value pnames to
	 * glProgramParameteri.
	 */
	glProgramParameteri(prog, GL_PROGRAM_BINARY_LENGTH, 0);
	pass = piglit_check_gl_error(GL_INVALID_ENUM)
		&& pass;

	if (!piglit_is_extension_supported("GL_ARB_geometry_shader4")) {
		static const GLenum gs4_enums[] = {
			GL_GEOMETRY_VERTICES_OUT_ARB,
			GL_GEOMETRY_INPUT_TYPE_ARB,
			GL_GEOMETRY_OUTPUT_TYPE_ARB
		};
		unsigned i;

		for (i = 0; i < ARRAY_SIZE(gs4_enums); i++) {
			glProgramParameteri(prog, gs4_enums[i], 0);

			pass = piglit_check_gl_error(GL_INVALID_ENUM)
				&& pass;
		}
	}

	/* The ARB_get_program_binary spec says:
	 *
	 *     "An INVALID_VALUE error is generated if the <value> argument to
	 *     ProgramParameteri is not TRUE or FALSE."
	 *
	 * Also check that setting an invalid value does not change the state
	 * of GL_PROGRAM_BINARY_RETRIEVABLE_HINT.
	 */
	glProgramParameteri(prog, GL_PROGRAM_BINARY_RETRIEVABLE_HINT, 7);
	pass = piglit_check_gl_error(GL_INVALID_VALUE)
		&& pass;

	glProgramParameteri(prog, GL_PROGRAM_BINARY_RETRIEVABLE_HINT, ~0);
	pass = piglit_check_gl_error(GL_INVALID_VALUE)
		&& pass;

	glGetProgramiv(prog, GL_PROGRAM_BINARY_RETRIEVABLE_HINT, &value);
	pass = piglit_check_gl_error(0)
		&& pass;

	if (value != 0) {
		fprintf(stderr,
			"Value of GL_PROGRAM_BINARY_RETRIEVABLE_HINT "
			"changed when it should not have.\n");
		pass = false;
	}

	/* The ARB_get_program_binary spec says:
	 *
	 *     "An INVALID_OPERATION error is generated if GetProgramBinary is
	 *     called when the program object, <program>, does not contain a
	 *     valid program binary as reflected by its LINK_STATUS state, or
	 *     if <bufSize> is not big enough to contain the entire program
	 *     binary."
	 */
	glGetProgramBinary(prog, buffer_size, &length,
			   &binaryFormat, buffer);
	pass = piglit_check_gl_error(GL_INVALID_OPERATION)
		&& pass;

	glLinkProgram(prog);
	glGetProgramiv(prog, GL_PROGRAM_BINARY_LENGTH, &length);
	pass = piglit_check_gl_error(0)
		&& pass;

	if (length > 0) {
		glGetProgramBinary(prog, length - 1, &length,
				   &binaryFormat, buffer);
		pass = piglit_check_gl_error(GL_INVALID_OPERATION)
			&& pass;
	}

	/* Every OpenGL specification since 1.0 says some variation of:
	 *
	 *     "If a negative number is provided where an argument of type
	 *     sizei or sizeiptr is specified, the error INVALID_VALUE is
	 *     generated."
	 */
	glGetProgramBinary(prog, -1, &length,
			   &binaryFormat, buffer);
	pass = piglit_check_gl_error(GL_INVALID_VALUE)
		&& pass;

	/* The ARB_get_program_binary spec says:
	 *
	 *     "An INVALID_VALUE error is generated if the <program> argument
	 *     to GetProgramBinary, ProgramBinary, or ProgramParameteri is not
	 *     the name of a program object previously created with
	 *     CreateProgram."
	 */
	glGetProgramBinary(0xDEADBEEF, buffer_size, &length, &binaryFormat,
			   buffer);
	pass = piglit_check_gl_error(GL_INVALID_VALUE)
		&& pass;

	glProgramParameteri(0xDEADBEEF, GL_PROGRAM_BINARY_RETRIEVABLE_HINT,
			    GL_TRUE);
	pass = piglit_check_gl_error(GL_INVALID_VALUE)
		&& pass;

	glGetProgramiv(prog, GL_PROGRAM_BINARY_LENGTH, &length);
	pass = piglit_check_gl_error(0)
		&& pass;

	if (length > 0) {
		glGetProgramBinary(prog, length, &length,
				   &binaryFormat, buffer);
		pass = piglit_check_gl_error(0)
			&& pass;

		if (buffer_size < length) {
			free(buffer);
			buffer = malloc(length);
			buffer_size = length;
		}

		glProgramBinary(0xDEADBEEF, binaryFormat, buffer,
				buffer_size);
		pass = piglit_check_gl_error(GL_INVALID_VALUE)
			&& pass;
	}

	free(buffer);
	glDeleteShader(vs);
	glDeleteShader(fs);
	glDeleteProgram(prog);

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}
