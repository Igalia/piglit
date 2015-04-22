/*
 * Copyright © 2015 Intel Corporation
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
 * \file atomic.c
 *
 * Test atomic counters with when a framebuffer with no attachments is bound.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_core_version = 31;

PIGLIT_GL_TEST_CONFIG_END

const char *vs_source =
	"#version 140\n"
	"in vec4 piglit_vertex;\n"
	"void main() {\n"
	"	gl_Position = piglit_vertex;\n"
	"}\n";

const char *fs_source =
	"#version 140\n"
	"#extension GL_ARB_shader_atomic_counters : enable\n"
	"layout(binding = 0, offset = 0) uniform atomic_uint counter;\n"
	"void main() {\n"
	"	atomicCounterIncrement(counter);\n"
	"}\n";

enum piglit_result
piglit_display(void)
{
	return PIGLIT_FAIL;
}

static void
reset_counter()
{
	/* Reset value of atomic counter. */
	uint32_t *ptr =
		(uint32_t *) glMapBufferRange(GL_ATOMIC_COUNTER_BUFFER, 0,
			sizeof(uint32_t), GL_MAP_WRITE_BIT);
	*ptr = 0;
	glUnmapBuffer(GL_ATOMIC_COUNTER_BUFFER);
}

static bool
compare_counter(uint32_t value, const char *subtest)
{
	/* Map atomic buffer and check that we have expected results. */
	bool result = false;
	uint32_t *ptr =
		(uint32_t *) glMapBufferRange(GL_ATOMIC_COUNTER_BUFFER, 0,
			sizeof(uint32_t), GL_MAP_READ_BIT);
	result = (*ptr == value);
	if (!result)
		fprintf(stderr, "%s (subtest %s): expected %u, got %u\n",
			__func__, subtest, value, *ptr);
	glUnmapBuffer(GL_ATOMIC_COUNTER_BUFFER);
	piglit_report_subtest_result(result ? PIGLIT_PASS : PIGLIT_FAIL,
		subtest);
	return result;
}

void
piglit_init(int argc, char **argv)
{
	GLuint buffer, fbo, vao;
	GLint prog;
	bool pass = true;
	uint32_t counter = 0;

	piglit_require_gl_version(31);
	piglit_require_extension("GL_ARB_framebuffer_no_attachments");
	piglit_require_extension("GL_ARB_shader_atomic_counters");

	/* Create fbo with no attachments. */
	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	/* Setup default width and height. */
	glFramebufferParameteri(GL_FRAMEBUFFER,
		GL_FRAMEBUFFER_DEFAULT_WIDTH, piglit_width);
	glFramebufferParameteri(GL_FRAMEBUFFER,
		GL_FRAMEBUFFER_DEFAULT_HEIGHT, piglit_height);

	/* Check that fbo is marked complete. */
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) !=
		GL_FRAMEBUFFER_COMPLETE)
		piglit_report_result(PIGLIT_FAIL);

	prog = piglit_build_simple_program(vs_source, fs_source);

	/* Check that there are no errors. */
	if (!prog || !piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	glUseProgram(prog);

	/* Create atomic counter buffer. */
	glGenBuffers(1, &buffer);
	glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, buffer);
	glBufferData(GL_ATOMIC_COUNTER_BUFFER,
		sizeof(uint32_t), &counter, GL_DYNAMIC_DRAW);

	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	/* Render rectangle using our program. */
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	piglit_draw_rect(-1, -1, 2, 2);

	pass = pass && compare_counter(piglit_width * piglit_height, "Basic");

	/* Reset counter and set 1x1 scissor rectangle. */
	reset_counter();

	glEnable(GL_SCISSOR_TEST);
	glScissor(0, 0, 1, 1);

	piglit_draw_rect(-1, -1, 2, 2);

	glDisable(GL_SCISSOR_TEST);

	pass = pass && compare_counter(1, "glScissor");

	/* Reset counter and set 2x2 viewport. */
	reset_counter();
	glViewport(0, 0, 2, 2);

	piglit_draw_rect(-1, -1, 2, 2);

	pass = pass && compare_counter(4, "glViewport");

	glDeleteFramebuffers(1, &fbo);
	glDeleteVertexArrays(1, &vao);
	glDeleteBuffers(1, &buffer);
	glDeleteProgram(prog);

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}
