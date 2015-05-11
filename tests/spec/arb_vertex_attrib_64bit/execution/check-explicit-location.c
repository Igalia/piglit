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
 * \file check-explicit-location.c
 * Basic test of GL_ARB_explicit_attrib_location + GL_ARB_vertex_attrib_64bit
 *
 * Load a shader that uses the location layout qualifier on an attribute.
 * Verify that the attribute is assigned that location.
 *
 * \author Ian Romanick + Dave Airlie
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_core_version = 33;

	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

const char *vs_text =
	"#version 330\n"
	"#extension GL_ARB_vertex_attrib_64bit: require\n"
	"#extension GL_ARB_gpu_shader_fp64: require\n"
	"layout(location = 0) in dvec4 vertex;\n"
	"layout(location = 1) in dvec4 vcolor;\n"
	"flat out dvec4 fscolor;\n"
	"void main()\n"
	"{\n"
	"gl_Position = vertex;\n"
	"fscolor = vcolor;\n"
	"}\n";

const char *fs_text =
	"#version 330\n"
	"#extension GL_ARB_gpu_shader_fp64: require\n"
	"flat in dvec4 fscolor;\n"
	"out vec4 color;\n"
	"void main()\n"
	"{\n"
	"	color = vec4(fscolor);\n"
	"}\n";

enum piglit_result
piglit_display(void)
{
	return PIGLIT_FAIL;
}

void piglit_init(int argc, char **argv)
{
	GLint vert, frag;
	GLint prog;
	GLboolean ok;

	piglit_require_gl_version(20);

	piglit_require_extension("GL_ARB_explicit_attrib_location");
	piglit_require_extension("GL_ARB_vertex_attrib_64bit");

	vert = piglit_compile_shader_text(GL_VERTEX_SHADER, vs_text);
	frag = piglit_compile_shader_text(GL_FRAGMENT_SHADER, fs_text);
	prog = glCreateProgram();
	glAttachShader(prog, vert);
	glAttachShader(prog, frag);
	glLinkProgram(prog);

	ok = piglit_link_check_status(prog);
	if (ok) {

		GLint loc = glGetAttribLocation(prog, "vertex");

		if (loc != 0) {
			fprintf(stderr,
				"Expected location of 'vertex' to be 0, got "
				"%d instead.\n", loc);
			ok = GL_FALSE;
		}
		loc = glGetAttribLocation(prog, "vcolor");

		if (loc != 1) {
			fprintf(stderr,
				"Expected location of 'vcolor' to be 0, got "
				"%d instead.\n", loc);
			ok = GL_FALSE;
		}
	}

	piglit_report_result(ok ? PIGLIT_PASS : PIGLIT_FAIL);
}

