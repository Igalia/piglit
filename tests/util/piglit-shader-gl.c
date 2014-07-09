/*
 * Copyright © 2010 Intel Corporation
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

#if defined(_MSC_VER)
#include <windows.h>
#endif

#include "piglit-util-gl-common.h"

void
piglit_require_GLSL(void)
{
	if (piglit_get_gl_version() < 20
	    && !(piglit_is_extension_supported("GL_ARB_shader_objects")
	         && piglit_is_extension_supported("GL_ARB_shading_language_100"))) {
		printf("GLSL not supported.\n");
		piglit_report_result(PIGLIT_SKIP);
	}
}

void
piglit_require_GLSL_version(int version)
{
	bool es;
	int major, minor;

	piglit_require_GLSL();

	piglit_get_glsl_version(&es, &major, &minor);

	if (es || 100 * major + minor < version) {
		printf("GLSL %d.%d not supported.\n",
		       version / 100, version % 100);
		piglit_report_result(PIGLIT_SKIP);
	}
}

void
piglit_require_vertex_shader(void)
{
	if (piglit_get_gl_version() < 20
	    && !(piglit_is_extension_supported("GL_ARB_shader_objects")
		 && piglit_is_extension_supported("GL_ARB_vertex_shader"))) {
		printf("GLSL vertex shaders are not supported.\n");
		piglit_report_result(PIGLIT_SKIP);
	}
}

void
piglit_require_fragment_shader(void)
{
	if (piglit_get_gl_version() < 20
	    && !(piglit_is_extension_supported("GL_ARB_shader_objects")
		 && piglit_is_extension_supported("GL_ARB_fragment_shader"))) {
		printf("GLSL fragment shaders are not supported.\n");
		piglit_report_result(PIGLIT_SKIP);
	}
}

/* Same function as link_check_status but for program pipeline */
static GLboolean
program_pipeline_check_status(GLuint pipeline, FILE *output)
{
	GLchar *info = NULL;
	GLint size;
	GLint ok;

	piglit_require_extension("GL_ARB_separate_shader_objects");

	glValidateProgramPipeline(pipeline);
	glGetProgramPipelineiv(pipeline, GL_VALIDATE_STATUS, &ok);

	/* Some drivers return a size of 1 for an empty log.  This is the size
	 * of a log that contains only a terminating NUL character.
	 */
	glGetProgramPipelineiv(pipeline, GL_INFO_LOG_LENGTH, &size);
	if (size > 1) {
		info = malloc(size);
		glGetProgramPipelineInfoLog(pipeline, size, NULL, info);
	}

	if (!ok) {
		fprintf(output, "Failed to validate the pipeline: %s\n",
			(info != NULL) ? info : "<empty log>");
	}
	else if (0 && info != NULL) {
		/* Enable this to get extra linking info.
		 * Even if there's no link errors, the info log may
		 * have some remarks.
		 */
		printf("Pipeline validation warning: %s\n", info);
	}

	free(info);

	return ok;
}

GLboolean
piglit_program_pipeline_check_status(GLuint pipeline)
{
	return program_pipeline_check_status(pipeline, stderr);
}

GLboolean
piglit_program_pipeline_check_status_quiet(GLuint pipeline)
{
	return program_pipeline_check_status(pipeline, stdout);
}
