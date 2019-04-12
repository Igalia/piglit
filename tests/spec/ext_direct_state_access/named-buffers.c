/*
 * Copyright © 2019 Advanced Micro Devices, Inc.
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

#include "piglit-util-gl.h"

static int
parse_gl_version(int argc, char** argv)
{
	int version;
	if (argc < 2) {
		piglit_loge("Usage: %s 15|30\n", argv[0]);
		exit(1);
	}
	version = atoi(argv[1]);
	if (version != 15 && version != 30) {
		piglit_loge("Usage: %s 15|30\n", argv[0]);
		exit(1);
	}
	return version;
}

static int gl_compat_version;

PIGLIT_GL_TEST_CONFIG_BEGIN

	gl_compat_version = parse_gl_version(argc, argv);
	config.supports_gl_compat_version = gl_compat_version;
	config.khr_no_error_support = PIGLIT_HAS_ERRORS;

PIGLIT_GL_TEST_CONFIG_END

static void
read_buffer(GLuint buffer, void* got, unsigned size)
{
	memset(got, 0, size);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glGetBufferSubData(GL_ARRAY_BUFFER, 0, size, got);
}

static enum piglit_result
test_NamedBufferDataEXT(void* d)
{
	bool pass = true;
	GLuint buffers[3];
	static const float data[4] = { 1, 2, 3, 4 };
	float got[4];

	glGenBuffers(ARRAY_SIZE(buffers), buffers);

	/* Test glNamedBufferDataEXT */
	glBindBuffer(GL_ARRAY_BUFFER, buffers[0]);
	glNamedBufferDataEXT(buffers[0], sizeof(data), data, GL_STATIC_DRAW);
	read_buffer(buffers[0], got, sizeof(got));
	pass = memcmp(data, got, sizeof(data)) == 0 && pass;

	/* The GL_EXT_direct_state_access says:
	 *
	 * If the buffer object named by the buffer parameter has not been
	 * previously bound or has been deleted since the last binding, the
	 * GL first creates a new state vector, initialized with a zero-sized
	 * memory buffer and comprising the state values listed in table 2.6.
	 */
	/* Test glNamedBufferDataEXT without calling glBindBuffer first */
	glNamedBufferDataEXT(buffers[1], sizeof(data), data, GL_STATIC_DRAW);
	read_buffer(buffers[1], got, sizeof(got));
	pass = memcmp(data, got, sizeof(data)) == 0 && pass;

	/* Test glNamedBufferDataEXT on a deleted buffer */
	glDeleteBuffers(1, &buffers[2]);
	glNamedBufferDataEXT(buffers[2], sizeof(data), data, GL_STATIC_DRAW);
	read_buffer(buffers[2], got, sizeof(got));
	pass = memcmp(data, got, sizeof(data)) == 0 && pass;

	glDeleteBuffers(ARRAY_SIZE(buffers), buffers);

	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

	/* The GL_EXT_direct_state_access says:
	 *
	 * There is no buffer corresponding to the name zero, these commands
	 * generate the INVALID_OPERATION error if the buffer parameter is zero.
	 */
	glNamedBufferDataEXT(0, sizeof(data), data, GL_STATIC_DRAW);
	pass = piglit_check_gl_error(GL_INVALID_OPERATION) && pass;

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

static enum piglit_result
test_NamedBufferSubDataEXT(void* d)
{
	bool pass = true;
	GLuint buffers[3];
	static const float data[4] = { 1, 2, 3, 4 };
	static const float expected[4] = { 1, 2, 1, 2 };
	float got[4];

	glGenBuffers(ARRAY_SIZE(buffers), buffers);

	/* Test glNamedBufferSubDataEXT */
	glBindBuffer(GL_ARRAY_BUFFER, buffers[0]);
	glNamedBufferDataEXT(buffers[0], sizeof(data), data, GL_STATIC_DRAW);
	glNamedBufferSubDataEXT(buffers[0], 2 * sizeof(float), 2 * sizeof(float), data);
	read_buffer(buffers[0], got, sizeof(got));
	pass = memcmp(expected, got, sizeof(expected)) == 0 && pass;

	/* The GL_EXT_direct_state_access says:
	 *
	 * If the buffer object named by the buffer parameter has not been
	 * previously bound or has been deleted since the last binding, the
	 * GL first creates a new state vector, initialized with a zero-sized
	 * memory buffer and comprising the state values listed in table 2.6.
	 */
	/* Test glNamedBufferSubDataEXT without calling glBindBuffer first */
	glNamedBufferDataEXT(buffers[1], sizeof(data), data, GL_STATIC_DRAW);
	glNamedBufferSubDataEXT(buffers[1], 2 * sizeof(float), 2 * sizeof(float), data);
	read_buffer(buffers[1], got, sizeof(got));
	pass = memcmp(expected, got, sizeof(expected)) == 0 && pass;

	/* Test glNamedBufferDataEXT on a deleted buffer */
	glDeleteBuffers(1, &buffers[2]);
	glNamedBufferDataEXT(buffers[2], sizeof(data), data, GL_STATIC_DRAW);
	glNamedBufferSubDataEXT(buffers[2], 2 * sizeof(float), 2 * sizeof(float), data);
	read_buffer(buffers[2], got, sizeof(got));
	pass = memcmp(expected, got, sizeof(expected)) == 0 && pass;

	glDeleteBuffers(ARRAY_SIZE(buffers), buffers);

	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

	/* The GL_EXT_direct_state_access says:
	 *
	 * There is no buffer corresponding to the name zero, these commands
	 * generate the INVALID_OPERATION error if the buffer parameter is zero.
	 */
	glNamedBufferSubDataEXT(0, 2 * sizeof(float), 2 * sizeof(float), data);
	pass = piglit_check_gl_error(GL_INVALID_OPERATION) && pass;

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

static enum piglit_result
test_MapNamedBufferEXT(void* d)
{
	bool pass = true;
	GLuint buffer;
	static const float data[4] = { 1, 2, 3, 4 };
	void* buf, *ptr;
	GLint value;

	if (gl_compat_version < 30) {
		return PIGLIT_SKIP;
	}

	glGenBuffers(1, &buffer);

	glNamedBufferDataEXT(buffer, sizeof(data), data, GL_STATIC_DRAW);

	/* Test glNamedBufferDataEXT */
	buf = glMapNamedBufferEXT(buffer, GL_READ_ONLY);
	if (!buf) {
		return PIGLIT_FAIL;
	}
	pass = memcmp(buf, data, sizeof(data)) == 0 && pass;
	glGetNamedBufferPointervEXT(buffer, GL_BUFFER_MAP_POINTER, &ptr);
	pass = buf == ptr && pass;
	glGetNamedBufferParameterivEXT(buffer, GL_BUFFER_ACCESS, &value);
	pass = value == GL_READ_ONLY && pass;
	glGetNamedBufferParameterivEXT(buffer, GL_BUFFER_MAPPED, &value);
	pass = value && pass;
	glUnmapNamedBufferEXT(buffer);

	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

	/* The GL_EXT_direct_state_access says:
	 *
	 * If the buffer object named by the buffer parameter has not been
	 * previously bound or has been deleted since the last binding, the
	 * GL first creates a new state vector, initialized with a zero-sized
	 * memory buffer and comprising the state values listed in table 2.6.
	 */
	glDeleteBuffers(1, &buffer);

	glGetNamedBufferPointervEXT(buffer, GL_BUFFER_MAP_POINTER, &ptr);
	pass = ptr == NULL &&
	       piglit_check_gl_error(GL_NO_ERROR)
	       && pass;

	glDeleteBuffers(1, &buffer);

	/* The GL_EXT_direct_state_access says:
	 *
	 * There is no buffer corresponding to the name zero, these commands
	 * generate the INVALID_OPERATION error if the buffer parameter is zero.
	 */
	glMapNamedBufferEXT(0, GL_READ_ONLY);
	pass = piglit_check_gl_error(GL_INVALID_OPERATION) && pass;

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

static enum piglit_result
test_MapNamedBufferRangeEXT(void* d)
{
	bool pass = true;
	GLuint buffer;
	static const float data[4] = { 1, 2, 3, 4 };
	void* buf;

	if (gl_compat_version < 30) {
		return PIGLIT_SKIP;
	}

	glGenBuffers(1, &buffer);

	glNamedBufferDataEXT(buffer, sizeof(data), data, GL_STATIC_DRAW);

	/* Test MapNamedBufferRangeEXT */
	buf = glMapNamedBufferRangeEXT(buffer, 0, sizeof(data), GL_MAP_READ_BIT);
	if (!buf) {
		return PIGLIT_FAIL;
	}
	pass = memcmp(buf, data, sizeof(data)) == 0 && pass;

	glDeleteBuffers(1, &buffer);

	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

	/* The GL_EXT_direct_state_access says:
	 *
	 * There is no buffer corresponding to the name zero, these commands
	 * generate the INVALID_OPERATION error if the buffer parameter is zero.
	 */
	glMapNamedBufferRangeEXT(0, 0, sizeof(data), GL_MAP_READ_BIT);
	pass = piglit_check_gl_error(GL_INVALID_OPERATION) && pass;

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

static enum piglit_result
test_FlushMappedNamedBufferRangeEXT(void* d)
{
	bool pass = true;
	GLuint buffer;
	static const float data[4] = { 1, 2, 3, 4 };
	float* buf;

	if (gl_compat_version < 30) {
		return PIGLIT_SKIP;
	}

	glGenBuffers(1, &buffer);

	glNamedBufferDataEXT(buffer, sizeof(data), data, GL_STATIC_DRAW);

	/* Test MapNamedBufferRangeEXT */
	buf = (float*) glMapNamedBufferRangeEXT(buffer,
						0,
						sizeof(data),
						GL_MAP_FLUSH_EXPLICIT_BIT | GL_MAP_WRITE_BIT);
	if (!buf) {
		return PIGLIT_FAIL;
	}

	buf[2] = 5;

	glFlushMappedNamedBufferRangeEXT(buffer, 3, 1);
	glUnmapNamedBufferEXT(buffer);

	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

	/* Remap and check modified value */
	buf = (float*) glMapNamedBufferRangeEXT(buffer, 0, sizeof(data), GL_MAP_READ_BIT);

	pass = buf[2] == 5 && pass;
	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	piglit_require_extension("GL_EXT_direct_state_access");
	static const struct piglit_subtest tests[] = {
		{
			"NamedBufferDataEXT",
			NULL,
			test_NamedBufferDataEXT
		},
		{
			"NamedBufferSubDataEXT",
			NULL,
			test_NamedBufferSubDataEXT
		},
		{
			"MapNamedBufferEXT",
			NULL,
			test_MapNamedBufferEXT
		},
		{
			"MapNamedBufferRangeEXT",
			NULL,
			test_MapNamedBufferRangeEXT
		},
		{
			"FlushMappedNamedBufferRangeEXT",
			NULL,
			test_FlushMappedNamedBufferRangeEXT,
		},
		{
			NULL
		}
	};

	return piglit_report_result(
		piglit_run_selected_subtests(tests, NULL, 0, PIGLIT_PASS));
}

enum piglit_result
piglit_display(void)
{
	/* UNREACHABLE */
	return PIGLIT_FAIL;
}

