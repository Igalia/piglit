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
 * \file sso-user-varying-02.c
 * Test separate shader objects with user-defined varyings.
 *
 * \author Ian Romanick <ian.d.romanick@intel.com>
 */
#include "piglit-util.h"

int piglit_width = 30, piglit_height = 30;
int piglit_window_mode = GLUT_RGB | GLUT_DOUBLE;

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
	enum piglit_result result = PIGLIT_SUCCESS;

	glClear(GL_COLOR_BUFFER_BIT);

	glUseShaderProgramEXT(GL_VERTEX_SHADER, prog[0]);
	glUseShaderProgramEXT(GL_FRAGMENT_SHADER, prog[1]);
	piglit_draw_rect(10, 10, 10, 10);

	/* The vertex shader is passing green to the fragment shader in an
	 * illegal way.  The rendered result must not be green!
	 */
	if (piglit_probe_pixel_rgb(15, 15, green))
		result = PIGLIT_FAILURE;

	if (!piglit_automatic)
		glutSwapBuffers();

	return result;
}

void
piglit_init(int argc, char **argv)
{
	GLboolean try_to_render;

	if (!GLEW_VERSION_2_0) {
		printf("Requires OpenGL 2.0\n");
		piglit_report_result(PIGLIT_SKIP);
	}

	piglit_require_extension("GL_EXT_separate_shader_objects");

	glClearColor(0.3, 0.3, 0.3, 0.0);

	prog[0] = glCreateShaderProgramEXT(GL_VERTEX_SHADER, vs_text);
	prog[1] = glCreateShaderProgramEXT(GL_FRAGMENT_SHADER, fs_text);

	/* Don't try to render if either program failed to link.  The
	 * GL_EXT_separate_shader_obejcts spec is really vague about whether
	 * or not linking will fail here.
	 */
	printf("Checking link result for vertex shader...\n");
	try_to_render = piglit_link_check_status_quiet(prog[0]);

	printf("Checking link result for fragment shader...\n");
	try_to_render = piglit_link_check_status_quiet(prog[1])
		&& try_to_render;

	if (!try_to_render)
		piglit_report_result(PIGLIT_SUCCESS);

	printf("\"Probe at (.., ..)\" returning mismatched results is "
	       "expected and correct.\n");
}
