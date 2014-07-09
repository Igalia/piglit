/**
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
 * Test that GetShaderiv() may now return GEOMETRY_SHADER if passed SHADER_TYPE
 *
 * From the GLSL 3.2 spec, section 6.1.10(Shader and Program Queries):
 * "The command
 *  	void GetShaderiv( uint shader, enum pname, int *params );
 *  returns properties of the shader object named shader in params. The
 *  parameter value to return is specified by pname.
 *    If pname is SHADER_TYPE, VERTEX_SHADER, GEOMETRY_SHADER, or
 *  FRAGMENT_SHADER is returned if shader is a vertex, geometry, or fragment
 *  shader object respectively."
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 32;
        config.supports_gl_core_version = 32;

	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

static const char *gstext =
	"#version 150\n"
	"layout(triangles) in;\n"
	"layout(triangle_strip, max_vertices = 3) out;\n"
	"in vec4 pos[];\n"
	"void main() {\n"
	"	for(int i = 0; i < 3; i++) {\n"
	"		gl_Position = pos[i];\n"
	"		EmitVertex();\n"
	"	}\n"
	"}\n";

void
piglit_init(int argc, char **argv)
{
	bool pass = true;
	GLuint gs = 0;
	GLint type;

	gs = piglit_compile_shader_text(GL_GEOMETRY_SHADER, gstext);

	glGetShaderiv(gs, GL_SHADER_TYPE, &type);
	if(type != GL_GEOMETRY_SHADER) {
		printf("Expected shader type GL_GEOMETRY_SHADER"
			" but received: %s\n",
			piglit_get_gl_enum_name(type));
		pass = false;
	}
	pass = piglit_check_gl_error(GL_NO_ERROR) & pass;

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}

enum piglit_result
piglit_display(void)
{
	return PIGLIT_FAIL;
}
