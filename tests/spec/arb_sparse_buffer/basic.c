/*
 * Copyright (c) 2017 Advanced Micro Devices, Inc.
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

/** \file
 *
 * Draw a colored quad from a vertex buffer residing in a sparse buffer.
 */

#include "piglit-util-gl.h"

#include <inttypes.h>

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 33;
	config.supports_gl_core_version = 33;

PIGLIT_GL_TEST_CONFIG_END

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

static const char vs_source[] =
"#version 130\n"
"\n"
"in vec4 pos;\n"
"in vec4 color;\n"
"\n"
"out vec4 fs_color;\n"
"\n"
"void main() {\n"
"   gl_Position = pos;\n"
"   fs_color = color;\n"
"}\n";

static const char fs_source[] =
"#version 130\n"
"\n"
"in vec4 fs_color;\n"
"\n"
"out vec4 out_color;\n"
"\n"
"void main() {\n"
"   out_color = fs_color;\n"
"}\n";

/* Interleaved position and color data. */
static const float vb_data[] = {
	-1.0, -1.0, 0.0, 1.0,	0.5, 1.0, 0.0, 1.0,
	-1.0,  1.0, 0.0, 1.0,	0.5, 1.0, 0.0, 1.0,
	 1.0, -1.0, 0.0, 1.0,	0.5, 1.0, 0.0, 1.0,
	 1.0,  1.0, 0.0, 1.0,	0.5, 1.0, 0.0, 1.0,
};

static int sparse_buffer_page_size;
static GLuint program;


static bool
do_run_one(uint64_t buffer_size, uint64_t commit_offset, uint64_t commit_size,
	   uint64_t vbuf_offset, bool vbuf_committed)
{
	GLuint buf;
	GLuint vao;

	/* Setup buffer commitment and data. */
	glGenBuffers(1, &buf);
	glBindBuffer(GL_ARRAY_BUFFER, buf);

	glBufferStorage(GL_ARRAY_BUFFER, buffer_size, NULL,
			GL_DYNAMIC_STORAGE_BIT | GL_SPARSE_STORAGE_BIT_ARB);

	glBufferPageCommitmentARB(GL_ARRAY_BUFFER, commit_offset, commit_size, true);

	glBufferSubData(GL_ARRAY_BUFFER, vbuf_offset, sizeof(vb_data), vb_data);

	if (!piglit_check_gl_error(GL_NO_ERROR))
		return false;

	/* Clear to red. */
	glClearColor(1.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	/* Draw. */
	glUseProgram(program);

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(float) * 8,
			      BUFFER_OFFSET(vbuf_offset));
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(float) * 8,
			      BUFFER_OFFSET(vbuf_offset + sizeof(float) * 4));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	glDeleteVertexArrays(1, &vao);
	glDeleteBuffers(1, &buf);

	if (!piglit_check_gl_error(GL_NO_ERROR))
		return false;

	/* Check that the result looks good. There's not really anything we can
	 * check if the right memory wasn't committed.
	 */
	if (vbuf_committed &&
	    !piglit_probe_rect_rgba(0, 0, piglit_width, piglit_height,
				    &vb_data[4]))
		return false;

	return true;
}

static bool
run_one(uint64_t buffer_size, uint64_t commit_offset, uint64_t commit_size,
	uint64_t vbuf_offset, bool vbuf_committed)
{
	bool pass = do_run_one(buffer_size, commit_offset, commit_size,
			       vbuf_offset, vbuf_committed);

	if (!pass) {
		printf("Previous error with:\n"
		       "    buffer_size = %"PRIu64"\n"
		       "    commit_offset = %"PRIu64"\n"
		       "    commit_size = %"PRIu64"\n"
		       "    vbuf_offset = %"PRIu64"\n",
		       buffer_size, commit_offset, commit_size, vbuf_offset);
	}

	return pass;
}

enum piglit_result
piglit_display(void)
{
	bool pass = true;
	uint64_t buffer_size;
	uint64_t commit_offset;

	/* The spec doesn't require this, but in practice it'd be surprising
	 * to see tiny page sizes, so let's not worry about the possibility.
	 */
	assert(sparse_buffer_page_size / 2 > sizeof(vb_data));

	buffer_size = sparse_buffer_page_size / 2;
	pass = run_one(buffer_size, 0, buffer_size, 0, true) && pass;

	buffer_size = 75 * sparse_buffer_page_size;
	pass = run_one(buffer_size, 12 * sparse_buffer_page_size,
		       sparse_buffer_page_size, 12 * sparse_buffer_page_size, true) && pass;

	commit_offset = 1llu * 1024 * 1024 * 1024;
	buffer_size = commit_offset + sparse_buffer_page_size / 2;
	pass = run_one(buffer_size, commit_offset, sparse_buffer_page_size / 2,
		       buffer_size - sizeof(vb_data), true) && pass;

	buffer_size = 10 * sparse_buffer_page_size;
	pass = run_one(buffer_size, sparse_buffer_page_size, 9 * sparse_buffer_page_size,
		       0, false) && pass;

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	piglit_require_extension("GL_ARB_sparse_buffer");

	glGetIntegerv(GL_SPARSE_BUFFER_PAGE_SIZE_ARB, &sparse_buffer_page_size);

	program = piglit_build_simple_program_unlinked(vs_source, fs_source);
	glBindAttribLocation(program, 0, "pos");
	glBindAttribLocation(program, 1, "color");
	glLinkProgram(program);

	if (!piglit_link_check_status(program))
		piglit_report_result(PIGLIT_FAIL);
}
