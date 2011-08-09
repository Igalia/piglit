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
 * \file clip-distance-not-sizeable-above-max.c
 *
 * From the GLSL 1.30 spec section 7.1 (Vertex Shader Special
 * Variables):
 *
 *   The gl_ClipDistance array is predeclared as unsized and must be
 *   sized by the shader either redeclaring it with a size or indexing
 *   it only with integral constant expressions. This needs to size
 *   the array to include all the clip planes that are enabled via the
 *   OpenGL API; if the size does not include all enabled planes,
 *   results are undefined. The size can be at most
 *   gl_MaxClipDistances. The number of varying components (see
 *   gl_MaxVaryingComponents) consumed by gl_ClipDistance will match
 *   the size of the array, no matter how many planes are enabled. The
 *   shader must also set all values in gl_ClipDistance that have been
 *   enabled via the OpenGL API, or results are undefined. Values
 *   written into gl_ClipDistance for planes that are not enabled have
 *   no effect.
 *
 * This test checks that the an error occurs when trying to set the
 * size of gl_ClipDistance larger than gl_MaxClipDistances.
 *
 * Note: we don't care about the specific error that is generated or
 * the precise circumstances under which it occurs--we just want to
 * make sure that gl_MaxClipDistances isn't too small.  So to provoke
 * the error into occurring, we also try to access the first
 * disallowed element of the array.
 */
#include "piglit-util.h"

int piglit_width = 100, piglit_height = 100;
int piglit_window_mode = GLUT_RGB | GLUT_DOUBLE;

static const char vert[] =
	"#version 130\n"
	"out float gl_ClipDistance[gl_MaxClipDistances + 1];\n"
	"void main()\n"
	"{\n"
	"  gl_Position = gl_Vertex;\n"
	"  gl_ClipDistance[gl_MaxClipDistances] = 1.0;\n"
	"}\n";

static const char frag[] =
	"#version 130\n"
	"void main()\n"
	"{\n"
	"  gl_FragColor = gl_Color;\n"
	"}\n";

enum piglit_result
piglit_display(void)
{
	return PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	const char *glsl_version_string;
	float glsl_version;
	GLint ok;
	GLuint prog;
	GLuint vs;
	GLuint fs;


	piglit_require_GLSL();

	glsl_version_string = (char *)
		glGetString(GL_SHADING_LANGUAGE_VERSION);
	glsl_version = (glsl_version_string == NULL)
		? 0.0 : strtod(glsl_version_string, NULL);
	if (glsl_version <= 1.299999) {
		printf("Test requires GLSL version >= 1.3.  "
		       "Actual version is %.1f.\n",
		       glsl_version);
		piglit_report_result(PIGLIT_SKIP);
	}

	vs = piglit_compile_shader_text(GL_VERTEX_SHADER, vert);
	fs = piglit_compile_shader_text(GL_FRAGMENT_SHADER, frag);
	prog = glCreateProgram();
	glAttachShader(prog, vs);
	glAttachShader(prog, fs);
	glLinkProgram(prog);
	glDeleteShader(vs);
	glDeleteShader(fs);

	ok = piglit_link_check_status_quiet(prog);
	if (ok) {
		fprintf(stderr,
			"Linking with a shader that accesses gl_ClipDistance "
			"beyond gl_MaxClipDistances succeeded when it should "
			"have failed.\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	piglit_report_result(PIGLIT_PASS);
}
