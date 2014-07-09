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
 * Test that GetProgramiv() now accepts GEOMETRY_INPUT_TYPE,
 * GEOMETRY_OUTPUT_TYPE and GEOMETRY_VERTICES_OUT.
 *
 * From the GLSL 3.2 spec, section 2.12.1 (Geometry Shader Input Primitives)
 * "The input primitive type may be queried by calling GetProgramiv with the
 *  symbolic constant GEOMETRY_INPUT_TYPE."
 *
 * "The output primitive type and maximum output vertex count of a linked
 *  program may be queried by calling GetProgramiv with the symbolic constants
 *  GEOMETRY_OUTPUT_TYPE and GEOMETRY_VERTICES_OUT, respectively."
 *
 * Also, from section 6.1.10(Shader and Program Queries):
 * "The command
 *  	void GetProgramiv( uint program, enum pname, int *params );
 *  returns properties of the program object named program in params. The
 *  parameter value to return is specified by pname."
 *
 * "If pname is GEOMETRY_VERTICES_OUT, the maximum number of vertices the
 *  geometry shader will output is returned. If pname is GEOMETRY_INPUT_TYPE,
 *  the geometry shader input type, which must be one of POINTS, LINES,
 *  LINES_ADJACENCY, TRIANGLES or TRIANGLES_ADJACENCY, is returned. If pname is
 *  GEOMETRY_OUTPUT_TYPE, the geometry shader output type, which must be one of
 *  POINTS, LINE_STRIP or TRIANGLE_STRIP, is returned. If GEOMETRY_VERTICES_OUT,
 *  GEOMETRY_INPUT_TYPE, or GEOMETRY_OUTPUT_TYPE are queried for a program which
 *  has not been linked successfully, or which does not contain objects to form
 *  a geometry shader, then an INVALID_OPERATION error is generated."
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 32;
        config.supports_gl_core_version = 32;

	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

static const char *vstext =
	"#version 150\n"
	"in vec4 piglit_vertex;\n"
	"out vec4 pos;\n"
	"void main() {\n"
	"	gl_Position = piglit_vertex;\n"
	"	pos = piglit_vertex;\n"
	"}\n";

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

static const char *fstext =
	"#version 150\n"
	"out vec4 color;\n"
	"void main() {\n"
	"	color = vec4(1.);\n"
	"}\n";

static GLuint prog, prog_no_gs;

void
piglit_init(int argc, char **argv)
{
	bool pass = true;
	GLuint vs = 0, gs = 0, fs = 0;
	GLint type;

	/* Create a program with geometry shader to test GetProgramiv() with
	 * new enum parameters.
	 */
	prog = glCreateProgram();
	vs = piglit_compile_shader_text(GL_VERTEX_SHADER, vstext);
	gs = piglit_compile_shader_text(GL_GEOMETRY_SHADER, gstext);
	fs = piglit_compile_shader_text(GL_FRAGMENT_SHADER, fstext);
	glAttachShader(prog, vs);
	glAttachShader(prog, gs);
	glAttachShader(prog, fs);

	glLinkProgram(prog);
	if(!piglit_link_check_status(prog)){
		glDeleteProgram(prog);
		piglit_report_result(PIGLIT_FAIL);
	}

	glGetProgramiv(prog, GL_GEOMETRY_INPUT_TYPE, &type);
	if(type != GL_TRIANGLES) {
		printf("Expected input type GL_TRIANGLES but received: %s\n",
			piglit_get_gl_enum_name(type));
		pass = false;
	}
	pass = piglit_check_gl_error(GL_NO_ERROR) & pass;

	glGetProgramiv(prog, GL_GEOMETRY_OUTPUT_TYPE, &type);
	if(type != GL_TRIANGLE_STRIP) {
		printf("Expected output type GL_TRIANGLE_STRIP"
			" but received: %s\n",
			piglit_get_gl_enum_name(type));
		pass = false;
	}
	pass = piglit_check_gl_error(GL_NO_ERROR) & pass;

	glGetProgramiv(prog, GL_GEOMETRY_VERTICES_OUT, &type);
	if(type != 3) {
		printf("Expected max verts = 3 but received: %d\n",
			type);
		pass = false;
	}
	pass = piglit_check_gl_error(GL_NO_ERROR) & pass;

	/* Create new program without a geometry shader and test errors */
	prog_no_gs = glCreateProgram();
	vs = piglit_compile_shader_text(GL_VERTEX_SHADER, vstext);
	fs = piglit_compile_shader_text(GL_FRAGMENT_SHADER, fstext);
	glAttachShader(prog_no_gs, vs);
	glAttachShader(prog_no_gs, fs);

	/* program not linked successfully yet should emit INVALID_OPERATION */
	glGetProgramiv(prog_no_gs, GL_GEOMETRY_INPUT_TYPE, &type);
	pass = piglit_check_gl_error(GL_INVALID_OPERATION) & pass;

	glGetProgramiv(prog_no_gs, GL_GEOMETRY_OUTPUT_TYPE, &type);
	pass = piglit_check_gl_error(GL_INVALID_OPERATION) & pass;

	glGetProgramiv(prog_no_gs, GL_GEOMETRY_VERTICES_OUT, &type);
	pass = piglit_check_gl_error(GL_INVALID_OPERATION) & pass;

	glLinkProgram(prog_no_gs);
	if(!piglit_link_check_status(prog_no_gs)){
		glDeleteProgram(prog_no_gs);
		piglit_report_result(PIGLIT_FAIL);
	}

	/* program without a geometry shader should emit INVALID_OPERATION */
	glGetProgramiv(prog_no_gs, GL_GEOMETRY_INPUT_TYPE, &type);
	pass = piglit_check_gl_error(GL_INVALID_OPERATION) & pass;

	glGetProgramiv(prog_no_gs, GL_GEOMETRY_OUTPUT_TYPE, &type);
	pass = piglit_check_gl_error(GL_INVALID_OPERATION) & pass;

	glGetProgramiv(prog_no_gs, GL_GEOMETRY_VERTICES_OUT, &type);
	pass = piglit_check_gl_error(GL_INVALID_OPERATION) & pass;

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}

enum piglit_result
piglit_display(void)
{
	return PIGLIT_FAIL;
}
