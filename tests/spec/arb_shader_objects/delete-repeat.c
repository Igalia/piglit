/* Copyright © 2011 Intel Corporation
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

#include "piglit-util-gl-common.h"

/**
 * @file delete-repeat.c
 *
 * Tests that refcounting of deleted shader objects is correct when
 * glDeleteProgram() is called multiple times.
 */

PIGLIT_GL_TEST_MAIN(
    32 /*window_width*/,
    32 /*window_height*/,
    GLUT_DOUBLE | GLUT_RGB | GLUT_ALPHA)

static const char *vs_source =
	"void main()\n"
	"{"
	"	gl_Position = gl_Vertex;\n"
	"}\n";

static const char *fs_source =
	"void main()\n"
	"{"
	"	gl_FragColor = vec4(0.0, 1.0, 0.0, 0.0);\n"
	"}\n";

enum piglit_result
piglit_display(void)
{
	GLuint vs, fs;
	bool pass = true;
	GLuint prog;
	float green[] = {0.0, 1.0, 0.0, 0.0};
	GLint status;

	/* Initial buffer clear. */
	glClearColor(1.0, 0.0, 0.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT);

	vs = piglit_compile_shader_text(GL_VERTEX_SHADER, vs_source);
	fs = piglit_compile_shader_text(GL_FRAGMENT_SHADER, fs_source);
	prog = piglit_link_simple_program(vs, fs);

	if (!vs || !fs || !prog)
		piglit_report_result(PIGLIT_FAIL);

	piglit_DeleteShader(vs);
	piglit_DeleteShader(fs);
	piglit_UseProgram(prog);
	piglit_DeleteProgram(prog);

	/* Try to blow out the refcount */
	piglit_DeleteProgram(prog);
	piglit_DeleteProgram(prog);
	piglit_DeleteProgram(prog);

	/* Sanity check: deleting didn't already unbind our shader program. */
	piglit_draw_rect(-1, -1, 2, 2);
	pass = piglit_probe_rect_rgba(0, 0, piglit_width, piglit_height,
				      green) && pass;

	/* The program should still report being deleted. */
	piglit_GetProgramiv(prog, GL_DELETE_STATUS, &status);
	if (!piglit_check_gl_error(0))
		piglit_report_result(PIGLIT_FAIL);
	if (status != GL_TRUE) {
		fprintf(stderr,
			"GL_DELETE_STATUS after a clear reported non-true %d\n",
			status);
		pass = false;
	}

	/* Now, disable the program and it should be finally deleted. */
	piglit_UseProgram(0);

	piglit_GetProgramiv(prog, GL_DELETE_STATUS, &status);
	pass = piglit_check_gl_error(GL_INVALID_VALUE) && pass;

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	piglit_require_vertex_shader();
	piglit_require_fragment_shader();
}
