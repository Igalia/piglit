/*
 * Copyright © 2013 Chris Forbes
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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */
#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

    config.supports_gl_compat_version = 30;
    config.window_visual = PIGLIT_GL_VISUAL_RGB;

PIGLIT_GL_TEST_CONFIG_END

enum piglit_result
piglit_display(void)
{
    return PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
    GLint maskOn = 0;
    piglit_require_extension("GL_ARB_texture_multisample");

    printf("Checking GL_SAMPLE_MASK exists\n");
    if (glIsEnabled(GL_SAMPLE_MASK)) {
        printf("GL_SAMPLE_MASK enabled by default, should be disabled.\n");
        piglit_report_result(PIGLIT_FAIL);
    }

    if (!piglit_check_gl_error(GL_NO_ERROR))
        piglit_report_result(PIGLIT_FAIL);

    printf("Checking GL_SAMPLE_MASK works with GetIntegerv\n");
    glGetIntegerv(GL_SAMPLE_MASK, &maskOn);

    if (!piglit_check_gl_error(GL_NO_ERROR))
        piglit_report_result(PIGLIT_FAIL);

    if (maskOn) {
        printf("GetIntegerv(GL_SAMPLE_MASK) true by default, should be false\n");
        piglit_report_result(PIGLIT_FAIL);
    }

    piglit_report_result(PIGLIT_PASS);
}
