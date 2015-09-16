/*
 * Copyright (c) 2014 - 2015 Intel Corporation
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

/** @file common.c
 *
 * Utility code for building a compute shader program.
 */

#include "common.h"

char *
concat(char *hunk0, ...)
{
	char *s = hunk0;
	char *hunk;
	va_list ap;

	va_start(ap, hunk0);

	while ((hunk = va_arg(ap, char *))) {
		char *t = s;
		asprintf(&s, "%s\n%s", t, hunk);
		free(t);
		free(hunk);
	}

	va_end(ap);
	return s;
}

GLuint
generate_cs_prog(unsigned x, unsigned y, unsigned z, char *ext,
		 char *src)
{
	char *source = NULL;

	if (ext == NULL)
		ext = hunk("");

	asprintf(&source,
		 "#version 330\n"
		 "#extension GL_ARB_compute_shader : enable\n"
		 "%s\n"
		 "layout(local_size_x = %d, local_size_y = %d, local_size_z = %d) in;\n"
		 "\n"
		 "%s\n",
		 ext,
		 x, y, z,
		 src
		 );
	free(ext);
	free(src);

	GLuint prog = glCreateProgram();

	GLuint shader =
		piglit_compile_shader_text_nothrow(GL_COMPUTE_SHADER, source);

	free(source);

	if (!shader) {
		glDeleteProgram(prog);
		return 0;
	}

	glAttachShader(prog, shader);

	glLinkProgram(prog);

	glDeleteShader(shader);

	if (!piglit_link_check_status(prog)) {
		glDeleteProgram(prog);
		return 0;
	}

	return prog;
}
