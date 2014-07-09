/* Copyright © 2012 Red Hat.
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

/** @file teximage3d-invalid-values
 *
 * From the GL_ARB_texture_cube_map_array spec:
 *
 * TexImage3D generates the error INVALID_VALUE if <target> is
 * TEXTURE_CUBE_MAP_ARRAY_ARB and <depth> is not a multiple of 6.
 * TexImage3D generates the error INVALID_VALUE if <target> is
 * TEXTURE_CUBE_MAP_ARRAY_ARB and <width> and <height> are not equal.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

    config.supports_gl_compat_version = 10;

    config.window_width= 32;
    config.window_height = 32;
    config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;

PIGLIT_GL_TEST_CONFIG_END

enum piglit_result
piglit_display(void)
{
	return PIGLIT_FAIL; /* UNREACHED */
}


void
piglit_init(int argc, char **argv)
{
	GLuint tex;
	char *data = NULL;

	piglit_require_extension("GL_ARB_texture_cube_map_array");

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, tex);

	/* multiple of 6 depth tests */
	/* less than 6 */
	glTexImage3D(GL_TEXTURE_CUBE_MAP_ARRAY, 0, GL_RGB, 64, 64, 4, 0, GL_RGB, GL_FLOAT, data);
	if (!piglit_check_gl_error(GL_INVALID_VALUE))
	     piglit_report_result(PIGLIT_FAIL);

	/* greater than 6 */
	glTexImage3D(GL_TEXTURE_CUBE_MAP_ARRAY, 0, GL_RGB, 64, 64, 14, 0, GL_RGB, GL_FLOAT, data);
	if (!piglit_check_gl_error(GL_INVALID_VALUE))
	     piglit_report_result(PIGLIT_FAIL);

	/* different w/h */
	glTexImage3D(GL_TEXTURE_CUBE_MAP_ARRAY, 0, GL_RGB, 64, 14, 6, 0, GL_RGB, GL_FLOAT, data);
	if (!piglit_check_gl_error(GL_INVALID_VALUE))
	     piglit_report_result(PIGLIT_FAIL);

	piglit_report_result(PIGLIT_PASS);
}
