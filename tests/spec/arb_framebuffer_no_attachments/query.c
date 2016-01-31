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
 * \file query.c
 *
 * Test occlusion queries with when a framebuffer with no attachments is bound.
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
	"out vec4 color;\n"
	"uniform int v = 0;\n"
	"void main() {\n"
	"       if (v != 0 && (int(gl_FragCoord.x) % 2) == 0) discard;\n"
	"	color = vec4(1);\n"
	"}\n";

static GLuint query;

enum piglit_result
piglit_display(void)
{
	return PIGLIT_FAIL;
}

static bool
compare_counter(uint32_t value, const char *subtest)
{
	/* Map atomic buffer and check that we have expected results. */
	bool result = false;
	uint32_t samples;
	glGetQueryObjectuiv(query, GL_QUERY_RESULT, &samples);
	result = (samples == value);
	if (!result)
		fprintf(stderr, "%s (subtest %s): expected %u, got %u\n",
			__func__, subtest, value, samples);
	piglit_report_subtest_result(result ? PIGLIT_PASS : PIGLIT_FAIL,
		"%s", subtest);
	return result;
}

void
piglit_init(int argc, char **argv)
{
	GLuint buffer, fbo, vao;
	GLint prog;
	bool pass = true;

	piglit_require_gl_version(31);
	piglit_require_extension("GL_ARB_framebuffer_no_attachments");

	glGenQueries(1, &query);

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
	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	/* Render rectangle using our program. */
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glBeginQuery(GL_SAMPLES_PASSED, query);
	piglit_draw_rect(-1, -1, 2, 2);
	glEndQuery(GL_SAMPLES_PASSED);

	pass &= compare_counter(piglit_width * piglit_height, "Basic");

	/* Set 1x1 scissor rectangle. */
	glEnable(GL_SCISSOR_TEST);
	glScissor(0, 0, 1, 1);

	glBeginQuery(GL_SAMPLES_PASSED, query);
	piglit_draw_rect(-1, -1, 2, 2);
	glEndQuery(GL_SAMPLES_PASSED);

	glDisable(GL_SCISSOR_TEST);

	pass &= compare_counter(1, "glScissor");

	/* Set 2x2 viewport. */
	glViewport(0, 0, 2, 2);

	glBeginQuery(GL_SAMPLES_PASSED, query);
	piglit_draw_rect(-1, -1, 2, 2);
	glEndQuery(GL_SAMPLES_PASSED);

	pass &= compare_counter(4, "glViewport");

	/* Set the uniform to 1 so that even x's are discarded */
	glViewport(0, 0, piglit_width, piglit_height);
	glUniform1i(glGetUniformLocation(prog, "v"), 1);

	glBeginQuery(GL_SAMPLES_PASSED, query);
	piglit_draw_rect(-1, -1, 2, 2);
	glEndQuery(GL_SAMPLES_PASSED);

	pass &= compare_counter(piglit_width * piglit_height / 2, "discard");

	/* Change the fb size */
	glUniform1i(glGetUniformLocation(prog, "v"), 0);
	glFramebufferParameteri(GL_FRAMEBUFFER,
		GL_FRAMEBUFFER_DEFAULT_WIDTH, piglit_width / 2);
	glFramebufferParameteri(GL_FRAMEBUFFER,
		GL_FRAMEBUFFER_DEFAULT_HEIGHT, piglit_height / 2);
	/* don't check completeness. */

	glBeginQuery(GL_SAMPLES_PASSED, query);
	piglit_draw_rect(-1, -1, 2, 2);
	glEndQuery(GL_SAMPLES_PASSED);

	pass &= compare_counter((piglit_width / 2) * (piglit_height / 2),
				"fb resize");

	glDeleteFramebuffers(1, &fbo);
	glDeleteVertexArrays(1, &vao);
	glDeleteBuffers(1, &buffer);
	glDeleteProgram(prog);
	glDeleteQueries(1, &query);

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}
