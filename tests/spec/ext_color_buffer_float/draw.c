/*
 * Copyright (C) 2019 Ilia Mirkin
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
 * \file draw.c
 *
 * Test that GL_EXT_color_buffer_float's blending restriction is
 * properly respected. Either the implementation rejects draws with
 * FP32 blending, or it exposes GL_EXT_float_blend.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN
	config.supports_gl_es_version = 30;
	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;
PIGLIT_GL_TEST_CONFIG_END

#define SIZE 128

enum piglit_result
piglit_display(void)
{
	return PIGLIT_SKIP; /* Never reached */
}

static void
check_draw_success()
{
	static const float green[4] = {0, 1, 0, 1};
	float *buffer;
	int i;

	if (!piglit_check_gl_error(GL_NO_ERROR)) {
		printf("FAIL: Basic drawing\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	glReadBuffer(GL_COLOR_ATTACHMENT0);
	buffer = malloc(SIZE * SIZE * 4 * sizeof(float));
	glReadPixels(0, 0, 128, 128, GL_RGBA, GL_FLOAT, buffer);
	for (i = 0; i < SIZE * SIZE; i++) {
		float *pixel = &buffer[4 * i];
		if (memcmp(pixel, green, sizeof(green))) {
			printf("FAIL: Basic draw color at (%d, %d)\n", i / SIZE, i % SIZE);
			printf("Expected: %f %f %f %f\n", green[0], green[1], green[2], green[3]);
			printf("Actual  : %f %f %f %f\n", pixel[0], pixel[1], pixel[2], pixel[3]);
			piglit_report_result(PIGLIT_FAIL);
		}
	}
	free(buffer);

	glClear(GL_COLOR_BUFFER_BIT);
}

static void
check_blend(bool float_blend)
{
	if (float_blend) {
		check_draw_success();
	} else if (!piglit_check_gl_error(GL_INVALID_OPERATION)) {
		printf("FAIL: Unexpected draw success in presence of blend.\n");
		piglit_report_result(PIGLIT_FAIL);
	}
}

void
piglit_init(int argc, char **argv)
{
	GLint prog;
	GLuint fb, rb[2];
	GLenum ret;
	bool float_blend, indexed;

	piglit_require_extension("GL_EXT_color_buffer_float");
	float_blend = piglit_is_extension_supported("GL_EXT_float_blend");
	indexed = piglit_is_extension_supported("GL_OES_draw_buffers_indexed");

	prog = piglit_build_simple_program(
			"#version 300 es\n"
			"in vec4 piglit_vertex;\n"
			"void main() { gl_Position = piglit_vertex; }\n",

			"#version 300 es\n"
			"precision highp float;\n"
			"out vec4 color;\n"
			"out vec4 color2;\n"
			"void main() { color2 = color = vec4(0, 1, 0, 1); }\n");
	glUseProgram(prog);

	glGenRenderbuffers(2, rb);
	glBindRenderbuffer(GL_RENDERBUFFER, rb[0]);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA32F, SIZE, SIZE);
	glBindRenderbuffer(GL_RENDERBUFFER, rb[1]);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA16F, SIZE, SIZE);

	glGenFramebuffers(1, &fb);
	glBindFramebuffer(GL_FRAMEBUFFER, fb);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rb[0]);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_RENDERBUFFER, rb[1]);

	ret = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (ret != GL_FRAMEBUFFER_COMPLETE) {
		printf("FAIL: Framebuffer incomplete\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	glViewport(0, 0, SIZE, SIZE);
	glClearColor(0.2, 0.2, 0.2, 0.2);
	glClear(GL_COLOR_BUFFER_BIT);

	piglit_draw_rect(-1, -1, 2, 2);
	check_draw_success();

	if (!indexed) {
		glEnable(GL_BLEND);
		piglit_draw_rect(-1, -1, 2, 2);

		check_blend(float_blend);
		piglit_report_result(PIGLIT_PASS);
	}

	/* RT0 = GL_RGBA32F. Draw should error. */
	glEnablei(GL_BLEND, 0);

	piglit_draw_rect(-1, -1, 2, 2);
	check_blend(float_blend);

	/* RT1 = GL_RGBA16F. Should draw fine. */
	glDisablei(GL_BLEND, 0);
	glEnablei(GL_BLEND, 1);

	piglit_draw_rect(-1, -1, 2, 2);
	check_draw_success();

	/* Both RT's enabled. Should fail. */
	glEnablei(GL_BLEND, 0);

	piglit_draw_rect(-1, -1, 2, 2);
	check_blend(float_blend);

	piglit_report_result(PIGLIT_PASS);
}
