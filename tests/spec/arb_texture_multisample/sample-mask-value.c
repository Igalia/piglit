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
    GLint maxMaskWords = 0, result = 0, mask = 0;
    int i;

    piglit_require_extension("GL_ARB_texture_multisample");

    glGetIntegerv(GL_MAX_SAMPLE_MASK_WORDS, &maxMaskWords);

    for (i=0; i < maxMaskWords; i++) {
        glGetIntegeri_v(GL_SAMPLE_MASK_VALUE, i, &mask);

        if (!piglit_check_gl_error(GL_NO_ERROR)) {
            printf("Could not get word %d of sample mask value\n", i);
            piglit_report_result(PIGLIT_FAIL);
        }

        if ((GLuint)mask != ~0u) {
            printf("Initial mask for word %d is bogus; expected all bits set, got %08x\n",
                   i, mask);
        }
    }

    printf("Checking that correct errors are generated for out of bounds\n");
    glGetIntegeri_v(GL_SAMPLE_MASK_VALUE, maxMaskWords, &result);

    if (!piglit_check_gl_error(GL_INVALID_VALUE))
        piglit_report_result(PIGLIT_FAIL);

    piglit_report_result(PIGLIT_PASS);
}
