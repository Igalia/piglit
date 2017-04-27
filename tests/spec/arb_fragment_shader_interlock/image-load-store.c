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
 * A test to check whether GL_ARB_fragment_shader_interlock operates as
 * expected. This test simulates blending behaviour by using image loads/stores
 * to a 3D texture. The blending formula used is:
 * result = current_alpha * current_color + (1 - current_alpha) * previous_color
 * Multisampling is also enabled and tested at 2, 4, 8 and 16.
 */

PIGLIT_GL_TEST_CONFIG_BEGIN
	config.supports_gl_compat_version = 42;
	config.supports_gl_core_version = 42;
	config.window_width = 100;
	config.window_height = 100;
	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DEPTH |
		PIGLIT_GL_VISUAL_DOUBLE;
PIGLIT_GL_TEST_CONFIG_END

static GLuint prog, vao, tex_frame, tex_blend, fbo;

static GLuint
make_fbo(void)
{
	GLuint fbo;
	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo );
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, tex_frame);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
		GL_TEXTURE_2D_MULTISAMPLE, tex_frame, 0);

	return fbo;
}

static GLuint
make_shader_program(void)
{
	static const char *vs_text =
		"#version 430\n"
		"layout(location = 0) in vec4 pos_in;\n"
		"layout(location = 1) in vec4 col_in;\n"
		"smooth out vec4 col_vary;\n"
		"void main()\n"
		"{\n"
		"	gl_Position = pos_in;\n"
		"	col_vary = col_in;\n"
		"}\n";

	static const char *fs_text =
		"#version 430\n"
		"#extension GL_ARB_fragment_shader_interlock: require\n"
		"layout(pixel_interlock_ordered) in;\n"
		"layout(rgba32f, binding = 0) uniform image3D img_output;\n"
		"layout(location = 1) uniform int sample_rate;\n"
		"smooth in vec4 col_vary;\n"
		"out vec4 col_out;\n"
		"void main()\n"
		"{\n"
		"	vec4 result = vec4(0.0, 0.0, 0.0, 1.0);\n"
		"	ivec3 current_sample_coord = ivec3(gl_FragCoord.x, gl_FragCoord.y, gl_SampleID);\n"
		"	ivec3 result_coord = ivec3(gl_FragCoord.x, gl_FragCoord.y, sample_rate);\n"
		"	int i;\n"
		"	beginInvocationInterlockARB();\n"
		"	vec4 current_sample_color = imageLoad(img_output, current_sample_coord);\n"
		"	result.rgb += col_vary.a * col_vary.rgb + (1 - col_vary.a) * current_sample_color.rgb;\n"
		"	imageStore(img_output, current_sample_coord, result);\n"
		"\n"
		"	for (i = 0; i < sample_rate; i++) {\n"
		"		if (i != gl_SampleID) {\n"
		"			ivec3 sample_coord = ivec3(gl_FragCoord.x, gl_FragCoord.y, i);\n"
		"			vec4 sample_color = imageLoad(img_output, sample_coord);\n"
		"			result.rgb += sample_color.rgb;\n"
		"		}\n"
		"	}\n"
		"	result.rgb /= sample_rate;\n"
		"	imageStore(img_output, result_coord, result);\n"
		"	endInvocationInterlockARB();\n"
		"	col_out = result;\n"
		"}\n";

	GLuint prog;

	prog = piglit_build_simple_program(vs_text, fs_text);
	glUseProgram(prog);

	glBindAttribLocation(prog, 0, "pos_in");
	glBindAttribLocation(prog, 1, "col_in");

	glLinkProgram(prog);

	if (!piglit_check_gl_error(GL_NO_ERROR)) {
		piglit_report_result(PIGLIT_FAIL);
	}

	return prog;
}

static GLuint
make_texture_buffer(void)
{
	GLuint tex;

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, tex);
	glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 2,
		GL_RGBA32F, piglit_width, piglit_height, false);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, tex);

	return tex;
}

static GLuint
make_texture_blend(void)
{
	GLuint tex;

	glGenTextures(1, &tex);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_3D, tex);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glBindImageTexture(0, tex, 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA32F);

	return tex;
}

static GLuint
make_vao(void)
{
	static const float pos_col[18][6] = {
		{ -1.0, -1.0, 0.0, 1.0, 0.0, 0.25 },
		{  0.0, -1.0, 0.0, 1.0, 0.0, 0.25 },
		{  0.0,  1.0, 0.0, 1.0, 0.0, 0.25 },
		{  0.0,  1.0, 0.0, 1.0, 0.0, 0.25 },
		{ -1.0,  1.0, 0.0, 1.0, 0.0, 0.25 },
		{ -1.0, -1.0, 0.0, 1.0, 0.0, 0.25 },
		{ -1.0, -1.0, 1.0, 0.0, 0.0, 0.25 },
		{  1.0, -1.0, 1.0, 0.0, 0.0, 0.25 },
		{  1.0,  1.0, 1.0, 0.0, 0.0, 0.25 },
		{  1.0,  1.0, 1.0, 0.0, 0.0, 0.25 },
		{ -1.0,  1.0, 1.0, 0.0, 0.0, 0.25 },
		{ -1.0, -1.0, 1.0, 0.0, 0.0, 0.25 },
		{ -1.0, -1.0, 0.0, 0.0, 1.0, 0.25 },
		{  1.0, -1.0, 0.0, 0.0, 1.0, 0.25 },
		{  1.0,  1.0, 0.0, 0.0, 1.0, 0.25 },
		{  1.0,  1.0, 0.0, 0.0, 1.0, 0.25 },
		{ -1.0,  1.0, 0.0, 0.0, 1.0, 0.25 },
		{ -1.0, -1.0, 0.0, 0.0, 1.0, 0.25 }
	};

	const int stride = sizeof(pos_col[0]);
	GLuint vbo, vao;

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(pos_col), pos_col, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, stride, (void *) 0);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, stride,
		(void *)(sizeof(float) * 2));

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	if (!piglit_check_gl_error(GL_NO_ERROR)) {
		piglit_report_result(PIGLIT_FAIL);
	}

	return vao;
}

void
piglit_init(int argc, char **argv)
{
	piglit_require_extension("GL_ARB_fragment_shader_interlock");

	glEnable(GL_MULTISAMPLE);
	glDisable(GL_CULL_FACE);
	glClearColor(0.0, 0.0, 0.0, 1.0);

	prog = make_shader_program();
	vao = make_vao();
	tex_frame = make_texture_buffer();
	fbo = make_fbo();
	tex_blend = make_texture_blend();
}

enum piglit_result
piglit_display(void)
{
	int samples[4] = { 2, 4, 8, 16 };
	bool pass = true;
	uint i, j, k;
	uint result1[4] = { 47, 35, 63, 255 };
	uint result2[4] = { 47, 0, 63, 255 };
	int max_samples;

	glViewport(0, 0, piglit_width, piglit_height);
	glGetIntegerv(GL_MAX_SAMPLES, &max_samples);

	for (i = 0; i < 4 && samples[i] <= max_samples; i++) {
		GLfloat *tex_data = calloc(piglit_width * piglit_height *
			(samples[i] + 1) * 4, sizeof(GLfloat));

		glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA32F, piglit_width, piglit_height,
			samples[i] + 1, 0, GL_RGBA, GL_FLOAT, tex_data);

		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, tex_frame);
		glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples[i],
			GL_RGBA8, piglit_width, piglit_height, false);
		glUniform1i(1, samples[i]);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT |
			GL_STENCIL_BUFFER_BIT);

		glUseProgram(prog);
		glDrawArrays(GL_TRIANGLES, 0, 18);

		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
		glDrawBuffer(GL_BACK);
		glBlitFramebuffer(0, 0, piglit_width, piglit_height, 0, 0, piglit_width,
			piglit_height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
		pass = piglit_check_gl_error(GL_NO_ERROR) && pass;
		piglit_present_results();

		glGetTexImage(GL_TEXTURE_3D, 0, GL_RGBA, GL_FLOAT, tex_data);
		for (j = 0; j < piglit_height; j++) {
			for (k = 0; k < piglit_width; k++) {
				uint l = ((piglit_width * piglit_height * samples[i]) +
					(j * piglit_width) + k) * 4;
				uint r = fabs(tex_data[l]) * 255;
				uint g = fabs(tex_data[l + 1]) * 255;
				uint b = fabs(tex_data[l + 2]) * 255;
				uint a = fabs(tex_data[l + 3]) * 255;

				if ((k < piglit_width / 2) && (r != result1[0] ||
					  g != result1[1] || b != result1[2] || a != result1[3])) {
					printf("observed %u %u     %u %u %u %u\n", j, k, r,
						g, b, a);
					printf("expected %u %u     %u %u %u %u\n", j, k,
						result1[0], result1[1], result1[2], result1[3]);
					pass = false;
					break;
				}

				if ((k > piglit_width / 2) && (r != result2[0] ||
					  g != result2[1] || b != result2[2] || a != result2[3])) {
					printf("observed %u %u     %u %u %u %u\n", j, k, r,
						g, b, a);
					printf("expected %u %u     %u %u %u %u\n", j, k,
						result1[0], result1[1], result1[2], result1[3]);
					pass = false;
					break;
				}
			}
			if (!pass)
				break;
		}

		free(tex_data);
		pass = piglit_check_gl_error(GL_NO_ERROR) && pass;
		if (!pass)
			break;
	}

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}
