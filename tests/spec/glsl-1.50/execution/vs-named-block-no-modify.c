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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */


/**
 * \file named-block-no-modify.c
 *
 * Test that uniform variables containted within a named uniform block cannot be
 * accessed by the glUniform* commands.
 *
 * Section 2.11.4 (Uniform Variables) of the GL 3.2 spec says:
 *  "Uniforms in a named uniform block are not assigned a location and may
 *   not be modified using the Uniform* commands."
 *
 * Test relies on the behavior of glGetUniform returning -1 for uniforms
 * that have not been assigned a location, such as those in a named uniform
 * block.
*/

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_core_version = 32;

PIGLIT_GL_TEST_CONFIG_END

/* the operations in this shader are not strictly relevant, only that they
 * do not get discarded */
static const char vs_text[] =
	"#version 150\n"
	"\n"
	"in vec4 piglit_vertex;\n"
	"\n"
	"uniform testBlock {\n"
	"	int a;\n"
	"	float b;\n"
	"	mat4 c;\n"
	"};\n"
	"\n"
	"flat out int oa;\n"
	"out float ob;\n"
	"out mat4 oc;\n"
	"\n"
	"void main() {\n"
	"	gl_Position = piglit_vertex;\n"
	"	oa = a + 1;\n"
	"	ob = b * 2;\n"
	"	oc[0] = c[0] * 1;\n"
	"	oc[1] = c[1] * 2;\n"
	"	oc[2] = c[2] * 3;\n"
	"	oc[3] = c[3] * 4;\n"
	"}\n";

/* again, operations are just to touch data */
static const char fs_text[] =
	"#version 150\n"
	"\n"
	"flat in int oa;\n"
	"in float ob;\n"
	"in mat4 oc;\n"
	"out vec4 FragColor;\n"
	"\n"
	"void main() {\n"
	"	FragColor = vec4(float(oa) * oc[0][0],\n"
	"			    ob * oc[1][1],\n"
	"			    oc[2][2],\n"
	"			    oc[3][3]);\n"
	"}\n";

static GLuint prog;

void
piglit_init(int argc, char **argv)
{
	bool pass = true;

	int a_loc = -2;
	int b_loc = -3;
	int c_loc = -4;

	prog = piglit_build_simple_program(vs_text, fs_text);

	glUseProgram(prog);

	/* get uniforms, locations should be -1 if glGetUniformLocation
	 * can't retrieve their location */

	a_loc = glGetUniformLocation(prog, "a");
	if(a_loc != -1) {
		printf("a_loc incorrectly assigned a location: %d", a_loc);
		pass = false;
	}

	b_loc = glGetUniformLocation(prog, "b");
	if(b_loc != -1) {
		printf("b_loc incorrectly assigned a location: %d", b_loc);
		pass = false;
	}

        c_loc = glGetUniformLocation(prog, "c");
	if(c_loc != -1) {
		printf("c_loc incorrectly assigned a location: %d", c_loc);
		pass = false;
	}

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}

enum piglit_result
piglit_display(void)
{
	/* should not reach */
	return PIGLIT_FAIL;
}
