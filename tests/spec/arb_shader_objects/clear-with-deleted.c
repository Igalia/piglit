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

#include "piglit-util-gl.h"

/**
 * @file clear-with-deleted.c
 *
 * Tests that refcounting of deleted shader objects is correct across
 * glClear().  This is similar to shaders/useprogram-refcount-1, but
 * uses glClear() instead of glDrawPixels() and is a bit more thorough
 * (makes sure it isn't deleted late, in addition to not being deleted
 * early).
 */

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;

PIGLIT_GL_TEST_CONFIG_END

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
	bool pass = true;
	GLuint prog;
	float green[] = {0.0, 1.0, 0.0, 0.0};
	GLint status;

	/* Initial buffer clear. */
	glClearColor(1.0, 0.0, 0.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT);

	prog = piglit_build_simple_program(vs_source, fs_source);

	glUseProgram(prog);
	glDeleteProgram(prog);

	/* Since the program is in use, it should be flagged for
	 * deletion but not deleted.
	 */
	glGetProgramiv(prog, GL_DELETE_STATUS, &status);
	if (!piglit_check_gl_error(0))
		piglit_report_result(PIGLIT_FAIL);
	if (status != GL_TRUE) {
		fprintf(stderr,
			"GL_DELETE_STATUS when deleted reported non-true %d\n",
			status);
		pass = false;
	}

	/* Sanity check: deleting didn't already unbind our shader program. */
	piglit_draw_rect(-1, -1, 2, 2);
	pass = piglit_probe_rect_rgba(0, 0, piglit_width, piglit_height,
				      green) && pass;

	glClearColor(1.0, 0.0, 0.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT);

	/* The main test: Can we still draw after a clear with a
	 * delete program?
	 */
	piglit_draw_rect(-1, -1, 2, 2);
	pass = piglit_probe_rect_rgba(0, 0, piglit_width, piglit_height,
				      green) && pass;

	/* The program should still report being deleted. */
	glGetProgramiv(prog, GL_DELETE_STATUS, &status);
	if (!piglit_check_gl_error(0))
		piglit_report_result(PIGLIT_FAIL);
	if (status != GL_TRUE) {
		fprintf(stderr,
			"GL_DELETE_STATUS after a clear reported non-true %d\n",
			status);
		pass = false;
	}

	/* Now, disable the program and it should be finally deleted. */
	glUseProgram(0);

	glGetProgramiv(prog, GL_DELETE_STATUS, &status);
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
