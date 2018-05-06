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
 * \file internalformat-size-checks.c
 * Verify a handful of conditions required by the following pnames:
 *   - INTERNALFORMAT_RED_SIZE
 *   - INTERNALFORMAT_GREEN_SIZE
 *   - INTERNALFORMAT_BLUE_SIZE
 *   - INTERNALFORMAT_ALPHA_SIZE
 *   - INTERNALFORMAT_DEPTH_SIZE
 *   - INTERNALFORMAT_STENCIL_SIZE
 *   - INTERNALFORMAT_SHARED_SIZE
 */

#include "common.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;
	config.window_visual = PIGLIT_GL_VISUAL_RGB;
	config.khr_no_error_support = PIGLIT_NO_ERRORS;

PIGLIT_GL_TEST_CONFIG_END

static const GLenum pnames[] = {
        GL_INTERNALFORMAT_RED_SIZE,
        GL_INTERNALFORMAT_GREEN_SIZE,
        GL_INTERNALFORMAT_BLUE_SIZE,
        GL_INTERNALFORMAT_ALPHA_SIZE,
        GL_INTERNALFORMAT_DEPTH_SIZE,
        GL_INTERNALFORMAT_STENCIL_SIZE,
        GL_INTERNALFORMAT_SHARED_SIZE,
};


enum piglit_result
piglit_display(void)
{
	return PIGLIT_FAIL;
}

/*
 * From spec:
 *
 * "- INTERNALFORMAT_<X>_SIZE
 *
 * For uncompressed internal formats, queries of these values return
 * the actual resolutions that would be used for storing image array
 * components for the resource.  For compressed internal formats, the
 * resolutions returned specify the component resolution of an
 * uncompressed internal format that produces an image of roughly the
 * same quality as the compressed algorithm.  For textures this query
 * will return the same information as querying
 * GetTexLevelParameter{if}v for TEXTURE_*_SIZE would return.  If the
 * internal format is unsupported, or if a particular component is not
 * present in the format, 0 is written to <params>."
 *
 * So try_textures_size check if it is 0 when not supported, and that
 * the returned value is the same that the one returned by
 * GetTextLevelParameter when supported.
 */
static bool
try_textures_size(const GLenum *targets, unsigned num_targets,
                  const GLenum *internalformats, unsigned num_internalformats,
                  const GLenum pname,
                  test_data *data)
{
        bool pass = true;
        unsigned i;
        unsigned j;

        for (i = 0; i < num_targets; i++) {
                for (j = 0; j < num_internalformats; j++) {
                        bool error_test;
                        bool value_test;
                        bool supported;

                        supported = check_query2_dependencies(pname, targets[i])
                                && test_data_check_supported(data, targets[i],
                                                             internalformats[j]);

                        test_data_execute(data, targets[i], internalformats[j],
                                          pname);

                        error_test =
                                piglit_check_gl_error(GL_NO_ERROR);

                        value_test = supported ?
                                test_data_check_against_get_tex_level_parameter(data,
                                                                                targets[i],
                                                                                pname,
                                                                                internalformats[j]) :
                                test_data_is_unsupported_response(data, pname);

                        if (error_test && value_test)
                                continue;

                        print_failing_case(targets[i], internalformats[j],
                                           pname, data);

                        pass = false;
                }
        }

	return pass;
}

static bool
check_textures_size(void)
{
        bool check_pass = true;
        test_data *data = test_data_new(0, 1);
        unsigned i;
        int testing64;

        for (i = 0; i < ARRAY_SIZE(pnames); i++) {
                bool pass = true;

                if (!piglit_is_gles()) {
                        if (pnames[i] == GL_INTERNALFORMAT_SHARED_SIZE ||
                            pnames[i] == GL_INTERNALFORMAT_STENCIL_SIZE) {
                                continue;
                        }
                }

                for (testing64 = 0; testing64 <= 1; testing64++) {
                        test_data_set_testing64(data, testing64);

                        pass = try_textures_size(texture_targets, ARRAY_SIZE(texture_targets),
                                                 valid_internalformats, num_valid_internalformats,
                                                 pnames[i],
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

        piglit_require_extension("GL_ARB_internalformat_query2");
        initialize_valid_internalformats();

        pass = check_textures_size()
                && pass;

        piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}
