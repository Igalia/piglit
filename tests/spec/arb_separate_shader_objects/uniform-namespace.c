/*
 * Copyright © 2014 Intel Corporation
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
 * \file uniform-namespace.c
 * Verify that the namespace of uniforms is per program, not per pipeline
 */
#include "piglit-util-gl.h"
#include "sso-common.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;
	config.supports_gl_core_version = 31;

	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;

PIGLIT_GL_TEST_CONFIG_END

static const char vs_template[] =
	"#version %u\n"
	"#extension GL_ARB_separate_shader_objects: require\n"
	"#extension GL_ARB_explicit_attrib_location: require\n"
	"\n"
	"layout(location=0) in vec4 piglit_vertex;\n"
	"\n"
	"uniform vec4 a;\n"
	"\n"
	"void main()\n"
	"{\n"
	"    gl_Position = piglit_vertex + a;\n"
	"}"
	;

static const char fs_template[] =
	"#version %u\n"
	"#extension GL_ARB_separate_shader_objects: require\n"
	"#extension GL_ARB_explicit_attrib_location: require\n"
	"\n"
	"uniform vec4 a;\n"
	"\n"
	"#if __VERSION__ >= 130\n"
	"layout(location = 0) out vec4 out_color;\n"
	"#else\n"
	"#define out_color gl_FragColor\n"
	"#endif\n"
	"\n"
	"void main()\n"
	"{\n"
	"    out_color = a;\n"
	"}"
	;

static GLuint vs = 0;
static GLuint fs = 0;

/**
 * Location of the "a" uniform in the vertex shader program.
 */
static GLint loc_vs = -1;

/**
 * Location of the "a" uniform in the fragment shader program.
 */
static GLint loc_fs = -1;

static GLuint
generate_program(const char *code_template, unsigned glsl_version,
		 GLenum program_target, GLint *uniform_loc)
{
	char *code = NULL;
	GLuint prog;

	(void)!asprintf(&code, code_template, glsl_version);
	prog = glCreateShaderProgramv(program_target, 1,
				      (const GLchar * const*) &code);
	free(code);

	piglit_link_check_status(prog);

	*uniform_loc = glGetUniformLocation(prog, "a");

	return prog;
}

void
piglit_init(int argc, char **argv)
{
	GLuint pipe = 0;
	unsigned glsl_version;

	piglit_require_vertex_shader();
	piglit_require_fragment_shader();
	piglit_require_extension("GL_ARB_separate_shader_objects");
	piglit_require_extension("GL_ARB_explicit_attrib_location");

	glsl_version = pick_a_glsl_version();

	vs = generate_program(vs_template, glsl_version, GL_VERTEX_SHADER,
			      &loc_vs);

	fs = generate_program(fs_template, glsl_version, GL_FRAGMENT_SHADER,
			      &loc_fs);

	if (vs == 0 || fs == 0)
		piglit_report_result(PIGLIT_FAIL);

	glGenProgramPipelines(1, &pipe);
	glBindProgramPipeline(pipe);
	glUseProgramStages(pipe, GL_VERTEX_SHADER_BIT, vs);
	glUseProgramStages(pipe, GL_FRAGMENT_SHADER_BIT, fs);

	if (!piglit_check_gl_error(0))
		piglit_report_result(PIGLIT_FAIL);
}

enum piglit_result
piglit_display(void)
{
	static const float gray[]  = { 0.5, 0.5, 0.5, 1.0 };
	static const float green[] = { 0.0, 1.0, 0.0, 0.0 };
	static const float red[]   = { 1.0, 0.0, 0.0, 0.0 };
	bool pass = true;

	glProgramUniform4fv(vs, loc_vs, 1, red);
	glProgramUniform4fv(fs, loc_fs, 1, green);

	glClearColor(gray[0], gray[1], gray[2], gray[3]);
	glClear(GL_COLOR_BUFFER_BIT);

	piglit_draw_rect(-1, -1, 1, 2);

	pass = piglit_probe_rect_rgb(0, 0,
				     piglit_width / 2, piglit_height,
				     gray)
		&& pass;

	pass = piglit_probe_rect_rgb(piglit_width / 2, 0,
				     piglit_width / 2, piglit_height,
				     green)
		&& pass;

	piglit_present_results();

	pass = piglit_check_gl_error(0) && pass;

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}
