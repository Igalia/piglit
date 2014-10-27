/*
 * Copyright © 2009 Intel Corporation
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

/** @file getactiveuniform-constant.c
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

static const char vs_code[] =
	"#version 120\n"
	"uniform vec4 a;\n"
	"uniform vec4 b;\n"
	"uniform vec4 c;\n"
	"uniform vec4 d;\n"
	"uniform int i;\n"
	"const vec4 vv[] =\n"
	"    vec4[](vec4( 1,  2,  3,  4),\n"
        "           vec4( 5,  6,  7,  8),\n"
        "           vec4( 9, 10, 11, 12),\n"
        "           vec4(13, 14, 15, 16));\n"
	"\n"
	"void main() {\n"
	"    gl_Position = a + b + c + d + vv[i]\n;"
	"}\n"
	;

static const char fs_code[] =
	"#version 120\n"
	"uniform vec4 e;\n"
	"uniform vec4 f;\n"
	"uniform vec4 g;\n"
	"uniform vec4 h;\n"
	"uniform int j;\n"
	"\n"
	"void main() {\n"
	"    const vec4 fv[] =\n"
	"        vec4[](vec4( 1,  2,  3,  4),\n"
        "               vec4( 5,  6,  7,  8),\n"
        "               vec4( 9, 10, 11, 12),\n"
        "               vec4(13, 14, 15, 16));\n"
	"\n"
	"    gl_FragColor = e + f + g + h + fv[j]\n;"
	"}\n"
	;

static const char *all_uniform_names[] = {
	"a",
	"b",
	"c",
	"d",
	"e",
	"f",
	"g",
	"h",
	"i",
	"j",
};

static bool uniform_seen[ARRAY_SIZE(all_uniform_names)] = {
	false,
};

void
piglit_init(int argc, char **argv)
{
	bool pass = true;
	GLint prog;
	GLint vs;
	GLint fs;
	GLint num;
	int i;

	piglit_require_GLSL_version(120);

	vs = piglit_compile_shader_text(GL_VERTEX_SHADER, vs_code);
	fs = piglit_compile_shader_text(GL_FRAGMENT_SHADER, fs_code);
	prog = piglit_link_simple_program(vs, fs);

	glGetProgramiv(prog, GL_ACTIVE_UNIFORMS, &num);
	for (i = 0; i < num; i++) {
		char name[256];
		GLsizei length;
		GLint size;
		GLenum type;
		bool found;
		unsigned j;

		glGetActiveUniform(prog,
				   i,
				   sizeof(name),
				   &length,
				   &size,
				   &type,
				   name);
		pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

		/* Try to find the name in the table of known names.
		 * If the name is not found, fail.
		 */
		found = false;
		for (j = 0; j < ARRAY_SIZE(all_uniform_names); j++) {
			if (strcmp(name, all_uniform_names[j]) == 0) {
				found = true;
				uniform_seen[j] = true;
				break;
			}
		}

		if (!found) {
			fprintf(stderr,
				"Uniform name \"%s\" returned by "
				"glGetActiveUniform, but should not have "
				"been.\n",
				name);
			pass = false;
		}
	}

	for (i = 0; i < ARRAY_SIZE(uniform_seen); i++) {
		if (!uniform_seen[i]) {
			fprintf(stderr,
				"Uniform name \"%s\" was not returned by "
				"glGetActiveUniform, but should have "
				"been.\n",
				all_uniform_names[i]);
			pass = false;
		}
	}
	
	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}

enum piglit_result
piglit_display(void)
{
	/* UNREACHED */
	return PIGLIT_FAIL;
}
