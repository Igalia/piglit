/*
 * Copyright (c) 2018 Intel Corporation
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

#include "piglit-util-gl.h"
#include "gpb-common.h"
#include <stdlib.h>

bool
gpb_save_program(GLuint prog, void **binary, GLsizei *length, GLenum *format)
{
	GLsizei binary_length;
	void *binary_buffer;
	GLenum binary_format;
	GLint ok;

	glGetProgramiv(prog, GL_LINK_STATUS, &ok);
	if (!ok) {
		fprintf(stderr, "Can't save/restore program that is "
			"not linked!\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	glGetProgramiv(prog, GL_PROGRAM_BINARY_LENGTH, &binary_length);
	if (!piglit_check_gl_error(GL_NO_ERROR)) {
		fprintf(stderr, "glGetProgramiv GL_PROGRAM_BINARY_LENGTH "
		        "error\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	binary_buffer = malloc(binary_length);
	if (!binary_buffer) {
		fprintf(stderr, "Failed to allocate buffer for "
		        "GetProgramBinary\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	glGetProgramBinary(prog, binary_length, &binary_length, &binary_format,
			   binary_buffer);
	if (!piglit_check_gl_error(GL_NO_ERROR)) {
		fprintf(stderr, "glGetProgramBinary error\n");
		free(binary_buffer);
		piglit_report_result(PIGLIT_FAIL);
	}

	*length = binary_length;
	*binary = binary_buffer;
	*format = binary_format;
	return true;
}

bool
gpb_restore_program(GLuint prog, void *binary, GLsizei length, GLenum format)
{
	GLint ok;

	glProgramBinary(prog, format, binary, length);
	if (!piglit_check_gl_error(GL_NO_ERROR)) {
		fprintf(stderr, "glProgramBinary error "
			"(should not happend according to spec.)\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	glGetProgramiv(prog, GL_LINK_STATUS, &ok);
	if (!ok) {
		fprintf(stderr, "link failure after glProgramBinary\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	return true;
}

bool
gpb_save_restore(GLuint *prog)
{
	GLsizei bin_length;
	void *binary;
	GLenum bin_format;
	GLuint new_prog;

	if (!gpb_save_program(*prog, &binary, &bin_length, &bin_format)) {
		fprintf(stderr,
			"failed to save program with GetProgramBinary\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	new_prog = glCreateProgram();
	if (!piglit_check_gl_error(GL_NO_ERROR)) {
		free(binary);
		piglit_report_result(PIGLIT_FAIL);
	}

	if (!gpb_restore_program(new_prog, binary, bin_length, bin_format)) {
		free(binary);
		fprintf(stderr, "failed to restore binary program\n");
		piglit_report_result(PIGLIT_FAIL);
	}
	free(binary);

	glUseProgram(new_prog);
	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	glDeleteProgram(*prog);
	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);
	*prog = new_prog;

	return true;
}
