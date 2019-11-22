/*
 * Copyright © 2019 Intel Corporation
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

/*
 * \file dual-src-blending-issue-1917.c
 *
 * Test exercises a bug on BSW/BDW Intel platforms originally found in
 * skia tests.
 *
 * 1) Draw discarding some pixels
 * 2) Enable dual source blending
 * 3) Draw with shader without discards using dual src blending
 * 4) As a result some pixels in the region of the first draw may be corrupted
 *
 * https://gitlab.freedesktop.org/mesa/mesa/issues/1917
 *
 * \author Danylo Piliaiev <danylo.piliaiev@globallogic.com>
 */

#include "piglit-util-gl.h"

static const int render_width = 128;
static const int render_height = 128;

PIGLIT_GL_TEST_CONFIG_BEGIN

#ifdef PIGLIT_USE_OPENGL
	config.supports_gl_core_version = 31;
#else // PIGLIT_USE_OPENGL_ES3
	config.supports_gl_es_version = 30;
#endif
	config.window_width = render_width;
	config.window_height = render_height;
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;
	config.khr_no_error_support = PIGLIT_NO_ERRORS;

PIGLIT_GL_TEST_CONFIG_END

static GLuint prog_discard, prog_blend;

#ifdef PIGLIT_USE_OPENGL
static const char *vs_text =
	"#version 130\n"
	"in vec4 piglit_vertex;\n"
	"void main() {\n"
	"    gl_Position = piglit_vertex;\n"
	"}\n"
	;

static const char *fs_discard_text =
	"#version 130\n"
	"uniform float render_width;\n"
	"out vec4 col0;\n"
	"void main() {\n"
	"    if (gl_FragCoord.x > render_width / 4.0)\n"
	"        discard;\n"
	"    else\n"
	"        col0 = vec4(0.0, 1.0, 0.0, 1.0);\n"
	"}\n"
;

static const char *fs_blend_text =
	"#version 130\n"
	"#extension GL_ARB_explicit_attrib_location: require\n"
	"layout (location = 0, index = 0) out vec4 col0;\n"
	"layout (location = 0, index = 1) out vec4 col1;\n"
	"void main() {\n"
	"    col0 = vec4(0.0, 1.0, 1.0, 1.0);\n"
	"    col1 = vec4(1.0);\n"
	"}\n"
	;
#else // PIGLIT_USE_OPENGL_ES3
static const char *vs_text =
	"#version 300 es\n"
	"in vec4 piglit_vertex;\n"
	"void main() {\n"
	"    gl_Position = piglit_vertex;\n"
	"}\n"
	;

static const char *fs_discard_text =
	"#version 300 es\n"
	"uniform highp float render_width;\n"
	"out highp vec4 col0;\n"
	"void main() {\n"
	"    if (gl_FragCoord.x > render_width / 4.0)\n"
	"        discard;\n"
	"    else\n"
	"        col0 = vec4(0.0, 1.0, 0.0, 1.0);\n"
	"}\n"
;

static const char *fs_blend_text =
	"#version 300 es\n"
	"#extension GL_EXT_blend_func_extended : enable\n"
	"layout (location = 0, index = 0) out mediump vec4 col0;\n"
	"layout (location = 0, index = 1) out mediump vec4 col1;\n"
	"void main() {\n"
	"    col0 = vec4(0.0, 1.0, 1.0, 1.0);\n"
	"    col1 = vec4(1.0);\n"
	"}\n"
	;
#endif

enum piglit_result
piglit_display(void)
{
	static const GLfloat expected_color[4] = { 0.0, 1.0, 0.0, 1.0 };
	bool pass = true;

	// Reproduction is not deterministic, 100 iteration was enough for it
	// to never pass on driver/hw which exhibited the issue.
	for (int i = 0; i < 100; i++) {
		glDisable(GL_BLEND);

		glClearColor(0.0, 0.0, 0.0, 1.0);
		glClear(GL_COLOR_BUFFER_BIT);

		// Viewport is not necessary to reproduce the original issue but
		// it considerably increases the chances.
		glViewport(0, 0, render_width / 2, render_height);

		glUseProgram(prog_discard);
		glUniform1f(glGetUniformLocation(prog_discard, "render_width"),
				render_width);

		piglit_draw_rect(-1, -1, 2, 1);

		glUseProgram(prog_blend);

		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC1_COLOR);

		piglit_draw_rect(-1, 0, 2, 1);

		pass &= piglit_probe_rect_rgba(0, 0,
									render_width / 4, render_height / 2,
									expected_color);
		if (!pass)
			break;
	}

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void
piglit_init(int argc, char**argv)
{
#ifdef PIGLIT_USE_OPENGL
	piglit_require_extension("GL_ARB_blend_func_extended");
#else // PIGLIT_USE_OPENGL_ES3
	piglit_require_extension("GL_EXT_blend_func_extended");
#endif

	prog_discard = piglit_build_simple_program(vs_text, fs_discard_text);
	prog_blend = piglit_build_simple_program(vs_text, fs_blend_text);
}
