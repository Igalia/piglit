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
 */

/**
 * \file glsl-explicit-attrib-location-03.c
 * Test GL_ARB_explicit_attrib_location with conflicting locations
 *
 * Link two vertex shaders that specify differing explicit locations for the
 * same attribute.  Verify that a link error is generated.
 *
 * \author Ian Romanick
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

enum piglit_result
piglit_display(void)
{
	return PIGLIT_FAIL;
}

void piglit_init(int argc, char **argv)
{
	GLint vert[2];
	GLint prog;
	GLboolean ok;

	piglit_require_gl_version(20);

	piglit_require_extension("GL_ARB_explicit_attrib_location");

	vert[0] =
		piglit_compile_shader(GL_VERTEX_SHADER,
				      "shaders/glsl-explicit-location-03a.vert");
	vert[1] =
		piglit_compile_shader(GL_VERTEX_SHADER,
				      "shaders/glsl-explicit-location-03b.vert");
	prog = glCreateProgram();
	glAttachShader(prog, vert[0]);
	glAttachShader(prog, vert[1]);
	glLinkProgram(prog);

	ok = piglit_link_check_status_quiet(prog);
	if (ok)
		fprintf(stderr,
			"Linking with conflicting explicit locations "
			"succeeded when it should have failed.\n");

	piglit_report_result(!ok ? PIGLIT_PASS : PIGLIT_FAIL);
}

