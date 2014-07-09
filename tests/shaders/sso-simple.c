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
 * \file sso-simple.c
 * Simple GL_EXT_separate_shader_objects rendering test
 *
 * \author Ian Romanick <ian.d.romanick@intel.com>
 */
#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

static const char vs_text[] =
	"void main() { gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex; "
	"gl_FrontColor = vec4(0.0, 1.0, 0.0, 1.0); }";

static const char good_fs_text[] =
	"void main() { gl_FragColor = gl_Color; }";

/* It is important that this shader *not* use gl_Color.
 */
static const char bad_fs_text[] =
	"void main() { gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0); }";

static GLuint prog[3];

enum piglit_result
piglit_display(void)
{
	static const float green[3] = { 0.0, 1.0, 0.0 };
	static const float blue[3]  = { 0.0, 0.0, 1.0 };
	enum piglit_result result = PIGLIT_PASS;
	float x = 10.0;

	glClear(GL_COLOR_BUFFER_BIT);
	glColor3fv(blue);

	/* Bind the separately linked vertex shader and the separately linked
	 * fragment shader using the new interfaces.  This should produce a
	 * green box.
	 */
	glUseShaderProgramEXT(GL_VERTEX_SHADER, prog[0]);
	glUseShaderProgramEXT(GL_FRAGMENT_SHADER, prog[1]);
	piglit_draw_rect(x, 10, 10, 10);
	if (!piglit_probe_pixel_rgb(x + 5, 15, green))
		result = PIGLIT_FAIL;

	x += 20.0;

	/* Bind the vertex shader that is already linked with a fragment
	 * shader and the separately linked fragment shader using the new
	 * interfaces.  This should produce a green box.
	 *
	 * If the linked optimized away the vertex shader writes to
	 * gl_FrontColor (because the fragment shader in prog[2] does not use
	 * it), this will produce incorrect results.
	 */
	glUseProgram(prog[2]);
	glUseShaderProgramEXT(GL_FRAGMENT_SHADER, prog[1]);
	piglit_draw_rect(x, 10, 10, 10);
	if (!piglit_probe_pixel_rgb(x + 5, 15, green))
		result = PIGLIT_FAIL;

	x += 20.0;

	/* Unbind any program from the vertex shader stage so that fixed
	 * function is used.  This should produce the same results as the
	 * vertex shader except that fixed-function outputs blue.
	 */
	glUseShaderProgramEXT(GL_VERTEX_SHADER, 0);
	piglit_draw_rect(x, 10, 10, 10);
	if (!piglit_probe_pixel_rgb(x + 5, 15, blue))
		result = PIGLIT_FAIL;

	if (!piglit_automatic)
		piglit_present_results();

	return result;
}

void
piglit_init(int argc, char **argv)
{
	GLuint vs;
	GLuint fs;

	piglit_require_gl_version(20);

	piglit_require_extension("GL_EXT_separate_shader_objects");

	glClearColor(0.3, 0.3, 0.3, 0.0);

	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);

	vs = piglit_compile_shader_text(GL_VERTEX_SHADER, vs_text);
	fs = piglit_compile_shader_text(GL_FRAGMENT_SHADER, bad_fs_text);

	prog[0] = piglit_link_simple_program(vs, 0);
	prog[1] = glCreateShaderProgramEXT(GL_FRAGMENT_SHADER, good_fs_text);
	prog[2] = piglit_link_simple_program(vs, fs);

	glDeleteShader(vs);
	glDeleteShader(fs);
}
