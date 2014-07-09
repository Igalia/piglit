/*
 * Copyright 2011 VMware, Inc.
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
 * \file glsl-link-bug38015.c
 *
 * Reproduces i965 crash from FDO bug 38015.
 *
 * \author Vinson Lee <vlee@vmware.com>
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

static const char fs_text[] =
	"#extension GL_ARB_shader_texture_lod : enable\n"
	"uniform sampler2D S0;"
	"void main(void) {"
	"  vec2 coord = vec2(0.0, 0.0);"
	"  vec2 ddx = vec2(0.0, 0.0);"
	"  vec2 ddy = vec2(0.0, 0.0);"
	"  gl_FragColor = texture2DGradARB(S0, coord, ddx, ddy);"
        "}";

enum piglit_result
piglit_display(void)
{
	return PIGLIT_FAIL;
}

void piglit_init(int argc, char **argv)
{
	GLint frag;
	GLint prog;
	const char *fs_text_ptr = fs_text;

	piglit_require_gl_version(20);

	/* Intentionally omit check for GL_ARB_shader_texture_lod. */

	frag = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(frag, 1, (const GLchar **) &fs_text_ptr, NULL);
	glCompileShader(frag);

	prog = glCreateProgram();
	glAttachShader(prog, frag);
	glLinkProgram(prog);

	piglit_report_result(PIGLIT_PASS);
}
