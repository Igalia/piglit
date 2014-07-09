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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 *
 * Authors:
 *    Eric Anholt <eric@anholt.net>
 *
 */

/** @file useprogram-flushverts-2.c
 *
 * Tests that a change in the shader results in previous vertices
 * getting flushed correctly with the previous shader.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

static GLint prog1, prog2;

enum piglit_result
piglit_display(void)
{
	GLboolean pass = GL_TRUE;
	float green[4] = {0.0, 1.0, 0.0, 0.0};
	float blue[4] = {0.0, 0.0, 1.0, 0.0};

	glUseProgram(prog1);
	glBegin(GL_TRIANGLE_FAN);
	glVertex2f(-1.0, -1.0);
	glVertex2f(-0.5, -1.0);
	glVertex2f(-0.5,  1.0);
	glVertex2f(-1.0,  1.0);
	glEnd();

	glUseProgram(prog2);
	glBegin(GL_TRIANGLE_FAN);
	glVertex2f(-0.5, -1.0);
	glVertex2f( 0.0, -1.0);
	glVertex2f( 0.0,  1.0);
	glVertex2f(-0.5,  1.0);
	glEnd();

	glUseProgram(prog1);
	glBegin(GL_TRIANGLE_FAN);
	glVertex2f(0.0, -1.0);
	glVertex2f(0.5, -1.0);
	glVertex2f(0.5,  1.0);
	glVertex2f(0.0,  1.0);
	glEnd();

	glUseProgram(prog2);
	glBegin(GL_TRIANGLE_FAN);
	glVertex2f(0.5, -1.0);
	glVertex2f(1.0, -1.0);
	glVertex2f(1.0,  1.0);
	glVertex2f(0.5,  1.0);
	glEnd();

	pass &= piglit_probe_pixel_rgba(piglit_width * 1 / 8, piglit_height / 2,
					green);
	pass &= piglit_probe_pixel_rgba(piglit_width * 3 / 8, piglit_height / 2,
					blue);
	pass &= piglit_probe_pixel_rgba(piglit_width * 5 / 8, piglit_height / 2,
					green);
	pass &= piglit_probe_pixel_rgba(piglit_width * 7 / 8, piglit_height / 2,
					blue);

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	GLint vs, fs1, fs2;
	const char *vs_source =
		"void main()\n"
		"{\n"
		"	gl_Position = gl_Vertex;\n"
		"}\n";
	const char *fs1_source =
		"void main()\n"
		"{\n"
		"	gl_FragColor = vec4(0.0, 1.0, 0.0, 0.0);\n"
		"}\n";
	const char *fs2_source =
		"void main()\n"
		"{\n"
		"	gl_FragColor = vec4(0.0, 0.0, 1.0, 0.0);\n"
		"}\n";

	piglit_require_gl_version(20);

	vs = piglit_compile_shader_text(GL_VERTEX_SHADER, vs_source);
	fs1 = piglit_compile_shader_text(GL_FRAGMENT_SHADER, fs1_source);
	fs2 = piglit_compile_shader_text(GL_FRAGMENT_SHADER, fs2_source);
	prog1 = piglit_link_simple_program(vs, fs1);
	prog2 = piglit_link_simple_program(vs, fs2);
}
