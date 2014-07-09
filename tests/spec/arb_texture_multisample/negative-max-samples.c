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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include "piglit-util-gl.h"

/**
 * @file negative-max-samples.c
 *
 * Tests that asking for more than the appropriate sample count limit fails,
 * with the correct errors.
 *
 * See also EXT_framebuffer_multisample/negative-max-samples for the simpler
 * case prior to ARB_texture_multisample.
 *
 * Skips if ARB_internalformat_query is supported -- ARB_internalformat_query allows
 * the limit to be higher for particular internalformats.
 */

PIGLIT_GL_TEST_CONFIG_BEGIN

    config.supports_gl_compat_version = 10;

    config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGB;

PIGLIT_GL_TEST_CONFIG_END

enum piglit_result
piglit_display(void)
{
    /* UNREACHED */
    return PIGLIT_FAIL;
}

struct subtest
{
    const char *name;
    int use_texture;
    GLenum internalformat;
    GLenum limit;
    GLenum error;
} subtests[] =
{
    /* multisample textures have separate limits for each of color/depth/integer,
     * all of which must be <= MAX_SAMPLES; GL_INVALID_OPERATION is generated if the
     * limit is exceeded. */
    { "tex_color", GL_TRUE, GL_RGBA,
        GL_MAX_COLOR_TEXTURE_SAMPLES, GL_INVALID_OPERATION },
    { "tex_depth", GL_TRUE, GL_DEPTH_COMPONENT,
        GL_MAX_DEPTH_TEXTURE_SAMPLES, GL_INVALID_OPERATION },
    { "tex_integer", GL_TRUE, GL_RGBA16I,
        GL_MAX_INTEGER_SAMPLES, GL_INVALID_OPERATION },

    /* non-integer formats for renderbuffers are still only checked against
     * MAX_SAMPLES, and generate GL_INVALID_VALUE if exceeded. */
    { "rb_color", GL_FALSE, GL_RGBA,
        GL_MAX_SAMPLES, GL_INVALID_VALUE },
    { "rb_depth", GL_FALSE, GL_DEPTH_COMPONENT,
        GL_MAX_SAMPLES, GL_INVALID_VALUE },
    /* integer formats for renderbuffers are checked against MAX_INTEGER_SAMPLES */
    { "rb_integer", GL_FALSE, GL_RGBA16I,
        GL_MAX_INTEGER_SAMPLES, GL_INVALID_OPERATION },

    /* sentinel */
    { 0 },
};

static void
check_subtest(struct subtest *t)
{
    int limit;
    glGetIntegerv(t->limit, &limit);

    if (t->use_texture) {
        GLuint texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, texture);
        glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, limit+1,
                t->internalformat, 64, 64, GL_TRUE);
    } else {
        GLuint rb;
        glGenRenderbuffers(1, &rb);
        glBindRenderbuffer(GL_RENDERBUFFER, rb);
        glRenderbufferStorageMultisample(GL_RENDERBUFFER,
                limit + 1, t->internalformat, 64, 64);
    }

    piglit_report_subtest_result(
            piglit_check_gl_error(t->error) ? PIGLIT_PASS : PIGLIT_FAIL,
            "%s", t->name);
}

void
piglit_init(int argc, char **argv)
{
    struct subtest *t;

    piglit_require_extension("GL_ARB_texture_multisample");

    if (piglit_is_extension_supported("GL_ARB_internalformat_query")) {
        printf("ARB_internalformat_query is supported and "
               "redefines this behavior; skipping\n");
        piglit_report_result(PIGLIT_SKIP);
    }

    for (t = subtests; t->name; t++)
        check_subtest(t);

    piglit_report_result(PIGLIT_PASS);
}
