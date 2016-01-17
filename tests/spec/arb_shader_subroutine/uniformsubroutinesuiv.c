/*
 * Copyright © 2016 Intel Corporation
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

/** @file uniformsubroutinesuiv.c
 *
 * When setting the index for a subroutine with an explicit location, inactive
 * locations in the indices param should be ignored.
 *
 * From Section 7.9. (SUBROUTINE UNIFORM VARIABLES) of the OpenGL 4.5 Core
 * spec:
 *
 *    "The command
 *
 *        void UniformSubroutinesuiv(enum shadertype, sizei count,
 *                                   const uint *indices);
 *
 *    will load all active subroutine uniforms for shader stage shadertype
 *    with subroutine indices from indices, storing indices[i] into the
 *    uniform at location i. The indices for any locations between zero and
 *    the value of ACTIVE_SUBROUTINE_UNIFORM_LOCATIONS minus one which are not
 *    used will be ignored."
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 32;
	config.supports_gl_core_version = 32;
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;

PIGLIT_GL_TEST_CONFIG_END

static GLuint prog;

static const char frag_shader_text[] =
	"#version 150\n"
	"#extension GL_ARB_shader_subroutine : require\n"
	"#extension GL_ARB_explicit_uniform_location : require\n"
	"#extension GL_ARB_explicit_attrib_location : require"
	"\n"
	"out vec4 fragColor;\n"
	"subroutine vec4 color_t();\n"
	"\n"
	"layout(location = 5) subroutine uniform color_t Color;\n"
	"\n"
	"subroutine(color_t)\n"
	"vec4 ColorRed()\n"
	"{\n"
	"  return vec4(1, 0, 0, 1);\n"
	"}\n"
	"\n"
	"subroutine(color_t)\n"
	"vec4 ColorBlue()\n"
	"{\n"
	"  return vec4(0, 0, 1, 1);\n"
	"}\n"
	"\n"
	"void main()\n"
	"{\n"
	"  fragColor = Color();\n"
	"}\n";

void
piglit_init(int argc, char **argv)
{
	bool pass = true;

	/* The first 5 indices should be ignored */
	const unsigned indices[6] = {0, 0, 0, 0, 0, 1};

	piglit_require_extension("GL_ARB_shader_subroutine");
	piglit_require_extension("GL_ARB_explicit_uniform_location");
	piglit_require_extension("GL_ARB_explicit_attrib_location");

	prog = piglit_build_simple_program(NULL, frag_shader_text);

	glUseProgram(prog);
	glUniformSubroutinesuiv(GL_FRAGMENT_SHADER, 6, indices);

	if (!piglit_check_gl_error(0)) {
		pass = false;
	}

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}

enum piglit_result piglit_display(void)
{
	/* UNREACHED */
	return PIGLIT_FAIL;
}
