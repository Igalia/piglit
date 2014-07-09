/*
 * Copyright © 2013 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
 * AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN
	config.supports_gl_compat_version = 10;
PIGLIT_GL_TEST_CONFIG_END

enum piglit_result piglit_display(void)
{
   return PIGLIT_FAIL;
}

void piglit_init(int argc, char **argv)
{
   piglit_require_extension("GL_ARB_texture_cube_map");
   piglit_require_extension("GL_ARB_seamless_cube_map");

   /* From the ARB_seamless_cube_map extension specification:
    * "The required state is one bit indicating whether seamless cube map
    *  filtering is enabled or disabled. Initially, it is disabled."
    *
    * The OpenGL 3.2 core specification has the exact same text.
    */
   if (glIsEnabled(GL_TEXTURE_CUBE_MAP_SEAMLESS))
	piglit_report_result(PIGLIT_FAIL);

   if (!piglit_check_gl_error(GL_NO_ERROR))
	piglit_report_result(PIGLIT_FAIL);

   piglit_report_result(PIGLIT_PASS);
}
