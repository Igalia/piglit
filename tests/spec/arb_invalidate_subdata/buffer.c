/*
 * Copyright 2016 Advanced Micro Devices, Inc.
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
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHOR(S) AND/OR THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * Authors:
 *      Nicolai Hähnle <nicolai.haehnle@amd.com>
 */

/*
 * No-op is a conforming implementation of glInvalidateBuffer(Sub)Data, so
 * this test only checks error conditions.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_core_version = 31;
	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

/*
 * Section 6.5 (Invalidating Buffer Data) of the OpenGL 4.5 (Compatibility
 * Profile) spec:
 *
 *     * An INVALID_VALUE error is generated if buffer is zero or is not the
 *     name of an existing buffer object.
 *     * An INVALID_VALUE error is generated if offset or length is negative,
 *     or if offset + length is greater than the value of BUFFER_SIZE for
 *     buffer.
 *     * An INVALID_OPERATION error is generated if buffer is currently mapped
 *     by MapBuffer or if the invalidate range intersects the range currently
 *     mapped by MapBufferRange, unless it was mapped with MAP_PERSISTENT_BIT
 *     set in the MapBufferRange access flags.
 */
static bool
check_errors_subdata()
{
	GLuint buffer;
	bool pass = true;

	glGenBuffers(1, &buffer);

	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;
	glInvalidateBufferSubData(buffer, 0, 0);
	pass = piglit_check_gl_error(GL_INVALID_VALUE) && pass;

	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glInvalidateBufferSubData(buffer, 0, 0);
	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

	glBufferData(GL_ARRAY_BUFFER, 1024, NULL, GL_STREAM_DRAW);
	glInvalidateBufferSubData(buffer, 0, 1024);
	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

	glInvalidateBufferSubData(buffer, -1, 0);
	pass = piglit_check_gl_error(GL_INVALID_VALUE) && pass;

	glInvalidateBufferSubData(buffer, 0, -1);
	pass = piglit_check_gl_error(GL_INVALID_VALUE) && pass;

	glInvalidateBufferSubData(buffer, 1023, 2);
	pass = piglit_check_gl_error(GL_INVALID_VALUE) && pass;

	glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);

	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;
	glInvalidateBufferSubData(buffer, 0, 1);
	pass = piglit_check_gl_error(GL_INVALID_OPERATION) && pass;

	glUnmapBuffer(GL_ARRAY_BUFFER);

	glMapBufferRange(GL_ARRAY_BUFFER, 256, 256, GL_MAP_WRITE_BIT);

	glInvalidateBufferSubData(buffer, 0, 256);
	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

	glInvalidateBufferSubData(buffer, 512, 512);
	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

	glInvalidateBufferSubData(buffer, 240, 100);
	pass = piglit_check_gl_error(GL_INVALID_OPERATION) && pass;

	glUnmapBuffer(GL_ARRAY_BUFFER);

	if (piglit_is_extension_supported("GL_ARB_buffer_storage")) {
		glBufferStorage(GL_ARRAY_BUFFER, 1024, NULL,
		                GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT);

		glMapBufferRange(GL_ARRAY_BUFFER, 256, 256,
				 GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT);

		glInvalidateBufferSubData(buffer, 240, 100);
		pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

		glUnmapBuffer(GL_ARRAY_BUFFER);
	}

	glDeleteBuffers(1, &buffer);

	return pass;
}

static bool
check_errors_data()
{
	GLuint buffer;
	bool pass = true;

	glGenBuffers(1, &buffer);

	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;
	glInvalidateBufferData(buffer);
	pass = piglit_check_gl_error(GL_INVALID_VALUE) && pass;

	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glInvalidateBufferData(buffer);
	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

	glBufferData(GL_ARRAY_BUFFER, 1024, NULL, GL_STREAM_DRAW);
	glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);

	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;
	glInvalidateBufferData(buffer);
	pass = piglit_check_gl_error(GL_INVALID_OPERATION) && pass;

	glUnmapBuffer(GL_ARRAY_BUFFER);

	if (piglit_is_extension_supported("GL_ARB_buffer_storage")) {
		glBufferStorage(GL_ARRAY_BUFFER, 1024, NULL,
		                GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT);

		glMapBufferRange(GL_ARRAY_BUFFER, 256, 256, GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT);
		glInvalidateBufferData(buffer);
		pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

		glUnmapBuffer(GL_ARRAY_BUFFER);
	}

	glDeleteBuffers(1, &buffer);

	return pass;
}

enum piglit_result
piglit_display(void)
{
	bool pass = true;

	pass = check_errors_subdata() && pass;
	pass = check_errors_data() && pass;

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	piglit_require_extension("GL_ARB_invalidate_subdata");
}
