/*
 * Copyright (c) 2010 VMware, Inc.
 * Copyright (c) 2015 Red Hat Inc.
 * Copyright (C) 2017 Valve Corporation
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

/** \file
 *
 * Test inherited vertex attributes from NV_vertex_attrib_integer_64bit.
 * Derived from Brian's gpu_shader4 tests and Dave's vertex_attrib_64bit tests.
 */

#include <inttypes.h>

#include "common.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_core_version = 33;

	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;
	config.khr_no_error_support = PIGLIT_NO_ERRORS;

PIGLIT_GL_TEST_CONFIG_END

static const char *TestName = "uint64_attribs";
static const GLuint Index = 3;

static GLboolean
check_uint64_attrib(const GLuint64EXT expected, const char *func)
{
	GLuint64EXT vals[4];

	glGetVertexAttribLui64vARB(Index, GL_CURRENT_VERTEX_ATTRIB_ARB, vals);

	if (expected != vals[0]) {
		fprintf(stderr, "%s: %s failed\n", TestName, func);
		fprintf(stderr, "  Expected: %"PRIu64"  Found: %"PRIu64"\n",
			expected, vals[0]);
		return GL_FALSE;
	}
	return GL_TRUE;
}

enum piglit_result
piglit_display(void)
{
	return PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	GLuint64EXT val = 1844674407370955165;

	piglit_require_extension("GL_ARB_bindless_texture");

	/* The ARB_bindless_texture spec says:
	 *
	 * "Interactions with NV_vertex_attrib_integer_64bit"
	 *
	 * "If NV_vertex_attrib_integer_64bit is not supported, this
	 *  extension inherits the {Get}VertexAttribL1ui64{v}ARB entry points
	 *  and UNSIGNED_INT64_ARB enum, as well as the functional edits
	 *  describing them. However, references to the uint64_t type in the
	 *  shader and providing 64-bit unsigned integer data to the shader
	 *  are removed."
	 */
	glVertexAttribL1ui64ARB(Index, val);
	if (!check_uint64_attrib(val, "glVertexAttribL1ui64ARB"))
		piglit_report_result(PIGLIT_FAIL);

	glVertexAttribL1ui64vARB(Index, &val);
	if (!check_uint64_attrib(val, "glVertexAttribL1ui64vARB"))
		piglit_report_result(PIGLIT_FAIL);

	piglit_report_result(PIGLIT_PASS);
}
