/*
 * Copyright © 2019 Intel Corporation
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

/** @file fface-invariant.c
 *
 * The OpenGL ES Shading Language 1.00 specification says:
 *
 *    "(4.6.4 Invariance and Linkage): [...]
 *     It is an error to declare gl_FrontFacing as invariant.  The invariance
 *     of gl_FrontFacing is the same as the invariance of gl_Position."
 *
 * Most of the errors in this section must be enforced at link time, but
 * disallowing the invariant qualifier on gl_FrontFacing could easily be
 * done as a compile time error (and earlier is usually preferable).  We
 * allow either in this test.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN
	config.supports_gl_es_version = 20;
	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;
PIGLIT_GL_TEST_CONFIG_END

enum piglit_result
piglit_display(void)
{
	/* unreachable */
	return PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	GLuint vs_shader, fs_shader, prog;

	const char *vs_source = "void main() { gl_Position = vec4(0); }\n";
	const char *fs_source = "invariant gl_FrontFacing;\n"
		                "void main() { gl_FragColor = vec4(0); }\n";

	vs_shader = piglit_compile_shader_text(GL_VERTEX_SHADER, vs_source);
	fs_shader = piglit_compile_shader_text_nothrow(GL_FRAGMENT_SHADER,
						       fs_source, true);

	/* A compile error is allowed. */
	if (!fs_shader)
		piglit_report_result(PIGLIT_PASS);

	prog = piglit_link_simple_program(vs_shader, fs_shader);

	/* A link error is allowed. */
	if (!prog)
		piglit_report_result(PIGLIT_PASS);

	piglit_report_result(PIGLIT_FAIL);
}
