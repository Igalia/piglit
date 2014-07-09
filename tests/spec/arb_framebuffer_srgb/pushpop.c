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

#include "piglit-util-gl.h"

/**
 * @file pushpop.c
 *
 * Tests that GL_FRAMEBUFFER_SRGB is under the color-buffer/enable
 * push/pop bits.
 */

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;

PIGLIT_GL_TEST_CONFIG_END

enum piglit_result
piglit_display(void)
{
	/* UNREACHED */
	return PIGLIT_FAIL;
}

static bool
test_enabled(bool val)
{
	GLboolean ret = glIsEnabled(GL_FRAMEBUFFER_SRGB);

	if (ret != val) {
		fprintf(stderr,
			"  GL_FRAMEBUFFER_SRGB %d doesn't match expected %d\n",
			ret, val);
		return false;
	} else {
		return true;
	}
}

static bool
pushpop(GLuint bits, const char *test)
{
	bool pushpop_affects = (bits & (GL_ENABLE_BIT |
					GL_COLOR_BUFFER_BIT)) != 0;
	printf("%s test:\n", test);

	glEnable(GL_FRAMEBUFFER_SRGB);
	glPushAttrib(bits);
	glDisable(GL_FRAMEBUFFER_SRGB);
	glPopAttrib();

	if (!test_enabled(pushpop_affects))
		return false;

	/* Now, test the bits the other direction. */
	glDisable(GL_FRAMEBUFFER_SRGB);
	glPushAttrib(bits);
	glEnable(GL_FRAMEBUFFER_SRGB);
	glPopAttrib();

	return test_enabled(!pushpop_affects);
}

void
piglit_init(int argc, char **argv)
{
	bool pass = true;

	if (!piglit_is_extension_supported("GL_EXT_framebuffer_sRGB"))
		piglit_require_extension("GL_ARB_framebuffer_sRGB");

	pass = pushpop(GL_ENABLE_BIT, "GL_ENABLE_BIT") && pass;
	pass = pushpop(GL_COLOR_BUFFER_BIT, "GL_COLOR_BUFFER_BIT") && pass;
	pass = pushpop(GL_FOG_BIT, "GL_FOG_BIT") && pass;

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}
