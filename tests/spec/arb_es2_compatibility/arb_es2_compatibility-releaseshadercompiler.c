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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 *
 * Authors:
 *    Eric Anholt <eric@anholt.net>
 *
 */

/** @file arb_es2_compatibility-releasecompiler.c
 *
 * Tests that compiling a shader works again after doing
 * glReleaseShaderCompiler().
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

#ifdef GL_ARB_ES2_compatibility
static const char vs_text[] =
	"#version 100\n"
	"uniform vec4 offset;\n"
	"attribute vec4 vertex;\n"
	"void main () {\n"
	"    gl_Position = vertex + offset;"
	"}\n"
	;

static const char fs_text[] =
	"#version 100\n"
	"uniform mediump vec4 color;\n"
	"void main () {\n"
	"    gl_FragColor = color;\n"
	"}\n"
	;

void
draw(const float *color, float x_offset)
{
	GLuint prog;
	GLint color_location;
	GLint offset_location;

	prog = piglit_build_simple_program(vs_text, fs_text);

	glBindAttribLocation(prog, 0, "vertex");
	glLinkProgram(prog);
	piglit_link_check_status(prog);

	glUseProgram(prog);
	color_location = glGetUniformLocation(prog, "color");
	offset_location = glGetUniformLocation(prog, "offset");

	glUniform4fv(color_location, 1, color);
	glUniform4f(offset_location, x_offset, 0.0f, 0.0f, 0.0f);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glDeleteProgram(prog);
}
#endif

enum piglit_result
piglit_display(void)
{
#ifdef GL_ARB_ES2_compatibility
	GLboolean pass = GL_TRUE;
	float green[] = {0.0, 1.0, 0.0, 0.0};
	float blue[] = {0.0, 0.0, 1.0, 0.0};

	draw(green, 0.0f);
	glReleaseShaderCompiler();
	draw(blue, 1.0f);

	pass &= piglit_probe_pixel_rgba(piglit_width / 4, piglit_height / 2,
					green);
	pass &= piglit_probe_pixel_rgba(piglit_width * 3 / 4, piglit_height / 2,
					blue);

	assert(!glGetError());

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
#else
	return PIGLIT_SKIP;
#endif /* GL_ARB_ES2_compatibility */
}

void
piglit_init(int argc, char **argv)
{
#ifdef GL_ARB_ES2_compatibility
	static const float verts[] = {
		-1.0,  1.0, 0.0, 1.0,
		-1.0, -1.0, 0.0, 1.0,
		+0.0,  1.0, 0.0, 1.0,
		+0.0, -1.0, 0.0, 1.0,
	};

	piglit_require_gl_version(20);

	if (!piglit_is_extension_supported("GL_ARB_ES2_compatibility")) {
		printf("Requires ARB_ES2_compatibility\n");
		piglit_report_result(PIGLIT_SKIP);
	}

	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(float) * 4,
			      verts);
	glEnableVertexAttribArray(0);
#endif
}
