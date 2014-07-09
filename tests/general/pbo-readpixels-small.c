/*
 * Copyright © 2009 Intel Corporation
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
 *
 * Authors:
 *    Eric Anholt <eric@anholt.net>
 *
 */

/** @file pbo-readpixels-small.c
 *
 * Tests that reading 2x2 BGRA UNSIGNED_BYTE buffers work correctly.
 *
 * This is the corresponding test to fbo-pbo-readpixels-small test to
 * make sure I didn't break any of the coordinate flipping (sure
 * enough, I did).
 *
 * https://bugs.freedesktop.org/show_bug.cgi?id=25921
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;
	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

static GLboolean
probe(int x, int y, uint32_t expected, uint32_t observed)
{
	if ((expected & 0xffffff) != (observed & 0xffffff)) {
		printf("Probe color at (%i,%i)\n", x, y);
		printf("  Expected: 0x%08x\n", expected);
		printf("  Observed: 0x%08x\n", observed);

		return GL_FALSE;
	} else {
		return GL_TRUE;
	}
}

enum piglit_result
piglit_display(void)
{
	GLboolean pass = GL_TRUE;
	uint32_t *addr;
	GLuint pbo;

	glClearColor(0.0, 0.0, 0.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT);

	glGenBuffersARB(1, &pbo);
	glBindBufferARB(GL_PIXEL_PACK_BUFFER, pbo);
	glBufferDataARB(GL_PIXEL_PACK_BUFFER, 4 * 4, NULL, GL_STREAM_DRAW_ARB);
	glPixelStorei(GL_PACK_ALIGNMENT, 1);

	glViewport(0, 0, 2, 2);
	piglit_ortho_projection(2, 2, GL_FALSE);

	/* bottom: green.  top: blue. */
	glColor4f(0.0, 1.0, 0.0, 0.0);
	piglit_draw_rect(0, 0, 2, 1);
	glColor4f(0.0, 0.0, 1.0, 0.0);
	piglit_draw_rect(0, 1, 2, 1);

	/* Read the whole buffer. */
	glReadPixels(0, 0, 2, 2,
		     GL_BGRA, GL_UNSIGNED_BYTE, (void *)(uintptr_t)0);
	addr = glMapBufferARB(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY_ARB);
	pass &= probe(0, 0, 0x0000ff00, addr[0]);
	pass &= probe(1, 0, 0x0000ff00, addr[1]);
	pass &= probe(0, 1, 0x000000ff, addr[2]);
	pass &= probe(1, 1, 0x000000ff, addr[3]);
	glUnmapBufferARB(GL_PIXEL_PACK_BUFFER);

	/* Read with an offset. */
	glReadPixels(1, 0, 1, 1,
		     GL_BGRA, GL_UNSIGNED_BYTE, (void *)(uintptr_t)4);
	addr = glMapBufferARB(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY_ARB);
	pass &= probe(1, 0, 0x0000ff00, addr[1]);
	glUnmapBufferARB(GL_PIXEL_PACK_BUFFER);

	/* Read with an offset. */
	glReadPixels(1, 1, 1, 1,
		     GL_BGRA, GL_UNSIGNED_BYTE, (void *)(uintptr_t)4);
	addr = glMapBufferARB(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY_ARB);
	pass &= probe(1, 1, 0x000000ff, addr[1]);
	glUnmapBufferARB(GL_PIXEL_PACK_BUFFER);

	piglit_present_results();

	glDeleteBuffersARB(1, &pbo);

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}


static void reshape(int width, int height)
{
	piglit_width = width;
	piglit_height = height;

	piglit_ortho_projection(width, height, GL_FALSE);
}

void
piglit_init(int argc, char **argv)
{
	reshape(piglit_width, piglit_height);
	piglit_require_extension("GL_ARB_pixel_buffer_object");
}
