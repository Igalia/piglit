/*
 * Copyright © 2013 Intel Corporation
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

/** @file link-no-vsfs.c
 *
 * From the GLES 2.0.25 spec (page 30):
 *
 *     "Linking can fail for a variety of reasons as specified in the
 *      OpenGL ES Shading Language Specification. Linking will also
 *      fail if one or more of the shader objects, attached to program
 *      are not compiled successfully, if program does not contain
 *      both a vertex shader and a fragment shader, or if more active
 *      uniform or active sampler variables are used in program than
 *      allowed (see section 2.10.4).
 *
 * This also appears in the 3.0.2 spec, page 48.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_es_version = 20;

	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

enum piglit_result
piglit_display(void)
{
	/* UNREACHED */
	return PIGLIT_FAIL;
}

static bool
test_link_fail(GLenum target, const char *source)
{
	GLuint shader, prog;
	GLint ok;

	shader = piglit_compile_shader_text(target, source);

	prog = glCreateProgram();
	glAttachShader(prog, shader);
	glLinkProgram(prog);
	glDeleteShader(shader);

	glGetProgramiv(prog, GL_LINK_STATUS, &ok);

	glDeleteProgram(prog);
	if (ok) {
		fprintf(stderr,
			"Linking with only a %s succeeded when it should have "
			"failed:\n",
			piglit_get_gl_enum_name(target));
		return false;
	}
	return true;
}

void
piglit_init(int argc, char **argv)
{
	const char *vs_source =
		"void main()\n"
		"{"
		"	gl_Position = vec4(0);"
		"}";
	const char *fs_source =
		"precision mediump float;\n"
		"void main()\n"
		"{"
		"	gl_FragColor = vec4(0);"
		"}";

	if (!test_link_fail(GL_VERTEX_SHADER, vs_source))
		piglit_report_result(PIGLIT_FAIL);
	if (!test_link_fail(GL_FRAGMENT_SHADER, fs_source))
		piglit_report_result(PIGLIT_FAIL);

	piglit_report_result(PIGLIT_PASS);
}
