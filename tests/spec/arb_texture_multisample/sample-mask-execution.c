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

    config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

/* test execution of sample masking.
 * - set mask to half the samples
 * - render a red thing
 * - set mask to the other half of the samples
 * - render a blue thing
 *
 * - blit from the MSAA buffer to the winsys buffer
 * - ensure that the pixels are purple
 */

GLuint fbo, tex;

enum piglit_result
piglit_display(void)
{
    float half_purple[] = { 0.5f, 0.5f, 0.0f, 1.0f };
    bool pass = true;

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glClearColor(0.2, 0.2, 0.2, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);

    glEnable(GL_SAMPLE_MASK);

    glSampleMaski(0, 0x3);              /* first and second samples */
    glColor4f(1.0, 0.0, 0.0, 1.0);
    piglit_draw_rect(-1,-1,2,2);

    glSampleMaski(0, 0xc);              /* third and fourth samples */
    glColor4f(0.0, 1.0, 0.0, 1.0);
    piglit_draw_rect(-1,-1,2,2);

    glDisable(GL_SAMPLE_MASK);

    if (!piglit_check_gl_error(GL_NO_ERROR))
        piglit_report_result(PIGLIT_FAIL);

    glFinish();

    glBindFramebuffer(GL_FRAMEBUFFER, piglit_winsys_fbo);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo);
    glBlitFramebuffer(0, 0, 64, 64, 0, 0, 64, 64,
                      GL_COLOR_BUFFER_BIT, GL_NEAREST);

    if (!piglit_check_gl_error(GL_NO_ERROR))
        piglit_report_result(PIGLIT_FAIL);

    glBindFramebuffer(GL_READ_FRAMEBUFFER, piglit_winsys_fbo);

    /* the resolve done by the blit should
     * blend the red and blue samples together */
    pass = piglit_probe_pixel_rgba(32, 32, half_purple) && pass;

    piglit_present_results();

    return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
    bool use_multisample_texture = false;

    piglit_require_extension("GL_ARB_texture_multisample");

    while (++argv,--argc) {
        if (!strcmp(*argv, "-tex"))
            use_multisample_texture = true;
    }

    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    if (use_multisample_texture) {
        /* use a multisample texture */
        printf("Using GL_TEXTURE_2D_MULTISAMPLE\n");

        glGenTextures(1, &tex);
        glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, tex);
        glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE,
                                4, GL_RGBA, 64, 64, GL_TRUE);

        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                               GL_TEXTURE_2D_MULTISAMPLE, tex, 0);
    }
    else {
        /* use a classic renderbuffer */
        printf("Using classic MSAA renderbuffer\n");

        glGenRenderbuffers(1, &tex);
        glBindRenderbuffer(GL_RENDERBUFFER, tex);
        glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_RGBA, 64, 64);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                  GL_RENDERBUFFER, tex);
    }
}
