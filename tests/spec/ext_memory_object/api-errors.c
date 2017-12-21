/*
 * Copyright (c) 2017 Timothy Arceri
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
 * NON-INFRINGEMENT.  IN NO EVENT SHALL VMWARE AND/OR THEIR SUPPLIERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/**
 * Tests that api errors are thrown where expected for the
 * GL_EXT_memory_object extension.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 20; /* Need 2.0 for DSA tests */
	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;
	config.khr_no_error_support = PIGLIT_HAS_ERRORS;

PIGLIT_GL_TEST_CONFIG_END


static bool
test_tex_storage_errors(GLenum target, bool dsa)
{
	const GLint width = 64, height = 4, depth = 8;
	GLuint tex;

	assert(target == GL_TEXTURE_1D ||
	       target == GL_TEXTURE_2D ||
	       target == GL_TEXTURE_3D);

	glGenTextures(1, &tex);
	glBindTexture(target, tex);

	/* Test that passing 0 for <memory> results in an error. */
	if (target == GL_TEXTURE_1D) {
		if (dsa) {
			glTextureStorageMem1DEXT(tex, 1, GL_RGBA8, width, 0,
						 0);
		} else {
			glTexStorageMem1DEXT(target, 1, GL_RGBA8, width, 0,
					     0);
		}
	}
	else if (target == GL_TEXTURE_2D) {
		if (dsa) {
			glTextureStorageMem2DEXT(tex, 1, GL_RGBA8, width,
						 height, 0, 0);
		} else {
			glTexStorageMem2DEXT(target, 1, GL_RGBA8, width,
					     height, 0, 0);
		}
	}
	else if (target == GL_TEXTURE_3D) {
		if (dsa) {
			glTextureStorageMem3DEXT(tex, 1, GL_RGBA8, width,
						 height, depth, 0, 0);
		} else {
			glTexStorageMem3DEXT(target, 1, GL_RGBA8, width,
					     height, depth, 0, 0);
		}
	}

	/* From the EXT_external_objects spec:
	 *
	 *    "An INVALID_VALUE error is generated if <memory> is 0 ..."
	 */
	return piglit_check_gl_error(GL_INVALID_VALUE);
}

static bool
test_tex_storage_ms_errors(GLenum target, bool dsa)
{
	const GLint width = 64, height = 4, depth = 8;
	GLuint tex;

	assert(target == GL_TEXTURE_2D_MULTISAMPLE ||
	       target == GL_TEXTURE_2D_MULTISAMPLE_ARRAY);

	glGenTextures(1, &tex);
	glBindTexture(target, tex);

	/* Test that passing 0 for <memory> results in an error. */
	if (target == GL_TEXTURE_2D_MULTISAMPLE) {
		if (dsa) {
			glTextureStorageMem2DMultisampleEXT(tex, 1, GL_RGBA8,
							    width, height,
							    GL_FALSE, 0, 0);
		} else {
			glTexStorageMem2DMultisampleEXT(target, 1, GL_RGBA8,
							width, height,
							GL_FALSE, 0, 0);
		}
	}
	else if (target == GL_TEXTURE_2D_MULTISAMPLE_ARRAY) {
		if (dsa) {
			glTextureStorageMem3DMultisampleEXT(tex, 1, GL_RGBA8,
							    width, height,
							    depth, GL_FALSE,
							    0, 0);
		} else {
			glTexStorageMem3DMultisampleEXT(target, 1, GL_RGBA8,
							width, height,
							depth, GL_FALSE,
							0, 0);
		}
	}

	/* From the EXT_external_objects spec:
	 *
	 *    "An INVALID_VALUE error is generated if <memory> is 0 ..."
	 */
	return piglit_check_gl_error(GL_INVALID_VALUE);
}

#define BUF_SIZE (12 * 4 * sizeof(float))

static bool
test_buffer_storage_errors(bool dsa)
{
	GLuint buffer;

	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);

	/* Test that passing 0 for <memory> results in an error. */
	if (dsa) {
		glNamedBufferStorageMemEXT(buffer, BUF_SIZE, 0, 0);
	} else {
		glBufferStorageMemEXT(GL_ARRAY_BUFFER, BUF_SIZE, 0, 0);
	}

	/* From the EXT_external_objects spec:
	 *
	 *    "An INVALID_VALUE error is generated if <memory> is 0 ..."
	 */
	return piglit_check_gl_error(GL_INVALID_VALUE);
}

#define X(f, desc)					     	\
	do {							\
		const bool subtest_pass = (f);			\
		piglit_report_subtest_result(subtest_pass	\
					     ? PIGLIT_PASS : PIGLIT_FAIL, \
					     (desc));		\
		pass = pass && subtest_pass;			\
	} while (0)

enum piglit_result
piglit_display(void)
{
	/* TODO: currently this test only tests for errors when we pass 0 for
	 * <memory>. We need to test for other errors.
	 */

	bool pass = true;
	bool dsa = piglit_is_extension_supported("GL_ARB_direct_state_access");

	X(test_tex_storage_errors(GL_TEXTURE_1D, false), "1D texture");
	X(test_tex_storage_errors(GL_TEXTURE_2D, false), "2D texture");
	X(test_tex_storage_errors(GL_TEXTURE_3D, false), "3D texture");

	if (dsa) {
		X(test_tex_storage_errors(GL_TEXTURE_1D, true), "1D texture direct state access");
		X(test_tex_storage_errors(GL_TEXTURE_2D, true), "2D texture direct state access");
		X(test_tex_storage_errors(GL_TEXTURE_3D, true), "3D texture direct state access");
	}

	if (piglit_is_extension_supported("GL_ARB_texture_storage_multisample")) {
		X(test_tex_storage_ms_errors(GL_TEXTURE_2D_MULTISAMPLE, false), "2D texture ms");
		X(test_tex_storage_ms_errors(GL_TEXTURE_2D_MULTISAMPLE_ARRAY, false), "3D texture ms");

		if (dsa) {
			X(test_tex_storage_ms_errors(GL_TEXTURE_2D_MULTISAMPLE, true), "2D texture ms direct state access");
			X(test_tex_storage_ms_errors(GL_TEXTURE_2D_MULTISAMPLE_ARRAY, true), "3D texture ms direct state access");
		}
	}

	if (piglit_is_extension_supported("GL_ARB_buffer_storage")) {
		X(test_buffer_storage_errors(false), "buffer storage");

		if (dsa) {
			X(test_buffer_storage_errors(true), "buffer storage direct state access");
		}
	}

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}


void
piglit_init(int argc, char **argv)
{
	/* From the EXT_external_objects spec:
	 *
	 *   "GL_EXT_memory_object requires ARB_texture_storage or a
	 *   version of OpenGL or OpenGL ES that incorporates it."
	 */
	piglit_require_extension("GL_ARB_texture_storage");
	piglit_require_extension("GL_EXT_memory_object");
}
