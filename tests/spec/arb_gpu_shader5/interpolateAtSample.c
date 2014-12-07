/*
 * Copyright (c) 2013 Intel Corporation
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
 * @file interpolateAtSample.c
 *
 * Test ARB_gpu_shader5 interpolateAtSample builtin
 *
 * Tests that interpolateAtSample(x) gives the correct result.
 *
 * R, 1-G channels are interesting; a correct implementation should produce
 * (0,1,0) in all pixels.
 *
 * We require 3.2, so the following assumptions are made:
 * - MAX_SAMPLES >= 4 (although we dont require exactly 4 samples; if only an
 *   8x mode is supported, the test should still work)
 * - GLSL 1.50 and Multisample textures are supported.
 */


#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_core_version = 32;

	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGB;

PIGLIT_GL_TEST_CONFIG_END

GLuint ms_fbo, vao, bo;
GLint draw_prog, test_prog;
GLint sample_pos_loc;

float green[] = { 0, 1, 0 };
float verts[][2] = { { -1, -1 }, { 1, -1 }, { -1, 1 }, { 1, 1 }, };
#define GAIN "5"			/* multiplier for absolute difference; make the error more visible. */


enum piglit_result
piglit_display(void)
{
	bool pass = true;
	GLfloat pos[2];
	glGetMultisamplefv(GL_SAMPLE_POSITION, 0, pos);
	/* API gives us 0..1 sample position; get it into
	 * -0.5..0.5 space.
	 */
	pos[0] -= 0.5f;
	pos[1] -= 0.5f;

	glViewport(0, 0, 64, 64);

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, ms_fbo);
	glUseProgram(draw_prog);
	glUniform2fv(sample_pos_loc, 1, pos);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, piglit_winsys_fbo);
	glUseProgram(test_prog);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	pass = piglit_probe_rect_rgb(0, 0, 64, 64, green) && pass;

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}


void
piglit_init(int argc, char**argv)
{
	GLuint tex;
	piglit_require_extension("GL_ARB_gpu_shader5");

	glGenFramebuffers(1, &ms_fbo);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, ms_fbo);
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, tex);
	glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4,
				GL_RGBA, 64, 64, GL_TRUE);
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
			       GL_TEXTURE_2D_MULTISAMPLE, tex, 0);

	if (!piglit_check_gl_error(GL_NO_ERROR)) {
		printf("fbo setup failed.\n");
		piglit_report_result(PIGLIT_SKIP);
	}

	/* Test quantity varies from -1 to +1 over 64 pixels --
	 * so moving 1px changes its value by 1/32.
	 */
	draw_prog = piglit_build_simple_program(
		"#version 150\n"
		"uniform vec2 sample_pos;\n"
		"in vec2 p;\n"
		"out vec2 test;\n"
		"out vec2 ref;\n"
		"void main() {\n"
		"	gl_Position = vec4(p, 0, 1);\n"
		"	test = p;\n"
		"	ref = p;\n"
		"	ref.xy += sample_pos / 32;\n"
		"}\n",

		"#version 150\n"
		"#extension GL_ARB_gpu_shader5: require\n"
		"const int sample_id = 0;\n"
		"in vec2 test;\n"
		"in vec2 ref;\n"
		"void main() {\n"
		"	gl_FragColor = vec4(" GAIN " * abs(\n"
		"		interpolateAtSample(test, sample_id) - ref), 0, 1);\n"
		"}\n");
	if (!draw_prog) {
		printf("draw_prog compile/link failed\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	test_prog = piglit_build_simple_program(
		"#version 150\n"
		"in vec2 p;\n"
		"void main() {\n"
		"	gl_Position = vec4(p, 0, 1);\n"
		"}\n",

		"#version 150\n"
		"uniform sampler2DMS s;\n"
		"void main() {\n"
		"	vec4 temp = \n"
		"		texelFetch(s, ivec2(gl_FragCoord.xy), 0) +\n"
		"		texelFetch(s, ivec2(gl_FragCoord.xy), 1) +\n"
		"		texelFetch(s, ivec2(gl_FragCoord.xy), 2) +\n"
		"		texelFetch(s, ivec2(gl_FragCoord.xy), 3);\n"
		"	gl_FragColor = vec4(temp.x, 1-temp.y, temp.z, temp.w);\n"
		"}\n");
	if (!test_prog) {
		printf("test_prog compile/link failed\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	sample_pos_loc = glGetUniformLocation(draw_prog, "sample_pos");

	glUseProgram(test_prog);
	glUniform1i(glGetUniformLocation(test_prog, "s"), 0);

	if (!piglit_check_gl_error(GL_NO_ERROR)) {
		printf("shader setup failed\n");
		piglit_report_result(PIGLIT_SKIP);
	}

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	
	glEnableVertexAttribArray(0);
	glGenBuffers(1, &bo);
	glBindBuffer(GL_ARRAY_BUFFER, bo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid const *)0);
}

