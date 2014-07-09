/*
 * Copyright © 2011 Intel Corporation
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

/** @file fbo-readpixels-depth-formats.c
 *
 * Tests that various formats of depth renderbuffers can be read
 * correctly using glReadPixels() with various format/type
 * combinations.
 */

#include "piglit-util-gl.h"

#define BUF_WIDTH 15
#define BUF_HEIGHT 15

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_width = BUF_WIDTH;
	config.window_height = BUF_WIDTH;
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;

PIGLIT_GL_TEST_CONFIG_END

/* Width of our stripes of z = 0.0, 0.5, 1.0 */
static int w = BUF_WIDTH / 3;

int depth_bits;

static bool
test_float(int x, int y, void *values)
{
	GLfloat value = ((GLfloat *)values)[y * BUF_WIDTH + x];
	GLfloat expected;
	GLfloat limit;

	if (x < w)
		expected = 0.0;
	else if (x < w * 2)
		expected = 0.5;
	else
		expected = 1.0;

	/* Default OpenGL "1 in 10^5" */
	limit = .00001;
	/* Framebuffer precision */
	if (depth_bits < 24)
		limit = 1.0 / (1 << depth_bits);

	if (fabs(value - expected) > limit) {
		fprintf(stderr,
			"    GL_FLOAT: "
			"Expected %f at (%d,%d), found %f\n",
			expected, x, y, value);
		return false;
	}

	return true;
}

static bool
test_unsigned_int(int x, int y, void *values)
{
	GLuint value = ((GLuint *)values)[y * BUF_WIDTH + x];
	GLuint expected;
	GLuint high_bits, low_bits;

	if (x < w)
		expected = 0x0;
	else if (x < w * 2)
		expected = 0x80000000;
	else
		expected = 0xffffffff;

	low_bits = (1 << (32 - depth_bits)) - 1;
	high_bits = ~low_bits;

	expected &= high_bits;
	expected |= expected >> depth_bits;

	if ((GLint)(value - expected) > 1 << (32 - depth_bits)) {
		fprintf(stderr,
			"    GL_UNSIGNED_INT: "
			"Expected 0x%08x at (%d,%d), found 0x%08x\n",
			expected, x, y, value);
		return false;
	}

	return true;
}

static bool
test_unsigned_short(int x, int y, void *values)
{
	GLushort value = ((GLushort *)values)[y * BUF_WIDTH + x];
	GLushort expected;

	if (x < w)
		expected = 0x0000;
	else if (x < w * 2)
		expected = 0x8000;
	else
		expected = 0xffff;

	if ((GLshort)(value - expected) > 1) {
		fprintf(stderr,
			"    GL_UNSIGNED_SHORT: "
			"Expected 0x%04x at (%d,%d), found 0x%04x\n",
			expected, x, y, value);
		return false;
	}

	return true;
}

static bool
test_unsigned_byte(int x, int y, void *values)
{
	GLubyte value = ((GLubyte *)values)[y * BUF_WIDTH + x];
	GLubyte expected;

	if (x < w)
		expected = 0x00;
	else if (x < w * 2)
		expected = 0x80;
	else
		expected = 0xff;

	if ((GLbyte)(value - expected) > 1) {
		fprintf(stderr,
			"    GL_UNSIGNED_BYTE: "
			"Expected 0x%02x at (%d,%d), found 0x%02x\n",
			expected, x, y, value);
		return false;
	}

	return true;
}

struct {
	GLenum token;
	char *name;
	bool (*test)(int x, int y, void *values);
} read_formats[] = {
	{ GL_FLOAT, "GL_FLOAT", test_float },
	{ GL_UNSIGNED_INT, "GL_UNSIGNED_INT", test_unsigned_int },
	{ GL_UNSIGNED_SHORT, "GL_UNSIGNED_SHORT", test_unsigned_short },
	{ GL_UNSIGNED_BYTE, "GL_UNSIGNED_BYTE", test_unsigned_byte },
};

static bool
test_with_format(GLenum internal_format, const char *name)
{
	GLuint rb, fb;
	GLenum status;
	bool pass = true;
	/* Storage for the values read.  The largest type is
	 * GLuint-sized, so this will be big enough for all types.
	 */
	GLuint values[BUF_WIDTH * BUF_HEIGHT];
	int i;

	printf("testing %s:\n", name);

	glGenFramebuffersEXT(1, &fb);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fb);
	assert(glGetError() == 0);

	glGenRenderbuffersEXT(1, &rb);
	glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, rb);
	glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, internal_format,
				 BUF_WIDTH, BUF_HEIGHT);
	glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT,
				     GL_DEPTH_ATTACHMENT_EXT,
				     GL_RENDERBUFFER_EXT,
				     rb);

	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	status = glCheckFramebufferStatusEXT (GL_FRAMEBUFFER_EXT);
	if (status != GL_FRAMEBUFFER_COMPLETE_EXT) {
		fprintf(stderr, "framebuffer incomplete\n");
		piglit_report_subtest_result(PIGLIT_SKIP, "%s", name);
		goto done;
	}

	glGetIntegerv(GL_DEPTH_BITS, &depth_bits);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_ALWAYS);
	glViewport(0, 0, BUF_WIDTH, BUF_HEIGHT);
	piglit_ortho_projection(BUF_WIDTH, BUF_HEIGHT, false);
	piglit_draw_rect_z(1.0, 0,     0, w,     BUF_HEIGHT);
	piglit_draw_rect_z(0.0, w,     0, w * 2, BUF_HEIGHT);
	piglit_draw_rect_z(-1.0, w * 2, 0, w * 3, BUF_HEIGHT);

	glPixelStorei(GL_PACK_ALIGNMENT, 1);

	for (i = 0; i < ARRAY_SIZE(read_formats); i++) {
		int x, y;
		bool format_passed = true;

		glReadPixels(0, 0, BUF_WIDTH, BUF_HEIGHT,
			     GL_DEPTH_COMPONENT, read_formats[i].token, values);

		for (y = 0; y < BUF_HEIGHT; y++) {
			for (x = 0; x < BUF_WIDTH; x++) {
				if (!read_formats[i].test(x, y, values)) {
					format_passed = false;
					break;
				}
			}
			if (x != BUF_WIDTH)
				break;

		}

		piglit_report_subtest_result((format_passed ?
					      PIGLIT_PASS : PIGLIT_FAIL),
					     "%s/%s",
					     name,
					     read_formats[i].name);
		pass = format_passed && pass;
	}

done:
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, piglit_winsys_fbo);
	glDeleteFramebuffersEXT(1, &fb);
	glDeleteRenderbuffersEXT(1, &rb);
	return pass;
}

#define ENTRY(token) { #token, token }
struct {
	const char *name;
	GLenum token;
} rb_internal_formats[] = {
	ENTRY(GL_DEPTH_COMPONENT),
	ENTRY(GL_DEPTH_COMPONENT32),
	ENTRY(GL_DEPTH_COMPONENT24),
	ENTRY(GL_DEPTH_COMPONENT16),
	ENTRY(GL_DEPTH_STENCIL_EXT),
	ENTRY(GL_DEPTH24_STENCIL8_EXT),
};

void piglit_init(int argc, char **argv)
{
	int i;
	bool pass = true;

	piglit_require_extension("GL_EXT_framebuffer_object");
	piglit_require_extension("GL_EXT_packed_depth_stencil");

	for (i = 0; i < ARRAY_SIZE(rb_internal_formats); i++) {
		pass = test_with_format(rb_internal_formats[i].token,
					rb_internal_formats[i].name) && pass;
	}

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}

enum piglit_result
piglit_display(void)
{
	/* UNREACHED */
	return PIGLIT_FAIL;
}
