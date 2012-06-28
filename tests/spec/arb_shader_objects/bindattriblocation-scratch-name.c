/*
 * Copyright © 2011 Intel Corporation
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
 * \file bindattriblocation-scratch-name.c
 * Verify that glBindAttribLocation doesn't keep the applications name pointer.
 *
 * This reproduces Mesa bugzilla #41499 (bugzilla #41508 is a dup of the same
 * issue).
 *
 * \author Ian Romanick <ian.d.romanick@intel.com>
 */

#include "piglit-util-gl-common.h"
#include "piglit-framework.h"

PIGLIT_GL_TEST_MAIN(
    10 /*window_width*/,
    10 /*window_height*/,
    GLUT_RGB | GLUT_DOUBLE)

static const GLchar *vertShaderText =
	"attribute vec4 attrib;\n"
	"void main() { gl_Position = attrib; }\n";

enum piglit_result
piglit_display(void)
{
	return PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	static const char name[] = "attrib";
	char alt_name[sizeof(name)];
	GLint vs;
	GLint prog;
	GLint attrib_loc;

	piglit_require_vertex_shader();

	vs = piglit_compile_shader_text(GL_VERTEX_SHADER, vertShaderText);
	if (vs == 0) {
		piglit_report_result(PIGLIT_FAIL);
	}

	prog = piglit_CreateProgram();
	piglit_AttachShader(prog, vs);

	/* Bind "attrib" to location 3.  Once the attribute is bound, smash
	 * the string containing the name.  After smashing the name, link the
	 * shader.  If the implementation kept our name pointer, there will be
	 * problems linking.
	 */
	memcpy(alt_name, name, sizeof(name));
	piglit_BindAttribLocation(prog, 3, alt_name);
	memset(alt_name, 0, sizeof(alt_name));
	piglit_LinkProgram(prog);

	if (!piglit_link_check_status(prog))
		piglit_report_result(PIGLIT_FAIL);

	attrib_loc = piglit_GetAttribLocation(prog, "attrib");
	if (attrib_loc != 3) {
		fprintf(stderr, "Expected location 3, got location %d\n",
			attrib_loc);
		piglit_report_result(PIGLIT_FAIL);
	}

	piglit_report_result(PIGLIT_PASS);
}
