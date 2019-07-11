/*
 * Copyright (c) 2019 Timothy Arceri
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
 * Verify that a binary program that was originally linked with the
 * GL_PROGRAM_SEPARABLE parameter set does not trigger GL pipeline validation
 * errors when calling UseProgramStages(). In other word this test makes sure
 * we store/restore the state of the program parameter GL_PROGRAM_SEPARABLE.
 */

#include "piglit-util-gl.h"
#include "gpb-common.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 30;
	config.window_visual = PIGLIT_GL_VISUAL_RGB;
	config.khr_no_error_support = PIGLIT_NO_ERRORS;

PIGLIT_GL_TEST_CONFIG_END

void
piglit_init(int argc, char **argv)
{
	GLuint vs_prog, fs_prog;
	GLint ok;
	static GLuint pipeline;

	static const char *vs_code =
		"void main()\n"
		"{\n"
		"    gl_Position = gl_Vertex;\n"
		"}\n";
	static const char *fs_code =
		"#version 120\n"
		"uniform vec4 color = vec4(0.0, 1.0, 0.0, 1.0);\n"
		"\n"
		"void main()\n"
		"{\n"
		"        gl_FragColor = color;\n"
		"}\n";

	piglit_require_extension("GL_ARB_get_program_binary");
	piglit_require_extension("GL_ARB_separate_shader_objects");

	vs_prog = glCreateShaderProgramv(GL_VERTEX_SHADER, 1, &vs_code);
	piglit_link_check_status(vs_prog);

	fs_prog = glCreateShaderProgramv(GL_FRAGMENT_SHADER, 1, &fs_code);
	piglit_link_check_status(fs_prog);


	glGenProgramPipelines(1, &pipeline);
	glUseProgramStages(pipeline, GL_VERTEX_SHADER_BIT, vs_prog);
	glUseProgramStages(pipeline, GL_FRAGMENT_SHADER_BIT, fs_prog);
	piglit_program_pipeline_check_status(pipeline);

	gpb_save_restore_sso(&vs_prog, pipeline, GL_VERTEX_SHADER_BIT);
	gpb_save_restore_sso(&fs_prog, pipeline, GL_FRAGMENT_SHADER_BIT);

	glValidateProgramPipeline(pipeline);
	glGetProgramPipelineiv(pipeline, GL_VALIDATE_STATUS, &ok);

	if (!ok)
		piglit_report_result(PIGLIT_FAIL);

	piglit_report_result(PIGLIT_PASS);
}

enum piglit_result
piglit_display(void)
{
	return PIGLIT_FAIL;
}
