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
 * Verify that when GL_COORD_REPLACE is set, fragment shader texture
 * coordinates (via the gl_TexCoord built-ins) don't get eliminated.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 21;

	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

static const char *vstext =
	"#version 130\n"
	"in vec3 vertex;\n"
	"void main() {\n"
	"	gl_Position = vec4(vertex, 1.);\n"
	"	gl_PointSize = 16;\n"
	"}\n";

static const char *fstext =
	"#version 130\n"
	"void main() {\n"
	"	gl_FragColor = gl_TexCoord[0];\n"
	"}\n";

static GLuint vao;
static GLuint vertBuff;
static GLuint indexBuf;

static GLfloat vertices[] = {
	0.0, 0.0, 0.0
};
static GLsizei vertSize = sizeof(vertices);

static GLuint indices[] = {
	0
};
static GLsizei indSize = sizeof(indices);

static GLuint prog;

void
piglit_init(int argc, char **argv)
{
	GLuint vertIndex;
	piglit_require_GLSL_version(130);

	prog = piglit_build_simple_program(vstext, fstext);

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

	glEnableVertexAttribArray(vertIndex);
	glVertexAttribPointer(vertIndex, 3, GL_FLOAT, GL_FALSE, 0, 0);
}

enum piglit_result
piglit_display(void)
{
	bool pass = true;
	int x, y;
	int ptSize = 16;

	glClearColor(.4, .4, .4, 1.);
	glClear(GL_COLOR_BUFFER_BIT);

	glBindVertexArray(vao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBuf);

	glEnable(GL_PROGRAM_POINT_SIZE);
	glEnable(GL_POINT_SPRITE);
	glTexEnvi(GL_POINT_SPRITE, GL_COORD_REPLACE, GL_TRUE);

	glDrawElements(GL_POINTS, ARRAY_SIZE(indices),
			GL_UNSIGNED_INT, NULL);

	for (y = 0; y < ptSize; y++) {
		for (x = 0; x < ptSize; x++) {
			float test[] = {(2 * x + 1) / (float)(ptSize * 2),
					1 - ((2 * y + 1) / (float)(ptSize * 2)),
					0};
			pass = piglit_probe_pixel_rgb(
					piglit_width/2 - ptSize/2 + x,
					piglit_height/2 - ptSize/2 + y,
					test)
				&& pass;
		}
	}

	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}
