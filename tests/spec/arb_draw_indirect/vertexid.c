/* Copyright © 2014 Intel Corporation
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

/** @file vertexid.c
 * Test indirect renderinger with gl_VertexID
 *
 * When rendering with glDrawArraysIndirect, the value of gl_VertexID observed
 * in the shader should start with the value of 'first' and increment from
 * there.
 *
 * When rendering with glDrawElementsIndirect, the value of gl_VertexID
 * observed in the shader should be the value retrieved from the index buffer
 * plus the value of basevertex.
 *
 * Run the program with no parameters to use glDrawArraysIndirect, or run the
 * program with "elements" parameter to use glDrawElementsIndirect.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_core_version = 31;
	config.supports_gl_compat_version = 31;

	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;

PIGLIT_GL_TEST_CONFIG_END

static const float green[]   = { 0, 1, 0, 1 };
static const float blue[]    = { 0, 0, 1, 1 };
static const float gold[]    = { 1, 1, 0, 1 };
static const float magenta[] = { 1, 0, 1, 1 };

typedef struct {
	GLuint count;
	GLuint primCount;
	GLuint first;
	GLuint reservedMustBeZero;
} DrawArraysIndirectCommand;

typedef struct {
	GLuint count;
	GLuint primCount;
	GLuint firstIndex;
	GLint  baseVertex;
	GLuint reservedMustBeZero;
} DrawElementsIndirectCommand;

static const DrawArraysIndirectCommand arrays_commands[] = {
	{ 4, 1, 4,  0 },
	{ 4, 1, 8,  0 },
	{ 4, 1, 12, 0 },
	{ 4, 1, 16, 0 },
};

static const DrawElementsIndirectCommand elements_commands[ARRAY_SIZE(arrays_commands)] = {
	{ 4, 1, 0, 4,  0 },
	{ 4, 1, 0, 8,  0 },
	{ 4, 1, 0, 12, 0 },
	{ 4, 1, 0, 16, 0 },
};

static bool use_arrays = true;

enum piglit_result
piglit_display(void)
{
	bool pass = true;
	unsigned i;

	glViewport(0, 0, piglit_width, piglit_height);
	glClearColor(0.2, 0.2, 0.2, 0.2);
	glClear(GL_COLOR_BUFFER_BIT);

	for (i = 0; i < ARRAY_SIZE(arrays_commands); i++) {
		if (use_arrays) {
			glDrawArraysIndirect(GL_TRIANGLE_FAN,
					     (void *)(sizeof(arrays_commands[0]) * i));
		} else {
			glDrawElementsIndirect(GL_TRIANGLE_FAN,
					       GL_UNSIGNED_INT,
					       (void *)(sizeof(elements_commands[0]) * i));
		}
	}

	pass = piglit_probe_rect_rgba(0, 0,
				      piglit_width / 2, piglit_height /2,
				      green)
		&& pass;
	pass = piglit_probe_rect_rgba(piglit_width / 2, 0,
				      piglit_width / 2, piglit_height / 2,
				      blue)
		&& pass;
	pass = piglit_probe_rect_rgba(0, piglit_height /2,
				      piglit_width / 2, piglit_height / 2,
				      gold)
		&& pass;
	pass = piglit_probe_rect_rgba(piglit_width / 2, piglit_height /2,
				      piglit_width / 2, piglit_height / 2,
				      magenta)
		&& pass;
	
	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	static const GLuint indices[] = { 0, 1, 2, 3 };
	static const GLfloat verts[] = {
		/* These vertices should never be accessed due to the way
		 * glDrawArraysIndirect and glDrawElementsIndirect are called.
		 */
		-0.5, -0.5,
		 0.5, -0.5,
		 0.5,  0.5,
		-0.5,  0.5,

		-1.0, -1.0,
		 0.0, -1.0,
		 0.0,  0.0,
		-1.0,  0.0,

		 0.0, -1.0,
		 1.0, -1.0,
		 1.0,  0.0,
		 0.0,  0.0,

		-1.0,  0.0,
		 0.0,  0.0,
		 0.0,  1.0,
		-1.0,  1.0,

		 0.0,  0.0,
		 1.0,  0.0,
		 1.0,  1.0,
		 0.0,  1.0,
	};

	GLuint prog = piglit_build_simple_program(
		"#version 140\n"
		"\n"
		"in vec4 piglit_vertex;\n"
		"out vec3 c;\n"
		"\n"
		"const vec3 colors[] = vec3[](\n"
		"	vec3(1, 0, 0),\n"
		"	vec3(1, 0, 0),\n"
		"	vec3(1, 0, 0),\n"
		"	vec3(1, 0, 0),\n"
		"\n"
		"	vec3(0, 1, 0),\n"
		"	vec3(0, 1, 0),\n"
		"	vec3(0, 1, 0),\n"
		"	vec3(0, 1, 0),\n"
		"\n"
		"	vec3(0, 0, 1),\n"
		"	vec3(0, 0, 1),\n"
		"	vec3(0, 0, 1),\n"
		"	vec3(0, 0, 1),\n"
		"\n"
		"	vec3(1, 1, 0),\n"
		"	vec3(1, 1, 0),\n"
		"	vec3(1, 1, 0),\n"
		"	vec3(1, 1, 0),\n"
		"\n"
		"	vec3(1, 0, 1),\n"
		"	vec3(1, 0, 1),\n"
		"	vec3(1, 0, 1),\n"
		"	vec3(1, 0, 1)\n"
		");\n"
		"void main() {\n"
		"       c = colors[gl_VertexID];\n"
		"	gl_Position = piglit_vertex;\n"
		"}\n",

		"#version 140\n"
		"in vec3 c;\n"
		"out vec4 fragcolor;\n"
		"\n"
		"void main() {\n"
		"	fragcolor = vec4(c, 1);\n"
		"}\n");

	GLuint vao;
	GLuint buf[3];

	piglit_require_extension("GL_ARB_draw_indirect");

	use_arrays = (argc < 2 || strcmp(argv[1], "elements") != 0);
	printf("Using glDraw%sIndirect...\n",
	       use_arrays ? "Arrays" : "Elements");

	glUseProgram(prog);

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glGenBuffers(ARRAY_SIZE(buf), buf);
	glBindBuffer(GL_ARRAY_BUFFER, buf[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts,
		     GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buf[1]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices,
		     GL_STATIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void *) 0);
	glEnableVertexAttribArray(0);

	if (use_arrays) {
		glBindBuffer(GL_DRAW_INDIRECT_BUFFER, buf[2]);
		glBufferData(GL_DRAW_INDIRECT_BUFFER,
			     sizeof(arrays_commands),
			     arrays_commands,
			     GL_STATIC_DRAW);
	} else {
		glBindBuffer(GL_DRAW_INDIRECT_BUFFER, buf[2]);
		glBufferData(GL_DRAW_INDIRECT_BUFFER,
			     sizeof(elements_commands),
			     elements_commands,
			     GL_STATIC_DRAW);
	}
}
