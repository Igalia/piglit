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
 * Test the syntax for accessing interface block members through the API
 *
 * From the GLSL 1.50 core spec, section 4.3.7 (Interface Blocks):
 * "Outside the shading language (i.e., in the API), members are similarly
 *  identified except the block name is always used in place of the instance
 *  name (API accesses are to interfaces, not to shaders). If there is no
 *  instance name, then the API does not use the block name to access a member,
 *  just the member name."
 *
 * "For blocks declared as arrays, the array index must also be included when
 *  accessing members"
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 32;
	config.supports_gl_core_version = 32;

PIGLIT_GL_TEST_CONFIG_END

static const char *vstext =
	"#version 150\n"
	"in vec4 vertex;\n"
	"void main()\n"
	"{\n"
	"	gl_Position = vertex;\n"
	"}\n";

static const char *gstext =
	"#version 150\n"
	"layout(points) in;\n"
	"layout(points, max_vertices = 3) out;\n"
	"out NoInst {\n"
	"	float a;\n"
	"	vec3 b;\n"
	"};\n"
	"out WithInst {\n"
	"	float c;\n"
	"	vec3 d;\n"
	"} inst;\n"
	"out WithInstArray {\n"
	"	float e;\n"
	"	vec3 f;\n"
	"} instArray[3];\n"
	"void main()\n"
	"{\n"
	"	a = 1.0;\n"
	"	b = vec3(2.0);\n"
	"	inst.c = 3.0;\n"
	"	inst.d = vec3(4.0);\n"
	"	for(int i = 0; i < 3; i++) {\n"
	"		instArray[i].e = 5.0 + 2 * i;\n"
	"		instArray[i].f = vec3(6.0 + 2 * i);\n"
	"	}\n"
	"}\n";

static GLuint prog;

static const char *valid_varying_names[] = {
	/* correct names to access block members */
	"a", "b",
	"WithInst.c", "WithInst.d",
	"WithInstArray[0].e", "WithInstArray[0].f",
	"WithInstArray[1].e", "WithInstArray[1].f",
	"WithInstArray[2].e", "WithInstArray[2].f"
};

static const char *invalid_varying_names[] = {
	/* incorrect names to access block members */
	"c", "d",
	"inst.c", "inst.d",
	"e", "f",
	"instArray.e", "instArray.f",
	"WithInstArray.e", "WithInstArray.f",
	"instArray[0].e", "instArray[0].f",
	"instArray[1].e", "instArray[1].f",
	"instArray[2].e", "instArray[2].f"
};

void
piglit_init(int argc, char **argv)
{
	bool pass = true;
	GLuint vs = 0, gs = 0;
	int i;

	prog = glCreateProgram();
	vs = piglit_compile_shader_text(GL_VERTEX_SHADER, vstext);
	gs = piglit_compile_shader_text(GL_GEOMETRY_SHADER, gstext);
	glAttachShader(prog, vs);
	glAttachShader(prog, gs);

	/* check that invalid_varying_names fail to link */
	for (i = 0; i < ARRAY_SIZE(invalid_varying_names); i++) {
		glTransformFeedbackVaryings(prog,
					    1,
					    &invalid_varying_names[i],
					    GL_INTERLEAVED_ATTRIBS);
		glLinkProgram(prog);
		if (piglit_link_check_status_quiet(prog)) {
			printf("%s is not valid but it was allowed.\n",
				invalid_varying_names[i]);
			pass = false;
		}
	}

	/* check that valid_varying_names link properly */
	glTransformFeedbackVaryings(prog,
				    ARRAY_SIZE(valid_varying_names),
				    valid_varying_names,
				    GL_INTERLEAVED_ATTRIBS);
	glLinkProgram(prog);
	if (!piglit_link_check_status(prog)) {
		glDeleteProgram(prog);
		printf("Transform feedback varyings failed to link properly"
			" with valid names.\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	glUseProgram(prog);

	for (i = 0; i < ARRAY_SIZE(valid_varying_names); i++) {
		char varName[50];
		GLsizei nameLength = 0, varSize = 0;
		GLenum varType = GL_NONE;
		glGetTransformFeedbackVarying(  prog,
						i,
						sizeof(varName),
						&nameLength,
						&varSize,
						&varType,
						varName);
		printf("Name: %s\t\tType: %s\n",
			varName, piglit_get_gl_enum_name(varType));
	}

	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}

enum piglit_result
piglit_display(void)
{
	/* DIES IN A FIRE */
	return PIGLIT_FAIL;
}
