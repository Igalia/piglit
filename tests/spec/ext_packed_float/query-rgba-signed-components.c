/*
 * Copyright (c) 2013 Bruce Merry
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
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NON-INFRINGEMENT.  IN NO EVENT SHALL THE AUTHORS OR COYPRIGTH
 * HOLDERS AND/OR THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
 * OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "piglit-util-gl.h"

typedef struct {
	GLenum format;
	const char *extension;
	int expected[4];
	int buffer; /* Draw buffer to attach the renderbuffer to */
} format_info;

static const format_info formats[] = {
	{ GL_RGBA8, NULL, { 0, 0, 0, 0 } },
	{ GL_R8I, "GL_ARB_texture_rg", { 1, 0, 0, 0 } },
	{ GL_RG8I, "GL_ARB_texture_rg", { 1, 1, 0, 0 } },
	{ GL_R8_SNORM, "GL_EXT_texture_snorm", { 1, 0, 0, 0 } },
	{ GL_LUMINANCE8_SNORM, "GL_EXT_texture_snorm", { 1, 1, 1, 0 } },
	{ GL_RGBA8UI_EXT, "GL_EXT_texture_integer", { 0, 0, 0, 0 } },
	{ GL_RGBA16F_ARB, "GL_ARB_texture_float", { 1, 1, 1, 1 } },
	{ GL_LUMINANCE16F_ARB, "GL_ARB_texture_float", { 1, 1, 1, 0 } },
	{ GL_RGB9_E5_EXT, "GL_EXT_texture_shared_exponent", { 0, 0, 0, 0 } },
	{ GL_R11F_G11F_B10F_EXT, "GL_EXT_packed_float", { 0, 0, 0, 0 } },
	{ GL_RGBA16F_ARB, "GL_ARB_texture_float", { 0, 0, 0, 0 }, 1 }
};

PIGLIT_GL_TEST_CONFIG_BEGIN
	config.supports_gl_compat_version = 10;
	config.window_visual = PIGLIT_GL_VISUAL_RGB;
	config.khr_no_error_support = PIGLIT_NO_ERRORS;
PIGLIT_GL_TEST_CONFIG_END

enum piglit_result
piglit_display(void)
{
	/* Should never be reached */
	return PIGLIT_FAIL;
}

/* Queries GL_RGBA_SIGNED_COMPONENTS_EXT and compares to expected.
 * If they do not match, prints an error. Returns true on match.
 */
static bool check_rgba_signed(const int *expected)
{
	int i;
	/* Start with nonsense values, to ensure they are written */
	GLint actual[4] = {2, 2, 2, 2};

	glGetIntegerv(GL_RGBA_SIGNED_COMPONENTS_EXT, actual);
	if (!piglit_check_gl_error(GL_NO_ERROR)) {
		return false;
	}

	for (i = 0; i < 4; i++) {
		if (expected[i] != actual[i]) {
			printf("Expected: (%d, %d, %d, %d)\n",
			       expected[0],
			       expected[1],
			       expected[2],
			       expected[3]);
			printf("Actual: (%d, %d, %d, %d)\n",
			       actual[0],
			       actual[1],
			       actual[2],
			       actual[3]);
			return false;
		}
	}
	return true;
}

static bool test_format(const format_info *f)
{
	GLuint rbo = 0;
	bool pass = true;

	if (f->extension != NULL
	    && !piglit_is_extension_supported(f->extension)) {
		printf("Skipping %s since %s not present\n",
		       piglit_get_gl_enum_name(f->format), f->extension);
		return pass;
	}

	glGenRenderbuffers(1, &rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	glRenderbufferStorage(
		GL_RENDERBUFFER,
		f->format, 16, 16);
	glFramebufferRenderbuffer(
		GL_DRAW_FRAMEBUFFER,
		GL_COLOR_ATTACHMENT0 + f->buffer,
		GL_RENDERBUFFER, rbo);
	if (glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER)
	    != GL_FRAMEBUFFER_COMPLETE) {
		printf("Skipping %s: framebuffer not complete\n",
		       piglit_get_gl_enum_name(f->format));
	} else {
		printf("Testing %s\n", piglit_get_gl_enum_name(f->format));
		if (!check_rgba_signed(f->expected))
			pass = false;
	}

	glFramebufferRenderbuffer(
		GL_DRAW_FRAMEBUFFER,
		GL_COLOR_ATTACHMENT0 + f->buffer,
		GL_RENDERBUFFER, 0);
	glDeleteRenderbuffers(1, &rbo);
	return pass;
}

void
piglit_init(int argc, char **argv)
{
	int expected[4] = {0, 0, 0, 0};
	bool pass = true;
	unsigned int i;
	const GLenum buffers[2] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};

	piglit_require_extension("GL_EXT_packed_float");

	/* With a normal window, all channels should be unsigned */
	printf("Testing window\n");
	if (!check_rgba_signed(expected))
		pass = false;

	if (piglit_is_extension_supported("GL_ARB_framebuffer_object")) {
		GLuint fbo = 0;

		glGenFramebuffers(1, &fbo);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
		glDrawBuffers(2, buffers);
		/* Test a variety of FBO formats */
		for (i = 0; i < ARRAY_SIZE(formats); i++) {
			pass = test_format(formats + i) && pass;
		}
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		glDeleteFramebuffers(1, &fbo);
	}

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}
