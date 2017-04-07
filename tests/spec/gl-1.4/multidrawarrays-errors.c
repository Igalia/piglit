/*
 * Copyright 2017 Advanced Micro Devices, Inc.
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
 */

/**
 * Test error conditions of glMultiDrawArrays.
 * glMultiDrawArrays is part of GL 1.4 and later.
 *
 * Based loosely on dlist-multidrawarrays.c.
 */

#include "piglit-util-gl.h"


PIGLIT_GL_TEST_CONFIG_BEGIN
	config.supports_gl_compat_version = 14;
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;
PIGLIT_GL_TEST_CONFIG_END


static const float verts[][2] = {
	{ -1.0f, -1.0f },
	{  1.0f, -1.0f },
	{  1.0f,  1.0f },
	{ -1.0f,  1.0f }
};

static const float zero[] = { 0.0f, 0.0f, 0.0f, 0.0f };

static bool
test_draw_negative_primcount()
{
	int first = 0;
	GLsizei count = 4;

	glClearColor(0.0, 0.0, 0.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT);

	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(2, GL_FLOAT, 0, verts);

	/* Section 2.3.1 (Errors) of the OpenGL 4.5 (Core Profile) spec says:
	 *
	 *    "Several error generation conditions are implicit in the
	 *     description of every GL command.
	 *
	 *       ...
	 *
	 *       * If a negative number is provided where an argument of type
	 *         sizei or sizeiptr is specified, an INVALID_VALUE error is
	 *         generated.
	 */
	glMultiDrawArrays(GL_TRIANGLE_STRIP, &first, &count, -1);
	if (!piglit_check_gl_error(GL_INVALID_VALUE))
		return false;

	return piglit_probe_rect_rgb(0, 0, piglit_width, piglit_height, zero);
}

static bool
test_draw_negative_count()
{
	static const int first[2] = { 0, 0 };
	static const GLsizei count[2] = { 4, -1 };

	glClearColor(0.0, 0.0, 0.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT);

	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(2, GL_FLOAT, 0, verts);

	/* Section 10.4 (Drawing Commands Using Vertex Arrays) of the
	 * OpenGL 4.5 (Core Profile) spec describes the following error
	 * condition for glDrawArraysOneInstance, which is used to describe
	 * the semantics of glMultiDrawArrays:
	 *
	 *    "An INVALID_VALUE error is generated if count is negative."
	 *
	 * Furthermore, section 2.3.1 (Errors) of the OpenGL 4.5 (Core Profile)
	 * spec says:
	 *
	 *    "Currently, when an error flag is set, results of GL operation
	 *     are undefined only if an OUT_OF_MEMORY error has occurred. In
	 *     other cases, there are no side effects unless otherwise noted;
	 *     the command which generates the error is ignored so that it has
	 *     no effect on GL state or framebuffer contents."
	 *
	 * We explicitly check that no draw occurred, even though only the
	 * second primitive results in an error.
	 */
	glMultiDrawArrays(GL_TRIANGLE_STRIP, first, count, 2);
	if (!piglit_check_gl_error(GL_INVALID_VALUE))
		return false;

	return piglit_probe_rect_rgba(0, 0, piglit_width, piglit_height, zero);
}


enum piglit_result
piglit_display(void)
{
	bool pass = true;

#define subtest(name) \
	do { \
		if (!test_##name()) { \
			printf(#name " test failed.\n"); \
			pass = false; \
		} \
	} while (false)

	subtest(draw_negative_count);
	subtest(draw_negative_primcount);

#undef subtest

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}


void
piglit_init(int argc, char **argv)
{
	/* nothing */
}
