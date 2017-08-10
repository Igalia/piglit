/*
 * Copyright (c) 2016 VMware, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * on the rights to use, copy, modify, merge, publish, distribute, sub
 * license, and/or sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NON-INFRINGEMENT.  IN NO EVENT SHALL VMWARE AND/OR THEIR SUPPLIERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/**
 * Test mixed integer/float FBO.
 *
 * If the argument 'int_second' is given the 0th color attachment will
 * be a unorm texture and the 1st color attachment will be an integer texture.
 * Otherwise, the 0th color attachment will be integer and the 1st color
 * attachment will be unorm.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN
	config.supports_gl_core_version = 31;
	config.window_visual = PIGLIT_GL_VISUAL_RGBA;
	config.khr_no_error_support = PIGLIT_NO_ERRORS;
PIGLIT_GL_TEST_CONFIG_END

static const char *vs_text =
	"#version 150\n"
	"in vec4 vertex;\n"
	"void main() \n"
	"{ \n"
	"   gl_Position = vertex; \n"
	"} \n";

static const char *fs_text =
	"#version 150\n"
	"out ivec4 outputInt;\n"
	"out vec4 outputFloat;\n"
	"void main() \n"
	"{ \n"
	"   outputInt = ivec4(1, 2, 3, 4); \n"
	"   outputFloat = vec4(0.25, 0.5, 0.75, 1.0); \n"
	"} \n";

const int width = 128, height = 128;
bool int_output_first = true;


static GLuint
create_program(void)
{
	GLuint program = piglit_build_simple_program(vs_text, fs_text);
	if (int_output_first) {
		glBindFragDataLocation(program, 0, "outputInt");
		glBindFragDataLocation(program, 1, "outputFloat");
	}
	else {
		glBindFragDataLocation(program, 0, "outputFloat");
		glBindFragDataLocation(program, 1, "outputInt");
	}

	glLinkProgram(program);
	if (!piglit_link_check_status(program))
		piglit_report_result(PIGLIT_FAIL);

	piglit_check_gl_error(GL_NO_ERROR);

	return program;
}


static GLuint
create_fbo(void)
{
	GLuint intTex, unormTex, fbo;

	glGenTextures(1, &intTex);
	glBindTexture(GL_TEXTURE_2D, intTex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8UI, width, height, 0,
		     GL_RGBA_INTEGER, GL_UNSIGNED_BYTE, NULL);

	glGenTextures(1, &unormTex);
	glBindTexture(GL_TEXTURE_2D, unormTex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0,
		     GL_RGBA, GL_UNSIGNED_BYTE, NULL);

	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	if (int_output_first) {
		glFramebufferTexture2D(GL_FRAMEBUFFER,
				       GL_COLOR_ATTACHMENT0_EXT,
				       GL_TEXTURE_2D, intTex, 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER,
				       GL_COLOR_ATTACHMENT1_EXT,
				       GL_TEXTURE_2D, unormTex, 0);
	}
	else {
		glFramebufferTexture2D(GL_FRAMEBUFFER,
				       GL_COLOR_ATTACHMENT0_EXT,
				       GL_TEXTURE_2D, unormTex, 0);
		glFramebufferTexture2D(GL_FRAMEBUFFER,
				       GL_COLOR_ATTACHMENT1_EXT,
				       GL_TEXTURE_2D, intTex, 0);
	}

	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE) {
		printf("Mixed int/float FBO is incomplete.  Skipping test.\n");
		piglit_report_result(PIGLIT_SKIP);
	}

	static const GLenum draw_bufs[2] = {
		GL_COLOR_ATTACHMENT0,
		GL_COLOR_ATTACHMENT1
	};
	glDrawBuffers(2, draw_bufs);

	if (!piglit_check_gl_error(GL_NO_ERROR)) {
		printf("Test setup failed\n");
		piglit_report_result(PIGLIT_SKIP);
	}

	return fbo;
}


enum piglit_result
piglit_display(void)
{
	const int int_clear[4] = { 99, 99, 99, 99 };
	const float float_clear[4] = { 0.33, 0.33, 0.33, 0.33 };

	if (int_output_first) {
		glClearBufferiv(GL_COLOR, 0, int_clear);
		glClearBufferfv(GL_COLOR, 1, float_clear);
	}
	else {
		glClearBufferfv(GL_COLOR, 0, float_clear);
		glClearBufferiv(GL_COLOR, 1, int_clear);
	}

	piglit_draw_rect(-1, -1, 2, 2);

	bool pass = true;

	/* check the int target */
	if (int_output_first) {
		glReadBuffer(GL_COLOR_ATTACHMENT0);
	}
	else {
		glReadBuffer(GL_COLOR_ATTACHMENT1);
	}
	const int expected_int[4] = {1, 2, 3, 4};
	if (!piglit_probe_rect_rgba_int(0, 0, width, height, expected_int)) {
		printf("Failed probing integer color buffer on"
		       " GL_COLOR_ATTACHMENT%d.\n",
		       (int) !int_output_first);
		pass = false;
	}

	/* check the unorm target */
	if (int_output_first) {
		glReadBuffer(GL_COLOR_ATTACHMENT1);
	}
	else {
		glReadBuffer(GL_COLOR_ATTACHMENT0);
	}
	const float expected_unorm[4] = {0.25, 0.5, 0.75, 1.0};
	if (!piglit_probe_rect_rgba(0, 0, width, height, expected_unorm)) {
		printf("Failed probing unorm color buffer on"
		       " GL_COLOR_ATTACHMENT%d.\n",
		       (int) int_output_first);
		pass = false;
	}

	if (piglit_is_extension_supported("GL_EXT_texture_integer")) {
		/* This query is only part of the extension, not core GL */
		GLboolean intMode = 0;
		glGetBooleanv(GL_RGBA_INTEGER_MODE_EXT, &intMode);
		if (!intMode) {
			printf("GL_RGBA_INTEGER_MODE_EXT incorrectly"
			       " returned false.\n");
			pass = false;
		}
	}

	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}


void
piglit_init(int argc, char **argv)
{
	if (argc > 1 && strcmp(argv[1], "int_second") == 0) {
		int_output_first = false;
	}

	GLuint fbo = create_fbo();
	GLuint program = create_program();

	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glUseProgram(program);
}
