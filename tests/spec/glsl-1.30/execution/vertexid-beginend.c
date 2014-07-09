/*
 * Copyright © 2011 Intel Corporation
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
 * \file vertexid-beginend.c
 *
 * Test that gl_VertexID has the correct values.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;
	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

static const char vs_text[] =
	"#version 130\n"
	"\n"
	"/* This is floating point so we can use immediate mode */\n"
	"out vec4 color;\n"
	"\n"
	"void main()\n"
	"{\n"
	"  gl_Position = ftransform();\n"
	"  color = vec4(equal(vec4(gl_VertexID), gl_Color));\n"
	"}\n";

static const char fs_text[] =
	"#version 130\n"
	"\n"
	"in vec4 color;\n"
	"\n"
	"void main()\n"
	"{\n"
	"  gl_FragColor = color;\n"
	"}\n";

void
piglit_init(int argc, char **argv)
{
	GLuint prog;

	piglit_require_GLSL_version(130);

	prog = piglit_build_simple_program(vs_text, fs_text);

	glUseProgram(prog);
}

enum piglit_result
piglit_display()
{
	bool pass;
	float green[] = {0, 1, 0, 0};

	glBegin(GL_TRIANGLE_FAN);
		glColor4f(0.5, 0.0, 0.5, 0.5);
		glVertex2f(-1, -1);

		glColor4f(0.5, 1.0, 0.5, 0.5);
		glVertex2f(1, -1);

		glColor4f(0.5, 2.0, 0.5, 0.5);
		glVertex2f(1, 1);

		glColor4f(0.5, 3.0, 0.5, 0.5);
		glVertex2f(-1, 1);
	glEnd();

	pass = piglit_probe_rect_rgba(0, 0, piglit_width, piglit_height,
				      green);

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}
