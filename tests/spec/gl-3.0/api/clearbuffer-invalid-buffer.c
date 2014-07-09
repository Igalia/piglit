/* Copyright © 2011 Intel Corporation
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
 * \file clearbuffer-invalid-buffer.c
 * Probe various invalid buffer settings for glClearBuffer
 *
 * \author Ian Romanick
 */
#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;
	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

/* These enums don't really exist, but they were accidentally shipped in some
 * versions of glext.h.
 */
#ifndef GL_DEPTH_BUFFER
#define GL_DEPTH_BUFFER                   0x8223
#endif

#ifndef GL_STENCIL_BUFFER
#define GL_STENCIL_BUFFER                 0x8224
#endif

enum piglit_result
piglit_display(void)
{
	return PIGLIT_FAIL;
}

#define string_enum(x) { # x, x }

struct testv {
	const char *name;
	GLenum value;
};

void piglit_init(int argc, char **argv)
{
	/* Try a bunch of enums that someone might try by accident.
	 */
	static const struct testv test_vectors[] = {
		string_enum(GL_DEPTH_BUFFER),
		string_enum(GL_STENCIL_BUFFER),
		string_enum(GL_DEPTH_STENCIL),
		string_enum(GL_DEPTH_STENCIL_ATTACHMENT),
		string_enum(GL_DEPTH_ATTACHMENT),
		string_enum(GL_STENCIL_ATTACHMENT),
		string_enum(GL_COLOR_ATTACHMENT0),
		string_enum(GL_DEPTH24_STENCIL8),
		string_enum(GL_COLOR_BUFFER_BIT),
		string_enum(GL_DEPTH_BUFFER_BIT),
		string_enum(GL_STENCIL_BUFFER_BIT),
		string_enum(GL_AUX_DEPTH_STENCIL_APPLE),
	};

	static const struct testv fi_test_vectors[] = {
		string_enum(GL_COLOR),
		string_enum(GL_DEPTH),
		string_enum(GL_STENCIL),
		string_enum(GL_DEPTH_BUFFER),
		string_enum(GL_STENCIL_BUFFER),
		string_enum(GL_DEPTH_STENCIL_ATTACHMENT),
		string_enum(GL_DEPTH_ATTACHMENT),
		string_enum(GL_STENCIL_ATTACHMENT),
		string_enum(GL_COLOR_ATTACHMENT0),
		string_enum(GL_DEPTH24_STENCIL8),
		string_enum(GL_COLOR_BUFFER_BIT),
		string_enum(GL_DEPTH_BUFFER_BIT),
		string_enum(GL_STENCIL_BUFFER_BIT),
		string_enum(GL_AUX_DEPTH_STENCIL_APPLE),
	};

	static const float zero_f[4] = {
		0.0f, 0.0f, 0.0f, 0.0f
	};

	static const int zero_i[4] = {
		0, 0, 0, 0
	};

	unsigned i;

	piglit_require_gl_version(30);

	/* Page 264 (page 280 of the PDF) of the OpenGL 3.0 spec says:
	 *
	 *     "ClearBuffer{if ui}v generates an INVALID ENUM error if buffer
	 *     is not COLOR, DEPTH, or STENCIL. ClearBufferfi generates an
	 *     INVALID ENUM error if buffer is not DEPTH STENCIL."
	 */
	for (i = 0; i < ARRAY_SIZE(test_vectors); i++) {
		if (!piglit_automatic)
			printf("Trying glClearBuffer{if ui}v(buffer = %s):\n",
			       test_vectors[i].name);

		glClearBufferfv(test_vectors[i].value, 0, zero_f);
		if (!piglit_check_gl_error(GL_INVALID_ENUM))
			piglit_report_result(PIGLIT_FAIL);

		glClearBufferiv(test_vectors[i].value, 0, zero_i);
		if (!piglit_check_gl_error(GL_INVALID_ENUM))
			piglit_report_result(PIGLIT_FAIL);

		glClearBufferuiv(test_vectors[i].value, 0, (GLuint *) zero_i);
		if (!piglit_check_gl_error(GL_INVALID_ENUM))
			piglit_report_result(PIGLIT_FAIL);
	}

	for (i = 0; i < ARRAY_SIZE(fi_test_vectors); i++) {
		if (!piglit_automatic)
			printf("Trying glClearBufferfi(buffer = %s):\n",
			       fi_test_vectors[i].name);

		glClearBufferfi(fi_test_vectors[i].value, 0, 0.0, 0);
		if (!piglit_check_gl_error(GL_INVALID_ENUM))
			piglit_report_result(PIGLIT_FAIL);
	}

	piglit_report_result(PIGLIT_PASS);
}
