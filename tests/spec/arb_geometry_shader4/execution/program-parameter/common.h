/*
 * Copyright © 2013 The Piglit project
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

/**
 * \file common.h
 *
 * Common code for glProgramParameter testing.
 */

#include "piglit-util-gl.h"


struct primitive_geom_info {
	GLenum type;
	GLenum error;
};


/* Primitive types passed as geometry shader input type and expected error. */
static const struct primitive_geom_info primitives_in[] = {
	{GL_POINTS, GL_NO_ERROR},

	{GL_LINES, GL_NO_ERROR},
	{GL_LINE_STRIP, GL_INVALID_VALUE},
	{GL_LINE_LOOP, GL_INVALID_VALUE},

	{GL_TRIANGLES, GL_NO_ERROR},
	{GL_TRIANGLE_STRIP, GL_INVALID_VALUE},
	{GL_TRIANGLE_FAN, GL_INVALID_VALUE},

	{GL_LINES_ADJACENCY, GL_NO_ERROR},
	{GL_LINE_STRIP_ADJACENCY, GL_INVALID_VALUE},

	{GL_TRIANGLES_ADJACENCY, GL_NO_ERROR},
	{GL_TRIANGLE_STRIP_ADJACENCY, GL_INVALID_VALUE},

	{GL_QUADS, GL_INVALID_VALUE},
	{GL_QUAD_STRIP, GL_INVALID_VALUE},
	{GL_POLYGON, GL_INVALID_VALUE},
};


static const char vs_text[] =
	"void main()\n"
	"{\n"
	"	gl_Position = vec4(0);\n"
	"}\n";

static const char gs_text[] =
	"#extension GL_ARB_geometry_shader4: enable\n"
	"uniform int vertex_count;\n"
	"void main()\n"
	"{\n"
	"	for (int i = 0; i < vertex_count; i++) {\n"
	"		gl_Position = vec4(0.0);\n"
	"		EmitVertex();\n"
	"	}\n"
	"}\n";

static const char fs_text[] =
	"void main()\n"
	"{\n"
	"	gl_FragColor = vec4(1.0);\n"
	"}\n";


static GLuint
create_shader(const char *const vs_text, const char *const gs_text,
	      const char *const fs_text)
{
	const char *const varying = "var";
	GLuint vs, gs, fs, prog;

	prog = glCreateProgram();
	vs = piglit_compile_shader_text(GL_VERTEX_SHADER, vs_text);
	glAttachShader(prog, vs);
	glDeleteShader(vs);
	gs = piglit_compile_shader_text(GL_GEOMETRY_SHADER, gs_text);
	glAttachShader(prog, gs);
	glDeleteShader(gs);
	if (fs_text) {
		fs = piglit_compile_shader_text(GL_FRAGMENT_SHADER, fs_text);
		glAttachShader(prog, fs);
		glDeleteShader(fs);
	} else {
		glTransformFeedbackVaryings(prog, 1, &varying,
				            GL_INTERLEAVED_ATTRIBS);
	}

	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	return prog;
}

