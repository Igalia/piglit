/* Copyright © 2012 Intel Corporation
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

/** @file genned-names.c
 *
 * Test that GL 3.1 core contexts properly throw errors when an object
 * is bound with a name that wasn't returned by the corresponding Gen
 * function.
 *
 * Citations are from the GL 3.1 specification 20090528.  The text is
 * of the form "BindRenderbuffer fails and an INVALID_OPERATION error
 * is generated if renderbuffer is not zero or a name returned from a
 * previous call to GenRenderbuffers, or if such a name has since been
 * deleted with DeleteRenderbuffers."
 */

#include "piglit-util-gl.h"
#include "minmax-test.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_core_version = 31;
	config.supports_gl_compat_version = 0;

	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;

PIGLIT_GL_TEST_CONFIG_END

enum piglit_result
piglit_display(void)
{
	/* UNREACHED */
	return PIGLIT_FAIL;
}

/* Page 32 */
static bool
test_bindbuffer()
{
	/* Targets from Table 2.5 of the GL 3.1 specification, page 31. */
	static const GLenum targets[] = {
		GL_ARRAY_BUFFER,
		GL_COPY_READ_BUFFER,
		GL_COPY_WRITE_BUFFER,
		GL_ELEMENT_ARRAY_BUFFER,
		GL_PIXEL_PACK_BUFFER,
		GL_PIXEL_UNPACK_BUFFER,
		GL_TEXTURE_BUFFER,
		GL_TRANSFORM_FEEDBACK_BUFFER,
		GL_UNIFORM_BUFFER,
	};
	int i;
	bool pass = true;

	for (i = 0; i < ARRAY_SIZE(targets); i++) {
		glBindBuffer(targets[i], 100 + i);
		if (!piglit_check_gl_error(GL_INVALID_OPERATION))
			pass = false;
	}

	return pass;
}

/* Page 41 */
static bool
test_bindvertexarray()
{
	glBindVertexArray(200);
	return piglit_check_gl_error(GL_INVALID_OPERATION);
}

/* Page 76 */
static bool
test_beginquery()
{
	bool pass;

	glBeginQuery(GL_SAMPLES_PASSED, 300);
	pass = piglit_check_gl_error(GL_INVALID_OPERATION);

	glEndQuery(GL_SAMPLES_PASSED);
	piglit_reset_gl_error();

	return pass;
}

/* Page 156 */
static bool
test_bindtexture()
{
	GLenum targets[] = {
		GL_TEXTURE_1D,
		GL_TEXTURE_1D_ARRAY,
		GL_TEXTURE_2D,
		GL_TEXTURE_2D_ARRAY,
		GL_TEXTURE_3D,
		GL_TEXTURE_CUBE_MAP,
		GL_TEXTURE_RECTANGLE,
		GL_TEXTURE_BUFFER,
	};
	int i;
	bool pass = true;

	for (i = 0; i < ARRAY_SIZE(targets); i++) {
		glBindTexture(targets[i], 400 + i);
		if (!piglit_check_gl_error(GL_INVALID_OPERATION))
			pass = false;
	}

	return pass;
}

/* Page 199 */
static bool
test_bindframebuffer()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 500);
	return piglit_check_gl_error(GL_INVALID_OPERATION);
}

/* Page 203 */
static bool
test_bindrenderbuffer()
{
	glBindRenderbuffer(GL_RENDERBUFFER, 600);
	return piglit_check_gl_error(GL_INVALID_OPERATION);
}

/* The transform feedback entrypoints don't explicitly specify the
 * genned name behavior in the 3.1 spec, but it inherits from
 * glBindBuffer()'s bheavior.
 */
static bool
test_bindbuffer_tfb()
{
	bool pass = true;

	glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, 600);
	pass = piglit_check_gl_error(GL_INVALID_OPERATION) && pass;

	glBindBufferRange(GL_TRANSFORM_FEEDBACK_BUFFER, 0, 601, 0, 1);
	pass = piglit_check_gl_error(GL_INVALID_OPERATION) && pass;

	return pass;
}

void
piglit_init(int argc, char **argv)
{
	bool pass = true;

	pass = test_bindbuffer() && pass;
	pass = test_bindvertexarray() && pass;
	pass = test_beginquery() && pass;
	pass = test_bindtexture() && pass;
	pass = test_bindframebuffer() && pass;
	pass = test_bindrenderbuffer() && pass;
	pass = test_bindbuffer_tfb() && pass;

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}
