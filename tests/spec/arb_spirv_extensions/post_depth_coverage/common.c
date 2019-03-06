/*
 * Copyright (c) 2019 Intel Corporation
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

#include "common.h"
#include "piglit-shader-test.h"

GLuint
compile_spirv_shader_from_file(GLenum target, const char *filename)
{
	GLuint shader;
	char filepath[4096];
	char *source;
	unsigned source_size;

	piglit_join_paths(filepath,
			  sizeof(filepath),
			  7, /* num parts */
			  piglit_source_dir(),
			  "tests",
			  "spec",
			  "arb_spirv_extensions",
			  "post_depth_coverage",
			  "shader_source",
			  filename);

	piglit_load_source_from_shader_test(filepath,
					    target,
					    true,
					    &source,
					    &source_size);

	shader = piglit_assemble_spirv(target,
				       strlen(source),
				       source);
	free(source);

	glSpecializeShaderARB(shader,
			      "main",
			      0, /* num specializations */
			      NULL, /* constant index */
			      NULL /* constant value */);

	return shader;
}

GLuint
build_spirv_program(const char *vert_filename,
		      const char *frag_filename)
{
	GLuint vert_shader =
		compile_spirv_shader_from_file(GL_VERTEX_SHADER,
					       vert_filename);
	GLuint frag_shader =
		compile_spirv_shader_from_file(GL_FRAGMENT_SHADER,
					       frag_filename);

	GLuint prog = piglit_link_simple_program(vert_shader, frag_shader);

	glDeleteShader(vert_shader);
	glDeleteShader(frag_shader);

	if (prog == 0)
		piglit_report_result(PIGLIT_FAIL);

	glLinkProgram(prog);

	if (!piglit_check_gl_error(GL_NO_ERROR)) {
		piglit_report_result(PIGLIT_FAIL);
	}

	return prog;
}

void
check_required_extensions(void)
{
	piglit_require_extension("GL_ARB_post_depth_coverage");
	piglit_require_extension("GL_ARB_spirv_extensions");

	int num_spirv_extensions, i;

	glGetIntegerv(GL_NUM_SPIR_V_EXTENSIONS, &num_spirv_extensions);

	for (i = 0; i < num_spirv_extensions; i++) {
		const char *ext = (const char *) glGetStringi(GL_SPIR_V_EXTENSIONS,i);
		if (strcmp(ext, "SPV_KHR_post_depth_coverage") == 0)
			return;
	}

	printf("Test requires SPV_KHR_post_depth_coverage\n");
	piglit_report_result(PIGLIT_SKIP);
}
