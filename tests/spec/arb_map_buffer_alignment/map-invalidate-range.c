/*
 * Copyright © 2013 Intel Corporation
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
 * \file map-invalidate-range.c
 * Try to exercise some corner cases of range mapping with alignment guarantees.
 *
 * Under certain circumstances, many OpenGL implementations will
 * allocate temporary storage for a mapping of a buffer object.  This
 * most commonly occurs when:
 *
 * 1. The buffer being mapped is being accessed by the GPU.
 * 2. The buffer is being mapped write-only.
 * 3. The range is mapped with invalidate (via \c GL_MAP_INVALIDATE_RANGE_BIT).
 *
 * Furthermore, at least some driver make different choices about the
 * allocation of the temporary storage depending on whether or not
 * explicit flushed (via \c GL_MAP_FLUSH_EXPLICIT_BIT) is requested.
 *
 * This test tries to make sure the temporary storage allocated for
 * the mapping still provides the alignment guarantees required by
 * GL_ARB_map_buffer_alignment.  This is accomplished by starting some
 * rendering that will use the entire buffer then immediately trying
 * to map some portion of the buffer.
 */
#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 15;

	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

static bool
do_test(uint8_t *vertex_data, unsigned num_verts, int buf_size, int map_size,
	int alignment, GLbitfield access)
{
	bool pass = true;
	int offset;

	for (offset = 0; offset < (buf_size - map_size); offset++) {
		void *ptr;
		uintptr_t mapping;

		glDrawArrays(GL_POINTS, 0, num_verts);
		glDrawArrays(GL_POINTS, 0, num_verts);
		glDrawArrays(GL_POINTS, 0, num_verts);
		glDrawArrays(GL_POINTS, 0, num_verts);
		glDrawArrays(GL_POINTS, 0, num_verts);
		glFlush();

		ptr = glMapBufferRange(GL_ARRAY_BUFFER,
				       offset,
				       map_size,
				       access);

		mapping = (uintptr_t) ptr;
		if ((mapping - offset) % alignment != 0) {
			printf("Bad mapping for offset = %d, alignment = %d: "
			       "%p\n",
			       offset, alignment, ptr);
			pass = false;
		}

		/* Invalidate throws away our data, so we have to restore the
		 * values.
		 */
		memcpy(ptr, vertex_data + offset, map_size);

		glUnmapBuffer(GL_ARRAY_BUFFER);
	}

	return pass;
}

void
piglit_init(int argc, char *argv[])
{
	GLuint bo;
	int alignment = 0;
	int buf_size;
	int map_size;
	bool pass = true;
	uint8_t *vertex_data;
	unsigned num_verts;

	piglit_require_extension("GL_ARB_map_buffer_range");
	piglit_require_extension("GL_ARB_map_buffer_alignment");

	glGetIntegerv(GL_MIN_MAP_BUFFER_ALIGNMENT, &alignment);

	buf_size = MAX2(10 * alignment, 4096);
	map_size = alignment - 1;

	vertex_data = calloc(1, buf_size);

	glGenBuffers(1, &bo);
	glBindBuffer(GL_ARRAY_BUFFER, bo);
	glBufferData(GL_ARRAY_BUFFER, buf_size, vertex_data, GL_STATIC_DRAW);

	glVertexPointer(2, GL_FLOAT, 0, 0);
	glEnableClientState(GL_VERTEX_ARRAY);
	num_verts = buf_size / (2 * sizeof(float));

	pass = do_test(vertex_data, num_verts, buf_size, map_size, alignment,
		       GL_MAP_WRITE_BIT
		       | GL_MAP_INVALIDATE_RANGE_BIT)
		&& pass;

	pass = do_test(vertex_data, num_verts, buf_size, map_size, alignment,
		       GL_MAP_WRITE_BIT
		       | GL_MAP_INVALIDATE_RANGE_BIT
		       | GL_MAP_FLUSH_EXPLICIT_BIT)
		&& pass;

	free(vertex_data);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDeleteBuffers(1, &bo);

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}

enum piglit_result
piglit_display(void)
{
	/* UNREACHED */
	return PIGLIT_FAIL;
}
