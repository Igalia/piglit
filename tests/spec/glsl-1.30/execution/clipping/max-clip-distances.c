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
 * \file max-clip-distances.c
 *
 * From the GLSL 1.30 spec, section 7.4 (Built-In Constants):
 *
 *   "The following built-in constants are provided to vertex and
 *   fragment shaders. The actual values used are implementation
 *   dependent, but must be at least the value shown.
 *
 *   ...
 *
 *   const int gl_MaxClipDistances = 8;
 *
 *   ...
 *
 *   const int gl_MaxClipPlanes = 8; // deprecated"
 *
 * And from the GL 3.0 spec, section N.3 (Changed Tokens):
 *
 *   "New token names are introduced to be used in place of old,
 *   inconsistent names.  However, the old token names continue to be
 *   supported, for backwards compatibility with code written for
 *   previous versions of OpenGL.
 *
 *   ...
 *
 *   New Token Name       Old Token Name
 *   ...
 *   MAX_CLIP_DISTANCES   MAX_CLIP_PLANES
 *
 * This test verifies that glGetIntegerv() returns the same result for
 * the tokens MAX_CLIP_DISTANCES and MAX_CLIP_PLANES, that this value
 * matches the value of gl_MaxClipDistances and gl_MaxClipPlanes
 * defined in the vertex and fragment shaders, and that this value is
 * at least 8.
 */
#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

static const char vert[] =
	"#version 130\n"
	"uniform int expected_value;\n"
	"uniform bool test_distances;\n"
	"uniform bool test_in_vs;\n"
	"void main()\n"
	"{\n"
	"  gl_Position = gl_Vertex;\n"
	"  if (test_in_vs) {\n"
	"    int value = test_distances ? gl_MaxClipDistances\n"
	"                               : gl_MaxClipPlanes;\n"
        "    gl_FrontColor = (value == expected_value)\n"
	"                  ? vec4(0.0, 1.0, 0.0, 1.0)\n"
	"                  : vec4(1.0, 0.0, 0.0, 1.0);\n"
	"  }\n"
	"}\n";

static const char frag[] =
	"#version 130\n"
	"uniform int expected_value;\n"
	"uniform bool test_distances;\n"
	"uniform bool test_in_vs;\n"
	"void main()\n"
	"{\n"
	"  if (test_in_vs) {\n"
	"    gl_FragColor = gl_Color;\n"
	"  } else {\n"
	"    int value = test_distances ? gl_MaxClipDistances\n"
	"                               : gl_MaxClipPlanes;\n"
        "    gl_FragColor = (value == expected_value)\n"
	"                 ? vec4(0.0, 1.0, 0.0, 1.0)\n"
	"                 : vec4(1.0, 0.0, 0.0, 1.0);\n"
	"  }\n"
	"}\n";

GLuint prog;

enum piglit_result
piglit_display(void)
{
	GLint max_clip_planes, max_clip_distances, expected_value;
	GLint test_distances, test_in_vs;
	float green[] = { 0.0, 1.0, 0.0, 1.0 };
	GLint loc;

	enum piglit_result result = PIGLIT_PASS;
	glGetIntegerv(GL_MAX_CLIP_PLANES, &max_clip_planes);
	printf("GL_MAX_CLIP_PLANES = %d\n", max_clip_planes);
	glGetIntegerv(GL_MAX_CLIP_DISTANCES, &max_clip_distances);
	printf("GL_MAX_CLIP_DISTANCES = %d\n", max_clip_distances);
	if (max_clip_planes != max_clip_distances) {
		printf("GL_MAX_CLIP_PLANES != GL_MAX_CLIP_DISTANCES\n");
		piglit_report_result(PIGLIT_FAIL);
	}
	if (max_clip_distances < 8) {
		printf("GL_MAX_CLIP_DISTANCES < 8\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	expected_value = max_clip_distances;
	loc = glGetUniformLocation(prog, "expected_value");
	glUniform1i(loc, expected_value);

	for (test_distances = 0; test_distances <= 1; ++test_distances) {
		loc = glGetUniformLocation(prog, "test_distances");
		glUniform1i(loc, test_distances);
		for (test_in_vs = 0; test_in_vs <= 1; ++test_in_vs) {
			bool pass;
			loc = glGetUniformLocation(prog, "test_in_vs");
			glUniform1i(loc, test_in_vs);
			piglit_draw_rect(-1, -1, 2, 2);
			pass = piglit_probe_rect_rgba(0, 0, piglit_width,
						      piglit_height, green);
			printf("Checking that gl_MaxClip%s == %d in %s: %s\n",
			       test_distances ? "Distances" : "Planes",
			       expected_value,
			       test_in_vs ? "VS" : "FS",
			       pass ? "pass" : "fail");
			if (!pass) {
				result = PIGLIT_FAIL;
			}
		}
	}

	return result;
}

void
piglit_init(int argc, char **argv)
{
	piglit_require_GLSL();
	piglit_require_GLSL_version(130);
	prog = piglit_build_simple_program(vert, frag);
	glUseProgram(prog);
}
