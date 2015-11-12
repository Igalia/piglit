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
 * \file samples-pnames.c
 * Verify a handful of conditions required by the spec for
 * NUM_SAMPLE_COUNTS and SAMPLES pnames.
 *
 * This cover query1 overrun test, plus query1 api-error.
 */

#include "common.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;
	config.window_visual = PIGLIT_GL_VISUAL_RGB;

PIGLIT_GL_TEST_CONFIG_END

static const GLenum without_multisample_targets[] = {
        GL_TEXTURE_1D,
        GL_TEXTURE_1D_ARRAY,
        GL_TEXTURE_2D,
        GL_TEXTURE_2D_ARRAY,
        GL_TEXTURE_3D,
        GL_TEXTURE_CUBE_MAP,
        GL_TEXTURE_CUBE_MAP_ARRAY,
        GL_TEXTURE_RECTANGLE,
        GL_TEXTURE_BUFFER
};

static const GLenum non_renderable_internalformats[] = {
	GL_COMPRESSED_RGB,
	GL_COMPRESSED_RGBA,
	GL_COMPRESSED_ALPHA,
	GL_COMPRESSED_LUMINANCE,
	GL_COMPRESSED_LUMINANCE_ALPHA,
	GL_COMPRESSED_INTENSITY,
	GL_COMPRESSED_SRGB,
	GL_COMPRESSED_SRGB_ALPHA,
	GL_COMPRESSED_SLUMINANCE,
	GL_COMPRESSED_SLUMINANCE_ALPHA,
	GL_COMPRESSED_RED,
	GL_COMPRESSED_RG,
	GL_COMPRESSED_RED_RGTC1,
	GL_COMPRESSED_SIGNED_RED_RGTC1,
	GL_COMPRESSED_RG_RGTC2,
	GL_COMPRESSED_SIGNED_RG_RGTC2,
	GL_COMPRESSED_RGBA_BPTC_UNORM_ARB,
	GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM_ARB,
	GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT_ARB,
	GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT_ARB,
	GL_COMPRESSED_RGB_S3TC_DXT1_EXT,
	GL_COMPRESSED_RGBA_S3TC_DXT1_EXT,
	GL_COMPRESSED_RGBA_S3TC_DXT3_EXT,
	GL_COMPRESSED_RGBA_S3TC_DXT5_EXT,
	GL_COMPRESSED_RGB_FXT1_3DFX,
	GL_COMPRESSED_RGBA_FXT1_3DFX,
	GL_COMPRESSED_SRGB_S3TC_DXT1_EXT,
	GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT,
	GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT,
	GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT,
	GL_COMPRESSED_LUMINANCE_LATC1_EXT,
	GL_COMPRESSED_SIGNED_LUMINANCE_LATC1_EXT,
	GL_COMPRESSED_LUMINANCE_ALPHA_LATC2_EXT,
	GL_COMPRESSED_SIGNED_LUMINANCE_ALPHA_LATC2_EXT,
	GL_YCBCR_MESA,
	GL_GREEN_INTEGER_EXT,
	GL_BLUE_INTEGER_EXT,
	GL_ALPHA_INTEGER_EXT,
	GL_RGB_INTEGER_EXT,
	GL_RGBA_INTEGER_EXT,
	GL_BGR_INTEGER_EXT,
	GL_BGRA_INTEGER_EXT,
	GL_LUMINANCE_INTEGER_EXT,
	GL_LUMINANCE_ALPHA_INTEGER_EXT,
	GL_RGB9_E5
};

enum piglit_result
piglit_display(void)
{
	return PIGLIT_FAIL;
}

static void
reset_data_and_copy(struct test_data *data,
                    struct test_data *data_copy)
{
        unsigned i;

        data_copy->testing64 = data->testing64;
        data_copy->params_size = data->params_size;
        sync_test_data(data_copy);

        for (i = 0; i < data->params_size; i++) {
                /* It seems very unlikely that an implementation will
                 * support 0xDEADBEEF samples*/
                set_params_at_index(data, i,  0xDEADBEEF);
                set_params_at_index(data_copy, i, 0xDEADBEEF);
        }
}

static bool
check_params_unmodified(struct test_data data,
                        struct test_data data_copy)
{
        unsigned i;
        bool result;

        for (i = 0; i < data.params_size; i++) {
                result = equal_at_index(data, data_copy, i);

                if (!result)
                        break;
        }

        return result;
}

static bool
try(const GLenum *targets, unsigned num_targets,
    const GLenum *internalformats, unsigned num_internalformats,
    GLenum pname, struct test_data data)
{
        struct test_data data_copy;
        bool pass = true;
        unsigned i;
        unsigned j;

        data_copy.params = NULL;

	for (i = 0; i < num_targets; i++) {
		for (j = 0; j < num_internalformats; j++) {
                        bool error_test;
                        bool value_test;

                        /* Needed for to check if the data was unmodified  */
                        reset_data_and_copy(&data, &data_copy);

                        /* First we try with size 0 */
                        data.callback(targets[i], internalformats[j],
                                      pname, 0, data.params);

                        error_test =
                                piglit_check_gl_error(GL_NO_ERROR);

                        value_test = check_params_unmodified(data, data_copy);

                        /* Now we try with a size */
                        data.callback(targets[i], internalformats[j],
                                      pname, data.params_size,
                                      data.params);

                        error_test = error_test &&
                                piglit_check_gl_error(GL_NO_ERROR);

                        if (pname == GL_NUM_SAMPLE_COUNTS) {
                                value_test = value_test &&
                                        check_params_zero(data);
                        } else {
                                value_test = value_test &&
                                        check_params_unmodified(data, data_copy);
                        }

                        if (error_test && value_test)
                                continue;

                        print_failing_case(targets[i], internalformats[j], pname, data);

                        pass = false;
                }
        }

        clean_test_data(&data_copy);

	return pass;
}

static bool
check_num_sample_counts(void)
{
        bool pass = true;
        struct test_data data;

        data.params = NULL;
        data.params_size = 64;

        for (data.testing64 = 0; data.testing64 <= 1; data.testing64++) {
                sync_test_data(&data);
                /* The GL_ARB_internalformat_query2 spec says:
                 *
                 * - NUM_SAMPLE_COUNTS: The number of sample counts that would
                 *   be returned by querying SAMPLES is returned in <params>.
                 *      * If <internalformat> is not color-renderable,
                 *        depth-renderable, or stencil-renderable (as defined
                 *        in section 4.4.4) ...
                 */
                pass = try(valid_targets, ARRAY_SIZE(valid_targets),
                           non_renderable_internalformats, ARRAY_SIZE(non_renderable_internalformats),
                           GL_NUM_SAMPLE_COUNTS, data)
                        && pass;

                /*
                 *        ... or if <target> does not support
                 *        multiple samples (ie other than
                 *        TEXTURE_2D_MULTISAMPLE,
                 *        TEXTURE_2D_MULTISAMPLE_ARRAY, or RENDERBUFFER), 0 is
                 *        returned.
                 */
                pass = try(without_multisample_targets, ARRAY_SIZE(without_multisample_targets),
                           valid_internalformats, ARRAY_SIZE(valid_internalformats),
                           GL_NUM_SAMPLE_COUNTS, data)
                        && pass;
        }

        piglit_report_subtest_result(pass ? PIGLIT_PASS : PIGLIT_FAIL,
                                     "%s", piglit_get_gl_enum_name(GL_NUM_SAMPLE_COUNTS));

        clean_test_data(&data);

        return pass;
}

static bool
check_samples(void)
{
        bool pass = true;
        struct test_data data;

        data.params = NULL;
        data.params_size = 64;

        /*
         * Note: Checking if we are getting a proper min and maximum
         * value, and if the returned values are positive in
         * descending order is done at minmax test, as that one needs
         * to use SAMPLES and NUM_SAMPLES_COUNT combined. This test
         * file is about testing each pname individually.
         */

        for (data.testing64 = 0; data.testing64 <= 1; data.testing64++) {
                sync_test_data(&data);

                /* The GL_ARB_internalformat_query2 spec for SAMPLES says:
                 *
                 * If <internalformat> is not color-renderable, depth-renderable, or
                 * stencil-renderable (as defined in section 4.4.4),
                 */

                pass = try(valid_targets, ARRAY_SIZE(valid_targets),
                           non_renderable_internalformats, ARRAY_SIZE(non_renderable_internalformats),
                           GL_SAMPLES, data)
                        && pass;

                /*
                 * or if <target> does not support multiple samples (ie other
                 * than TEXTURE_2D_MULTISAMPLE, TEXTURE_2D_MULTISAMPLE_ARRAY,
                 * or RENDERBUFFER), <params> is not modified.
                 */

                pass = try(without_multisample_targets, ARRAY_SIZE(without_multisample_targets),
                           valid_internalformats, ARRAY_SIZE(valid_internalformats),
                           GL_SAMPLES, data)
                        && pass;
        }

        piglit_report_subtest_result(pass ? PIGLIT_PASS : PIGLIT_FAIL,
                                     "%s", piglit_get_gl_enum_name(GL_SAMPLES));

        clean_test_data(&data);

        return pass;
}

void
piglit_init(int argc, char **argv)
{
        bool pass = true;

        piglit_require_extension("GL_ARB_framebuffer_object");
        piglit_require_extension("GL_ARB_internalformat_query2");

        pass = check_num_sample_counts() && pass;
        pass = check_samples() && pass;

        piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}
