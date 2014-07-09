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

#define SURFACE_WIDTH 64
#define SURFACE_HEIGHT 64
#define SURFACE_DEPTH 2     // for GL_TEXTURE_2D_MULTISAMPLE_ARRAY

struct attachment_info
{
    GLenum target;
    GLenum attachment;
    bool multisample;
    bool fixedsamplelocations;
    GLuint format;      // override internalformat; if zero, will choose something
                        // reasonable based on the attachment
    int layer;          // for GL_TEXTURE_2D_MULTISAMPLE_ARRAY, the layer to attach
};

struct test_info
{
    char const *name;
    int expected;
    struct attachment_info attachments[4];
};

struct test_info tests[] = {
    {   "single_msaa_color", GL_FRAMEBUFFER_COMPLETE,
        {   { GL_TEXTURE_2D_MULTISAMPLE, GL_COLOR_ATTACHMENT0, GL_TRUE, GL_TRUE },
            { 0 },
        }
    },
    {   "msaa_mrt_color", GL_FRAMEBUFFER_COMPLETE,
        {   { GL_TEXTURE_2D_MULTISAMPLE, GL_COLOR_ATTACHMENT0, GL_TRUE, GL_TRUE },
            { GL_TEXTURE_2D_MULTISAMPLE, GL_COLOR_ATTACHMENT1, GL_TRUE, GL_TRUE },
            { 0 },
        }
    },
    {   "msaa_mixed_texture_and_renderbuffer", GL_FRAMEBUFFER_COMPLETE,
        {   { GL_TEXTURE_2D_MULTISAMPLE, GL_COLOR_ATTACHMENT0, GL_TRUE, GL_TRUE },
            { GL_RENDERBUFFER, GL_COLOR_ATTACHMENT0, GL_TRUE, GL_TRUE },
            { 0 },
        }
    },
    {   "mixed_msaa_and_plain", GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE,
        {   { GL_TEXTURE_2D_MULTISAMPLE, GL_COLOR_ATTACHMENT0, GL_TRUE, GL_TRUE },
            { GL_RENDERBUFFER, GL_COLOR_ATTACHMENT1, GL_FALSE, GL_TRUE },
            { 0 },
        }
    },
    {   "msaa_mrt_color_nofixed", GL_FRAMEBUFFER_COMPLETE,
        {   { GL_TEXTURE_2D_MULTISAMPLE, GL_COLOR_ATTACHMENT0, GL_TRUE, GL_FALSE },
            { GL_TEXTURE_2D_MULTISAMPLE, GL_COLOR_ATTACHMENT1, GL_TRUE, GL_FALSE },
            { 0 },
        }
    },
    {   "mix_fixedmode", GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE,
        {   { GL_TEXTURE_2D_MULTISAMPLE, GL_COLOR_ATTACHMENT0, GL_TRUE, GL_TRUE },
            { GL_TEXTURE_2D_MULTISAMPLE, GL_COLOR_ATTACHMENT1, GL_TRUE, GL_FALSE },
            { 0 },
        }
    },
    {   "mix_fixedmode_with_renderbuffer", GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE,
        {   { GL_TEXTURE_2D_MULTISAMPLE, GL_COLOR_ATTACHMENT0, GL_TRUE, GL_FALSE },
            { GL_RENDERBUFFER, GL_COLOR_ATTACHMENT1, GL_TRUE, GL_TRUE },
            { 0 },
        }
    },
    {   "msaa_depth", GL_FRAMEBUFFER_COMPLETE,
        {   { GL_TEXTURE_2D_MULTISAMPLE, GL_DEPTH_ATTACHMENT, GL_TRUE, GL_TRUE },
            { 0 },
        }
    },
    {   "msaa_depth_stencil", GL_FRAMEBUFFER_COMPLETE,
        {   {   GL_TEXTURE_2D_MULTISAMPLE, GL_DEPTH_ATTACHMENT, GL_TRUE, GL_TRUE,
                GL_DEPTH_STENCIL
            },
            { 0 },
        }
    },
    {   "msaa_classic_stencil", GL_FRAMEBUFFER_COMPLETE,
        {   { GL_TEXTURE_2D_MULTISAMPLE, GL_COLOR_ATTACHMENT0, GL_TRUE, GL_TRUE },
            { GL_RENDERBUFFER, GL_STENCIL_ATTACHMENT, GL_TRUE, GL_TRUE },
            { 0 },
        }
    },
    {   "msaa_color_layer", GL_FRAMEBUFFER_COMPLETE,
        {   { GL_TEXTURE_2D_MULTISAMPLE_ARRAY, GL_COLOR_ATTACHMENT0, GL_TRUE, GL_TRUE,
              0 /* format */, 0 /* layer */ },
            { 0 },
        }
    },
    {   "msaa_color_nonzero_layer", GL_FRAMEBUFFER_COMPLETE,
        {   { GL_TEXTURE_2D_MULTISAMPLE_ARRAY, GL_COLOR_ATTACHMENT0, GL_TRUE, GL_TRUE,
              0 /* format */, 1 /* layer */ },
            { 0 },
        }
    },
    { 0 },
};

static GLuint
choose_format(struct attachment_info *att)
{
    if (att->format)
        return att->format;

    switch(att->attachment) {
    case GL_DEPTH_ATTACHMENT:
        return GL_DEPTH_COMPONENT;
    case GL_STENCIL_ATTACHMENT:
        return GL_STENCIL_INDEX;
    default:
        return GL_RGBA;
    }
}

static enum piglit_result
check_sample_positions(int expected_sample_count)
{
    GLint samples;
    int i;

    glGetIntegerv(GL_SAMPLES, &samples);
    if (!piglit_check_gl_error(GL_NO_ERROR))
        return PIGLIT_FAIL;

    if (samples < expected_sample_count) {
        printf("Expected sample count at least %d, got %d\n",
               expected_sample_count, samples);
        return PIGLIT_FAIL;
    }

    for (i = 0; i < samples; i++) {
        float sample_pos[2];

        glGetMultisamplefv(GL_SAMPLE_POSITION, i, sample_pos);

        if (!piglit_check_gl_error(GL_NO_ERROR))
            return PIGLIT_FAIL;

        printf("Sample %d position %2.2f %2.2f\n",
                i, sample_pos[0], sample_pos[1] );

        if (sample_pos[0] < 0 || sample_pos[0] > 1 ||
                sample_pos[1] < 0 || sample_pos[1] > 1) {
            printf("Sample %d out of range\n", i );
            return PIGLIT_FAIL;
        }
    }

    return PIGLIT_PASS;
}

static enum piglit_result
exec_test(struct test_info *info, int sample_count)
{
    GLuint fb, tex, rb;
    GLint result;
    struct attachment_info *att;
    GLint maxColorSamples, maxDepthSamples;

    glGetIntegerv(GL_MAX_COLOR_TEXTURE_SAMPLES, &maxColorSamples);
    glGetIntegerv(GL_MAX_DEPTH_TEXTURE_SAMPLES, &maxDepthSamples);

    glGenFramebuffers(1, &fb);
    glBindFramebuffer(GL_FRAMEBUFFER, fb);

    printf("Testing fbo completeness for config '%s'\n", info->name);

    for (att=info->attachments; att->target; att++) {
        int attachment_sample_count = att->multisample ? sample_count : 0;
        printf("  Att target=%s att=%s samples=%d dims=%d,%d,%d fixed=%d\n",
               piglit_get_gl_enum_name(att->target),
               piglit_get_gl_enum_name(att->attachment),
               attachment_sample_count,
               SURFACE_WIDTH, SURFACE_HEIGHT,
               att->target == GL_TEXTURE_2D_MULTISAMPLE_ARRAY ? SURFACE_DEPTH : 1,
               att->fixedsamplelocations);

        switch (att->target) {
        case GL_TEXTURE_2D_MULTISAMPLE:
        case GL_TEXTURE_2D_MULTISAMPLE_ARRAY:
            if (att->attachment == GL_DEPTH_ATTACHMENT && sample_count > maxDepthSamples)
                return PIGLIT_SKIP;
            if ((att->attachment == GL_COLOR_ATTACHMENT0 ||
                att->attachment == GL_COLOR_ATTACHMENT1) && sample_count > maxColorSamples)
                return PIGLIT_SKIP;
        }

        switch (att->target) {
        case GL_TEXTURE_2D_MULTISAMPLE:
            glGenTextures(1, &tex);
            glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, tex);
            glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE,
                                    attachment_sample_count, choose_format(att),
                                    SURFACE_WIDTH, SURFACE_HEIGHT,
                                    att->fixedsamplelocations);

            if (!piglit_check_gl_error(GL_NO_ERROR))
                return PIGLIT_FAIL;

            glFramebufferTexture2D(GL_FRAMEBUFFER, att->attachment,
                                   att->target, tex, 0);
            break;

        case GL_TEXTURE_2D_MULTISAMPLE_ARRAY:
            glGenTextures(1, &tex);
            glBindTexture(GL_TEXTURE_2D_MULTISAMPLE_ARRAY, tex);
            glTexImage3DMultisample(GL_TEXTURE_2D_MULTISAMPLE_ARRAY,
                    attachment_sample_count, choose_format(att),
                    SURFACE_WIDTH, SURFACE_HEIGHT, SURFACE_DEPTH,
                    att->fixedsamplelocations);

            if (!piglit_check_gl_error(GL_NO_ERROR))
                return PIGLIT_FAIL;

            glFramebufferTextureLayer(GL_FRAMEBUFFER, att->attachment,
                    tex, 0, att->layer);
            break;

        case GL_RENDERBUFFER:
            /* RENDERBUFFER has fixedsamplelocations implicitly */
            assert(att->fixedsamplelocations);
            glGenRenderbuffers(1, &rb);
            glBindRenderbuffer(GL_RENDERBUFFER, rb);
            if (att->multisample) {
                glRenderbufferStorageMultisample(GL_RENDERBUFFER,
                                                 attachment_sample_count, choose_format(att),
                                                 SURFACE_WIDTH, SURFACE_HEIGHT);
            }
            else {
                /* non-MSAA renderbuffer */
                glRenderbufferStorage(GL_RENDERBUFFER, choose_format(att),
                                      SURFACE_WIDTH, SURFACE_HEIGHT);
            }

            glFramebufferRenderbuffer(GL_FRAMEBUFFER,
                                      att->attachment, att->target, rb);

            if (!piglit_check_gl_error(GL_NO_ERROR))
                return PIGLIT_FAIL;
            break;

        default:
            assert(!"Unsupported target");
        }
    }

    result = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (result != info->expected) {
        printf("glCheckFramebufferStatus: expected %s, got %s\n",
               piglit_get_gl_enum_name(info->expected),
               piglit_get_gl_enum_name(result));
        return PIGLIT_FAIL;
    }

    if (result == GL_FRAMEBUFFER_COMPLETE && info->attachments->multisample)
        return check_sample_positions(sample_count);

    return PIGLIT_PASS;
}

void
usage(int argc, char **argv)
{
    printf("usage: %s <sample-count>\n", argv[0]);
    piglit_report_result(PIGLIT_SKIP);
}

void
piglit_init(int argc, char **argv)
{
    struct test_info *info;
    enum piglit_result result = PIGLIT_PASS;
    int sample_count;
    int max_samples;

    if (argc != 2)
        usage(argc, argv);

    sample_count = atoi(argv[1]);
    glGetIntegerv(GL_MAX_SAMPLES, &max_samples);
    if (sample_count > max_samples) {
        printf("Sample count of %d not supported.\n", sample_count);
        piglit_report_result(PIGLIT_SKIP);
    }

    for (info = tests; info->name; info++)
        piglit_report_subtest_result(exec_test(info, sample_count), "%s", info->name);

    piglit_report_result(result);
}
