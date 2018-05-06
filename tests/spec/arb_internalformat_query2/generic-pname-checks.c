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
 * \file gneric-pname-checks.c
 *
 * Verify a handful of conditions required by the spec for a set of
 * pnames with the most generic conditions. Specifically it tests for the
 * pnames that only require:
 *
 *  a) That if not supported, the returned value is always the same
 *  b) If supported, the returned value is among a fixed set of
 *     possible values.
 *
 */

#include "common.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;
	config.window_visual = PIGLIT_GL_VISUAL_RGB;
	config.khr_no_error_support = PIGLIT_NO_ERRORS;

PIGLIT_GL_TEST_CONFIG_END

static const GLenum pnames_common_outcome[] = {
        GL_FRAMEBUFFER_RENDERABLE,
        GL_FRAMEBUFFER_RENDERABLE_LAYERED,
        GL_FRAMEBUFFER_BLEND,
        GL_READ_PIXELS,
        GL_MANUAL_GENERATE_MIPMAP,
        GL_AUTO_GENERATE_MIPMAP,
        GL_SRGB_READ,
        GL_SRGB_WRITE,
        GL_SRGB_DECODE_ARB,
        GL_VERTEX_TEXTURE,
        GL_TESS_CONTROL_TEXTURE,
        GL_TESS_EVALUATION_TEXTURE,
        GL_GEOMETRY_TEXTURE,
        GL_FRAGMENT_TEXTURE,
        GL_COMPUTE_TEXTURE,
        GL_TEXTURE_SHADOW,
        GL_TEXTURE_GATHER,
        GL_TEXTURE_GATHER_SHADOW,
        GL_SHADER_IMAGE_LOAD,
        GL_SHADER_IMAGE_STORE,
        GL_SHADER_IMAGE_ATOMIC,
        GL_SIMULTANEOUS_TEXTURE_AND_DEPTH_TEST,
        GL_SIMULTANEOUS_TEXTURE_AND_STENCIL_TEST,
        GL_SIMULTANEOUS_TEXTURE_AND_DEPTH_WRITE,
        GL_SIMULTANEOUS_TEXTURE_AND_STENCIL_WRITE,
        GL_CLEAR_BUFFER,
        GL_TEXTURE_VIEW,
};

static const GLint possible_values_common[] = {
        GL_NONE,
        GL_CAVEAT_SUPPORT,
        GL_FULL_SUPPORT,
};

static const GLenum pnames_true_false[] = {
        GL_COLOR_RENDERABLE,
        GL_DEPTH_RENDERABLE,
        GL_STENCIL_RENDERABLE,
        GL_TEXTURE_COMPRESSED,
};

static const GLint possible_values_true_false[] = {
        GL_TRUE,
        GL_FALSE
};

/* From query2 spec:
 *
 * "TEXTURE_IMAGE_FORMAT:
 * <skip>
 * Possible values include any value that is legal to pass for the
 * <format> parameter to the Tex*Image*D commands, or NONE if the
 * resource is not supported for this operation."
 *
 * From 4.2 core spec:
 * "TexImage3D
 * <skip>
 * format, type, and data specify the format of the image data, the
 * type of those data, and a reference to the image data in the cur-
 * rently bound pixel unpack buffer or client memory, as described in
 * section 3.7.2. The format STENCIL_INDEX is not allowed."
 *
 * This is basically Table 3.3 (defined at section 3.7.2) minus
 * STENCIL_INDEX.
 */
static GLint possible_values_texture_image_format[] = {
        /* Table 3.3 minus STENCIL_INDEX */
        GL_DEPTH_COMPONENT,
        GL_DEPTH_STENCIL,
        GL_RED,
        GL_GREEN,
        GL_BLUE,
        GL_RG,
        GL_RGB,
        GL_RGBA,
        GL_BGR,
        GL_BGRA,
        GL_RED_INTEGER,
        GL_GREEN_INTEGER,
        GL_BLUE_INTEGER,
        GL_RG_INTEGER,
        GL_RGB_INTEGER,
        GL_RGBA_INTEGER,
        GL_BGR_INTEGER,
        GL_BGRA_INTEGER,
        /* GL_NONE from query2 TEXTURE_IMAGE_FORMAT spec */
        GL_NONE
};

/*
 * From query2 spec:
 *
 * "GET_TEXTURE_IMAGE_FORMAT:
 *  <skip>
 * Possible values include any value that is legal to pass for the
 * <format> parameter to GetTexImage, or NONE if the resource does not
 * support this operation, or if GetTexImage is not supported."
 *
 * From 4.2 core spec (section 6.1.4):
 * "format is a pixel format from table 3.3"
 * "Calling GetTexImage with a format of STENCIL_INDEX causes the
 *  error INVALID_ENUM ."
 *
 * So on 4.2 the possible values would be the same that
 * texture_image_format.
 *
 * But, since 4.4 (section 8.11.4) STENCIL_INDEX is a valid enum, and
 * INVALID_OPERATION would be raised if used in combination with a
 * wrong internalformat.
 *
 * So possible_values_get_texture_image_format would include
 * STENCIL_INDEX. When passing possible values to
 * GET_TEXTURE_IMAGE_FORMAT, it would need to fallback to
 * possible_values_texture_image_format if needed.
 */
static GLint possible_values_get_texture_image_format[] = {
        /* Table 3.3 */
        GL_STENCIL_INDEX,
        GL_DEPTH_COMPONENT,
        GL_DEPTH_STENCIL,
        GL_RED,
        GL_GREEN,
        GL_BLUE,
        GL_RG,
        GL_RGB,
        GL_RGBA,
        GL_BGR,
        GL_BGRA,
        GL_RED_INTEGER,
        GL_GREEN_INTEGER,
        GL_BLUE_INTEGER,
        GL_RG_INTEGER,
        GL_RGB_INTEGER,
        GL_RGBA_INTEGER,
        GL_BGR_INTEGER,
        GL_BGRA_INTEGER,
        /* GL_NONE from query2 TEXTURE_IMAGE_FORMAT spec */
        GL_NONE
};

/*
 * From query2 spec:
 *
 * "TEXTURE_IMAGE_TYPE:
 * <skip>
 * Possible values include any value that is legal to pass for the
 * <type> parameter to the Tex*Image*D commands, or NONE if the
 * resource is not supported for this operation."
 *
 * From 4.2 core spec:
 * "TexImage3D
 * <skip>
 * format, type, and data specify the format of the image data, the
 * type of those data, and a reference to the image data in the cur-
 * rently bound pixel unpack buffer or client memory, as described in
 * section 3.7.2. The format STENCIL_INDEX is not allowed."
 *
 * This is basically Table 3.2 (defined at section 3.7.2)
 */
static GLint possible_values_texture_image_type[] = {
        /* Table 3.2 */
        GL_UNSIGNED_BYTE,
        GL_BYTE,
        GL_UNSIGNED_SHORT,
        GL_SHORT,
        GL_UNSIGNED_INT,
        GL_INT,
        GL_HALF_FLOAT,
        GL_FLOAT,
        GL_UNSIGNED_BYTE_3_3_2,
        GL_UNSIGNED_BYTE_2_3_3_REV,
        GL_UNSIGNED_SHORT_5_6_5,
        GL_UNSIGNED_SHORT_5_6_5_REV,
        GL_UNSIGNED_SHORT_4_4_4_4,
        GL_UNSIGNED_SHORT_4_4_4_4_REV,
        GL_UNSIGNED_SHORT_5_5_5_1,
        GL_UNSIGNED_SHORT_1_5_5_5_REV,
        GL_UNSIGNED_INT_8_8_8_8,
        GL_UNSIGNED_INT_8_8_8_8_REV,
        GL_UNSIGNED_INT_10_10_10_2,
        GL_UNSIGNED_INT_2_10_10_10_REV,
        GL_UNSIGNED_INT_24_8,
        GL_UNSIGNED_INT_10F_11F_11F_REV,
        GL_UNSIGNED_INT_5_9_9_9_REV,
        GL_FLOAT_32_UNSIGNED_INT_24_8_REV,
        /* GL_NONE from query2 TEXTURE_IMAGE_TYPE spec */
        GL_NONE
};

enum piglit_result
piglit_display(void)
{
	return PIGLIT_FAIL;
}

/*
 * Executes try_basic on a list of pnames/possible values.
 *
 * check_basic and try_basic are splitted because for some pnames, we
 * would need to check more than just try_basic.
 */
static bool
check_basic(const GLenum *pnames, unsigned num_pnames,
            const GLint *possible_values, unsigned num_possible_values)
{
        bool check_pass = true;
        test_data *data = test_data_new(0, 1);
        unsigned i;
        int testing64;

        for (i = 0; i < num_pnames; i++) {
                bool pass = true;

                for (testing64 = 0; testing64 <= 1; testing64++) {
                        test_data_set_testing64(data, testing64);

                        pass = try_basic(valid_targets, ARRAY_SIZE(valid_targets),
                                         valid_internalformats, num_valid_internalformats,
                                         pnames[i],
                                         possible_values, num_possible_values,
                                         data)
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
        GLenum pname;

	piglit_require_extension("GL_ARB_internalformat_query2");
        initialize_valid_internalformats();

        pname = GL_INTERNALFORMAT_PREFERRED;
        pass = check_basic(&pname, 1, NULL, 0) && pass;

        pass = check_basic(pnames_common_outcome, ARRAY_SIZE(pnames_common_outcome),
                           possible_values_common, ARRAY_SIZE(possible_values_common))
                && pass;

        pass = check_basic(pnames_true_false, ARRAY_SIZE(pnames_true_false),
                           possible_values_true_false, ARRAY_SIZE(possible_values_true_false))
                && pass;

        pname = GL_TEXTURE_IMAGE_FORMAT;
        pass = check_basic(&pname, 1, possible_values_texture_image_format,
                           ARRAY_SIZE(possible_values_texture_image_format))
                && pass;

        pname = GL_TEXTURE_IMAGE_TYPE;
        pass = check_basic(&pname, 1, possible_values_texture_image_type,
                           ARRAY_SIZE(possible_values_texture_image_type))
                && pass;

        pname = GL_GET_TEXTURE_IMAGE_FORMAT;

        /* To know why this gl version if needed, see comment at
         * possible_values_get_texture_image_format */
        if (piglit_get_gl_version() < 44) {
                pass = check_basic(&pname, 1, possible_values_texture_image_format,
                                   ARRAY_SIZE(possible_values_texture_image_format))
                        && pass;
        } else {
                pass = check_basic(&pname, 1, possible_values_get_texture_image_format,
                                   ARRAY_SIZE(possible_values_get_texture_image_format))
                        && pass;
        }

        /*
         * From spec:
         * "GET_TEXTURE_IMAGE_TYPE:
         *  <skip>
         *  Possible values include any value that is legal to pass
         *  for the <type> parameter to GetTexImage, or NONE if the
         *  resource does not support this operation, or if
         *  GetTexImage is not supported."
         *
         * From 4.2 spec (section 6.1.4) this is table 3.2, that are
         * also the possible values for TEXTURE_IMAGE_TYPE, so we
         * reuse that list here.
         *
         **/
        pname = GL_GET_TEXTURE_IMAGE_TYPE;
        pass = check_basic(&pname, 1, possible_values_texture_image_type,
                           ARRAY_SIZE(possible_values_texture_image_type))
                && pass;

        piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}
