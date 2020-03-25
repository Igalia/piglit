/*
 * Copyright © 2020 Advanced Micro Devices, Inc.
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

/** @file issue1258.c
 *
 * This is a reproducer for issue 1258.
 * When using separable programs and ssbo a use-after-free can occur in
 * st_bind_ssbos if these 2 conditions are met:
 *   - the program has been relinked while its pipeline is not bound
 *   - a new ssbo has been attached
 *
 * Based on rendering.c test.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 32;
	config.supports_gl_core_version = 32;
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;
	config.khr_no_error_support = PIGLIT_NO_ERRORS;

PIGLIT_GL_TEST_CONFIG_END

static const char *vert_shader_text =
	"#version 130\n"
	"#extension GL_ARB_separate_shader_objects: require\n"
	"\n"
	"in vec4 piglit_vertex;\n"
	"\n"
	"void main()\n"
	"{\n"
	"   gl_Position = piglit_vertex;\n"
	"}\n";

static const char *frag_shader_text =
	"#version 130\n"
	"#extension GL_ARB_shader_storage_buffer_object: require\n"
	"#extension GL_ARB_separate_shader_objects: require\n"
	"\n"
	"buffer ssbo_color { float color; };\n"
	"\n"
	"void main()\n"
	"{\n"
	"   gl_FragColor = vec4(color);\n"
	"}\n";

static GLuint pipeline, prog_vs, prog_fs;
static GLuint buffers[2];
static GLint ssbo_size;
static GLint ssbo_index;
static float* color;

void
piglit_init(int argc, char **argv)
{
	piglit_require_extension("GL_ARB_shader_storage_buffer_object");
	piglit_require_extension("GL_ARB_separate_shader_objects");

	color = (float*) malloc(piglit_width * piglit_height * 4 * sizeof(float));
	for (int i = 0; i < (piglit_width * piglit_height * 4); i++)
		color[i] = 0.75;

	glViewport(0, 0, piglit_width, piglit_height);
	prog_vs = piglit_build_simple_program_unlinked(vert_shader_text, NULL);
	glProgramParameteri(prog_vs, GL_PROGRAM_SEPARABLE, GL_TRUE);
	glLinkProgram(prog_vs);
	piglit_link_check_status(prog_vs);
	prog_fs = piglit_build_simple_program(NULL, frag_shader_text);
	glProgramParameteri(prog_fs, GL_PROGRAM_SEPARABLE, GL_TRUE);
	glLinkProgram(prog_fs);
	piglit_link_check_status(prog_fs);

	glGenProgramPipelines(1, &pipeline);
	glUseProgramStages(pipeline, GL_VERTEX_SHADER_BIT, prog_vs);
	glUseProgramStages(pipeline, GL_FRAGMENT_SHADER_BIT, prog_fs);
	piglit_program_pipeline_check_status(pipeline);

	ssbo_index = glGetProgramResourceIndex(prog_fs,
					       GL_SHADER_STORAGE_BLOCK,
					       "ssbo_color");
	GLenum prop = GL_BUFFER_DATA_SIZE;
	glGetProgramResourceiv(prog_fs, GL_SHADER_STORAGE_BLOCK, ssbo_index,
			       1, &prop, 1, NULL, &ssbo_size);

	/* Create 2 SSBO */
	glGenBuffers(ARRAY_SIZE(buffers), buffers);
	for (int i = 0; i < ARRAY_SIZE(buffers); i++) {
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, buffers[i]);
		glBufferData(GL_SHADER_STORAGE_BUFFER, ssbo_size,
			     &color[0], GL_DYNAMIC_DRAW);
	}


	if (!piglit_check_gl_error(0))
		piglit_report_result(PIGLIT_FAIL);

	glClearColor(0.2, 0.2, 0.2, 0.2);
}

bool
draw(void)
{
	glViewport(0, 0, piglit_width, piglit_height);
	glClear(GL_COLOR_BUFFER_BIT);

	if (!piglit_check_gl_error(GL_NO_ERROR))
		return PIGLIT_FAIL;

	piglit_draw_rect(-1, -1, 2, 2);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	return piglit_probe_image_color(
		0, 0,
		piglit_width, piglit_height,
		GL_RGBA, color);
}


enum piglit_result
piglit_display(void)
{
	glBindProgramPipeline(pipeline);
	glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 0, buffers[0], 0, ssbo_size);
	glShaderStorageBlockBinding(prog_fs, ssbo_index, 0);
	if (!draw()) {
		return PIGLIT_FAIL;
	}

	glBindProgramPipeline(0);
	glLinkProgram(prog_fs);
	glBindProgramPipeline(pipeline);

	glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 0, buffers[1], 0, ssbo_size);
	glShaderStorageBlockBinding(prog_fs, ssbo_index, 0);
	if (!draw()) {
		return PIGLIT_FAIL;
	}

	piglit_present_results();

	return PIGLIT_PASS;
}
