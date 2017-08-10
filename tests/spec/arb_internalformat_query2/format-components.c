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
 * \file format-components.c
 *
 * Tests if the values returned by:
 *
 *  * COLOR_COMPONENTS
 *  * STENCIL_COMPONENTS
 *  * DEPTH_COMPONENTS
 *
 * are correct for all the internalformats.
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

static const GLenum pnames[] = {
        GL_COLOR_COMPONENTS,
        GL_STENCIL_COMPONENTS,
        GL_DEPTH_COMPONENTS,
};

static GLboolean
is_color_format(GLenum internalformat)
{
        switch (internalformat) {
        case GL_RED:
        case GL_RG:
        case GL_RGB:
        case GL_RGBA:
        case GL_R8:
        case GL_R8_SNORM:
        case GL_R16:
        case GL_R16_SNORM:
        case GL_RG8:
        case GL_RG8_SNORM:
        case GL_RG16:
        case GL_RG16_SNORM:
        case GL_R3_G3_B2:
        case GL_RGB4:
        case GL_RGB5:
        case GL_RGB8:
        case GL_RGB8_SNORM:
        case GL_RGB10:
        case GL_RGB12:
        case GL_RGB16:
        case GL_RGB16_SNORM:
        case GL_RGBA2:
        case GL_RGBA4:
        case GL_RGB5_A1:
        case GL_RGBA8:
        case GL_RGBA8_SNORM:
        case GL_RGB10_A2:
        case GL_RGB10_A2UI:
        case GL_RGBA12:
        case GL_RGBA16:
        case GL_RGBA16_SNORM:
        case GL_SRGB8:
        case GL_SRGB8_ALPHA8:
        case GL_R16F:
        case GL_RG16F:
        case GL_RGB16F:
        case GL_RGBA16F:
        case GL_R32F:
        case GL_RG32F:
        case GL_RGB32F:
        case GL_RGBA32F:
        case GL_R11F_G11F_B10F:
        case GL_RGB9_E5:
        case GL_R8I:
        case GL_R8UI:
        case GL_R16I:
        case GL_R16UI:
        case GL_R32I:
        case GL_R32UI:
        case GL_RG8I:
        case GL_RG16I:
        case GL_RG16UI:
        case GL_RG32I:
        case GL_RG32UI:
        case GL_RGB8I:
        case GL_RGB8UI:
        case GL_RGB16I:
        case GL_RGB16UI:
        case GL_RGB32I:
        case GL_RGB32UI:
        case GL_RGBA8I:
        case GL_RGBA8UI:
        case GL_RGBA16I:
        case GL_RGBA16UI:
        case GL_RGBA32I:
        case GL_RGBA32UI:
        case GL_COMPRESSED_RED:
        case GL_COMPRESSED_RG:
        case GL_COMPRESSED_RGB:
        case GL_COMPRESSED_RGBA:
        case GL_COMPRESSED_SRGB:
        case GL_COMPRESSED_SRGB_ALPHA:
        case GL_COMPRESSED_RED_RGTC1:
        case GL_COMPRESSED_SIGNED_RED_RGTC1:
        case GL_COMPRESSED_RG_RGTC2:
        case GL_COMPRESSED_SIGNED_RG_RGTC2:
        case GL_COMPRESSED_RGBA_BPTC_UNORM:
        case GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM:
        case GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT:
        case GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT:
                return GL_TRUE;
        default:
                return GL_FALSE;
        }
}

static GLboolean
is_depth_format(GLenum internalformat)
{
        switch (internalformat) {
        case GL_DEPTH_COMPONENT:
        case GL_DEPTH_COMPONENT16:
        case GL_DEPTH_COMPONENT24:
        case GL_DEPTH_COMPONENT32:
        case GL_DEPTH_COMPONENT32F:
        case GL_DEPTH_STENCIL:
        case GL_DEPTH24_STENCIL8_EXT:
        case GL_DEPTH32F_STENCIL8:
                return GL_TRUE;
        default:
                return GL_FALSE;
        }
}
static GLboolean
is_stencil_format(GLenum internalformat)
{
        switch (internalformat) {
        case GL_STENCIL_INDEX:
        case GL_DEPTH_STENCIL:
        case GL_DEPTH24_STENCIL8_EXT:
        case GL_DEPTH32F_STENCIL8:
                return GL_TRUE;
        default:
                return GL_FALSE;
        }
}

static bool
try(const GLenum *targets, unsigned num_targets,
    const GLenum *internalformats, unsigned num_internalformats,
    const GLenum pname, test_data *data)
{
        bool pass = true;
        unsigned i;
        unsigned j;

        for (i = 0; i < num_targets; i++) {
                for (j = 0; j < num_internalformats; j++) {
                        bool error_test;
                        bool value_test = true;
                        bool supported;

                        supported = check_query2_dependencies(pname, targets[i])
                                && test_data_check_supported(data, targets[i],
                                                             internalformats[j]);

                        test_data_execute(data, targets[i], internalformats[j],
                                          pname);

                        error_test =
                                piglit_check_gl_error(GL_NO_ERROR);

                        if (!supported) {
                                value_test = test_data_is_unsupported_response(data, pname);
                        } else {
                                GLint returned_value = test_data_value_at_index(data, 0);
                                GLint expected_value = -1;

                                switch(pname) {
                                case GL_COLOR_COMPONENTS:
                                        expected_value = is_color_format(internalformats[j]);
                                        break;
                                case GL_STENCIL_COMPONENTS:
                                        expected_value = is_stencil_format(internalformats[j]);
                                        break;
                                case GL_DEPTH_COMPONENTS:
                                        expected_value = is_depth_format(internalformats[j]);
                                        break;
                                default:
                                        assert(!"incorrect pname for test");
                                        break;
                                }

                                value_test = (returned_value == expected_value);
                        }

                        if (error_test && value_test)
                                continue;

                        print_failing_case(targets[i], internalformats[j], pname, data);

                        pass = false;
                }
        }

        return pass;
}

static bool
check_format_components(void)
{
        bool check_pass = true;
        test_data *data = test_data_new(0, 1);
        unsigned i;
        int testing64;

        for (i = 0; i < ARRAY_SIZE(pnames); i++) {
                bool pass = true;

                for (testing64 = 0; testing64 <= 1; testing64++) {
                        test_data_set_testing64(data, testing64);

                        pass = try(valid_targets, ARRAY_SIZE(valid_targets),
                                   valid_internalformats, ARRAY_SIZE(valid_internalformats),
                                   pnames[i], data)
                                && pass;
                }

                piglit_report_subtest_result(pass ? PIGLIT_PASS : PIGLIT_FAIL,
                                             "%s", piglit_get_gl_enum_name(pnames[i]));

                check_pass = check_pass && pass;
        }

        test_data_clear(&data);

        return check_pass;
}

void
piglit_init(int argc, char **argv)
{
        bool pass = true;

        piglit_require_extension("GL_ARB_internalformat_query2");

        pass = check_format_components()
                && pass;

        piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}
