/*
 * Copyright © 2013 Marek Olšák <maraeo@gmail.com>
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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

/**
 * Tests if PRIMITIVES_GENERATED works with transform feedback disabled.
 *
 * From EXT_transform_feedback:
 *    "the primitives-generated count is incremented every time a primitive
 *     reaches the Discarding Rasterization stage"
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;

PIGLIT_GL_TEST_CONFIG_END

static const char *vstext = "void main() { gl_Position = gl_Vertex; }";

void piglit_init(int argc, char **argv)
{
	unsigned qresult;
	int expected = 2;
	GLuint vs;
	GLuint prog;
	GLuint q;

	/* Check the driver. */
	piglit_require_gl_version(15);
	piglit_require_GLSL();
	piglit_require_transform_feedback();

	glGenQueries(1, &q);

	glEnable(GL_RASTERIZER_DISCARD);

	/* Create shaders. */
	vs = piglit_compile_shader_text(GL_VERTEX_SHADER, vstext);
	prog = glCreateProgram();
	glAttachShader(prog, vs);
	glLinkProgram(prog);
	if (!piglit_link_check_status(prog)) {
		glDeleteProgram(prog);
		piglit_report_result(PIGLIT_FAIL);
	}
	glUseProgram(prog);

	/* Draw a rectangle; make sure two primitives were generated. */
	glBeginQuery(GL_PRIMITIVES_GENERATED, q);
	piglit_draw_rect(-1, -1, 2, 2);
	glEndQuery(GL_PRIMITIVES_GENERATED);
	glGetQueryObjectuiv(q, GL_QUERY_RESULT, &qresult);

	if (qresult != expected) {
		printf("Primitives generated: %i,  Expected: %i\n",
		       qresult, expected);
		piglit_report_result(PIGLIT_FAIL);
	}

	piglit_report_result(PIGLIT_PASS);
}

enum piglit_result piglit_display(void)
{
	return PIGLIT_FAIL; /* should not get here */
}
