/*
 * Copyright (c) 2015 Intel Corporation.
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

/*
 * A rudimentary test to check whether the correct values are being written
 * to gl_SampleMaskIn when ARB_post_depth_coverage is enabled.
 *
 * Same test than tests/spec/arb_post_depth_coverage/basic.c but loading
 * SPIR-V shaders instead. Requires SPV_KHR_post_depth_coverage.
 */

#include "common.h"

PIGLIT_GL_TEST_CONFIG_BEGIN
	config.supports_gl_core_version = 33;
	config.window_width = 160;
	config.window_height = 160;
	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DEPTH | PIGLIT_GL_VISUAL_DOUBLE;
PIGLIT_GL_TEST_CONFIG_END

#define VS_FILENAME "vs.shader_source"
#define FS_1_FILENAME "fs.shader_source"
#define FS_2_FILENAME "basic-fs.shader_source"

static GLuint prog1, prog2, vao, ssbo;
static GLint *sample_mask;

static GLuint
make_ssbo(void)
{
	GLuint ssbo;
	glGenBuffers(1, &ssbo);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);

	if (!piglit_check_gl_error(GL_NO_ERROR)) {
		piglit_report_result(PIGLIT_FAIL);
	}

	return ssbo;
}

static GLuint
make_vao(void)
{
	static const float pos_tc[12][2] = {
		{ -1.0, -1.0 },
		{  0.0, -1.0 },
		{  0.0,  1.0 },
		{  0.0,  1.0 },
		{ -1.0,  1.0 },
		{ -1.0, -1.0 },
		{ -1.0, -1.0 },
		{  1.0, -1.0 },
		{  1.0,  1.0 },
		{  1.0,  1.0 },
		{ -1.0,  1.0 },
		{ -1.0, -1.0 }
	};
	const int stride = sizeof(pos_tc[0]);
	GLuint vbo, vao;

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(pos_tc), pos_tc, GL_STATIC_DRAW);
	piglit_check_gl_error(GL_NO_ERROR);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, stride, (void *) 0);

	glEnableVertexAttribArray(0);

	if (!piglit_check_gl_error(GL_NO_ERROR)) {
		piglit_report_result(PIGLIT_FAIL);
	}

	return vbo;
}

void
piglit_init(int argc, char **argv)
{
	check_required_extensions();

	prog1 = build_spirv_program(VS_FILENAME, FS_1_FILENAME);
	prog2 = build_spirv_program(VS_FILENAME, FS_2_FILENAME);
	vao = make_vao();
	ssbo = make_ssbo();
}


enum piglit_result
piglit_display(void)
{
	float green[4] = {0.0, 1.0, 0.0, 1.0};
	float red[4] = {1.0, 0.0, 0.0, 1.0};
	bool pass = true;
	int i, j;

	glEnable(GL_DEPTH_TEST);
	glViewport(0, 0, piglit_width, piglit_height);
	sample_mask = (GLint*) malloc (sizeof(GLint) * (piglit_width * piglit_height));
	for (i = 0; i < piglit_width * piglit_height; i++) {
		sample_mask[i] = 0;
	}
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GLint) * (piglit_width *
		piglit_height), &sample_mask[0], GL_DYNAMIC_COPY);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, ssbo);

	glClearColor(0.0, 0.0, 0.0, 1.0);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_STENCIL_TEST);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	glUseProgram(prog1);
	glStencilFunc(GL_ALWAYS, 1, 0xFF);
	glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glUseProgram(prog2);
	glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
	glUniform1i(2, piglit_width);
	glDrawArrays(GL_TRIANGLES, 6, 6);

	glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(GLint) *
		piglit_width * piglit_height, &sample_mask[0]);

	for (i = 0; i < piglit_width; i++) {
		for (j = 0; j < piglit_height; j++) {
			if (i >= piglit_width / 2) {
				if (sample_mask[piglit_width * j + i] != 1) {
					pass = false;
					break;
				}
			} else {
				if (sample_mask[piglit_width * j + i] != 0) {
					pass = false;
					break;
				}
			}
		}
	}

	pass = piglit_probe_rect_rgba(0, 0, piglit_width / 2, piglit_height,
		green) && pass;
	pass = piglit_probe_rect_rgba(piglit_width / 2, 0, piglit_width / 2,
		piglit_height, red) && pass;
	piglit_present_results();

	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

	free(sample_mask);
	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}
