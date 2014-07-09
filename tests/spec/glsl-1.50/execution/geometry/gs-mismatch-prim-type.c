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
 * From the GLSL 3.2 spec, section 2.12.1 (Geometry Shader Input Primitives):
 *
 * "If a geometry shader is active, any command that transfers vertices to the
 *  GL will generate an INVALID_OPERATION error if the primitive mode parameter
 *  is incompatible with the input primitive type of the currently active
 *  program object, as discussed below."
 *
 * "Geometry shaders that operate on points are valid only for the POINTS
 *  primitive type."
 * "Geometry shaders that operate on line segments are valid only for the LINES,
 *  LINE_STRIP, and LINE_LOOP primitive types."
 * "Geometry shaders that operate on line segments with adjacent vertices are
 *  valid only for the LINES_ADJACENCY and LINE_STRIP_ADJACENCY primitive
 *  types."
 * "Geometry shaders that operate on triangles are valid for the TRIANGLES,
 *  TRIANGLE_STRIP and TRIANGLE_FAN primitive types."
 * "Geometry shaders that operate on triangles with adjacent vertices are valid
 *  for the TRIANGLES_ADJACENCY and TRIANGLE_STRIP_ADJACENCY primitive types."
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 32;
        config.supports_gl_core_version = 32;

	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

static const char *vstext =
	"#version 150\n"
	"in vec3 vertex;\n"
	"out vec3 pos;\n"
	"void main() {\n"
	"	gl_Position = vec4(vertex, 1.);\n"
	"	pos = vertex;\n"
	"}\n";

static const char *gstemplate =
	"#version 150\n"
	"#define LAYOUT_TYPE %s\n"
	"layout(LAYOUT_TYPE) in;\n"
	"layout(triangle_strip, max_vertices = 3) out;\n"
	"in vec3 pos[];\n"
	"void main() {\n"
	"	for(int i = 0; i < pos.length(); i++) {\n"
	"		gl_Position = vec4(pos[i], 1.);\n"
	"		EmitVertex();\n"
	"	}\n"
	"}\n";

static const char *fstext =
	"#version 150\n"
	"out vec4 color;\n"
	"void main() {\n"
	"	color = vec4(1.);\n"
	"}\n";

static GLuint vao;
static GLuint vertBuff;
static GLuint indexBuf;

static GLfloat vertices[] = {
	-1.0, 1.0, 0.0,
	 1.0, 1.0, 0.0,
	 1.0,-1.0, 0.0,
	-1.0,-1.0, 0.0
};
static GLsizei vertSize = sizeof(vertices);

static GLuint indices[] = {
	0, 1, 2, 0, 2, 3
};
static GLsizei indSize = sizeof(indices);

static GLuint prog;

static const struct test_set
{
	GLenum prim_type;
	const char *layout_type;
} tests[] = {
	{GL_POINTS, "points"},
	{GL_LINES, "lines"},
	{GL_LINE_STRIP, "lines"},
	{GL_LINE_LOOP, "lines"},
	{GL_LINES_ADJACENCY, "lines_adjacency"},
	{GL_LINE_STRIP_ADJACENCY, "lines_adjacency"},
	{GL_TRIANGLES, "triangles"},
	{GL_TRIANGLE_STRIP, "triangles"},
	{GL_TRIANGLE_FAN, "triangles"},
	{GL_TRIANGLES_ADJACENCY, "triangles_adjacency"},
	{GL_TRIANGLE_STRIP_ADJACENCY, "triangles_adjacency"}
};

const char* layout;

void
piglit_init(int argc, char **argv)
{
	GLuint vs = 0, gs = 0, fs = 0;
	char* gstext = NULL;
	GLuint vertIndex;

	/* Parse params */
	if (argc != 2) {
		printf("%s failed\n", argv[0]);
		piglit_report_result(PIGLIT_FAIL);
	}

	layout = argv[1];
	if (layout == NULL) {
		printf("%s failed\n", argv[0]);
		piglit_report_result(PIGLIT_FAIL);
	}

	prog = glCreateProgram();
	vs = piglit_compile_shader_text(GL_VERTEX_SHADER, vstext);
	asprintf(&gstext, gstemplate, layout);
	gs = piglit_compile_shader_text(GL_GEOMETRY_SHADER, gstext);
	fs = piglit_compile_shader_text(GL_FRAGMENT_SHADER, fstext);
	glAttachShader(prog, vs);
	glAttachShader(prog, gs);
	glAttachShader(prog, fs);
	free(gstext);

	glLinkProgram(prog);
	if(!piglit_link_check_status(prog)){
		glDeleteProgram(prog);
		piglit_report_result(PIGLIT_FAIL);
	}

	glUseProgram(prog);

	glGenBuffers(1, &vertBuff);
	glBindBuffer(GL_ARRAY_BUFFER, vertBuff);
	glBufferData(GL_ARRAY_BUFFER, vertSize, vertices, GL_STATIC_DRAW);

	glGenBuffers(1, &indexBuf);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuf);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indSize,
			indices, GL_STATIC_DRAW);

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	vertIndex = glGetAttribLocation(prog, "vertex");

	glBindBuffer(GL_ARRAY_BUFFER, vertBuff);
	glEnableVertexAttribArray(vertIndex);
	glVertexAttribPointer(vertIndex, 3, GL_FLOAT, GL_FALSE, 0, 0);
}

enum piglit_result
piglit_display(void)
{
	bool pass = true;
	int i = 0;

	glClearColor(0.2, 0.2, 0.2, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	glBindVertexArray(vao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuf);

	for(i = 0; i < ARRAY_SIZE(tests); i++) {
		glDrawElements(tests[i].prim_type, ARRAY_SIZE(indices),
				GL_UNSIGNED_INT, NULL);

		if(strcmp(layout, tests[i].layout_type) == 0)
			pass = piglit_check_gl_error(GL_NO_ERROR) && pass;
		else
			pass = piglit_check_gl_error(GL_INVALID_OPERATION) & pass;
	}

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}
