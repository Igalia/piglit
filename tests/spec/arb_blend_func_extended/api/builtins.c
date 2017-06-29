/*
 * Copyright (c) 2015 Ryan Houdek <Sonicadvance1@gmail.com>
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

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_es_version = 20;
	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;
	config.khr_no_error_support = PIGLIT_NO_ERRORS;

PIGLIT_GL_TEST_CONFIG_END

bool compile_simple_program(const char* vs_text, const char* fs_text)
{
	GLuint vs;
	GLuint fs;
	GLuint prog;
	bool status;

	prog = glCreateProgram();

	vs = piglit_compile_shader_text_nothrow(GL_VERTEX_SHADER, vs_text);
	fs = piglit_compile_shader_text_nothrow(GL_FRAGMENT_SHADER, fs_text);

	if (!vs || !fs)
		return false;

	glAttachShader(prog, vs);
	glAttachShader(prog, fs);
	glLinkProgram(prog);

	status = piglit_link_check_status(prog);
	glDeleteProgram(prog);
	return status;
}

enum piglit_result
piglit_display(void)
{
	return PIGLIT_FAIL;
}

void piglit_init(int argc, char **argv)
{
	bool pass = true;
	char fs_text[256];

	piglit_require_extension("GL_EXT_blend_func_extended");

	static const char *vs_text =
		"#version 100\n"
		"void main() {\n"
		"        gl_Position = vec4(0);\n"
		"}\n"
		;

	static const char *fs_template =
		"#version 100\n"
		"#extension GL_EXT_blend_func_extended : enable\n"
		"void main() {\n"
		"	%s = vec4(0);\n"
		"	%s = vec4(0);\n"
		"}\n"
		;

	// Tests that should pass
	// Regular FragColor
	snprintf(fs_text, 256, fs_template,
	         "gl_FragColor",
		   "gl_SecondaryFragColorEXT");
	pass = compile_simple_program(vs_text, fs_text) && pass;

	// Regular FragData
	snprintf(fs_text, 256, fs_template,
	         "gl_FragData[0]",
		   "gl_SecondaryFragDataEXT[0]");
	pass = compile_simple_program(vs_text, fs_text) && pass;

	// Tests that should fail
	// FragColor & SecondaryFragData
	snprintf(fs_text, 256, fs_template,
	         "gl_FragColor",
		   "gl_SecondaryFragDataEXT[0]");
	pass = !compile_simple_program(vs_text, fs_text) && pass;

	// FragData & SecondaryFragColor
	snprintf(fs_text, 256, fs_template,
	         "gl_FragData[0]",
		   "gl_SecondaryFragColorEXT");
	pass = !compile_simple_program(vs_text, fs_text) && pass;

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);

}
