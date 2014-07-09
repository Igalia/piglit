/*
 * Copyright © 2010 Intel Corporation
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
 * \file sso-uniforms-01.c
 * Test setting uniform values with separate shader objects
 *
 * Creates two shader programs with a \c vec4 uniform named "color".  The
 * value of this uniform in each shader is set to a different value.  The
 * shaders are used, and the two instances of the "color" uniform is combined
 * to produce a result.
 *
 * \author Ian Romanick <ian.d.romanick@intel.com>
 */
#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

static const char vs_text[] =
	"uniform vec4 color;\n"
	"void main() { gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex; "
	"gl_TexCoord[0] = color; }";

static const char fs_text[] =
	"uniform vec4 color;\n"
	"void main() { gl_FragColor = gl_TexCoord[0] + color; }";

static GLuint prog[2];

enum piglit_result
piglit_display(void)
{
	static const float green[3] = { 0.0, 1.0, 0.0 };
	static const float blue[3]  = { 0.0, 0.0, 1.0 };
	enum piglit_result result = PIGLIT_PASS;

	glClear(GL_COLOR_BUFFER_BIT);
	glColor3fv(blue);

	/* Bind the separately linked vertex shader and the separately linked
	 * fragment shader using the new interfaces.  This should produce a
	 * green box.
	 */
	glUseShaderProgramEXT(GL_VERTEX_SHADER, prog[0]);
	glUseShaderProgramEXT(GL_FRAGMENT_SHADER, prog[1]);
	piglit_draw_rect(10, 10, 10, 10);
	if (!piglit_probe_pixel_rgb(15, 15, green))
		result = PIGLIT_FAIL;

	if (!piglit_automatic)
		piglit_present_results();

	return result;
}

void
piglit_init(int argc, char **argv)
{
	GLint loc;

	piglit_require_gl_version(20);

	piglit_require_extension("GL_EXT_separate_shader_objects");

	glClearColor(0.3, 0.3, 0.3, 0.0);

	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);

	prog[0] = glCreateShaderProgramEXT(GL_VERTEX_SHADER, vs_text);
	prog[1] = glCreateShaderProgramEXT(GL_FRAGMENT_SHADER, fs_text);

	loc = glGetUniformLocation(prog[0], "color");
	if (loc < 0) {
		printf("Unable to get uniform location in separate vertex "
		       "shader");
		piglit_report_result(PIGLIT_FAIL);
	}

	glActiveProgramEXT(prog[0]);
	glUniform4f(loc, 0.5, 0.5, -0.5, 0.0);

	loc = glGetUniformLocation(prog[1], "color");
	if (loc < 0) {
		printf("Unable to get uniform location in separate fragment "
		       "shader");
		piglit_report_result(PIGLIT_FAIL);
	}

	glActiveProgramEXT(prog[1]);
	glUniform4f(loc, -0.5, 0.5, 0.5, 1.0);
}
