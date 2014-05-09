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

/** \file builtin-gl-sample-id.cpp
 *  This test verifies that using gl_SampleID in fragment shader program
 *  works as per ARB_sample_shading specification.
 *
 **/

#include "piglit-fbo.h"
using namespace piglit_util_fbo;

const int pattern_width = 128; const int pattern_height = 128;

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 31;
	config.supports_gl_core_version = 31;

	config.window_width = pattern_width;
	config.window_height = pattern_height;
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;

PIGLIT_GL_TEST_CONFIG_END

static int num_samples;
static unsigned prog_0, prog_1;
static Fbo multisampled_fbo, multisampled_tex;

static void
print_usage_and_exit(char *prog_name)
{
	printf("Usage: %s <num_samples>\n", prog_name);
	piglit_report_result(PIGLIT_FAIL);
}

void
compile_shader(void)
{
	static const char *vert =
		"#version 140\n"
		"in vec4 piglit_vertex;\n"
		"void main()\n"
		"{\n"
		"  gl_Position = piglit_vertex;\n"
		"}\n";
	static const char *frag_0 =
		"#version 130\n"
		"#extension GL_ARB_sample_shading : enable\n"
		"uniform int samples;\n"
		"out ivec4 out_color;\n"
		"void main()\n"
		"{\n"
		"    out_color = ivec4(0, gl_SampleID, 0, 1);\n"
		"}\n";

	static const char *frag_template =
		"#version 140\n"
		"%s\n"
		"uniform %s ms_tex;\n"
		"uniform int samples;\n"
		"out vec4 out_color;\n"
		"void main()\n"
		"{\n"
		"  int i = 0;\n"
		"  bool pass = true;\n"
		   /* Use do-while to include 'samples = 0' case. */
		"  do {\n"
		"    ivec4 sample_color =\n"
		"    texelFetch(ms_tex, ivec2(gl_FragCoord.xy)%s);\n"
		"    if (sample_color.g != i)\n"
		"      pass = false;\n"
		"    i++;\n"
		"  } while (i < samples);\n"
		"\n"
		"  if (pass)\n"
		"    out_color = vec4(0.0, 1.0, 0.0, 1.0);\n"
		"  else\n"
		"    out_color = vec4(1.0, 0.0, 0.0, 1.0);\n"
		"}\n";

	/* Compile program */
	prog_0 = piglit_build_simple_program(vert, frag_0);
	if (!piglit_link_check_status(prog_0)) {
		piglit_report_result(PIGLIT_FAIL);
	}


	char *frag_1;
	if (num_samples)
		asprintf(&frag_1, frag_template,
			 "#extension GL_ARB_texture_multisample : require",
			 "isampler2DMS", ", i");
	else
		asprintf(&frag_1, frag_template, "", "isampler2DRect", "");

	prog_1 = piglit_build_simple_program(vert, frag_1);
	if (!piglit_link_check_status(prog_1)) {
		piglit_report_result(PIGLIT_FAIL);
	}
}

void
piglit_init(int argc, char **argv)
{
	if (argc != 2)
		print_usage_and_exit(argv[0]);

	/* 1st arg: num_samples */
	char *endptr = NULL;
	num_samples = strtol(argv[1], &endptr, 0);
	if (endptr != argv[1] + strlen(argv[1]))
		print_usage_and_exit(argv[0]);

	piglit_require_extension("GL_ARB_texture_multisample");
	piglit_require_extension("GL_ARB_sample_shading");
	piglit_require_GLSL_version(140);

	/* Skip the test if num_samples > GL_MAX_SAMPLES */
	GLint max_samples;
	glGetIntegerv(GL_MAX_SAMPLES, &max_samples);
	if (num_samples > max_samples)
		piglit_report_result(PIGLIT_SKIP);

	FboConfig msConfig(num_samples, pattern_width, pattern_height);
	msConfig.color_format = GL_RGBA_INTEGER;
	msConfig.color_internalformat = GL_RGBA8UI;
	multisampled_fbo.setup(msConfig);

	msConfig.num_tex_attachments = 1;
	msConfig.num_rb_attachments = 0; /* default value is 1 */
	multisampled_tex.setup(msConfig);

	compile_shader();
	if (!piglit_check_gl_error(GL_NO_ERROR)) {
		piglit_report_result(PIGLIT_FAIL);
	}
}

bool test_builtin_sample_id(Fbo ms_fbo)
{
	int samples;
	bool result = true;
	float expected[4] = {0.0, 1.0, 0.0, 1.0};

	glUseProgram(prog_0);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, ms_fbo.handle);
	glGetIntegerv(GL_SAMPLES, &samples);
	glClear(GL_COLOR_BUFFER_BIT);
	glUniform1i(glGetUniformLocation(prog_0, "samples"), samples);
        piglit_draw_rect(-1, -1, 2, 2);

	if(ms_fbo.config.num_tex_attachments == 0) {
		/* Blit the framebuffer with multisample renderbuffer attachment
		 * into the framebuffer with multisample texture attachment.
		 */
		glBindFramebuffer(GL_READ_FRAMEBUFFER, ms_fbo.handle);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, multisampled_tex.handle);
		glClear(GL_COLOR_BUFFER_BIT);
		glBlitFramebuffer(0, 0,
				  ms_fbo.config.width,
				  ms_fbo.config.height,
				  0, 0,
				  ms_fbo.config.width,
				  ms_fbo.config.height,
				  GL_COLOR_BUFFER_BIT, GL_NEAREST);
	}

	glBindFramebuffer(GL_READ_FRAMEBUFFER, multisampled_tex.handle);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, piglit_winsys_fbo);
	glClear(GL_COLOR_BUFFER_BIT);

	glUseProgram(prog_1);
	glUniform1i(glGetUniformLocation(prog_1, "ms_tex"), 0);
	glUniform1i(glGetUniformLocation(prog_1, "samples"), samples);
	piglit_draw_rect(-1, -1, 2, 2);

	glBindFramebuffer(GL_READ_FRAMEBUFFER, piglit_winsys_fbo);
	result = piglit_probe_rect_rgba(0, 0, pattern_width,
					pattern_width, expected)
               && result;
	piglit_present_results();
	printf("FBO attachment = %s, result = %s\n",
	       ms_fbo.config.num_tex_attachments > 0 ?
	       "TEXTURE" :
	       "RENDERBUFFER",
	       result ? "pass" : "fail");
	return result;
}

enum piglit_result
piglit_display()
{
	bool pass = true;
	pass = test_builtin_sample_id(multisampled_tex) && pass;
	pass = test_builtin_sample_id(multisampled_fbo) && pass;
	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}
