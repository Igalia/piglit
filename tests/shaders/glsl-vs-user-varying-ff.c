/*
 * Copyright © 2010 Intel Corporation
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
 * \file glsl-vs-user-varying-ff.c
 * Test a vertex shader with a user-defined varying used with fixed function
 *
 * \author Ian Romanick <ian.d.romanick@intel.com>
 */
#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

static const char vs_text[] =
	"varying vec4 color;\n"
	"\n"
	"void main()\n"
	"{\n"
	"  gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;\n"
	"  color = vec4(0.0, 1.0, 0.0, 1.0);\n"
	"  gl_FrontColor = vec4(0.0, 1.0, 0.0, 1.0);\n"
	"}\n"
	;

GLuint prog;

enum piglit_result
piglit_display(void)
{
	static const float green[3] = { 0.0, 1.0, 0.0 };
	enum piglit_result result = PIGLIT_PASS;

	glClear(GL_COLOR_BUFFER_BIT);

	piglit_draw_rect(10, 10, 10, 10);

	if (!piglit_probe_pixel_rgb(15, 15, green))
		result = PIGLIT_FAIL;

	if (!piglit_automatic)
		piglit_present_results();

	return result;
}

void
piglit_init(int argc, char **argv)
{
	GLuint vs;

	piglit_require_gl_version(20);

	glClearColor(0.3, 0.3, 0.3, 0.0);
	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);

	vs = piglit_compile_shader_text(GL_VERTEX_SHADER, vs_text);

	prog = piglit_link_simple_program(vs, 0);

	glDeleteShader(vs);

	/* Don't try to render if the program failed to link, and linking
	 * had better succeed!
	 */
	if (!piglit_link_check_status(prog))
		piglit_report_result(PIGLIT_FAIL);

	glUseProgram(prog);
}
