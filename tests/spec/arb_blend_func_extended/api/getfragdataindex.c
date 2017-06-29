/* Copyright © 2011 Intel Corporation
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
 * \file getfragdataindex.c
 *
 * \author Dave Airlie
 * heavily inspired by getfragdatalocation.c from Ian Romanick
 */
#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

#ifdef PIGLIT_USE_OPENGL
	config.supports_gl_compat_version = 10;
#else // PIGLIT_USE_OPENGL_ES3
	config.supports_gl_es_version = 30;
#endif
	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;
	config.khr_no_error_support = PIGLIT_NO_ERRORS;

PIGLIT_GL_TEST_CONFIG_END

#ifdef PIGLIT_USE_OPENGL
static const char *vs_text =
	"#version 130\n"
	"in vec4 vertex;\n"
	"void main() { gl_Position = vertex; }\n"
	;

static const char *fs_text =
	"#version 130\n"
	"out vec4 v;\n"
	"out vec4 a[2];\n"
	"void main() {\n"
	"    v = vec4(0.0);\n"
	"    a[0] = vec4(1.0);\n"
	"    a[1] = vec4(2.0);\n"
	"}\n"
	;
#else // PIGLIT_USE_OPENGL_ES3
static const char *vs_text =
	"#version 300 es\n"
	"in vec4 vertex;\n"
	"void main() { gl_Position = vertex; }\n"
	;

static const char *fs_text =
	"#version 300 es\n"
	"#extension GL_EXT_blend_func_extended : enable\n"
	"out highp vec4 v;\n"
	"out highp vec4 a[2];\n"
	"void main() {\n"
	"    v = vec4(0.0);\n"
	"    a[0] = vec4(1.0);\n"
	"    a[1] = vec4(2.0);\n"
	"}\n"
	;
#endif

enum piglit_result
piglit_display(void)
{
	return PIGLIT_FAIL;
}

void piglit_init(int argc, char **argv)
{
	GLint max_draw_buffers, max_dual_source;
	GLuint prog;
	GLuint vs;
	GLuint fs;
	GLint idx;

#ifdef PIGLIT_USE_OPENGL
	piglit_require_gl_version(30);
	piglit_require_extension("GL_ARB_blend_func_extended");
#else // PIGLIT_USE_OPENGL_ES3
	piglit_require_extension("GL_EXT_blend_func_extended");
#endif

	/* This test needs some number of draw buffers, so make sure the
	 * implementation isn't broken.  This enables the test to generate a
	 * useful failure message.
	 */
	glGetIntegerv(GL_MAX_DRAW_BUFFERS, &max_draw_buffers);
	if (max_draw_buffers < 8) {
		fprintf(stderr,
			"OpenGL 3.0 requires GL_MAX_DRAW_BUFFERS >= 8.  "
			"Only got %d!\n",
			max_draw_buffers);
		piglit_report_result(PIGLIT_FAIL);
	}
	glGetIntegerv(GL_MAX_DUAL_SOURCE_DRAW_BUFFERS, &max_dual_source);
	if (max_dual_source < 1) {
		fprintf(stderr,
			"blend_func_extended requires GL_MAX_DUAL_SOURCE_DRAW_BUFFERS >= 1.  "
			"Only got %d!\n",
			max_dual_source);
		piglit_report_result(PIGLIT_FAIL);
	}

	prog = glCreateProgram();
	vs = piglit_compile_shader_text(GL_VERTEX_SHADER, vs_text);
	fs = piglit_compile_shader_text(GL_FRAGMENT_SHADER, fs_text);
	glAttachShader(prog, vs);
	glAttachShader(prog, fs);
	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	/* Page 237 (page 253 of the PDF) of the OpenGL 3.0 spec says:
	 *
	 *     "If program has not been successfully linked, the error INVALID
	 *     OPERATION is generated. If name is not a varying out variable,
	 *     or if an error occurs, -1 will be returned."
	 */
	if (!piglit_khr_no_error) {
		printf("Querying index before linking...\n");
#ifdef PIGLIT_USE_OPENGL
		idx = glGetFragDataIndex(prog, "v");
#else // PIGLIT_USE_OPENGLES3
		idx = glGetFragDataIndexEXT(prog, "v");
#endif
		if (!piglit_check_gl_error(GL_INVALID_OPERATION))
			piglit_report_result(PIGLIT_FAIL);

		if (idx != -1) {
			fprintf(stderr, "Expected index = -1, got %d\n", idx);
			piglit_report_result(PIGLIT_FAIL);
		}
	}

	glLinkProgram(prog);
	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	if (!piglit_link_check_status(prog)) {
		piglit_report_result(PIGLIT_FAIL);
	}

	printf("Querying index of nonexistent variable...\n");
#ifdef PIGLIT_USE_OPENGL
	idx = glGetFragDataIndex(prog, "waldo");
#else // PIGLIT_USE_OPENGLES3
	idx = glGetFragDataIndexEXT(prog, "waldo");
#endif
	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	if (idx != -1) {
		fprintf(stderr, "Expected index = -1, got %d\n", idx);
		piglit_report_result(PIGLIT_FAIL);
	}
	piglit_report_result(PIGLIT_PASS);
}
