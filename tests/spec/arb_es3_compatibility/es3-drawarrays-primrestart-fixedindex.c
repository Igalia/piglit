/*
 * Copyright © 2014 Advanced Micro Devices, Inc.
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
 * Test if primitive restart is disabled for glDrawArrays while both
 * PRIMITIVE_RESTART and PRIMITIVE_RESTART_FIXED_INDEX are enabled.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN
	config.supports_gl_compat_version = 33;
	config.supports_gl_core_version = 33;
	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;
PIGLIT_GL_TEST_CONFIG_END

static const GLchar *vstext =
	"#version 330\n"
	"attribute vec2 piglit_vertex;\n"
	"attribute int piglit_texcoord;\n"
	"out vec4 color;\n"
	"void main()\n"
	"{\n"
	"	gl_Position = vec4(piglit_vertex, 0.0, 1.0);\n"
	"	color = vec4(0.0, 1.0, 0.0, 1.0);\n"
	"} \n";

static const char *fstext =
	"#version 330\n"
	"in vec4 color;\n"
	"void main() {\n"
	"	gl_FragColor = color;\n"
	"}\n";

static GLint prog;

enum piglit_result
piglit_display(void)
{
	static const float green[] = {0, 1, 0, 1};
	enum piglit_result result;

	glClear(GL_COLOR_BUFFER_BIT);
	glDrawArrays(GL_TRIANGLES, 0, 7);

	result = piglit_probe_rect_rgba(0, 0, piglit_width, piglit_height,
					green)
		? PIGLIT_PASS : PIGLIT_FAIL;

	piglit_present_results();
	return result;
}

void
piglit_init(int argc, char **argv)
{
	static const float pos[] = {
		-1, -1,
		-1,  1,
		 1, -1,
		 1,  1,
		 1, -1,
		-1,  1,
		-1,  1 /* should be dropped */
	};
	GLuint vao, buf;

	piglit_require_gl_version(33);
	piglit_require_extension("GL_ARB_ES3_compatibility");
	prog = piglit_build_simple_program(vstext, fstext);
	glUseProgram(prog);

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glGenBuffers(1, &buf);
	glBindBuffer(GL_ARRAY_BUFFER, buf);
	glBufferData(GL_ARRAY_BUFFER, sizeof(pos), pos, GL_STATIC_DRAW);

	glEnableVertexAttribArray(PIGLIT_ATTRIB_POS);
	glVertexAttribPointer(PIGLIT_ATTRIB_POS, 2, GL_FLOAT, 0, 0, NULL);

	glEnable(GL_PRIMITIVE_RESTART_FIXED_INDEX);
	glEnable(GL_PRIMITIVE_RESTART);
	glPrimitiveRestartIndex(3);
}
