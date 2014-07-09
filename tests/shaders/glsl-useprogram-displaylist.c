/*
 * Copyright © 2009 Intel Corporation
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
 *
 * Authors:
 *    Ben Holmes <shranzel@hotmail.com>
 */

/*
 * tests for bug fdo 23746. The bug prevents glUseProgram from working when
 * called within a display list.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

static GLint progr;
static GLint progg;
static GLuint list;

static const char vertShaderText[] =
	"void main() { gl_Position = gl_Vertex; }";

static const char fragShaderTextRed[] =
	"void main() { gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0); }";

static const char fragShaderTextGreen[] =
	"void main() { gl_FragColor = vec4(0.0, 1.0, 0.0, 1.0); }";

void
piglit_init(int argc, char **argv)
{
	GLint fsr;
	GLint fsg;
	GLint vs;

	piglit_require_gl_version(20);

	glClearColor(0.2, 0.2, 0.2, 1.0);

	vs = piglit_compile_shader_text(GL_VERTEX_SHADER, vertShaderText);
	fsr = piglit_compile_shader_text(GL_FRAGMENT_SHADER, fragShaderTextRed);
	fsg =
	  piglit_compile_shader_text(GL_FRAGMENT_SHADER, fragShaderTextGreen);

	progr = piglit_link_simple_program(vs, fsr);
	progg = piglit_link_simple_program(vs, fsg);

	list = glGenLists(1);
	glNewList(list, GL_COMPILE);
		glUseProgram(progg);
	glEndList();
}

enum piglit_result
piglit_display(void)
{
	static const GLfloat green[3] = {0.0, 1.0, 0.0};
	GLboolean pass = GL_TRUE;

	glClear(GL_COLOR_BUFFER_BIT);

	glUseProgram(progr);
	glCallList(list);

	piglit_draw_rect(-0.5, -0.5, 1.0, 1.0);
	pass = piglit_probe_pixel_rgb(piglit_width / 2, piglit_height / 2,
				      green);

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}
