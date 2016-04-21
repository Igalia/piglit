/*
 * Copyright © 2015 Gregory Hainaut <gregory.hainaut@gmail.com>
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
 * \file rendezvous_by_name.c
 * Simple test for separate shader objects that use rendezvous-by-name.
 *
 * Related to issue: https://bugs.freedesktop.org/show_bug.cgi?id=79783
 *
 * The test ensures deadcode optimization of input variables doesn't break
 * the rendezvous by name of the variables.
 */
#include "piglit-util-gl.h"
#include "sso-common.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;
	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

static GLuint pipeline_3_out_1_in;
static GLuint pipeline_1_out_3_in;
static GLuint pipeline_inactive;

static const char *vs_code_3_out_template =
	"#version %d\n"
	"#extension GL_ARB_separate_shader_objects: require\n"
	"#extension GL_ARB_explicit_attrib_location: require\n"
	"\n"
	"layout(location = 0) in vec4 piglit_vertex;\n"
	"\n"
	"out vec4 blue;\n"
	"out vec4 green;\n"
	"out vec4 red;\n"
	"\n"
	"void main()\n"
	"{\n"
	"    gl_Position = piglit_vertex;\n"
	"    red   = vec4(1, 0, 0, 0);\n"
	"    green = vec4(0, 1, 0, 0);\n"
	"    blue  = vec4(0, 0, 1, 0);\n"
	"}\n"
	;

static const char *vs_code_1_out_template =
	"#version %d\n"
	"#extension GL_ARB_separate_shader_objects: require\n"
	"#extension GL_ARB_explicit_attrib_location: require\n"
	"\n"
	"layout(location = 0) in vec4 piglit_vertex;\n"
	"\n"
	"out vec4 blue;\n"
	"out vec4 green;\n"
	"out vec4 red;\n"
	"\n"
	"void main()\n"
	"{\n"
	"    gl_Position = piglit_vertex;\n"
	"    green = vec4(0, 1, 0, 0);\n"
	"}\n"
	;

static const char *vs_code_inactive_template =
	"#version %d\n"
	"#extension GL_ARB_separate_shader_objects: require\n"
	"#extension GL_ARB_explicit_attrib_location: require\n"
	"\n"
	"layout(location = 0) in vec4 piglit_vertex;\n"
	"\n"
	"#define MAX_VARYING %d\n"
	"out vec4 a_dummy[MAX_VARYING];\n"
	"out vec4 green;\n"
	"out vec4 z_dummy[MAX_VARYING];\n"
	"\n"
	"void main()\n"
	"{\n"
	"    gl_Position = piglit_vertex;\n"
	"    green = vec4(0, 1, 0, 0);\n"
	"    for(int i = 0; i < MAX_VARYING; i++) {\n"
	"        a_dummy[i] = vec4(1, 0, 0, 1);\n"
	"        z_dummy[i] = vec4(0, 0, 1, 1);\n"
	"    }\n"
	"}\n"
	;

static const char *fs_code_1_in_template =
	"#version %d\n"
	"#extension GL_ARB_separate_shader_objects: require\n"
	"#extension GL_ARB_explicit_attrib_location: enable\n"
	"\n"
	"#if __VERSION__ >= 130\n"
	"layout(location = 0) out vec4 out_color;\n"
	"#else\n"
	"#define out_color gl_FragColor\n"
	"#endif\n"
	"\n"
	"in vec4 blue;\n"
	"in vec4 green;\n"
	"in vec4 red;\n"
	"\n"
	"void main()\n"
	"{\n"
	"    out_color = vec4(green.xyz, 1);\n"
	"}\n"
	;

static const char *fs_code_3_in_template =
	"#version %d\n"
	"#extension GL_ARB_separate_shader_objects: require\n"
	"#extension GL_ARB_explicit_attrib_location: enable\n"
	"\n"
	"#if __VERSION__ >= 130\n"
	"layout(location = 0, index = 0) out vec4 out_color;\n"
	"layout(location = 0, index = 1) out vec4 avoid_opt;\n"
	"#else\n"
	"#define out_color gl_FragColor\n"
	"#endif\n"
	"\n"
	"in vec4 blue;\n"
	"in vec4 green;\n"
	"in vec4 red;\n"
	"\n"
	"void main()\n"
	"{\n"
	"    out_color = vec4(green.xyz, 1);\n"
	"    avoid_opt = vec4(blue + red);\n"
	"}\n"
	;

static const char *fs_code_inactive_template =
	"#version %d\n"
	"#extension GL_ARB_separate_shader_objects: require\n"
	"#extension GL_ARB_explicit_attrib_location: enable\n"
	"\n"
	"#if __VERSION__ >= 130\n"
	"layout(location = 0) out vec4 out_color;\n"
	"#else\n"
	"#define out_color gl_FragColor\n"
	"#endif\n"
	"\n"
	"#define MAX_VARYING %d\n"
	"in vec4 a_dummy[MAX_VARYING];\n"
	"in vec4 green;\n"
	"in vec4 z_dummy[MAX_VARYING];\n"
	"\n"
	"void main()\n"
	"{\n"
	"    out_color = vec4(green.xyz, 1);\n"
	"}\n"
	;

enum piglit_result
piglit_display(void)
{
	static const float expected[] = {
		0.0f, 1.0f, 0.0f, 1.0f
	};
	int h_width = piglit_width / 2;
	int h_height = piglit_height / 2;
	bool pass;
	bool pass1;
	bool pass2;
	bool pass3;

	glClearColor(0.1f, 0.1f, 0.1f, 0.1f);
	glClear(GL_COLOR_BUFFER_BIT);

	/*
	 * Test 1: 3 active output in the VS + 1 active input in the FS.
	 * Screen location: bottom left
	 */
	glBindProgramPipeline(pipeline_3_out_1_in);
	piglit_draw_rect(-1, -1, 1, 1);

	/*
	 * Test 2: 1 active output in the VS + 3 active input in the FS.
	 * Screen location: top left
	 */
	glBindProgramPipeline(pipeline_1_out_3_in);
	piglit_draw_rect(-1, 0, 1, 1);

	/*
	 * Test 3: Link separate VS/FS together. Expect to optimize inactive variables
	 * Screen location: right
	 */
	if (pipeline_inactive) {
		glBindProgramPipeline(pipeline_inactive);
		piglit_draw_rect(0, -1, 1, 2);
	}

	/*
	 * Probe and report result
	 */
	pass1 = piglit_probe_rect_rgba(0, 0, h_width, h_height, expected);
	pass2 = piglit_probe_rect_rgba(0, h_height, h_width, h_height, expected);
	pass3 = !pipeline_inactive ||
		piglit_probe_rect_rgba(h_width, h_height, h_width, h_height, expected);

	piglit_present_results();

	pass = pass1 & pass2 & pass3;

	piglit_report_subtest_result(pass1 ? PIGLIT_PASS : PIGLIT_FAIL,
			"3 VS output => 1 FS input");
	piglit_report_subtest_result(pass2 ? PIGLIT_PASS : PIGLIT_FAIL,
			"1 VS output => 3 FS input");

	if (pipeline_inactive) {
		piglit_report_subtest_result(pass3 ? PIGLIT_PASS : PIGLIT_FAIL,
				"Unactive varying optimization in multi-shade separated program");
	}

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void piglit_init(int argc, char **argv)
{
	unsigned glsl_version;
	GLuint vs_prog_3_out;
	GLuint vs_prog_1_out;
	GLuint fs_prog_3_in;
	GLuint fs_prog_1_in;
	GLuint vs_fs_prog_separate_inactive;
	char *vs_source;
	char *fs_source;
	GLint max_varying;
	bool pass = true;

	piglit_require_vertex_shader();
	piglit_require_fragment_shader();
	piglit_require_GLSL_version(130); /* Support layout index on output color */
	piglit_require_extension("GL_ARB_separate_shader_objects");
	piglit_require_extension("GL_ARB_explicit_attrib_location");
	piglit_require_extension("GL_ARB_blend_func_extended");

	glsl_version = pick_a_glsl_version();

	glGetIntegerv(GL_MAX_VARYING_COMPONENTS, &max_varying);
	max_varying = (max_varying / 4u) - 1u;

	/*
	 * Program compilation and link
	 */
	printf("Compile vs_prog_3_out\n");
	vs_prog_3_out = format_and_link_program(GL_VERTEX_SHADER,
			vs_code_3_out_template, glsl_version);

	printf("Compile vs_prog_1_out\n");
	vs_prog_1_out = format_and_link_program(GL_VERTEX_SHADER,
			vs_code_1_out_template, glsl_version);

	printf("Compile fs_prog_3_in\n");
	fs_prog_3_in = format_and_link_program(GL_FRAGMENT_SHADER,
			fs_code_3_in_template, glsl_version);

	printf("Compile fs_prog_1_in\n");
	fs_prog_1_in = format_and_link_program(GL_FRAGMENT_SHADER,
			fs_code_1_in_template, glsl_version);

	(void)!asprintf(&vs_source, vs_code_inactive_template, glsl_version, max_varying);
	(void)!asprintf(&fs_source, fs_code_inactive_template, glsl_version, max_varying);

	pass &= piglit_check_gl_error(0);

	printf("Compile vs_fs_prog_separate_inactive\n");
	vs_fs_prog_separate_inactive = piglit_build_simple_program_unlinked(vs_source, fs_source);
	/* Manual linking so we can pack 2 separate-aware shaders into a single program */
	glProgramParameteri(vs_fs_prog_separate_inactive, GL_PROGRAM_SEPARABLE, GL_TRUE);
	glLinkProgram(vs_fs_prog_separate_inactive);

	if (!piglit_link_check_status(vs_fs_prog_separate_inactive)) {
		piglit_report_subtest_result(PIGLIT_SKIP,
				"Unactive varying optimization in multi-shade separated program");
		vs_fs_prog_separate_inactive = 0; // Skip program
		piglit_reset_gl_error(); // Clear pending error
	}

	free(vs_source);
	free(fs_source);

	/*
	 * Pipeline creation
	 */
	glGenProgramPipelines(1, &pipeline_3_out_1_in);
	glGenProgramPipelines(1, &pipeline_1_out_3_in);
	glBindProgramPipeline(pipeline_3_out_1_in);
	glUseProgramStages(pipeline_3_out_1_in,
			GL_VERTEX_SHADER_BIT, vs_prog_3_out);
	glUseProgramStages(pipeline_3_out_1_in,
			GL_FRAGMENT_SHADER_BIT, fs_prog_1_in);

	glBindProgramPipeline(pipeline_1_out_3_in);
	glUseProgramStages(pipeline_1_out_3_in,
			GL_VERTEX_SHADER_BIT, vs_prog_1_out);
	glUseProgramStages(pipeline_1_out_3_in,
			GL_FRAGMENT_SHADER_BIT, fs_prog_3_in);

	if (vs_fs_prog_separate_inactive) {
		glGenProgramPipelines(1, &pipeline_inactive);
		glBindProgramPipeline(pipeline_inactive);
		glUseProgramStages(pipeline_inactive,
				GL_VERTEX_SHADER_BIT | GL_FRAGMENT_SHADER_BIT,
				vs_fs_prog_separate_inactive);
	} else {
		pipeline_inactive = 0; // Skip the test
	}

	if (!piglit_check_gl_error(0) || !pass)
		piglit_report_result(PIGLIT_FAIL);
}
