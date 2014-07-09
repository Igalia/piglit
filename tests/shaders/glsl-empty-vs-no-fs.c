/*
 * Copyright 2009 VMware, Inc.
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sub license, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice (including the
 * next paragraph) shall be included in all copies or substantial portions
 * of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.
 * IN NO EVENT SHALL TUNGSTEN GRAPHICS AND/OR ITS SUPPLIERS BE LIABLE FOR
 * ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * Authors:
 *    Alex Corscadden
 *    Vinson Lee
 */

/*
 * Test an empty GLSL vertex shader without a fragment shader. The program
 * may not link, but if it does, should not trigger a driver crash.
 *
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_width = 250;
	config.window_height = 250;
	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

static const char *vs_source = "void main() {}";

enum piglit_result
piglit_display(void)
{
	GLint vs;
	GLint prog;
	GLint linked;
	int i;

	for (i = 0; i < 32; i++) {
		vs = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vs, 1, &vs_source, NULL);
		glCompileShader(vs);

		prog = glCreateProgram();
		glAttachShader(prog, vs);
		glLinkProgram(prog);
		glGetProgramiv(prog, GL_LINK_STATUS, &linked);
		if (linked) {
			glUseProgram(prog);
		}

		glFlush();

		glDeleteProgram(prog);
		glDeleteShader(vs);
	}

	return PIGLIT_PASS;
}

void
piglit_init(int argc, char **argv)
{
	piglit_require_gl_version(20);
}
