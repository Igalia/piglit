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
 * \file rendezvous_by_name_interpolation.c
 * Simple test for separate shader objects that use rendezvous-by-name.
 *
 * This test ensures that multiple interpolation qualifiers don't break
 * interface matching.
 *
 * We first test matching of a VS and FS with matching interpolation
 * qualifiers. Next we ensure that non-matching interpolation qualifiers
 * also work. Technically interpolation mismatching is only allowed
 * starting with GLSL 4.5 however its unlikely any implementation inforces
 * this constraint.
 */
#include "piglit-util-gl.h"
#include "sso-common.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;
	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

static GLuint pipeline_match[4];
static GLuint pipeline_unmatch[4];

static const char *vs_code_template =
	"#version %d\n"
	"#extension GL_ARB_separate_shader_objects: require\n"
	"#extension GL_ARB_explicit_attrib_location: require\n"
	"\n"
	"layout(location = 0) in vec4 piglit_vertex;\n"
	"\n"
	"%s out vec4 blue;\n"
	"%s out vec4 green;\n"
	"%s out vec4 red;\n"
	"\n"
	"void main()\n"
	"{\n"
	"    gl_Position = piglit_vertex;\n"
	"    red   = vec4(1, 0, 0, 0);\n"
	"    green = vec4(0, 1, 0, 0);\n"
	"    blue  = vec4(0, 0, 1, 0);\n"
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
	"%s in vec4 blue;\n"
	"%s in vec4 green;\n"
	"%s in vec4 red;\n"
	"\n"
	"void main()\n"
	"{\n"
	"    out_color = vec4(red.r, green.g, blue.b, 1);\n"
	"}\n"
	;

static const char *qualifiers[] = {
	"",
	"flat",
	"smooth",
	"noperspective"
};

enum piglit_result
piglit_display(void)
{
	static const float expected[] = {
		1.0f, 1.0f, 1.0f, 1.0f
	};
	bool pass = true;
	int i;

	glClearColor(0.1f, 0.1f, 0.1f, 0.1f);
	glClear(GL_COLOR_BUFFER_BIT);

	for (i = 0; i < 4; i++) {
		float w = 0.5;
		float x = -1.0 + w * i;
		/*
		 * Match qualifier on bottom row
		 */
		glBindProgramPipeline(pipeline_match[i]);
		piglit_draw_rect(x, -1, w, 1);

		/*
		 * Unmatch qualifier on top row
		 */
		glBindProgramPipeline(pipeline_unmatch[i]);
		piglit_draw_rect(x, 0, w, 1);
	}

	pass &= piglit_probe_rect_rgba(0, 0, piglit_width, piglit_height,
				      expected);

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

GLuint format_and_link_program_with_qualifiers(GLenum type, const char* code,
		unsigned glsl_version, unsigned q1, unsigned q2, unsigned q3)
{
	char *source;
	GLuint prog;

	(void)!asprintf(&source, code, glsl_version, qualifiers[q1], qualifiers[q2], qualifiers[q3]);
	prog = glCreateShaderProgramv(type, 1,
			(const GLchar *const *) &source);

	piglit_link_check_status(prog);
	free(source);

	return prog;
}

void piglit_init(int argc, char **argv)
{
	unsigned i;
	unsigned glsl_version;
	GLuint vs_prog[4];
	GLuint fs_prog_match[4];
	GLuint fs_prog_unmatch[4];

	piglit_require_vertex_shader();
	piglit_require_fragment_shader();
	piglit_require_GLSL_version(130); /* Support layout index on output color */
	piglit_require_extension("GL_ARB_separate_shader_objects");
	piglit_require_extension("GL_ARB_explicit_attrib_location");
	piglit_require_extension("GL_ARB_blend_func_extended");

	glsl_version = pick_a_glsl_version();

	/*
	 * Program compilation and link
	 */
	for (i = 0; i < 4; i++) {
		int next = (i+1) % 4;
		int prev = (i-1) % 4;
		printf("Compile vs_prog[%d]\n", i);
		vs_prog[i] = format_and_link_program_with_qualifiers(GL_VERTEX_SHADER,
				vs_code_template, glsl_version, prev, i, next);

		printf("Compile fs_prog_match[%d]\n", i);
		fs_prog_match[i] = format_and_link_program_with_qualifiers(GL_FRAGMENT_SHADER,
				fs_code_template, glsl_version, prev, i, next);

		printf("Compile fs_prog_unmatch[%d]\n", i);
		fs_prog_unmatch[i] = format_and_link_program_with_qualifiers(GL_FRAGMENT_SHADER,
				fs_code_template, glsl_version, next, prev, i);
	}

	/*
	 * Pipeline creation
	 */
	glGenProgramPipelines(4, pipeline_match);
	glGenProgramPipelines(4, pipeline_unmatch);
	for (i = 0; i < 4; i++) {
		glBindProgramPipeline(pipeline_match[i]);
		glUseProgramStages(pipeline_match[i],
				GL_VERTEX_SHADER_BIT, vs_prog[i]);
		glUseProgramStages(pipeline_match[i],
				GL_FRAGMENT_SHADER_BIT, fs_prog_match[i]);

		glBindProgramPipeline(pipeline_unmatch[i]);
		glUseProgramStages(pipeline_unmatch[i],
				GL_VERTEX_SHADER_BIT, vs_prog[i]);
		glUseProgramStages(pipeline_unmatch[i],
				GL_FRAGMENT_SHADER_BIT, fs_prog_unmatch[i]);
	}

	if (!piglit_check_gl_error(0))
		piglit_report_result(PIGLIT_FAIL);
}
