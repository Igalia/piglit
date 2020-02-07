/*
 * Copyright 2014 Intel Corporation
 * Copyright © 2020 Advanced Micro Devices, Inc.
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

/** @file api_errors.c
 *
 * A test of glCopyImageSubDataNV that exercises the failure scenarios of the
 * API
 */
#include "piglit-util-gl.h"
#include "piglit-util.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 13;
	config.khr_no_error_support = PIGLIT_HAS_ERRORS;

PIGLIT_GL_TEST_CONFIG_END

static GLuint
image_create(GLenum target)
{
	GLuint name;
	if (target == GL_RENDERBUFFER_EXT)
		glGenRenderbuffers(1, &name);
	else
		glGenTextures(1, &name);
	return name;
}

static void
image_delete(GLenum target, GLuint name)
{
	if (target == GL_RENDERBUFFER_EXT)
		glDeleteRenderbuffers(1, &name);
	else
		glDeleteTextures(1, &name);
}

static void
image_storage(GLenum target, GLuint name, GLenum internal_format,
	      GLsizei width, GLsizei height)
{
	if (target == GL_RENDERBUFFER_EXT) {
		glBindRenderbuffer(target, name);
		glRenderbufferStorage(target, internal_format, width, height);
	} else {
		glBindTexture(target, name);
		glTexStorage2D(target, 4, internal_format, width, height);
	}
}

/* only testing legal targets below */
static const GLenum targets[] = {
	GL_TEXTURE_1D,
	GL_TEXTURE_1D_ARRAY,
	GL_TEXTURE_2D,
	GL_TEXTURE_RECTANGLE,
	GL_TEXTURE_2D_ARRAY,
	GL_TEXTURE_2D_MULTISAMPLE,
	GL_TEXTURE_2D_MULTISAMPLE_ARRAY,
	GL_TEXTURE_CUBE_MAP,
	GL_TEXTURE_CUBE_MAP_ARRAY,
	GL_TEXTURE_3D
};

static bool
test_simple_errors(GLenum src_target, GLenum dst_target)
{
	bool pass = true;
	GLuint i, src, src2, dst;

	src = image_create(src_target);
	dst = image_create(dst_target);

	/* Test all three combinations of incomplete src or dst  */
	glCopyImageSubDataNV(src, src_target, 0, 0, 0, 0,
			     dst, dst_target, 0, 0, 0, 0, 0, 0, 0);
	pass &= piglit_check_gl_error(GL_INVALID_OPERATION);

	image_storage(src_target, src, GL_RGBA8, 32, 32);
	assert(piglit_check_gl_error(GL_NO_ERROR));

	glCopyImageSubDataNV(src, src_target, 0, 0, 0, 0,
			     dst, dst_target, 0, 0, 0, 0, 0, 0, 0);
	pass &= piglit_check_gl_error(GL_INVALID_OPERATION);

	image_storage(dst_target, dst, GL_RGBA8, 32, 32);
	assert(piglit_check_gl_error(GL_NO_ERROR));

	/* We want to test with empty src but valid dst */
	src2 = image_create(src_target);

	glCopyImageSubDataNV(src2, src_target, 0, 0, 0, 0,
			     dst, dst_target, 0, 0, 0, 0, 0, 0, 0);
	pass &= piglit_check_gl_error(GL_INVALID_OPERATION);

	/* This is no longer needed */
	image_delete(src_target, src2);

	/* The NV_copy_image spec says:
	 *
	 *     "INVALID_ENUM is generated if either target is
	 *      not RENDERBUFFER or a valid non-proxy texture target,
	 *      or is TEXTURE_BUFFER, or is one of the cubemap face
	 *      selectors described in table 3.23, or if the target
	 *      does not match the type of the object."
	 */

	if (src_target != GL_RENDERBUFFER_EXT) {
		for (i = 0; i < ARRAY_SIZE(targets); ++i) {
			if (targets[i] == src_target)
				continue;

			/* here, targets[i] doesn't match src object's target */
			glCopyImageSubDataNV(src, targets[i], 0, 0, 0, 0,
					     dst, dst_target, 0, 0, 0, 0,
					   0, 0, 0);
			pass &= piglit_check_gl_error(GL_INVALID_ENUM);
			if (!pass)
				return false;
		}
	}

	/* The NV_copy_image spec says:
	 *
	 *     "INVALID_ENUM is generated if either target is
	 *      not RENDERBUFFER or a valid non-proxy texture target,
	 *      or is TEXTURE_BUFFER, or is one of the cubemap face
	 *      selectors described in table 3.23, or if the target
	 *      does not match the type of the object."
	 */
	if (dst_target != GL_RENDERBUFFER_EXT) {
		for (i = 0; i < ARRAY_SIZE(targets); ++i) {
			if (targets[i] == dst_target)
				continue;

			/* here, targets[i] doesn't match dst object's target */
			glCopyImageSubDataNV(src, src_target, 0, 0, 0, 0,
					     dst, targets[i], 0, 0, 0, 0,
					   0, 0, 0);
			pass &= piglit_check_gl_error(GL_INVALID_ENUM);
			if (!pass)
				return false;
		}
	}

	/* The NV_copy_image spec says:
	 *
	 *     "INVALID_VALUE is generated if either name does not
	 *      correspond to a valid renderbuffer or texture object
	 *      according to the corresponding target parameter"
	 */
	/* 4523 should be a bogus renderbuffer/texture */
	glCopyImageSubDataNV(4523, src_target, 0, 0, 0, 0,
			     dst, dst_target, 0, 0, 0, 0, 0, 0, 0);
	pass &= piglit_check_gl_error(GL_INVALID_VALUE);
	glCopyImageSubDataNV(src, src_target, 0, 0, 0, 0,
			     4523, dst_target, 0, 0, 0, 0, 0, 0, 0);
	pass &= piglit_check_gl_error(GL_INVALID_VALUE);

	/* Invalid level */
	glCopyImageSubDataNV(src, src_target, 5, 0, 0, 0,
			     dst, dst_target, 0, 0, 0, 0, 0, 0, 0);
	pass &= piglit_check_gl_error(GL_INVALID_VALUE);
	glCopyImageSubDataNV(src, src_target, 0, 0, 0, 0,
			     dst, dst_target, 5, 0, 0, 0, 0, 0, 0);
	pass &= piglit_check_gl_error(GL_INVALID_VALUE);

	/* Region out of bounds */
	glCopyImageSubDataNV(src, src_target, 0, 7, 5, 2,
			     dst, dst_target, 0, 0, 0, 0, 26, 25, 20);
	pass &= piglit_check_gl_error(GL_INVALID_VALUE);
	glCopyImageSubDataNV(src, src_target, 0, 7, 5, 2,
			     dst, dst_target, 0, 0, 0, 0, 25, 30, 20);
	pass &= piglit_check_gl_error(GL_INVALID_VALUE);
	glCopyImageSubDataNV(src, src_target, 0, 7, 5, 2,
			     dst, dst_target, 0, 0, 0, 0, 25, 24, 31);
	pass &= piglit_check_gl_error(GL_INVALID_VALUE);
	glCopyImageSubDataNV(src, src_target, 0, 0, 0, 0,
			     dst, dst_target, 0, 7, 5, 2, 26, 25, 20);
	pass &= piglit_check_gl_error(GL_INVALID_VALUE);
	glCopyImageSubDataNV(src, src_target, 0, 0, 0, 0,
			     dst, dst_target, 0, 7, 5, 2, 25, 30, 20);
	pass &= piglit_check_gl_error(GL_INVALID_VALUE);
	glCopyImageSubDataNV(src, src_target, 0, 0, 0, 0,
			     dst, dst_target, 0, 7, 5, 2, 25, 24, 31);
	pass &= piglit_check_gl_error(GL_INVALID_VALUE);

	image_delete(src_target, src);
	image_delete(dst_target, dst);

	return pass;
}

void
piglit_init(int argc, char **argv)
{
	bool pass = true;

	piglit_require_extension("GL_NV_copy_image");
	piglit_require_extension("GL_EXT_framebuffer_object");

	pass &= test_simple_errors(GL_TEXTURE_2D, GL_TEXTURE_2D);
	pass &= test_simple_errors(GL_RENDERBUFFER, GL_TEXTURE_2D);
	pass &= test_simple_errors(GL_TEXTURE_2D, GL_RENDERBUFFER);
	pass &= test_simple_errors(GL_RENDERBUFFER, GL_RENDERBUFFER);

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}

enum piglit_result
piglit_display(void)
{
	/* UNREACHABLE */
	return PIGLIT_FAIL;
}
