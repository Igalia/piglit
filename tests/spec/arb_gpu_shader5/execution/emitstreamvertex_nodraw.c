/*
 * Copyright (c) 2014 Intel Corporation
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
 * @file emitstreamvertex_no_draw.c
 *
 * Test that a vertex emitted in stream 1 is not processed by fragment
 * shader.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 32;
	config.supports_gl_core_version = 32;
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGB;
	config.window_width = 100;
	config.window_height = 100;

PIGLIT_GL_TEST_CONFIG_END

GLuint vertexbuffer;
GLint program;

static const GLfloat g_vertex_buffer_data[] = {
	-0.5f, -0.5f, 0.0f,
	0.5f, -0.5f, 0.0f,
	0.0f,  0.50f, 0.0f,
};

enum piglit_result
piglit_display(void)
{
	int pass;
	float c[3] = {1.0, 0.0 , 0.0};
	float c_clear[3] = {0.0, 0.0 , 0.0};

	glUseProgram(program);

	/*
	 * Workaround: if define glPointSize == 1, piglit_probe_pixel_rgb()
	 * will fail unless the window is resized.
	 */
	glPointSize(2);

	glViewport(0, 0, piglit_width, piglit_height);
	/* Clear the back buffer to black */
	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glVertexAttribPointer(
			0,
			3,
			GL_FLOAT,
			GL_FALSE,
			0,
			(void*)0);

	glDrawArrays(GL_POINTS, 0, 3);
	glDisableVertexAttribArray(0);

	/* Probe that the point in stream=1 is not drawn. */
	pass = piglit_probe_pixel_rgb(piglit_width/2, piglit_height/2, c_clear);
	/* Probe that the rest of points are actually drawn. */
	pass &= piglit_probe_pixel_rgb(piglit_width*1/4, piglit_height/4, c);
	pass &= piglit_probe_pixel_rgb(piglit_width*3/4, piglit_height*1/4, c);
	pass &= piglit_probe_pixel_rgb(piglit_width/2, piglit_height*3/4, c);
	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

static const char *vs_source =
	"#version 330\n"
	"\n"
	"layout(location=0) in vec3 inVertexPosition;\n"
	"\n"
	"void main()\n"
	"{\n"
	"   gl_Position = vec4(inVertexPosition, 1);\n"
	"}\n";

static const char *gs_source =
	"#version 330\n"
	"#extension GL_ARB_gpu_shader5: enable\n"
	"\n"
	"layout(points) in;\n"
	"layout(points, stream=0, max_vertices=2) out;\n"
	"void main()\n"
	"{\n"
	"    gl_Position = gl_in[0].gl_Position;\n"
	"    EmitVertex();\n"
	"    EndPrimitive();\n"
	"\n"
	"    gl_Position = vec4(0.0, 0.0, 0.0, 1.0);\n"
	"    EmitStreamVertex(1);\n"
	"    EndStreamPrimitive(1);\n"
	"}\n";

static const char *fs_source =
	"#version 330\n"
	"out vec3 color;\n"
	"void main()\n"
	"{\n"
	"	    color = vec3(1.0, 0.0, 0.0);\n"
	"}\n";

void
piglit_init(int argc, char **argv)
{
	GLint max_streams;
	GLint vs, fs, gs;
	GLuint VertexArrayID;

	piglit_require_GLSL_version(150);
	piglit_require_extension("GL_ARB_gpu_shader5");

	glGetIntegerv(GL_MAX_VERTEX_STREAMS, &max_streams);

	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	vs = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vs, 1, (const GLchar **) &vs_source, NULL);
	glCompileShader(vs);
	vs = piglit_compile_shader_text(GL_VERTEX_SHADER, vs_source);
	gs = piglit_compile_shader_text(GL_GEOMETRY_SHADER, gs_source);
	fs = piglit_compile_shader_text(GL_FRAGMENT_SHADER, fs_source);

	program = glCreateProgram();
	glAttachShader(program, vs);
	glAttachShader(program, gs);
	glAttachShader(program, fs);
	glLinkProgram(program);

	if (!piglit_link_check_status(program)) {
		piglit_report_result(PIGLIT_FAIL);
		return;
	}

	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	glGenBuffers(1, &vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data),
		     g_vertex_buffer_data, GL_STATIC_DRAW);
}
