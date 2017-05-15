/*
 * Copyright (c) 2014 Intel Corporation
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

#include "cs-ids-common.h"

static struct piglit_gl_test_config *piglit_config;

PIGLIT_GL_TEST_CONFIG_BEGIN
	piglit_config = &config;
	config.supports_gl_compat_version = 33;
	config.supports_gl_core_version = 33;
	config.khr_no_error_support = PIGLIT_NO_ERRORS;
PIGLIT_GL_TEST_CONFIG_END


const char passthrough_vs_src[] =
	"#version 330\n"
	"\n"
	"in vec4 piglit_vertex;\n"
	"\n"
	"void main() {\n"
	"    gl_Position = piglit_vertex;\n"
	"}\n"
	;


static const char *green_fs_src =
	"#version 330\n"
	"\n"
	"void main()\n"
	"{\n"
	"    gl_FragColor = vec4(0.0, 1.0, 0.0, 1.0);\n"
	"}\n";

enum piglit_result
piglit_display(void)
{
	return PIGLIT_FAIL;
}


void
piglit_init(int argc, char **argv)
{
	enum piglit_result result;
	GLuint render_program;
	int i;
	float green[4] = { 0.0, 1.0, 0.0, 1.0 };

	cs_ids_common_init();
	cs_ids_set_local_id_test();
	cs_ids_set_local_size(4, 4, 4);
	cs_ids_set_global_size(4, 4, 4);

	render_program =
		piglit_build_simple_program(passthrough_vs_src,
					    green_fs_src);
	glClearColor(0.0, 0.0, 0.0, 0.0);

	for (i = 0; i < 2; i++) {
		result = cs_ids_run_test();
		if (result != PIGLIT_PASS)
			piglit_report_result(result);

		glUseProgram(render_program);
		glClear(GL_COLOR_BUFFER_BIT);
		piglit_draw_rect(-1, -1, 2, 2);
		result = piglit_probe_rect_rgba(0, 0, piglit_width,
						piglit_height, green)
			? PIGLIT_PASS : PIGLIT_FAIL;
	}

	cs_ids_common_destroy();

	piglit_report_result(result);
}
