/*
 * Copyright © 2015 Intel Corporation
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
 * This tests a bug in Mesa where explicit locations are not taken into
 * account when assigning varying locations which results in two
 * inputs/outputs being given the same location.
 */
#include "piglit-util-gl.h"
#include "sso-common.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;
	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

static GLuint pipeline = 0;
static GLuint pipeline_arrays = 0;
static GLuint pipeline_arrays_of_arrays = 0;

static const char *vs_code_template =
	"#version %d\n"
	"#extension GL_ARB_separate_shader_objects: require\n"
	"#extension GL_ARB_explicit_attrib_location: require\n"
	"\n"
	"layout(location = 0) in vec4 piglit_vertex;\n"
	"\n"
	"layout(location = 0) out vec3 a;\n"
        "out vec3 d;\n"
        "out vec3 e;\n"
	"layout(location = 1) out vec3 b;\n"
        "out vec3 f;\n"
	"layout(location = 2) out vec3 c;\n"
	"\n"
	"void main()\n"
	"{\n"
	"    gl_Position = piglit_vertex;\n"
	"    a = vec3(0.25, 0, 0);\n"
	"    b = vec3(0, 0.25, 0);\n"
	"    c = vec3(0, 0, 0.25);\n"
	"    d = vec3(0.5, 0, 0);\n"
	"    e = vec3(0, 0.5, 0);\n"
	"    f = vec3(0, 0, 0.5);\n"
	"}\n"
	;

static const char *fs_code_template =
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
	"layout(location = 0) in vec3 a; /* should get vec3(0.25, 0, 0) */\n"
        "in vec3 d;                      /* should get vec3(0.5, 0, 0)  */\n"
        "in vec3 e;                      /* should get vec3(0, 0.5, 0)  */\n"
	"layout(location = 1) in vec3 b; /* should get vec3(0, 0.25, 0) */\n"
        "in vec3 f;                      /* should get vec3(0, 0, 0.5)  */\n"
	"layout(location = 2) in vec3 c; /* should get vec3(0, 0, 0.25) */\n"
	"\n"
	"void main()\n"
	"{\n"
	"    out_color = vec4(a.x + d.x, b.y + e.y, c.z + f.z, 1);\n"
	"}\n"
	;

static const char *vs_arrays_code_template =
	"#version %d\n"
	"#extension GL_ARB_separate_shader_objects: require\n"
	"#extension GL_ARB_explicit_attrib_location: require\n"
	"\n"
	"layout(location = 0) in vec4 piglit_vertex;\n"
	"\n"
	"out vec3 c[2];\n"
	"layout(location = 1) out vec3 a;\n"
	"layout(location = 2) out vec3 b[2];\n"
	"out vec3 d;\n"
	"\n"
	"void main()\n"
	"{\n"
	"    gl_Position = piglit_vertex;\n"
	"    a = vec3(0.25, 0, 0);\n"
	"    b[0] = vec3(0, 0.25, 0);\n"
	"    b[1] = vec3(0, 0, 0.25);\n"
	"    c[0] = vec3(0.5, 0, 0);\n"
	"    c[1] = vec3(0, 0.5, 0);\n"
	"    d = vec3(0, 0, 0.5);\n"
	"}\n"
	;

static const char *fs_arrays_code_template =
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
	"in vec3 c[2];                      /* should get vec3(0.5, 0, 0)\n"
	"                                    * and vec3(0, 0.5, 0)\n"
	"                                    */\n"
	"layout(location = 1) in vec3 a;    /* should get vec3(0.25, 0, 0) */\n"
	"layout(location = 2) in vec3 b[2]; /* should get vec3(0, 0.25, 0)\n"
	"                                    * and vec3(0, 0, 0.25)\n"
	"                                    */\n"
	"in vec3 d;                         /* should get vec3(0, 0, 0.5)  */\n"
	"\n"
	"void main()\n"
	"{\n"
	"    out_color = vec4(a.x + c[0].x, b[0].y + c[1].y, b[1].z + d.z, 1);\n"
	"}\n"
	;

static const char *vs_aoa_code_template =
	"#version %d\n"
	"#extension GL_ARB_separate_shader_objects: require\n"
	"#extension GL_ARB_explicit_attrib_location: require\n"
	"#extension GL_ARB_arrays_of_arrays: require\n"
	"\n"
	"layout(location = 0) in vec4 piglit_vertex;\n"
	"\n"
	"out vec3 c[2][2];\n"
	"layout(location = 2) out vec3 a[2][2];\n"
	"out vec3 d;\n"
	"layout(location = 8) out vec3 b;\n"
	"\n"
	"void main()\n"
	"{\n"
	"    gl_Position = piglit_vertex;\n"
	"    a[0][0] = vec3(0.25, 0, 0);\n"
	"    a[0][1] = vec3(0, 0.25, 0);\n"
	"    a[1][0] = vec3(0.125, 0, 0);\n"
	"    a[1][1] = vec3(0, 0.125, 0);\n"
	"    b = vec3(0, 0, 0.25);\n"
	"    c[0][0] = vec3(0.5, 0, 0);\n"
	"    c[0][1] = vec3(0, 0.5, 0);\n"
	"    c[1][0] = vec3(0.0625, 0, 0);\n"
	"    c[1][1] = vec3(0, 0.0625, 0);\n"
	"    d = vec3(0, 0, 0.5);\n"
	"}\n"
	;

static const char *fs_aoa_code_template =
	"#version %d\n"
	"#extension GL_ARB_separate_shader_objects: require\n"
	"#extension GL_ARB_explicit_attrib_location: enable\n"
	"#extension GL_ARB_arrays_of_arrays: require\n"
	"\n"
	"#if __VERSION__ >= 130\n"
	"layout(location = 0) out vec4 out_color;\n"
	"#else\n"
	"#define out_color gl_FragColor\n"
	"#endif\n"
	"\n"
	"in vec3 c[2][2];\n"
	"layout(location = 2) in vec3 a[2][2];\n"
	"layout(location = 8) in vec3 b;\n"
	"in vec3 d;\n"
	"\n"
	"void main()\n"
	"{\n"
	"    float red = a[0][0].x + a[1][0].x + c[0][0].x + c[1][0].x;\n"
	"    float green = a[0][1].y + a[1][1].y + c[0][1].y + c[1][1].y;\n"
	"    float blue = b.z + d.z;\n"
	"    out_color = vec4(red, green, blue, 1);\n"
	"}\n"
	;

enum piglit_result
piglit_display(void)
{
	static const float expected[] = {
		0.75f, 0.75f, 0.75f, 1.0f
	};
	static const float expected_aoa[] = {
		0.9375f, 0.9375f, 0.75f, 1.0f
	};
	bool pass, pass1, pass2, pass3;
	int h_width = piglit_width / 2;
	int h_height = piglit_height / 2;

	glClearColor(0.1f, 0.1f, 0.1f, 0.1f);
	glClear(GL_COLOR_BUFFER_BIT);

	/*
	 * Test 1: Test for overlap of location assignment for varying.
	 */
	glBindProgramPipeline(pipeline);
	piglit_draw_rect(-1, -1, 1, 1);

	/*
	 * Test 2: Test for overlap of location assignment for varying arrays.
	 */
	glBindProgramPipeline(pipeline_arrays);
	piglit_draw_rect(-1, 0, 1, 1);

	/*
	 * Test 3: Test for overlap of location assignment for varying
	 * arrays of arrays.
	 */
	if (pipeline_arrays_of_arrays) {
		glBindProgramPipeline(pipeline_arrays_of_arrays);
		piglit_draw_rect(0, -1, 1, 2);
	}

	/*
	 * Probe and report result
	 */
	pass1 = piglit_probe_rect_rgba(0, 0, h_width, h_height, expected);
	pass2 = piglit_probe_rect_rgba(0, h_height, h_width, h_height,
				       expected);
	pass3 = !pipeline_arrays_of_arrays ||
		piglit_probe_rect_rgba(h_width, h_height, h_width, h_height,
				       expected_aoa);

	piglit_present_results();

	pass = pass1 & pass2 & pass3;

	piglit_report_subtest_result(pass1 ? PIGLIT_PASS : PIGLIT_FAIL,
			"Varying location assignment overlap");

	piglit_report_subtest_result(pass2 ? PIGLIT_PASS : PIGLIT_FAIL,
			"Varying arrays location assignment overlap");

	if (pipeline_arrays_of_arrays) {
		piglit_report_subtest_result(pass3 ? PIGLIT_PASS : PIGLIT_FAIL,
			"Varying arrays of arrays location assignment "
			"overlap");
	}

	piglit_present_results();
	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void piglit_init(int argc, char **argv)
{
	unsigned glsl_version;
	GLuint vs_prog, vs_prog_arrays, vs_prog_arrays_of_arrays;
	GLuint fs_prog, fs_prog_arrays, fs_prog_arrays_of_arrays;

	piglit_require_vertex_shader();
	piglit_require_fragment_shader();
	piglit_require_GLSL_version(120); /* Required for in/out and arrays */
	piglit_require_extension("GL_ARB_separate_shader_objects");
	piglit_require_extension("GL_ARB_explicit_attrib_location");

	glsl_version = pick_a_glsl_version();

	/* Link and build pipeline for Varying test */
	vs_prog = format_and_link_program(GL_VERTEX_SHADER, vs_code_template,
					 glsl_version);

	fs_prog = format_and_link_program(GL_FRAGMENT_SHADER,
					 fs_code_template, glsl_version);

	glGenProgramPipelines(1, &pipeline);
	glUseProgramStages(pipeline,
			   GL_VERTEX_SHADER_BIT,
			   vs_prog);
	glUseProgramStages(pipeline,
			   GL_FRAGMENT_SHADER_BIT,
			   fs_prog);
	piglit_program_pipeline_check_status(pipeline);

	/* Link and build pipeline for Varying arrays test */
	vs_prog_arrays = format_and_link_program(GL_VERTEX_SHADER,
						 vs_arrays_code_template,
						 glsl_version);

	fs_prog_arrays = format_and_link_program(GL_FRAGMENT_SHADER,
						 fs_arrays_code_template,
						 glsl_version);

	glGenProgramPipelines(1, &pipeline_arrays);
	glUseProgramStages(pipeline_arrays,
			   GL_VERTEX_SHADER_BIT,
			   vs_prog_arrays);
	glUseProgramStages(pipeline_arrays,
			   GL_FRAGMENT_SHADER_BIT,
			   fs_prog_arrays);
	piglit_program_pipeline_check_status(pipeline_arrays);
	/* Link and build pipeline for Varying arrays test */
	if (piglit_is_extension_supported("GL_ARB_arrays_of_arrays")) {
		vs_prog_arrays_of_arrays =
			format_and_link_program(GL_VERTEX_SHADER,
						vs_aoa_code_template,
						glsl_version);

		fs_prog_arrays_of_arrays =
			format_and_link_program(GL_FRAGMENT_SHADER,
						fs_aoa_code_template,
						glsl_version);

		glGenProgramPipelines(1, &pipeline_arrays_of_arrays);
		glUseProgramStages(pipeline_arrays_of_arrays,
				   GL_VERTEX_SHADER_BIT,
				   vs_prog_arrays_of_arrays);
		glUseProgramStages(pipeline_arrays_of_arrays,
				  GL_FRAGMENT_SHADER_BIT,
				  fs_prog_arrays_of_arrays);
		piglit_program_pipeline_check_status(pipeline_arrays_of_arrays);
	}

	if (!piglit_check_gl_error(0))
		piglit_report_result(PIGLIT_FAIL);
}
