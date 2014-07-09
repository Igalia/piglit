/*
 * Copyright (c) 2010 VMware, Inc.
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
 * @file api-teximage.c
 *
 * Tests GL_EXT_texture_integer's error behavior with glTexImage2D().
 *
 * The GL_EXT_texture_integer spec doesn't specify how glDrawPixels
 * with an integer format is supposed to work.  glDrawPixels generally
 * generates fragments for a fragment shader with the gl_Color from
 * the immediate data in the DrawPixels call.  However, with
 * GL_EXT_texture_integer formats, the immediate data is now integer
 * despite gl_Color being a floating-point vec4, and the spec for
 * other cases of possible integer-versus-float conflicts resolves
 * that the results are undefined.  It doesn't specify any particular
 * conversion specific to drawpixels.
 *
 * In particular, in order for glDrawPixels of integer to be actually
 * useful to a user, it needs to put integer values into the fragment
 * shader without conversion, and there's no defined way to map the
 * DrawPixels input to some user-defined (integer) fragment shader
 * input.
 *
 * The GL 3.0 specification adds the following additional text in
 * section 3.7.4 ("Rasterization of Pixel Rectangles) on page 151 of
 * the GL 3.0 specification:
 *
 *     "If format contains integer components, as shown in
 *      table 3.6, an INVALID OPERATION error is generated."
 *
 * The NVIDIA driver, which exposes both 3.0 and
 * GL_EXT_texture_integer, follows this behavior.  Resolve that this
 * behavior is a correction to the GL_EXT_texture_integer
 * specification and check that impleentations follow that.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;
	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

enum piglit_result
piglit_display(void)
{
	static const int black[4] = {0, 0, 0, 0};
	static const float green[4] = {0, 1, 0, 0};
	bool pass = GL_TRUE;

	/* We don't have to do an integer FBO for this test, because
	 * no error is specified in the non-integer FBO case:
	 *
	 *     "Results of rasterization are undefined if any of the
	 *      selected draw buffers of the draw framebuffer have an
	 *      integer format and no fragment shader is active.  "
	 */
	glClearColor(0.0, 1.0, 0.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT);

	glDrawPixels(1, 1, GL_RGBA_INTEGER_EXT, GL_UNSIGNED_INT, black);
	pass = piglit_check_gl_error(GL_INVALID_OPERATION) && pass;

	/* The text in GL 3.0 specification banning
	 * glDrawPixels(integer format) precedes the restriction from
	 * GL_EXT_texture_integer which is still included in that
	 * section:
	 *
	 *     "If format is one of the integer component formats as
	 *      defined in table 3.6 and type is FLOAT, the error
	 *      INVALID ENUM occurs."
	 *
	 * Based on this, we test for GL_INVALID_OPERATION even for FLOAT.
	 */
	glDrawPixels(1, 1, GL_RGBA_INTEGER_EXT, GL_FLOAT, black);
	pass = piglit_check_gl_error(GL_INVALID_OPERATION) && pass;

	/* Make sure that we really didn't render anything. */
	pass = piglit_probe_rect_rgba(0, 0, piglit_width, piglit_height, green) && pass;

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	piglit_require_extension("GL_EXT_texture_integer");
}
