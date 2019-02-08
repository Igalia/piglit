/*
 * Copyright © 2013, 2019 Intel Corporation
 * Copyright © 2015 Advanced Micro Devices, Inc.
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
 * This test uses separable program objects with 2 shaders (VS, GS)
 * and tests that the same interface matching rules by location apply
 * in between the VS -> GS interface as if it would not be separable.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 0;
	config.supports_gl_core_version = 32;
	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

static const char *vs_code =
	"#version 150\n"
	"#extension GL_ARB_separate_shader_objects: require\n"
	"#extension GL_ARB_explicit_attrib_location: require\n"
	"\n"
	"layout(location = 0) in vec4 piglit_vertex;\n"
	"\n"
	"layout(location = 2) out vec3 a;\n"
	"\n"
	"void main()\n"
	"{\n"
	"    gl_Position = piglit_vertex;\n"
	"    a = vec3(0.5, 0, 0.3);\n"
	"}\n"
	;

static const char *gs_code =
	"#version 150\n"
	"#extension GL_ARB_separate_shader_objects: require\n"
	"#extension GL_ARB_explicit_attrib_location: require\n"
	"\n"
	"layout(triangles) in;\n"
	"layout(triangle_strip, max_vertices = 3) out;\n"
	"\n"
	"layout(location = 1) in vec3 va[];\n"
	"\n"
	"layout(location = 3) out vec3 ga;\n"
	"\n"
	"void main()\n"
	"{\n"
	"    for (int i = 0; i < 3; i++) {"
	"        gl_Position = gl_in[i].gl_Position;\n"
	"        ga = va[i] * 1.35;\n"
	"        EmitVertex();\n"
	"    }\n"
	"}\n"
	;

enum piglit_result
piglit_display(void)
{
	/* UNREACHED */
	return PIGLIT_FAIL;
}

void piglit_init(int argc, char **argv)
{
	GLuint prog;
	bool pass;

	piglit_require_extension("GL_ARB_separate_shader_objects");
	piglit_require_extension("GL_ARB_explicit_attrib_location");

	prog = piglit_build_simple_program_unlinked_multiple_shaders(
				GL_VERTEX_SHADER, vs_code,
				GL_GEOMETRY_SHADER, gs_code,
				0);

	glProgramParameteri(prog, GL_PROGRAM_SEPARABLE, GL_TRUE);
	piglit_check_gl_error(GL_NO_ERROR);

	glLinkProgram(prog);
	pass = !piglit_link_check_status_quiet(prog);

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}
