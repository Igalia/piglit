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

/*
 * Check that glGetActiveAttrib returns the correct size values,
 * for arrays of various sizes.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_core_version = 32;
	config.supports_gl_compat_version = 32;

PIGLIT_GL_TEST_CONFIG_END

static const char *vs_source =
	"#version 150\n"
	"in int a[1];\n"
	"in int b[2];\n"
	"in int c[3];\n"
	"in int d[4];\n"
	"in int e[5];\n"
	"\n"
	"out vec4 color;\n"
	"void main()\n"
	"{\n"
	"	color = vec4(a[0] + b[0] + b[1],\n"
	"		     c[0] + c[1] + c[2],\n"
	"		     d[0] + d[1] + d[2] + d[3],\n"
	"		     e[0] + e[1] + e[2] + e[3] + e[4]);\n"
	"}\n";

static const char *fs_source =
	"#version 150\n"
	"in vec4 color;\n"
	"void main()\n"
	"{\n"
	"	gl_FragColor = color;\n"
	"}\n";

enum piglit_result
piglit_display(void)
{
	/* UNREACHED */
	return PIGLIT_FAIL;
}

/*
 * Find the given attribute size then, check if the passed expected size
 * is equal to the actual size.
 */
bool
getAttribLocTest(GLint program, int active_attribs, int max_name_length,
		char *attrib_name, int expected_size)
{
	bool pass = true;
	int size = -1;
	GLenum type = GL_NONE;
	char *name = malloc(max_name_length-1);

	int i;
	for(i = 0; i < active_attribs; i++) {
		glGetActiveAttrib(program, i, max_name_length,
				  NULL, &size, &type, name);

		if(strcmp(attrib_name, name) == 0) {
			break;
		}
	}

	/* Check if no attrib was found */
	if(i == active_attribs) {
		pass = false;
		printf("Attribute '%s' not found\n", attrib_name);
	}

	/* Check for non-matching sizes */
	if(size != expected_size) {
		pass = false;
		printf("Attribute '%s': size %d; expected %d\n",
			name, size, expected_size);
	}

	free(name);
	return pass;
}

void
piglit_init(int argc, char **argv)
{
	bool pass = true;

	GLint prog = 0;

	GLint active_attribs = 0;
	GLint max_length = 0;

	piglit_require_GLSL_version(150);

	prog = piglit_build_simple_program(vs_source, fs_source);
	glUseProgram(prog);

	glGetProgramiv(prog, GL_ACTIVE_ATTRIBUTES, &active_attribs);
	glGetProgramiv(prog, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &max_length);

	/*
	 * Check the size of the given attribute with the
	 * corresponding expected array size.
	 */
	pass = getAttribLocTest(prog, active_attribs, max_length, "a", 1) && pass;
	pass = getAttribLocTest(prog, active_attribs, max_length, "b", 2) && pass;
	pass = getAttribLocTest(prog, active_attribs, max_length, "c", 3) && pass;
	pass = getAttribLocTest(prog, active_attribs, max_length, "d", 4) && pass;
	pass = getAttribLocTest(prog, active_attribs, max_length, "e", 5) && pass;

	glDeleteProgram(prog);

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}
