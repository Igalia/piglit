/*
 * Copyright © 2017 Broadcom
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

/* Tests the following language of GL_OES_required_internalformat:
 *
 *     "An implementation must accept all of the values for
 *      <internalformat> specified in Tables 3.4, 3.4.x, 3.4.y.
 *      Furthermore, an implementation must respect the minimum
 *      precision requirements of sized internal formats -- those with
 *      explicit component resolutions -- by storing each component
 *      with at least the number of bits prescribed."
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_es_version = 20;

	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

static const GLenum size_tokens[] = {
	GL_RENDERBUFFER_RED_SIZE,
	GL_RENDERBUFFER_GREEN_SIZE,
	GL_RENDERBUFFER_BLUE_SIZE,
	GL_RENDERBUFFER_ALPHA_SIZE,
	GL_RENDERBUFFER_DEPTH_SIZE,
	GL_RENDERBUFFER_STENCIL_SIZE,
};

static const struct format {
	GLenum format;
	int sizes[ARRAY_SIZE(size_tokens)];
	const char *extension;
} formats[] = {
	/* See table 3.4.x of the spec. */
	{ GL_RGBA4,             { 4,  4,  4,  4,  0, 0 }, NULL },
	{ GL_RGB5_A1,           { 5,  5,  5,  1,  0, 0 }, NULL },
	{ GL_RGBA8,             { 8,  8,  8,  8,  0, 0 }, NULL },
	{ GL_RGB565,            { 5,  6,  5,  0,  0, 0 }, NULL },
	{ GL_RGB8,              { 8,  8,  8,  0,  0, 0 }, "GL_OES_rgb8_rgba8" },
	{ GL_STENCIL_INDEX1,    { 0,  0,  0,  0,  0, 1 }, "GL_OES_stencil1" },
	{ GL_STENCIL_INDEX4,    { 0,  0,  0,  0,  0, 4 }, "GL_OES_stencil4" },
	{ GL_STENCIL_INDEX8,    { 0,  0,  0,  0,  0, 8 }, NULL },
	{ GL_DEPTH_COMPONENT16, { 0,  0,  0,  0, 16, 0 }, NULL },
	{ GL_DEPTH_COMPONENT24, { 0,  0,  0,  0, 24, 0 }, "GL_OES_depth24" },
	{ GL_DEPTH_COMPONENT32, { 0,  0,  0,  0, 32, 0 }, "GL_OES_depth32" },
	{ GL_DEPTH24_STENCIL8,  { 0,  0,  0,  0, 24, 8 }, "GL_OES_packed_depth_stencil" },

	/* Other extensions not listed in the spec's table. */
	{ GL_SRGB8_ALPHA8,      { 8,  8,  8,  8,  0, 0 }, "GL_EXT_sRGB" },
	{ GL_R11F_G11F_B10F,    {11, 11, 10,  0,  0, 0 }, "GL_NV_packed_float" },
	{ GL_SRGB8,             { 8,  8,  8,  0,  0, 0 }, "GL_NV_sRGB_formats" },
};

void
piglit_init(int argc, char **argv)
{
	enum piglit_result result = PIGLIT_PASS;
	GLuint rb;
	int i;

	piglit_require_extension("GL_OES_required_internalformat");

	glGenRenderbuffers(1, &rb);
	glBindRenderbuffer(GL_RENDERBUFFER, rb);

	printf("%20s   R  G  B  A  D  S    R  G  B  A  D  S\n", "");
	printf("%20s   ------------------------------------\n", "");

	for (i = 0; i < ARRAY_SIZE(formats); i++) {
		const struct format *f = &formats[i];
		GLint sizes[ARRAY_SIZE(size_tokens)];
		int j;
		bool err = false;

		if (f->extension) {
			if (!piglit_is_extension_supported(f->extension)) {
				printf("%20s: %38s SKIP (%s)\n",
				       piglit_get_gl_enum_name(f->format), "",
				       f->extension);
				continue;
			}
		}

		glRenderbufferStorage(GL_RENDERBUFFER, f->format, 1, 1);

		for (j = 0; j < ARRAY_SIZE(size_tokens); j++) {
			glGetRenderbufferParameteriv(GL_RENDERBUFFER,
						     size_tokens[j], &sizes[j]);

			if (sizes[j] < f->sizes[j])
				err = true;
		}

		/* If the implementation threw an error for
		 * glRenderbufferStorage (likely) or
		 * glGetRenderbufferParameter, don't bother printing
		 * sizes.
		 */
		if (glGetError() != GL_NO_ERROR) {
			err = true;
			for (j = 0; j < ARRAY_SIZE(size_tokens); j++)
				sizes[j] = -1;
		}

		printf("%20s: %2d %2d %2d %2d %2d %2d / %2d %2d %2d %2d %2d %2d%s\n",
		       piglit_get_gl_enum_name(f->format),
		       f->sizes[0], f->sizes[1], f->sizes[2], f->sizes[3],
		       f->sizes[4], f->sizes[5],
		       sizes[0], sizes[1], sizes[2], sizes[3],
		       sizes[4], sizes[5],
		       err ? ": ERROR" : "");

		if (err)
			result = PIGLIT_FAIL;
	}

	piglit_report_result(result);
}

enum piglit_result
piglit_display(void)
{
	/* Unreached */
	return PIGLIT_FAIL;
}
