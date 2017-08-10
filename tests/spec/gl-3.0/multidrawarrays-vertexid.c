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

/** @file multidrawarrays-vertexid.c
 * Test using gl_VertexID in conjunction with glMultiDrawArrays
 *
 * The value of gl_VertexID observed in the shader should start with the value
 * of 'first' and increment from there.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 30;
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;
	config.khr_no_error_support = PIGLIT_NO_ERRORS;

PIGLIT_GL_TEST_CONFIG_END

static const float green[]   = { 0, 1, 0, 1 };
static const float blue[]    = { 0, 0, 1, 1 };
static const float gold[]    = { 1, 1, 0, 1 };
static const float magenta[] = { 1, 0, 1, 1 };

enum piglit_result
piglit_display(void)
{
	bool pass = true;
	static const GLint first[] = { 4, 8, 12, 16 };
	static const GLsizei count[ARRAY_SIZE(first)] = { 4, 4, 4, 4 };

	glViewport(0, 0, piglit_width, piglit_height);
	glClearColor(0.2, 0.2, 0.2, 0.2);
	glClear(GL_COLOR_BUFFER_BIT);

	glMultiDrawArrays(GL_TRIANGLE_FAN,
			  first,
			  count,
			  ARRAY_SIZE(count));

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
	static const GLfloat verts[] = {
		/* These vertices should never be accessed due to the way
		 * glMultiDrawArrays is called.
		 */
		-1.0, -1.0,
		 1.0, -1.0,
		 1.0,  1.0,
		-1.0,  1.0,

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
		"#version 130\n"
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

		"#version 130\n"
		"in vec3 c;\n"
		"\n"
		"void main() {\n"
		"	gl_FragColor = vec4(c, 1);\n"
		"}\n");

	GLuint vao;
	GLuint buf;

	glUseProgram(prog);

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glGenBuffers(1, &buf);
	glBindBuffer(GL_ARRAY_BUFFER, buf);
	glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts,
		     GL_STATIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void *) 0);
	glEnableVertexAttribArray(0);
}
