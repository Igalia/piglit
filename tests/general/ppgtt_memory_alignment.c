/*
 * Copyright © 2018 Intel Corporation
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
/*
 * \file ppgtt_memory_alignment.c
 *
 * Test is explores an internal memory alignments, that are
 * significant for kernel.
 *
 * Bugzilla: https://bugs.freedesktop.org/show_bug.cgi?id=106997
 * Tests: 24839663a402 (intel/ppgtt: memory address alignment)
 * Tests: a363bb2cd0e2 (i965: Allocate VMA in userspace for full-PPGTT systems.)
 *
 * \author Sergii Romantsov <sergii.romantsov@gmail.com>
 * Created on: Oct 11, 2018
 */

#include "piglit-util-gl.h"
#include "unistd.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 20;

	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;
	config.requires_displayed_window = true;

PIGLIT_GL_TEST_CONFIG_END

static enum piglit_result g_pass = PIGLIT_PASS;
static GLuint g_cur_size = 0;
static GLubyte *g_buffer = NULL;

static void
free_buffer(void)
{
	if (g_buffer) {
		free(g_buffer);
		g_buffer = NULL;
	}
}

static void
test_fail_check(void)
{
	/* Intel: Function is needed to check fail-status of execution:
	 * currently if batch-buffer wasn't submitted then Mesa will exit
	 * application. Otherwise piglit_display has logic to detect fail
	 * across call glGetError.
	 */
	if (g_pass != PIGLIT_PASS) {
		fprintf(stderr, "Test failed for buffer size: %x \n",
			g_cur_size);
		free_buffer();
		piglit_report_result(g_pass);
	}
}

enum piglit_result
piglit_display(void)
{
	return g_pass;
}

void
piglit_init(int argc, char **argv)
{
	atexit(test_fail_check);

	/* Maximal value of cache-size supported by driver */
	const GLsizei cache_max_size = 64 * 1024 * 1024;
	const GLsizei cache_extra_size = cache_max_size * 16;
	const unsigned int page_size = getpagesize();
	const GLsizei size_inconsistency = page_size / 4 + 1;
	GLsizei size = 0;
	GLuint buf;

	g_buffer = calloc(sizeof(GLubyte), cache_extra_size);

	glClearColor(0.5, 0.5, 0.5, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	glGenBuffers(1, &buf);
	glBindBuffer(GL_ARRAY_BUFFER, buf);

	while (size < cache_extra_size) {
		glBufferData(GL_ARRAY_BUFFER, g_cur_size, g_buffer,
			     GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_UNSIGNED_BYTE, GL_FALSE,
				      0, 0);
		glEnableVertexAttribArray(0);

		glDrawArrays(GL_TRIANGLES, 0, 1);
		g_pass = piglit_check_gl_error(GL_NO_ERROR) && g_pass;

		g_cur_size = size + size_inconsistency;
		size = size ? size * 2 : page_size;

		glFlush();
		g_pass = piglit_check_gl_error(GL_NO_ERROR) && g_pass;
	}

	glDeleteBuffers(1, &buf);

	free_buffer();
}
