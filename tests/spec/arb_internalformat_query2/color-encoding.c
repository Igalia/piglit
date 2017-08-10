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
 * \file color-encoding.c
 * Verify a handful of conditions required by COLOR_ENCODING
 */

#include "common.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;
	config.window_visual = PIGLIT_GL_VISUAL_RGB;
	config.khr_no_error_support = PIGLIT_NO_ERRORS;

PIGLIT_GL_TEST_CONFIG_END

enum piglit_result
piglit_display(void)
{
	return PIGLIT_FAIL;
}

static const GLint color_format_possible_values[] = {
        GL_NONE,
        GL_LINEAR,
        GL_SRGB,
};

static const GLenum color_internalformats[] = {
        GL_RED,
        GL_RG,
        GL_RGB,
        GL_RGBA,
        GL_R8,
        GL_R8_SNORM,
        GL_R16,
        GL_R16_SNORM,
        GL_RG8,
        GL_RG8_SNORM,
        GL_RG16,
        GL_RG16_SNORM,
        GL_R3_G3_B2,
        GL_RGB4,
        GL_RGB5,
        GL_RGB8,
        GL_RGB8_SNORM,
        GL_RGB10,
        GL_RGB12,
        GL_RGB16,
        GL_RGB16_SNORM,
        GL_RGBA2,
        GL_RGBA4,
        GL_RGB5_A1,
        GL_RGBA8,
        GL_RGBA8_SNORM,
        GL_RGB10_A2,
        GL_RGB10_A2UI,
        GL_RGBA12,
        GL_RGBA16,
        GL_RGBA16_SNORM,
        GL_SRGB8,
        GL_SRGB8_ALPHA8,
        GL_R16F,
        GL_RG16F,
        GL_RGB16F,
        GL_RGBA16F,
        GL_R32F,
        GL_RG32F,
        GL_RGB32F,
        GL_RGBA32F,
        GL_R11F_G11F_B10F,
        GL_RGB9_E5,
        GL_R8I,
        GL_R8UI,
        GL_R16I,
        GL_R16UI,
        GL_R32I,
        GL_R32UI,
        GL_RG8I,
        GL_RG16I,
        GL_RG16UI,
        GL_RG32I,
        GL_RG32UI,
        GL_RGB8I,
        GL_RGB8UI,
        GL_RGB16I,
        GL_RGB16UI,
        GL_RGB32I,
        GL_RGB32UI,
        GL_RGBA8I,
        GL_RGBA8UI,
        GL_RGBA16I,
        GL_RGBA16UI,
        GL_RGBA32I,
        GL_RGBA32UI,
        GL_COMPRESSED_RED,
        GL_COMPRESSED_RG,
        GL_COMPRESSED_RGB,
        GL_COMPRESSED_RGBA,
        GL_COMPRESSED_SRGB,
        GL_COMPRESSED_SRGB_ALPHA,
        GL_COMPRESSED_RED_RGTC1,
        GL_COMPRESSED_SIGNED_RED_RGTC1,
        GL_COMPRESSED_RG_RGTC2,
        GL_COMPRESSED_SIGNED_RG_RGTC2,
        GL_COMPRESSED_RGBA_BPTC_UNORM,
        GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM,
        GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT,
        GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT,
};

static const GLint non_color_format_possible_values[] = {
        GL_NONE,
};

static const GLenum non_color_internalformats[] = {
        GL_DEPTH_COMPONENT,
        GL_DEPTH_STENCIL,
        GL_DEPTH_COMPONENT16,
        GL_DEPTH_COMPONENT24,
        GL_DEPTH_COMPONENT32,
        GL_DEPTH_COMPONENT32F,
        GL_DEPTH24_STENCIL8,
        GL_DEPTH32F_STENCIL8,
};

static bool
check_color_encoding(void)
{
        bool pass = true;
        test_data *data = test_data_new(0, 1);
        int testing64;

        for (testing64 = 0; testing64 <= 1; testing64++) {
                test_data_set_testing64(data, testing64);

                /*
                 * From spec:
                 * "COLOR_ENCODING:
                 * <skip>
                 *
                 * Possible values for color buffers are LINEAR or
                 * SRGB, for linear or sRGB-encoded color components,
                 * respectively..."
                 */
                pass = try_basic(valid_targets, ARRAY_SIZE(valid_targets),
                                 color_internalformats, ARRAY_SIZE(color_internalformats),
                                 GL_COLOR_ENCODING,
                                 color_format_possible_values,
                                 ARRAY_SIZE(color_format_possible_values),
                                 data)
                        && pass;

                /*
                 * From spec (continuing previous comment)
                 * "For non-color formats (such as depth or stencil),
                 * or for unsupported resources, the value NONE is
                 * returned."
                 */
                pass = try_basic(valid_targets, ARRAY_SIZE(valid_targets),
                                 non_color_internalformats, ARRAY_SIZE(non_color_internalformats),
                                 GL_COLOR_ENCODING,
                                 non_color_format_possible_values,
                                 ARRAY_SIZE(non_color_format_possible_values),
                                 data)
                        && pass;
        }

        piglit_report_subtest_result(pass ? PIGLIT_PASS : PIGLIT_FAIL,
                                     "%s", piglit_get_gl_enum_name(GL_COLOR_ENCODING));

        test_data_clear(&data);

        return pass;
}

void
piglit_init(int argc, char **argv)
{
        bool pass = true;

        piglit_require_extension("GL_ARB_internalformat_query2");

        pass = check_color_encoding()
                && pass;

        piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}
