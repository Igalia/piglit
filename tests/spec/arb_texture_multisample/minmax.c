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
#include "minmax-test.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

    config.supports_gl_compat_version = 10;
    config.supports_gl_core_version = 31;
    config.window_visual = PIGLIT_GL_VISUAL_RGB;

PIGLIT_GL_TEST_CONFIG_END

void
piglit_init(int argc, char **argv)
{
    piglit_require_extension("GL_ARB_texture_multisample");
    piglit_print_minmax_header();

    piglit_test_min_int(GL_MAX_SAMPLE_MASK_WORDS, 1);
    piglit_test_min_int(GL_MAX_COLOR_TEXTURE_SAMPLES, 1);
    piglit_test_min_int(GL_MAX_DEPTH_TEXTURE_SAMPLES, 1);
    piglit_test_min_int(GL_MAX_INTEGER_SAMPLES, 1);

    piglit_report_result(piglit_minmax_pass ? PIGLIT_PASS : PIGLIT_FAIL);
}

enum piglit_result
piglit_display(void)
{
    return PIGLIT_FAIL;
}
