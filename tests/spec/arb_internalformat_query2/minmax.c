/*
 * Copyright © 2015 Intel Corporation
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

/**
 * \file minmax.c
 * Verify that minimum value requirements for implementation limits
 * are satisfied. Equivalent to query1 minmax, but testing against
 * GetInternalformati64v too.
 */

#include <limits.h>
#include "common.h"
#include <inttypes.h>  /* for PRIu64 macro */

#define ERROR_HEADER(data) test_data_get_testing64(data) ?   \
        fprintf(stderr, "64 bit query: ") : \
        fprintf(stderr, "32 bit query: ");

PIGLIT_GL_TEST_CONFIG_BEGIN

        config.supports_gl_compat_version = 10;
        config.window_visual = PIGLIT_GL_VISUAL_RGB;
	config.khr_no_error_support = PIGLIT_NO_ERRORS;

PIGLIT_GL_TEST_CONFIG_END

/* These are all the formats that are required to be color-renderable
 * by the OpenGL 3.0 spec.
 *
 * But note that GL_ALPHA8 was removed on 3.1 and beyond on core, or
 * if ARB_compatibility is missing, so we need to take that into
 * account.
 */
static const GLenum valid_formats[] = {
        GL_RGBA32F,
        GL_RGBA16,
        GL_RGBA16F,
        GL_RGBA8,
        GL_SRGB8_ALPHA8,
        GL_R11F_G11F_B10F,
        GL_RG32F,
        GL_RG16,
        GL_RG16F,
        GL_RG8,
        GL_R32F,
        GL_R16,
        GL_R16F,
        GL_R8,
        GL_ALPHA8,
};

static const GLenum valid_integer_formats[] = {
        GL_RGBA32I,
        GL_RGBA32UI,
        GL_RGBA16I,
        GL_RGBA16UI,
        GL_RGBA8I,
        GL_RGBA8UI,
        GL_RG32I,
        GL_RG32UI,
        GL_RG16I,
        GL_RG16UI,
        GL_RG8I,
        GL_RG8UI,
        GL_R32I,
        GL_R32UI,
        GL_R16I,
        GL_R16UI,
        GL_R8I,
        GL_R8UI,
};

static const GLenum valid_depth_formats[] = {
        GL_DEPTH_COMPONENT16,
        GL_DEPTH_COMPONENT24,
        GL_DEPTH_COMPONENT32F,
};

static const GLenum valid_targets_with_tms[] = {
        GL_RENDERBUFFER,
        GL_TEXTURE_2D_MULTISAMPLE,
        GL_TEXTURE_2D_MULTISAMPLE_ARRAY,
};

enum piglit_result
piglit_display(void)
{
        return PIGLIT_FAIL;
}

static bool
real_try(GLenum target, GLenum format, GLint max_samples,
         const char *max_samples_name,
         test_data *data_counts,
         test_data *data_samples)
{
        bool pass = true;
        int buffer_size_in_elements = 0;
        unsigned i;
        GLint previous;

        test_data_set_params_size(data_counts, 1);
        test_data_execute(data_counts, target, format,
                          GL_NUM_SAMPLE_COUNTS);

        buffer_size_in_elements = test_data_value_at_index(data_counts, 0);
        pass = piglit_check_gl_error(0)
                && pass;

        /* The GL_ARB_internalformat_query spec says:
         *
         *     "Add new table 6.X Internalformat-specific
         *     Implementation Dependent Values after 6.52"
         *
         *                                                       Minimum
         *     Get Value         Type    Get Command              Value
         *     ---------         ----    -----------              -------
         *     SAMPLES           0*xZ+   GetInternalformativ       fn1
         *     NUM_SAMPLE_COUNTS Z+      GetInternalformativ       1
         *
         *     fn1: see section 6.X."
         */
        if (buffer_size_in_elements < 1) {
                ERROR_HEADER(data_counts);
                fprintf(stderr,
                        "GL_NUM_SAMPLE_COUNTS is %d for %s/%s\n",
                        buffer_size_in_elements,
                        piglit_get_gl_enum_name(target),
                        piglit_get_gl_enum_name(format));
                return false;
        }

        test_data_set_params_size(data_samples, buffer_size_in_elements);

        /* Try GL_SAMPLES
         */
        test_data_execute(data_samples, target,
                          format, GL_SAMPLES);
        pass = piglit_check_gl_error(0)
                && pass;

        /* The GL_ARB_internalformat_query spec says:
         *
         *     "- SAMPLES: The sample counts supported for this
         *        <format> and <target> are written into <params>, in
         *        descending order. Only positive values are
         *        returned."
         *
         * We take "positive" to mean greater than zero.  Zero isn't a
         * valid sample count for multisampling.  It's the special
         * value used to request non-multisampling.
         */
        previous = INT_MAX;
        for (i = 0; i < test_data_get_params_size(data_samples); i++) {
                if (test_data_value_at_index(data_samples, i) <= 0) {
                        ERROR_HEADER(data_samples);
                        fprintf(stderr,
                                "Invalid sample count [%u] = %" PRIi64 " returned "
                                "for %s/%s (num sample counts = %i)\n",
                                i, test_data_value_at_index(data_samples, i),
                                piglit_get_gl_enum_name(target),
                                piglit_get_gl_enum_name(format),
                                buffer_size_in_elements);
                        pass = false;
                }

                if (previous == test_data_value_at_index(data_samples, i)) {
                        ERROR_HEADER(data_samples);
                        fprintf(stderr,
                                "Duplicate values [%u] = [%u] = %" PRIi64 " returned "
                                "for %s/%s (num sample counts = %i)\n",
                                i - 1, i, test_data_value_at_index(data_samples, i),
                                piglit_get_gl_enum_name(target),
                                piglit_get_gl_enum_name(format),
                                buffer_size_in_elements);
                        pass = false;
                }

                if (previous < test_data_value_at_index(data_samples, i)) {
                        ERROR_HEADER(data_samples);
                        fprintf(stderr,
                                "Values not in descending order "
                                "([%u] = %d) < ([%u] = %" PRIi64 ") returned "
                                "for %s/%s (num sample counts = %i)\n",
                                i - 1, previous,
                                i, test_data_value_at_index(data_samples, i),
                                piglit_get_gl_enum_name(target),
                                piglit_get_gl_enum_name(format),
                                buffer_size_in_elements);
                        pass = false;
                }

                previous = test_data_value_at_index(data_samples, i);
        }

        /* The GL_ARB_internalformat_query spec says:
         *
         *     "The maximum value in SAMPLES is guaranteed to be at
         *     least the lowest of the following:
         *
         *       - The value of GetIntegerv(MAX_INTEGER_SAMPLES), if
         *         <internalformat> is a signed or unsigned integer format.
         *       - The value of GetIntegerv(MAX_DEPTH_TEXTURE_SAMPLES), if
         *         <internalformat> is a depth/stencil-renderable format and
         *         <target> is TEXTURE_2D_MULTISAMPLE or
         *         TEXTURE_2D_MULTISAMPLE_ARRAY.
         *       - The value of GetIntegerv(MAX_COLOR_TEXTURE_SAMPLES), if
         *         <internalformat> is a color-renderable format and <target>
         *         is TEXTURE_2D_MULTISAMPLE or TEXTURE_2D_MULTISAMPLE_ARRAY.
         *       - The value of GetIntegerv(MAX_SAMPLES)."
         *
         * Separate tests will verify the values for GL_MAX_*_SAMPLES.
         */
        if (test_data_value_at_index(data_samples, 0) < max_samples) {
                ERROR_HEADER(data_samples);
                fprintf(stderr,
                        "GL_SAMPLES (%" PRIi64 ") smaller than %s (%d) "
                        "for %s/%s\n",
                        test_data_value_at_index(data_samples, 0),
                        max_samples_name,
                        max_samples,
                        piglit_get_gl_enum_name(target),
                        piglit_get_gl_enum_name(format));
                pass = false;
        }

        return pass;
}


/*
 * This is a wrapping method that handles the need to test using
 * GetInternalformativ and GetInternalformati64v. This way piglit_init
 * is basically the same that query1 counterpart, and real_try is just
 * try() at query1, but using the test_data structs. Also helps on the
 * indentation.
 */
static bool
try(GLenum target, GLenum format, GLint max_samples,
    const char *max_samples_name)
{
        /* real params_size will be set at real_try() */
        test_data *data_counts = test_data_new(0, 0);
        test_data *data_samples = test_data_new(0, 0);
        bool result;
        int testing64;

        for (testing64 = 0; testing64 <= 1; testing64++) {
                test_data_set_testing64(data_counts, testing64);
                test_data_set_testing64(data_samples, testing64);

                result = real_try(target, format, max_samples, max_samples_name,
                                  data_counts, data_samples);
        }

        test_data_clear(&data_counts);
        test_data_clear(&data_samples);

        return result;
}

void
piglit_init(int argc, char **argv)
{
        bool pass = true;
        unsigned i;
        const bool tms_supported =
                piglit_is_extension_supported("GL_ARB_texture_multisample");
        GLint max_samples;
        GLint valid_formats_size = ARRAY_SIZE(valid_formats);

        piglit_require_extension("GL_ARB_framebuffer_object");
        piglit_require_extension("GL_ARB_internalformat_query2");
        initialize_valid_internalformats();

        /* Need GL 3 or extensions to support the valid_formats[] above */
        if (piglit_get_gl_version() < 30) {
                piglit_require_extension("GL_ARB_texture_rg");
                piglit_require_extension("GL_ARB_texture_float");
        }

        /* GL_ALPHA8 was removed on OpenGL 3.1 core, or if
         * ARB_compatibility is missing, so on that case we skip that
         * format
         */
        if (piglit_get_gl_version() >= 31 &&
            (piglit_is_core_profile ||
             !piglit_is_extension_supported("GL_ARB_compatibility")))
                valid_formats_size = valid_formats_size - 1;

        glGetIntegerv(GL_MAX_SAMPLES, &max_samples);
        for (i = 0; i < valid_formats_size; i++) {
                pass = try(GL_RENDERBUFFER,
                           valid_formats[i],
                           max_samples,
                           "GL_MAX_SAMPLES")
                        && pass;
        }

        if (!tms_supported) {
                for (i = 0; i < ARRAY_SIZE(valid_depth_formats); i++) {
                        pass = try(GL_RENDERBUFFER,
                                   valid_depth_formats[i],
                                   max_samples,
                                   "GL_MAX_SAMPLES")
                                && pass;
                }

                /* The OpenGL 3.1 spec says:
                 *
                 *     "The error INVALID_OPERATION may be generated if
                 *     internalformat is a signed or unsigned integer format,
                 *     samples is greater than one, and the implementation
                 *     does not support multisampled integer renderbuffers
                 *     (see “Required Renderbuffer Formats” below)."
                 *
                 * In OpenGL 3.2 or ARB_texture_multisample the query
                 * GL_MAX_INTEGER_SAMPLES is used to determine the
                 * maximum number of samples for integer buffers.
                 * This is checked in the other code path.
                 */
                for (i = 0; i < ARRAY_SIZE(valid_integer_formats); i++) {
                        pass = try(GL_RENDERBUFFER,
                                   valid_integer_formats[i],
                                   1,
                                   "one")
                                && pass;
                }
        } else {
                for (i = 0; i < ARRAY_SIZE(valid_targets_with_tms); i++) {
                        const char *max_samples_name;
                        unsigned j;

                        if (valid_targets_with_tms[i] == GL_RENDERBUFFER) {
                                glGetIntegerv(GL_MAX_SAMPLES, &max_samples);
                                max_samples_name = "GL_MAX_SAMPLES";
                        } else {
                                glGetIntegerv(GL_MAX_COLOR_TEXTURE_SAMPLES,
                                              &max_samples);
                                max_samples_name = "GL_MAX_COLOR_TEXTURE_SAMPLES";
                        }

                        for (j = 0; j < valid_formats_size; j++) {
                                pass = try(valid_targets_with_tms[i],
                                           valid_formats[j],
                                           max_samples,
                                           max_samples_name)
                                        && pass;
                        }

                        if (valid_targets_with_tms[i] == GL_RENDERBUFFER) {
                                glGetIntegerv(GL_MAX_SAMPLES, &max_samples);
                                max_samples_name = "GL_MAX_SAMPLES";
                        } else {
                                glGetIntegerv(GL_MAX_DEPTH_TEXTURE_SAMPLES,
                                              &max_samples);
                                max_samples_name = "GL_MAX_DEPTH_TEXTURE_SAMPLES";
                        }

                        for (j = 0; j < ARRAY_SIZE(valid_depth_formats); j++) {
                                pass = try(valid_targets_with_tms[i],
                                           valid_depth_formats[j],
                                           max_samples,
                                           max_samples_name)
                                        && pass;
                        }

                        glGetIntegerv(GL_MAX_INTEGER_SAMPLES, &max_samples);
                        max_samples_name = "GL_MAX_INTEGER_SAMPLES";

                        for (j = 0; j < ARRAY_SIZE(valid_integer_formats); j++) {
                                pass = try(valid_targets_with_tms[i],
                                           valid_integer_formats[j],
                                           max_samples,
                                           max_samples_name)
                                        && pass;
                        }
                }
        }

        piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}
