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
 * \file sso-user-varying-01.c
 * Test separate shader objects with user-defined varyings.
 *
 * \author Ian Romanick <ian.d.romanick@intel.com>
 */
#include "piglit-util.h"

PIGLIT_GL_TEST_MAIN(
    30 /*window_width*/,
    30 /*window_height*/,
    GLUT_RGB | GLUT_DOUBLE)

static const char vs_text[] =
	"varying vec4 color;\n"
	"\n"
	"void main()\n"
	"{\n"
	"  gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;\n"
	"  color = vec4(0.0, 1.0, 0.0, 1.0);\n"
	"}\n"
	;

static const char fs_text[] =
	"varying vec4 color;\n"
	"\n"
	"void main()\n"
	"{\n"
	"  gl_FragColor = color;\n"
	"}\n"
	;

GLuint prog[2];

enum piglit_result
piglit_display(void)
{
	static const float green[3] = { 0.0, 1.0, 0.0 };
	enum piglit_result result = PIGLIT_PASS;

	glClear(GL_COLOR_BUFFER_BIT);

	glUseShaderProgramEXT(GL_VERTEX_SHADER, prog[0]);
	glUseShaderProgramEXT(GL_FRAGMENT_SHADER, prog[1]);
	piglit_draw_rect(10, 10, 10, 10);

	/* The vertex shader is passing green to the fragment shader in an
	 * illegal way.  The rendered result must not be green!
	 */
	if (piglit_probe_pixel_rgb(15, 15, green))
		result = PIGLIT_FAIL;

	if (!piglit_automatic)
		glutSwapBuffers();

	return result;
}

void
piglit_init(int argc, char **argv)
{
	GLboolean try_to_render;
	GLuint vs;
	GLuint fs;

	if (piglit_get_gl_version() < 20) {
		printf("Requires OpenGL 2.0\n");
		piglit_report_result(PIGLIT_SKIP);
	}

	piglit_require_extension("GL_EXT_separate_shader_objects");

	glClearColor(0.3, 0.3, 0.3, 0.0);
	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);

	vs = piglit_compile_shader_text(GL_VERTEX_SHADER, vs_text);
	fs = piglit_compile_shader_text(GL_FRAGMENT_SHADER, fs_text);

	prog[0] = piglit_link_simple_program(vs, 0);
	prog[1] = piglit_link_simple_program(0,  fs);

	glDeleteShader(vs);
	glDeleteShader(fs);

	/* Don't try to render if either program failed to link, and linking
	 * had better succeed!
	 */
	try_to_render = piglit_link_check_status(prog[0]);
	try_to_render = piglit_link_check_status(prog[1])
		&& try_to_render;

	if (!try_to_render)
		piglit_report_result(PIGLIT_FAIL);

	printf("\"Probe at (.., ..)\" returning mismatched results is "
	       "expected and correct.\n");
}
