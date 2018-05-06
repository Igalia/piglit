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
 * \file filter.c
 *
 * Verify the outcome for the GetInternalformativ pname FILTER. From
 * the spec it just says that:
 *
 * "Possible values returned are FULL_SUPPORT, CAVEAT_SUPPORT, or NONE."
 *
 * In addition to ensure that the returned value is one of them (so as
 * generic-pname would do) this test also checks for the well known
 * cases were multi-texel filtering is not supported:
 *
 * * Multi-sample textures ( GL_TEXTURE_2D_MULTISAMPLE,
 *   GL_TEXTURE_2D_MULTISAMPLE_ARRAY). [1][2]
 * * Any resource using a integer internalformat [3][4]
 * * Texture buffer objects [5]
 *
 *  [1] https://www.opengl.org/registry/specs/ARB/texture_multisample.txt
 *  [2] "2.11.12 Multisample Texel Fetches" at 4.2 core spec
 *  [3] https://www.opengl.org/registry/specs/EXT/texture_integer.txt
 *  [4] "3.9.14 Texture Completeness" at 4.2 core spec
 *  [5] https://www.opengl.org/registry/specs/ARB/texture_buffer_object.txt
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

static bool
is_multisample_target(GLenum target)
{
        switch(target) {
        case GL_TEXTURE_2D_MULTISAMPLE:
        case GL_TEXTURE_2D_MULTISAMPLE_ARRAY:
                return true;

        default:
                return false;
        }
}

static bool
is_integer_internalformat(GLenum internalformat)
{
        switch(internalformat) {
        /* integer internalformats from
         * common.h:valid_internalformats, or in other words, from 4.2
         * core spec table 3.12 */
        case GL_RGB10_A2UI:
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
                return true;

        default:
                return false;
        }
}

/*
 * Already known cases (by OpenGL spec) where multi-texel filtering is
 * supported.
 */
static bool
is_multi_texel_filtering_supported(GLenum target,
                                   GLenum internalformat)
{
        return (target != GL_TEXTURE_BUFFER &&
                !is_multisample_target(target) &&
                !is_integer_internalformat(internalformat));

}

/*
 * print_failing_case just prints out details of which case
 * failed. Here we are trying to add debug info about why the test
 * failed. It assumes that fails on getting the wrong value, not on
 * getting a opengl error
 */
static void
print_failing_details(GLenum target,
                      GLenum internalformat)
{
        if (target == GL_TEXTURE_BUFFER || is_multisample_target(target))
                fprintf(stderr, "\tTarget %s doesn't support multi-texel filtering\n",
                        piglit_get_gl_enum_name(target));

        if (is_integer_internalformat(internalformat))
                fprintf(stderr, "\tInteger internalformats like %s doesn't "
                        "support multi-texel filtering\n",
                        piglit_get_gl_enum_name(internalformat));

}
/* Equivalent to common:try_basic, but also checking that the
 * well-known cases that doesn't support multi-texel filtering is not
 * supported.*/
bool
try_local(const GLenum *targets, unsigned num_targets,
          const GLenum *internalformats, unsigned num_internalformats,
          const GLenum pname,
          test_data *data)
{
        bool pass = true;
        unsigned i;
        unsigned j;
        static const GLint possible_values[] = {
                GL_NONE,
                GL_CAVEAT_SUPPORT,
                GL_FULL_SUPPORT,
        };

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

                        if (!supported ||
                            !is_multi_texel_filtering_supported(targets[i],
                                                                internalformats[j])) {
                                value_test = test_data_is_unsupported_response(data, pname);
                        } else {
                                value_test =
                                        test_data_check_possible_values(data,
                                                                        possible_values,
                                                                        ARRAY_SIZE(possible_values));
                        }

                        if (error_test && value_test)
                                continue;

                        /* If we are here, the test is failing */
                        print_failing_case(targets[i], internalformats[j],
                                           pname, data);

                        if (!value_test)
                                print_failing_details(targets[i], internalformats[j]);

                        pass = false;
                }
        }

	return pass;
}

static bool
check_filter()
{
        bool pass = true;
        test_data *data = test_data_new(0, 1);
        int testing64;

        for (testing64 = 0; testing64 <= 1; testing64++) {
                test_data_set_testing64(data, testing64);
                pass = try_local(valid_targets, ARRAY_SIZE(valid_targets),
                                 valid_internalformats, num_valid_internalformats,
                                 GL_FILTER, data)
                        && pass;
        }

        piglit_report_subtest_result(pass ? PIGLIT_PASS : PIGLIT_FAIL,
                                     "%s", piglit_get_gl_enum_name(GL_FILTER));

        test_data_clear(&data);

        return pass;
}

void
piglit_init(int argc, char **argv)
{
        bool pass = true;

        piglit_require_extension("GL_ARB_internalformat_query2");

        pass = check_filter() && pass;

        piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}
