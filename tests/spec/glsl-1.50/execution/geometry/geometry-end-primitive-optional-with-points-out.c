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
 * Test that when the GS output layout is "points" EndPrimitive() is optional.
 *
 * From the GLSL 1.50 spec, section 8.10 (Geometry Shader Functions):
 *
 * "If the output layout is declared to be “points”, calling EndPrimitive()
 *  is optional."
 */

#include "piglit-util-gl-common.h"

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

static const char *gstext =
	"#version 150\n"
	"layout(triangles) in;\n"
	"layout(points, max_vertices = 3) out;\n"
	"in vec3 pos[];\n"
	"void main() {\n"
	"	for(int i = 0; i < 3; i++) {\n"
	"		gl_Position = vec4(pos[i], 1.);\n"
	"		EmitVertex();\n"
	"	}\n"
	"}\n";

static const char *fstext =
	"#version 150\n"
	"out vec4 color;\n"
	"void main() {\n"
	"	color = vec4(0., 1., 0., 1.);\n"
	"}\n";

static GLuint vao;
static GLuint vertBuff;
static GLuint indexBuf;

static GLfloat vertices[] = {
	-0.9, 0.9, 0.0,
	 0.9, 0.9, 0.0,
	 0.9,-0.9, 0.0
};
static GLsizei vertSize = sizeof(vertices);

static GLuint indices[] = {
	0, 1, 2
};
static GLsizei indSize = sizeof(indices);

static GLuint prog;

void
piglit_init(int argc, char **argv)
{
	GLuint vs = 0, gs = 0, fs = 0;
	GLuint vertIndex;

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
	float green[] = {0, 1, 0};

	glClearColor(0, 0, 0, 1.);
	glClear(GL_COLOR_BUFFER_BIT);

	glBindVertexArray(vao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuf);

	glDrawElements(GL_TRIANGLES, ARRAY_SIZE(indices),
			GL_UNSIGNED_INT, NULL);

	pass = piglit_probe_pixel_rgb(.05 * piglit_width,
				      .95 * piglit_height,
				      green) && pass;
	pass = piglit_probe_pixel_rgb(.95 * piglit_width,
				      .95 * piglit_height,
				      green) && pass;
	pass = piglit_probe_pixel_rgb(.95 * piglit_width,
				      .05 * piglit_height,
				      green) && pass;

	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}
