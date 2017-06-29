/* Copyright © 2015 Intel Corporation
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
 * \file output-location.c
 *
 * \author Tapani Pälli
 * heavily inspired by getfragdataindex.c from Dave Airlie
 */
#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

#ifdef PIGLIT_USE_OPENGL
	config.supports_gl_core_version = 31;
#else // PIGLIT_USE_OPENGLES3
	config.supports_gl_es_version = 30;
#endif
	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;
	config.khr_no_error_support = PIGLIT_NO_ERRORS;

PIGLIT_GL_TEST_CONFIG_END

#ifdef PIGLIT_USE_OPENGL
static const char *vs_text =
	"#version 150\n"
	"in vec4 vertex;\n"
	"void main() { gl_Position = vertex; }\n"
	;

static const char *fs_template =
	"#version 150\n"
	"#extension GL_ARB_explicit_attrib_location : require\n"
	"layout(location = 0, index = 0) out vec4 a;\n"
	"layout(location = %d, index = 1) out vec4 b;\n"
	"void main() {\n"
	"    a = vec4(0.0);\n"
	"    b = vec4(1.0);\n"
	"}\n"
	;
#else // PIGLIT_USE_OPENGLES3
static const char *vs_text =
	"#version 300 es\n"
	"in vec4 vertex;\n"
	"void main() { gl_Position = vertex; }\n"
	;

static const char *fs_template =
	"#version 300 es\n"
	"#extension GL_EXT_blend_func_extended : enable\n"
	"layout(location = 0, index = 0) out highp vec4 a;\n"
	"layout(location = %d, index = 1) out highp vec4 b;\n"
	"void main() {\n"
	"    a = vec4(0.0);\n"
	"    b = vec4(1.0);\n"
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
	GLint max_dual_source;
	GLuint prog;
	char fs_text[256];


#ifdef PIGLIT_USE_OPENGL
	piglit_require_GLSL_version(150);
	piglit_require_extension("GL_ARB_blend_func_extended");
#else // PIGLIT_USE_OPENGLES3
	piglit_require_extension("GL_EXT_blend_func_extended");
#endif
	glGetIntegerv(GL_MAX_DUAL_SOURCE_DRAW_BUFFERS, &max_dual_source);

	if (max_dual_source < 1) {
		fprintf(stderr,
			"ARB_blend_func_extended requires "
			"GL_MAX_DUAL_SOURCE_DRAW_BUFFERS >= 1. "
			"Only got %d!\n",
			max_dual_source);
		piglit_report_result(PIGLIT_FAIL);
	}

	/* Set >= GL_MAX_DUAL_SOURCE_DRAW_BUFFERS location for 'b' */
	snprintf(fs_text, 256, fs_template, max_dual_source);

	prog = piglit_build_simple_program_unlinked(vs_text, fs_text);

	/* Linking should fail as the location set is too big. */
	glLinkProgram(prog);

	if (piglit_link_check_status(prog))
		piglit_report_result(PIGLIT_FAIL);

	piglit_report_result(PIGLIT_PASS);
}
