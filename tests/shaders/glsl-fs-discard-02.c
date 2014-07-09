/*
 * Copyright © 2009 Marek Olšák (maraeo@gmail.com)
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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 *
 * Authors:
 *    Marek Olšák <mareao@gmail.com>
 *    Eric Anholt <eric@anholt.net>
 *
 */

/** @file glsl-fs-discard-02.c
 *
 * Tests that discarding fragments doesn't let early depth writes through.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_DEPTH;

PIGLIT_GL_TEST_CONFIG_END

static char vs_code[] =
	"varying vec4 color;"
	"\n"
	"void main()\n"
	"{\n"
	"	gl_Position = gl_Vertex;\n"
	"	if (gl_Vertex.z > 0.75)\n"
	"		color = vec4(1.0, 0.0, 0.0, gl_Vertex.z);\n"
	"	else if (gl_Vertex.z > 0.25)\n"
	"		color = vec4(0.0, 1.0, 0.0, gl_Vertex.z);\n"
	"	else\n"
	"		color = vec4(0.0, 0.0, 1.0, gl_Vertex.z);\n"
	"}\n";

static char fs_code[] =
	"varying vec4 color;\n"
	"\n"
	"void main()\n"
	"{\n"
	"	if (color.w < 0.25)\n"
	"		discard;"
	"    gl_FragColor = vec4(color.xyz, 0.0);\n"
	"}\n";

static GLuint setup_shaders()
{
	GLuint vs, fs, prog;

	vs = piglit_compile_shader_text(GL_VERTEX_SHADER, vs_code);
	fs = piglit_compile_shader_text(GL_FRAGMENT_SHADER, fs_code);
	prog = piglit_link_simple_program(vs, fs);

	glUseProgram(prog);
	return prog;
}

static GLboolean test()
{
	GLint prog;
	GLboolean pass = GL_TRUE;
	float green[4] = {0, 1, 0, 0};

	prog = setup_shaders();

	glClear(GL_DEPTH_BUFFER_BIT);

	glDepthFunc(GL_LEQUAL);
	glEnable(GL_DEPTH_TEST);

	piglit_draw_rect_z(1.0, -1, -1, 2, 2); // red
	piglit_draw_rect_z(0.0, -1, -1, 2, 2); // discard
	piglit_draw_rect_z(0.5, -1, -1, 2, 2); // green

	assert(glGetError() == 0);

	pass = pass && piglit_probe_rect_rgba(0, 0, piglit_width, piglit_height, green);

	glDeleteProgram(prog);

	return pass;
}

enum piglit_result piglit_display(void)
{
	GLboolean pass;

	pass = test();

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void piglit_init(int argc, char **argv)
{
	piglit_require_vertex_shader();
	piglit_require_fragment_shader();
}

