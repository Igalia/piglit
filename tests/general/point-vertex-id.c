/*
 * Copyright © 2014 Intel Corporation
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

#include "piglit-util-gl.h"

/**
 * @file point-vertex-id.c
 *
 * Tests glPolygonMode(GL_POINT) used in combination with gl_VertexID.
 * See bug #84677
 */

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 20;

	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

static const char
vertex_shader[] =
	"#version 130\n"
	"\n"
	"uniform vec2 pos[6];\n"
	"\n"
	"void\n"
	"main()\n"
	"{\n"
	"        gl_Position = vec4(pos[gl_VertexID], 0.0, 1.0);\n"
	"        gl_FrontColor = vec4(1.0);\n"
	"}\n";

struct vertex {
	int x, y;
	GLubyte edge_flag;
};

static const struct vertex
vertices[] = {
	{ 10, 10, GL_TRUE },
	{ 20, 10, GL_TRUE },
	{ 10, 20, GL_TRUE },
	/* This triangle won't be drawn because none of the vertices
	 * are an edge */
	{ 30, 10, GL_FALSE },
	{ 40, 10, GL_FALSE },
	{ 30, 20, GL_FALSE },
};

enum piglit_result
piglit_display(void)
{
	bool pass = true;
	float black[4] = {0.0, 0.0, 0.0, 0.0};
	float white[4] = {1.0, 1.0, 1.0, 1.0};
	GLint pos_location;
	GLuint program;
	int i;

	program = piglit_build_simple_program(vertex_shader, NULL);

	glUseProgram(program);

	glClear(GL_COLOR_BUFFER_BIT);

	pos_location = glGetUniformLocation(program, "pos");

	for (i = 0; i < ARRAY_SIZE(vertices); i++) {
		glUniform2f(pos_location + i,
			    (vertices[i].x + 0.5f) * 2.0f /
			    piglit_width - 1.0f,
			    (vertices[i].y + 0.5f) * 2.0f /
			    piglit_height - 1.0f);
	}

	glEnableClientState(GL_EDGE_FLAG_ARRAY);
	glEdgeFlagPointer(sizeof (struct vertex),
			  &vertices[0].edge_flag);

	glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
	glDrawArrays(GL_TRIANGLES, 0, ARRAY_SIZE(vertices));

	/* Area below the dots */
	pass = pass && piglit_probe_rect_rgba(0, 0,
					      piglit_width,
					      vertices[0].y,
					      black);
	/* Left of the dots */
	pass = pass && piglit_probe_rect_rgba(0, vertices[0].y,
					      vertices[0].x,
					      vertices[1].y - vertices[0].y + 1,
					      black);
	/* In-between the dots */
	pass = pass && piglit_probe_rect_rgba(vertices[0].x + 1,
					      vertices[0].y,
					      vertices[1].x - vertices[0].x - 1,
					      vertices[1].y - vertices[0].y + 1,
					      black);
	/* Right of the dots */
	pass = pass && piglit_probe_rect_rgba(vertices[1].x + 1,
					      vertices[0].y,
					      piglit_width - vertices[1].x - 1,
					      vertices[1].y - vertices[0].y + 1,
					      black);
	/* Above the dots */
	pass = pass && piglit_probe_rect_rgba(0, vertices[2].y + 1,
					      piglit_width,
					      piglit_height - vertices[2].y - 1,
					      black);

	/* At the dots */
	for (i = 0; i < ARRAY_SIZE(vertices); i++) {
		if (vertices[i].edge_flag) {
			pass = pass && piglit_probe_pixel_rgba(vertices[i].x,
							       vertices[i].y,
							       white);
		}
	}

	glUseProgram(0);
	glDeleteProgram(program);

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	piglit_require_GLSL_version(130);
}
