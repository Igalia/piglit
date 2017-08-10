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
 * \file image-texture.c
 *
 * Verify conditions defined on the spec for the pnames that have to return
 * values in the Table 3.22 of the OpenGL 4.2 spec:
 *
 *  * TEXEL_SIZE
 *  * IMAGE_COMPATIBILITY_CLASS
 *  * IMAGE_PIXEL_FORMAT
 *  * IMAGE_PIXEL_TYPE
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
        GL_IMAGE_TEXEL_SIZE,
        GL_IMAGE_COMPATIBILITY_CLASS,
        GL_IMAGE_PIXEL_FORMAT,
        GL_IMAGE_PIXEL_TYPE,
};

struct imagetexture {
        GLenum format;
        int texel_size;
        GLenum pixel_format;
        GLenum pixel_type;
        GLenum compatibility_class;
};

/* Table 3.22, in OpenGL 4.2 Core specification */
static const struct imagetexture imagetexture_table[] = {
        {GL_RGBA32F, 128, GL_RGBA, GL_FLOAT, GL_IMAGE_CLASS_4_X_32},
        {GL_RGBA16F, 64, GL_RGBA, GL_HALF_FLOAT, GL_IMAGE_CLASS_4_X_16},
        {GL_RG32F, 64, GL_RG, GL_FLOAT, GL_IMAGE_CLASS_2_X_32},
        {GL_RG16F, 32, GL_RG, GL_HALF_FLOAT, GL_IMAGE_CLASS_2_X_16},
        {GL_R11F_G11F_B10F, 32, GL_RGB, GL_UNSIGNED_INT_10F_11F_11F_REV, GL_IMAGE_CLASS_11_11_10},
        {GL_R32F, 32, GL_RED, GL_FLOAT, GL_IMAGE_CLASS_1_X_32},
        {GL_R16F, 16, GL_RED, GL_HALF_FLOAT, GL_IMAGE_CLASS_1_X_16},
        {GL_RGBA32UI, 128, GL_RGBA_INTEGER, GL_UNSIGNED_INT, GL_IMAGE_CLASS_4_X_32},
        {GL_RGBA16UI, 64, GL_RGBA_INTEGER, GL_UNSIGNED_SHORT, GL_IMAGE_CLASS_4_X_16},
        {GL_RGB10_A2UI, 32, GL_RGBA_INTEGER, GL_UNSIGNED_INT_2_10_10_10_REV, GL_IMAGE_CLASS_10_10_10_2},
        {GL_RGBA8UI, 32, GL_RGBA_INTEGER, GL_UNSIGNED_BYTE, GL_IMAGE_CLASS_4_X_8},
        {GL_RG32UI, 64, GL_RG_INTEGER, GL_UNSIGNED_INT, GL_IMAGE_CLASS_2_X_32},
        {GL_RG16UI, 32, GL_RG_INTEGER, GL_UNSIGNED_SHORT, GL_IMAGE_CLASS_2_X_16},
        {GL_RG8UI, 16, GL_RG_INTEGER, GL_UNSIGNED_BYTE, GL_IMAGE_CLASS_2_X_8},
        {GL_R32UI, 32, GL_RED_INTEGER, GL_UNSIGNED_INT, GL_IMAGE_CLASS_1_X_32},
        {GL_R16UI, 16, GL_RED_INTEGER, GL_UNSIGNED_SHORT, GL_IMAGE_CLASS_1_X_16},
        {GL_R8UI, 8, GL_RED_INTEGER, GL_UNSIGNED_BYTE, GL_IMAGE_CLASS_1_X_8},
        {GL_RGBA32I, 128, GL_RGBA_INTEGER, GL_INT, GL_IMAGE_CLASS_4_X_32},
        {GL_RGBA16I, 64, GL_RGBA_INTEGER, GL_SHORT, GL_IMAGE_CLASS_4_X_16},
        {GL_RGBA8I, 32, GL_RGBA_INTEGER, GL_BYTE, GL_IMAGE_CLASS_4_X_8},
        {GL_RG32I, 64, GL_RG_INTEGER, GL_INT, GL_IMAGE_CLASS_2_X_32},
        {GL_RG16I, 32, GL_RG_INTEGER, GL_SHORT, GL_IMAGE_CLASS_2_X_16},
        {GL_RG8I, 16, GL_RG_INTEGER, GL_BYTE, GL_IMAGE_CLASS_2_X_8},
        {GL_R32I, 32, GL_RED_INTEGER, GL_INT, GL_IMAGE_CLASS_1_X_32},
        {GL_R16I, 16, GL_RED_INTEGER, GL_SHORT, GL_IMAGE_CLASS_1_X_16},
        {GL_R8I, 8, GL_RED_INTEGER, GL_BYTE, GL_IMAGE_CLASS_1_X_8},
        {GL_RGBA16, 64, GL_RGBA, GL_UNSIGNED_SHORT, GL_IMAGE_CLASS_4_X_16},
        {GL_RGB10_A2, 32, GL_RGBA, GL_UNSIGNED_INT_2_10_10_10_REV, GL_IMAGE_CLASS_10_10_10_2},
        {GL_RGBA8, 32, GL_RGBA, GL_UNSIGNED_BYTE, GL_IMAGE_CLASS_4_X_8},
        {GL_RG16, 32, GL_RG, GL_UNSIGNED_SHORT, GL_IMAGE_CLASS_2_X_16},
        {GL_RG8, 16, GL_RG, GL_UNSIGNED_BYTE, GL_IMAGE_CLASS_2_X_8},
        {GL_R16, 16, GL_RED, GL_UNSIGNED_SHORT, GL_IMAGE_CLASS_1_X_16},
        {GL_R8, 8, GL_RED, GL_UNSIGNED_BYTE, GL_IMAGE_CLASS_1_X_8},
        {GL_RGBA16_SNORM, 64, GL_RGBA, GL_SHORT, GL_IMAGE_CLASS_4_X_16},
        {GL_RGBA8_SNORM, 32, GL_RGBA, GL_BYTE, GL_IMAGE_CLASS_4_X_8},
        {GL_RG16_SNORM, 32, GL_RG, GL_SHORT, GL_IMAGE_CLASS_2_X_16},
        {GL_RG8_SNORM, 16, GL_RG, GL_BYTE, GL_IMAGE_CLASS_2_X_8},
        {GL_R16_SNORM, 16, GL_RED, GL_SHORT, GL_IMAGE_CLASS_1_X_16},
        {GL_R8_SNORM, 8, GL_RED, GL_BYTE, GL_IMAGE_CLASS_1_X_8},
};

static bool
try(const GLenum *targets, unsigned num_targets,
    const GLenum pname, test_data *data)
{
        bool pass = true;
        unsigned i,j;

        for (i = 0; i < num_targets; i++) {
                for (j = 0; j < ARRAY_SIZE(imagetexture_table); j++) {
                        bool error_test;
                        bool value_test;
                        bool supported;
                        GLint expected_value = -1;

                        supported = check_query2_dependencies(pname, targets[i])
                                && test_data_check_supported(data, targets[i],
                                                             imagetexture_table[j].format)
                                && (targets[i] != GL_RENDERBUFFER);

                        test_data_execute(data, targets[i],
                                          imagetexture_table[j].format,
                                          pname);

                        error_test =
                                piglit_check_gl_error(GL_NO_ERROR);

                        if (supported) {
                                GLint returned_value = test_data_value_at_index(data, 0);

                                switch(pname) {
                                case GL_IMAGE_TEXEL_SIZE:
                                        expected_value = imagetexture_table[j].texel_size;
                                        break;
                                case GL_IMAGE_COMPATIBILITY_CLASS:
                                        expected_value =
                                                imagetexture_table[j].compatibility_class;
                                        break;
                                case GL_IMAGE_PIXEL_FORMAT:
                                        expected_value = imagetexture_table[j].pixel_format;
                                        break;
                                case GL_IMAGE_PIXEL_TYPE:
                                        expected_value = imagetexture_table[j].pixel_type;
                                        break;
                                default:
                                        assert("incorrect pname for test");
                                        break;
                                }

                                value_test = (expected_value == returned_value);
                        } else {
                                value_test = test_data_is_unsupported_response(data, pname);
                        }

                        if (error_test && value_test)
                                continue;

                        print_failing_case_full(targets[i],
                                                imagetexture_table[j].format,
                                                pname, expected_value, data);

                        pass = false;
                }
        }
        return pass;
}

static bool
check_image_texture(void)
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

        pass =  check_image_texture()
                && pass;

        piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}
