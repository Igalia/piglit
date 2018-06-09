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

/**
 * \file restore-implicit-use-program.c
 *
 * Ref: https://bugs.freedesktop.org/show_bug.cgi?id=106810
 *
 * From section 7.3 (Program Objects) of the OpenGL 4.5 spec:
 *
 *    "If LinkProgram or ProgramBinary successfully re-links a program
 *     object that is active for any shader stage, then the newly generated
 *     executable code will be installed as part of the current rendering
 *     state for all shader stages where the program is active.
 *     Additionally, the newly generated executable code is made part of
 *     the state of any program pipeline for all stages where the program
 *     is attached."
 */

#include "piglit-util-gl.h"
#include "gpb-common.h"
#include <stdlib.h>

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 30;
	config.window_visual = PIGLIT_GL_VISUAL_RGB;
	config.khr_no_error_support = PIGLIT_NO_ERRORS;

PIGLIT_GL_TEST_CONFIG_END

void
piglit_init(int argc, char **argv)
{
	GLsizei bin_length;
	void *green_binary;
	GLenum bin_format;
	int num_formats = 0;
	float red[] = { 1.0, 0.0, 0.0, 1.0 };
	float green[] = { 0.0, 1.0, 0.0, 1.0 };
	GLuint green_prog, red_then_green_prog;
	bool pass = true;

	static const char vs_source[] =
		"void main()\n"
		"{\n"
		"    gl_Position = gl_Vertex;\n"
		"}\n";
	static const char green_fs_source[] =
		"#version 120\n"
		"void main()\n"
		"{\n"
		"        gl_FragColor = vec4(0.0, 1.0, 0.0, 1.0);\n"
		"}\n";
	static const char red_fs_source[] =
		"#version 120\n"
		"void main()\n"
		"{\n"
		"        gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);\n"
		"}\n";

	piglit_require_extension("GL_ARB_get_program_binary");

	glGetIntegerv(GL_NUM_PROGRAM_BINARY_FORMATS, &num_formats);
	if (num_formats == 0)
		piglit_report_result(PIGLIT_SKIP);

	green_prog = piglit_build_simple_program(vs_source, green_fs_source);
	glUseProgram(green_prog);

	glClearColor(1.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);
	piglit_draw_rect(-1.0f, -1.0f, 2.0f, 2.0f);
	pass = piglit_probe_rect_rgba(0, 0, piglit_width, piglit_height,
				      green) && pass;

	red_then_green_prog = piglit_build_simple_program(vs_source,
							  red_fs_source);
	glUseProgram(red_then_green_prog);

	if (!gpb_save_program(green_prog, &green_binary, &bin_length,
			      &bin_format)) {
		fprintf(stderr,
			"failed to save program with GetProgramBinary\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	/* We just built and activated the red program. We saved out
	 * the green program, but that should not impact drawing with
	 * the red program.
	 */
	glClearColor(0.0, 1.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);
	piglit_draw_rect(-1.0f, -1.0f, 2.0f, 2.0f);
	pass = piglit_probe_rect_rgba(0, 0, piglit_width, piglit_height,
				      red) && pass;

	/* Restore the 'green' program over the 'red_then_green_prog'
	 * handle. Since red_then_green_prog is currently bound, we
	 * expect supsequent draws will be green without having to
	 * rebind the program.
	 */
	if (!gpb_restore_program(red_then_green_prog, green_binary, bin_length,
				 bin_format)) {
		free(green_binary);
		fprintf(stderr, "failed to restore binary program\n");
		piglit_report_result(PIGLIT_FAIL);
	}
	free(green_binary);

	/* Verify drawing now produces green. */
	glClearColor(1.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);
	piglit_draw_rect(-1.0f, -1.0f, 2.0f, 2.0f);
	pass = piglit_probe_rect_rgba(0, 0, piglit_width, piglit_height,
				      green) && pass;

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}

enum piglit_result
piglit_display(void)
{
	return PIGLIT_FAIL;
}
