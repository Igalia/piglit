/*
 * Copyright © 2018 Timothy Arceri
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
 * Test immediate mode can draw GL_PATCHES.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN
	config.supports_gl_compat_version = 32;
	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;
	config.khr_no_error_support = PIGLIT_NO_ERRORS;
PIGLIT_GL_TEST_CONFIG_END

unsigned int prog;

static const char *const vs_source =
"#version 150 compatibility\n"
"in vec4 piglit_vertex;\n"
"void main() { gl_Position = piglit_vertex; }\n";

static const char *const tcs_source =
"#version 150 compatibility\n"
"#extension GL_ARB_tessellation_shader: require\n"
"layout(vertices = 3) out;\n"
"out vec4 color[];\n"
"void main() {\n"
"	gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;\n"
"	gl_TessLevelOuter = float[4](1.0, 1.0, 1.0, 0.0);\n"
"	gl_TessLevelInner = float[2](0.0, 0.0);\n"
"	color[gl_InvocationID] = vec4(0, 1, 0, 1);\n"
"}\n";

static const char *const tes_source =
"#version 150 compatibility\n"
"#extension GL_ARB_tessellation_shader: require\n"
"layout(triangles) in;\n"
"in vec4 color[];\n"
"void main() { \n"
"	gl_Position = gl_in[0].gl_Position * gl_TessCoord[0]\n"
"	            + gl_in[1].gl_Position * gl_TessCoord[1]\n"
"	            + gl_in[2].gl_Position * gl_TessCoord[2];\n"
"\n"
"	gl_FrontColor = color[0] * gl_TessCoord[0]\n"
"	           + color[1] * gl_TessCoord[1]\n"
"	           + color[2] * gl_TessCoord[2];\n"
"}\n";

enum piglit_result
piglit_display(void)
{
	static const GLfloat green[4] = { 0, 1, 0, 1 };
	enum piglit_result result;

	glClearColor(0.1, 0.1, 0.1, 0.1);
	glClear(GL_COLOR_BUFFER_BIT);

	glBegin(GL_PATCHES);
	glVertex2f(-1.0, -1.0);
	glVertex2f(1.0, -1.0);
	glVertex2f(-1.0, 1.0);
	glVertex2f(-1.0, 1.0);
	glVertex2f(1.0, -1.0);
	glVertex2f(1.0, 1.0);
	glEnd();

	if (piglit_probe_pixel_rgb(piglit_width-1, piglit_height-1, green))
		result = PIGLIT_PASS;
	else
		result = PIGLIT_FAIL;

	if (!piglit_check_gl_error(GL_NO_ERROR))
		result = PIGLIT_FAIL;

	return result;
}

void
piglit_init(int argc, char **argv)
{
	piglit_require_extension("GL_ARB_tessellation_shader");

	prog = piglit_build_simple_program_multiple_shaders(
			GL_VERTEX_SHADER, vs_source,
			GL_TESS_CONTROL_SHADER, tcs_source,
			GL_TESS_EVALUATION_SHADER, tes_source,
			0);

	glUseProgram(prog);
}
