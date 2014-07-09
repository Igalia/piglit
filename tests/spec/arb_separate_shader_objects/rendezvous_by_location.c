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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

/**
 * \file rendezvous_by_location.c
 * Simple test for separate shader objects that use rendezvous-by-location.
 *
 * There are two ways one might expect rendezvous-by-location to fail.  One
 * predicatble failure mode is for variables between two program objects to be
 * linked in the order they appear in the shader text.  Another predicatble
 * failure mode is for variables between two program objects to be linked by
 * name.
 *
 * This test tries both modes using a single fragement shader program.  This
 * program outputs two varibles, a and b, with locations specified.  Two
 * fragment shader programs are created, each having input variables a and b,
 * with locations specified.  In the first case, a and b are listed in the
 * same order as in the vertex shader, but the locations are reversed (vertex
 * shader output a has the location of fragment shader input b).  In the
 * second case, a and b are list in the reverse order as in the vertex shader.
 * However, the assigned locations are the same as in the other fragment
 * shader.
 */
#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;
	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

static GLuint pipeline_same_declaration_order;
static GLuint pipeline_same_location_order;

static const char *vs_code_template =
	"#version %d\n"
	"#extension GL_ARB_separate_shader_objects: require\n"
	"#extension GL_ARB_explicit_attrib_location: require\n"
	"\n"
	"layout(location = 0) in vec4 piglit_vertex;\n"
	"\n"
	"layout(location = 2) out vec3 a;\n"
	"layout(location = 3) out vec3 b;\n"
	"\n"
	"void main()\n"
	"{\n"
	"    gl_Position = piglit_vertex;\n"
	"    a = vec3(0, 0, 1);\n"
	"    b = vec3(1, 0, 0);\n"
	"}\n"
	;

static const char *fs_code_same_declaration_order_template =
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
	"layout(location = 3) in vec3 a; /* should get vec3(1, 0, 0) */\n"
	"layout(location = 2) in vec3 b; /* should get vec3(0, 0, 1) */\n"
	"\n"
	"void main()\n"
	"{\n"
	"    out_color = vec4(cross(b, a), 1);\n"
	"}\n"
	;

static const char *fs_code_same_location_order_template =
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
	"layout(location = 2) in vec3 b; /* should get vec3(0, 0, 1) */\n"
	"layout(location = 3) in vec3 a; /* should get vec3(1, 0, 0) */\n"
	"\n"
	"void main()\n"
	"{\n"
	"    out_color = vec4(cross(b, a), 1);\n"
	"}\n"
	;

enum piglit_result
piglit_display(void)
{
	static const float expected[] = {
		0.0f, 1.0f, 0.0f, 1.0f
	};
	bool pass;

	glClearColor(0.1f, 0.1f, 0.1f, 0.1f);
	glClear(GL_COLOR_BUFFER_BIT);

	glBindProgramPipeline(pipeline_same_declaration_order);
	piglit_draw_rect(-1, -1, 1, 2);

	glBindProgramPipeline(pipeline_same_location_order);
	piglit_draw_rect(0, -1, 1, 2);
	pass = piglit_probe_rect_rgba(0, 0, piglit_width, piglit_height,
				      expected);

	piglit_present_results();
	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void piglit_init(int argc, char **argv)
{
	unsigned glsl_version;
	GLuint vs_prog;
	GLuint fs_prog_same_declaration_order;
	GLuint fs_prog_same_location_order;
	bool es;
	int glsl_major;
	int glsl_minor;
	char *source;

	piglit_require_vertex_shader();
	piglit_require_fragment_shader();
	piglit_require_extension("GL_ARB_separate_shader_objects");
	piglit_require_extension("GL_ARB_explicit_attrib_location");

	/* Some NVIDIA drivers have issues with layout qualifiers, 'in'
	 * keywords, and 'out' keywords in "lower" GLSL versions.  If the
	 * driver supports GLSL >= 1.40, use 1.40.  Otherwise, pick the
	 * highest version that the driver supports.
	 */
	piglit_get_glsl_version(&es, &glsl_major, &glsl_minor);
	glsl_version = ((glsl_major * 100) + glsl_minor) >= 140
		? 140 : ((glsl_major * 100) + glsl_minor);

	asprintf(&source, vs_code_template, glsl_version);
	vs_prog = glCreateShaderProgramv(GL_VERTEX_SHADER, 1,
					 (const GLchar *const *) &source);
	piglit_link_check_status(vs_prog);
	free(source);

	asprintf(&source, fs_code_same_declaration_order_template, glsl_version);
	fs_prog_same_declaration_order =
		glCreateShaderProgramv(GL_FRAGMENT_SHADER, 1,
				       (const GLchar *const *) &source);
	piglit_link_check_status(fs_prog_same_declaration_order);
	free(source);

	asprintf(&source, fs_code_same_location_order_template, glsl_version);
	fs_prog_same_location_order =
		glCreateShaderProgramv(GL_FRAGMENT_SHADER, 1,
				       (const GLchar *const *) &source);
	piglit_link_check_status(fs_prog_same_location_order);
	free(source);

	glGenProgramPipelines(1, &pipeline_same_declaration_order);
	glUseProgramStages(pipeline_same_declaration_order,
			   GL_VERTEX_SHADER_BIT,
			   vs_prog);
	glUseProgramStages(pipeline_same_declaration_order,
			   GL_FRAGMENT_SHADER_BIT,
			   fs_prog_same_declaration_order);
	piglit_program_pipeline_check_status(pipeline_same_declaration_order);

	glGenProgramPipelines(1, &pipeline_same_location_order);
	glUseProgramStages(pipeline_same_location_order,
			   GL_VERTEX_SHADER_BIT,
			   vs_prog);
	glUseProgramStages(pipeline_same_location_order,
			   GL_FRAGMENT_SHADER_BIT,
			   fs_prog_same_location_order);
	piglit_program_pipeline_check_status(pipeline_same_location_order);

	if (!piglit_check_gl_error(0))
		piglit_report_result(PIGLIT_FAIL);
}
