/*
 * Copyright © 2016 Jamey Sharp
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
 * \file shadersource-no-compile.c
 * OpenGL 4.5 Core Profile section 7.1, in the documentation for CompileShader,
 * says: "Changing the source code of a shader object with ShaderSource does not
 * change its compile status or the compiled shader code."
 *
 * This test creates a shader, compiles it, changes its source, and links it.
 * The spec requires rendering done with this shader to be consistent with the
 * old source, not the new source, since the shader isn't compiled again after
 * the source is changed.
 *
 * According to Karol Herbst, the game "Divinity: Original Sin - Enhanced
 * Edition" depends on this odd quirk of the spec. See:
 * https://lists.freedesktop.org/archives/mesa-dev/2016-March/109789.html
 */
#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 20;

	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

static const char vs_text[] =
	"void main() { gl_Position = gl_Vertex; }";

/* good_fs_text uses a constant green color, while bad_fs_text uses a
 * constant red color, so that we can tell which version of the fragment
 * shader got executed. Both are distinct from the clear-color so we can
 * tell if the shader ran at all.
 */

static const char good_fs_text[] =
	"void main() { gl_FragColor = vec4(0.0, 1.0, 0.0, 1.0); }";

static const char bad_fs_text[] =
	"void main() { gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0); }";

enum piglit_result
piglit_display(void)
{
	static const float green[3] = { 0.0, 1.0, 0.0 };

	glClear(GL_COLOR_BUFFER_BIT);

	piglit_draw_rect(-1, -1, 2, 2);
	if (!piglit_probe_pixel_rgb(15, 15, green))
		return PIGLIT_FAIL;

	return PIGLIT_PASS;
}

void
piglit_init(int argc, char **argv)
{
	GLuint vs = piglit_compile_shader_text(GL_VERTEX_SHADER, vs_text);
	GLuint fs = piglit_compile_shader_text(GL_FRAGMENT_SHADER, good_fs_text);
	const GLchar *bad_fs_texts[] = { bad_fs_text };
	GLuint prog;

	/* Change the shader source, but don't recompile it before linking. */
	glShaderSource(fs, 1, bad_fs_texts, NULL);
	prog = piglit_link_simple_program(vs, fs);
	if (!prog)
		piglit_report_result(PIGLIT_FAIL);

	glDeleteShader(vs);
	glDeleteShader(fs);

	glUseProgram(prog);

	glClearColor(0.3, 0.3, 0.3, 0.0);
}
