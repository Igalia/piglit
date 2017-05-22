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

#include "piglit-util-gl.h"

/*
 * A test to check whether the right values are written to gl_SampleMaskIn
 * when ARB_post_depth_coverage and multisampling are enabled. Tests at
 * 2, 4, 8, 16 sample rates.
 */

PIGLIT_GL_TEST_CONFIG_BEGIN
	config.supports_gl_compat_version = 43;
	config.supports_gl_core_version = 43;
	config.window_width = 160;
	config.window_height = 160;
	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DEPTH |
		PIGLIT_GL_VISUAL_DOUBLE;
PIGLIT_GL_TEST_CONFIG_END

static GLuint prog1, prog2, vao, ssbo, tex_color, tex_depth, fbo;
static GLint *sample_mask;

static const char *vs_text =
	"#version 430\n"
	"in vec4 pos_in;\n"
	"void main()\n"
	"{\n"
	"	gl_Position = pos_in;\n"
	"}\n";

static const char *fs_text1 =
	"#version 430\n"
	"out vec4 color;\n"
	"void main()\n"
	"{\n"
	"  gl_FragDepth = 0.5f;\n"
	"	color = vec4(0.0, 1.0, 0.0, 1.0);\n"
	"}\n";

static const char *fs_text2 =
	"#version 430\n"
	"#extension GL_ARB_post_depth_coverage: enable\n"
	"out vec4 color;\n"
	"layout(early_fragment_tests) in;\n"
	"layout(post_depth_coverage) in;\n"
	"layout(std430, binding = 0) buffer MaskOutput {\n"
	"	int data[];\n"
	"} mask_output;\n"
	"layout(location = 1) uniform int width;\n"
	"layout(location = 2) uniform int samples;\n"
	"void main()\n"
	"{\n"
	"	int index = int(gl_FragCoord.y) * width + int(gl_FragCoord.x);\n"
	"	atomicAdd(mask_output.data[index], bitCount(gl_SampleMaskIn[0]));\n"
	"	color = vec4(1.0, 0.0, 0.0, 1.0);\n"
	"}\n";

static GLuint
make_shader_program1(void)
{
	GLuint prog;

	prog = piglit_build_simple_program(vs_text, fs_text1);
	glUseProgram(prog);

	glBindAttribLocation(prog, 0, "pos_in");

	glLinkProgram(prog);

	if (!piglit_check_gl_error(GL_NO_ERROR)) {
		piglit_report_result(PIGLIT_FAIL);
	}

	return prog;
}

static GLuint
make_shader_program2(void)
{
	GLuint prog;

	prog = piglit_build_simple_program(vs_text, fs_text2);
	glUseProgram(prog);

	glBindAttribLocation(prog, 0, "pos_in");

	glLinkProgram(prog);

	if (!piglit_check_gl_error(GL_NO_ERROR)) {
		piglit_report_result(PIGLIT_FAIL);
	}

	return prog;
}

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
make_fbo(void)
{
	GLuint fbo;
	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo );
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, tex_color);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
		GL_TEXTURE_2D_MULTISAMPLE, tex_color, 0);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, tex_depth);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
		GL_TEXTURE_2D_MULTISAMPLE, tex_depth, 0);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);

	return fbo;
}

static GLuint
make_texture_color(void)
{
	GLuint tex;

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, tex);
	glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 2,
		GL_RGBA32F, piglit_width, piglit_height, false);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);

	return tex;
}

static GLuint
make_texture_depth(void)
{
	GLuint tex;

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, tex);
	glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 2,
		GL_DEPTH24_STENCIL8, piglit_width, piglit_height, false);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);

	return tex;
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
	piglit_require_extension("GL_ARB_post_depth_coverage");

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_STENCIL_TEST);
	glEnable(GL_MULTISAMPLE);
	glClearColor(0.0, 0.0, 0.0, 1.0);

	prog1 = make_shader_program1();
	prog2 = make_shader_program2();
	vao = make_vao();
	ssbo = make_ssbo();
	tex_color = make_texture_color();
	tex_depth = make_texture_depth();
	fbo = make_fbo();
}


enum piglit_result
piglit_display(void)
{
	int samples[4] = { 2, 4, 8, 16 };
	int max_samples;
	bool pass = true;
	int i, j, k;

	glViewport(0, 0, piglit_width, piglit_height);
	glGetIntegerv(GL_MAX_SAMPLES, &max_samples);

	for (j = 0; j < 4 && samples[j] <= max_samples; j++) {
		sample_mask = (GLint*) calloc (piglit_width * piglit_height,
			sizeof(GLint));
		glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(GLint) * piglit_width *
			piglit_height, &sample_mask[0], GL_DYNAMIC_DRAW);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo);

		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, tex_color);
		glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples[j],
			GL_RGBA8, piglit_width, piglit_height, false);
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, tex_depth);
		glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples[j],
			GL_DEPTH24_STENCIL8, piglit_width, piglit_height, false);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT |
			GL_STENCIL_BUFFER_BIT);

		glUseProgram(prog1);
		glStencilFunc(GL_ALWAYS, 1, 0xFF);
		glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		glUseProgram(prog2);
		glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
		glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
		glUniform1i(1, piglit_width);
		glUniform1i(2, samples[j]);
		glDrawArrays(GL_TRIANGLES, 6, 6);

		glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(GLint) *
			piglit_width * piglit_height, sample_mask);

		for (i = 0; i < piglit_width; i++) {
			for (k = 0; k < piglit_height; k++) {
				if (i >= piglit_width / 2) {
					if (sample_mask[piglit_width * k + i] != samples[j]) {
						pass = false;
						break;
					}
				} else {
					if (sample_mask[piglit_width * k + i] != 0) {
						pass = false;
						break;
					}
				}
			}

			if (!pass)
				break;
		}

		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
		glDrawBuffer(GL_BACK);
		glBlitFramebuffer(0, 0, piglit_width, piglit_height, 0, 0, piglit_width,
			piglit_height, GL_COLOR_BUFFER_BIT, GL_NEAREST);

		piglit_present_results();
		free(sample_mask);
		if (!pass)
			break;
	}

	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}
