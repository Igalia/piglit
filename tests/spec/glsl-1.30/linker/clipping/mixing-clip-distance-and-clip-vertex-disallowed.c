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
 * \file mixing-clip-distance-and-clip-vertex-disallowed.c
 *
 * From the GLSL 1.30 spec, section 7.1 (Vertex Shader Special
 * Variables):
 *
 *   It is an error for a shader to statically write both
 *   gl_ClipVertex and gl_ClipDistance.
 *
 * This test verifies that an error is generated if the shader
 * contains writes to both variables, even if those writes would never
 * both occur in the same render.
 */
#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

static const char vert[] =
	"#version 130\n"
	"uniform bool use_ClipDistance;\n"
	"void main()\n"
	"{\n"
	"  gl_Position = vec4(0.0);\n"
	"  if (use_ClipDistance) {\n"
	"    gl_ClipDistance[0] = 1.0;\n"
	"  } else {\n"
	"    gl_ClipVertex = vec4(0.0);\n"
	"  }\n"
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

	prog = piglit_build_simple_program_unlinked(vert, frag);
	glLinkProgram(prog);

	ok = piglit_link_check_status_quiet(prog);
	if (ok) {
		fprintf(stderr,
			"Linking with a shader that accesses both "
			"gl_ClipDistance and gl_ClipVertex succeeded when it "
			"should have failed.\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	piglit_report_result(PIGLIT_PASS);
}
