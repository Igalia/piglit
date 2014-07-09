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
 * Test that built-in vertex input variables are enumerated by GetActiveAttrib()
 *
 * This is not explicitly stated in any specs before 4.3 core but it seems to
 * be clarified in later specs
 *
 * From GL 4.3 core spec, section 11.1.1 (Vertex Attributes):
 * "For GetActiveAttrib, all active vertex shader input variables are
 *  enumerated, including the special built-in inputs gl_VertexID and
 *  gl_InstanceID."
 *
 * From GL 4.3 core spec, section F.5 (Change Log for Released Specifications):
 * "Specify in section 11.1.1 that special built-in inputs and outputs such as
 *  gl_VertexID should be enumerated in the PROGRAM_INPUT and PROGRAM_OUTPUT
 *  interfaces, as well as the legacy function GetActiveAttrib. Add spec
 *  language counting the built-ins gl_VertexID and gl_InstanceID against the
 *  active attribute limit (Bug 9201)."
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 31;
        config.supports_gl_core_version = 31;

	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

static const char *vstext =
	"#version 140\n"
	"in vec4 piglit_vertex;\n"
	"flat out int instID;\n"
	"flat out int vertID;\n"
	"void main() {\n"
	"	gl_Position = piglit_vertex;\n"
	"	instID = gl_InstanceID;\n"
	"	vertID = gl_VertexID;\n"
	"}\n";

static const char *fstext =
	"#version 140\n"
	"flat in int instID;\n"
	"flat in int vertID;\n"
	"out vec4 color;\n"
	"void main() {\n"
	"	color = vec4(instID + vertID);\n"
	"}\n";

static GLuint prog;

bool
check_that_attrib_is_active(const char *attrib_name)
{
	int i;
	int numAttribs = 0;
	GLsizei length;
	GLint size;
	GLenum type;
	GLchar name[100];

	glGetProgramiv(prog, GL_ACTIVE_ATTRIBUTES, &numAttribs);
	for (i = 0; i < numAttribs; i++) {
		glGetActiveAttrib(prog, i, sizeof(name), &length, &size,
				  &type, name);
		if (strcmp(name, attrib_name) == 0) {
			return piglit_check_gl_error(GL_NO_ERROR);
		}
	}

	printf("%s was not counted as active.\n", attrib_name);
	return piglit_check_gl_error(GL_NO_ERROR) && false;
}

void
piglit_init(int argc, char **argv)
{
	bool pass = true;

	prog = piglit_build_simple_program(vstext, fstext);

	glLinkProgram(prog);
	if (!piglit_link_check_status(prog)) {
		glDeleteProgram(prog);
		piglit_report_result(PIGLIT_FAIL);
	}

	pass = check_that_attrib_is_active("piglit_vertex") && pass;

	pass = check_that_attrib_is_active("gl_InstanceID") && pass;

	pass = check_that_attrib_is_active("gl_VertexID") && pass;

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}

enum piglit_result
piglit_display(void)
{
	/* UNREACHABLE */
	return PIGLIT_FAIL;
}
