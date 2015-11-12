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
 * \file internalformat-type-checks.c
 * Verify a handful of conditions required by the following pnames:
 *   - INTERNALFORMAT_RED_TYPE
 *   - INTERNALFORMAT_GREEN_TYPE
 *   - INTERNALFORMAT_BLUE_TYPE
 *   - INTERNALFORMAT_ALPHA_TYPE
 *   - INTERNALFORMAT_DEPTH_TYPE
 *   - INTERNALFORMAT_STENCIL_TYPE
 */

#include "common.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;
	config.window_visual = PIGLIT_GL_VISUAL_RGB;

PIGLIT_GL_TEST_CONFIG_END

static const GLenum pnames[] = {
        GL_INTERNALFORMAT_RED_TYPE,
        GL_INTERNALFORMAT_GREEN_TYPE,
        GL_INTERNALFORMAT_BLUE_TYPE,
        GL_INTERNALFORMAT_ALPHA_TYPE,
        GL_INTERNALFORMAT_DEPTH_TYPE,
        /* There is no equivalent GL_TEXTURE_STENCIL_TYPE, so we can't
         * test against GetTexLevelParameter here */
        /* GL_INTERNALFORMAT_STENCIL_TYPE, */
};

/* From spec:
 *
 * "For textures this query will return the same information
 *  as querying GetTexLevelParameter{if}v for TEXTURE_*_TYPE
 *  would return."
 *
 * The following are the pnames we would need to use when
 * calling GetTexLevelParameter (so equivalent to pnames)
 */
static const GLenum equivalent_pnames[] = {
        GL_TEXTURE_RED_TYPE,
        GL_TEXTURE_GREEN_TYPE,
        GL_TEXTURE_BLUE_TYPE,
        GL_TEXTURE_ALPHA_TYPE,
        GL_TEXTURE_DEPTH_TYPE,
};

/* From spec:
 *
 * Possible values return include, NONE, SIGNED_NORMALIZED,
 * UNSIGNED_NORMALIZED, FLOAT, INT, UNSIGNED_INT, representing
 * missing, signed normalized fixed point, unsigned normalized fixed
 * point, floating-point, signed unnormalized integer and unsigned
 * unnormalized integer components.
 */

static const GLint possible_values[] = {
        GL_NONE,
        GL_SIGNED_NORMALIZED,
        GL_UNSIGNED_NORMALIZED,
        GL_FLOAT,
        GL_INT,
        GL_UNSIGNED_INT
};

enum piglit_result
piglit_display(void)
{
	return PIGLIT_FAIL;
}

/*
 * From spec:
 *
 * - INTERNALFORMAT_{X}_TYPE
 *
 * For uncompressed internal formats, queries for these values return
 * the data type used to store the component.
 * For compressed internal formats the types returned specify how
 * components are interpreted after decompression.
 * For textures this query returns the same information as querying
 * GetTexLevelParameter{if}v for TEXTURE_*TYPE would return.  Possible
 * values return include, NONE, SIGNED_NORMALIZED,
 * UNSIGNED_NORMALIZED, FLOAT, INT, UNSIGNED_INT, representing
 * missing, signed normalized fixed point, unsigned normalized fixed
 * point, floating-point, signed unnormalized integer and unsigned
 * unnormalized integer components. NONE is returned for all component
 * types if the format is unsupported.
 *
 * So try_textures_size check if it is 0 when not supported, and that
 * the returned value is on that list of possible values and the same
 * that the one returned by GetTextLevelParameter when supported.
 */
static bool
try_textures_type(const GLenum *targets, unsigned num_targets,
                  const GLenum *internalformats, unsigned num_internalformats,
                  const GLenum pname,
                  const GLenum equivalent_pname,
                  struct test_data data)
{
        bool pass = true;
        unsigned i;
        unsigned j;

        for (i = 0; i < num_targets; i++) {
                for (j = 0; j < num_internalformats; j++) {
                        bool error_test;
                        bool value_test;
                        bool supported;

                        supported = check_supported(targets[i], internalformats[j], data);

                        data.callback(targets[i], internalformats[j],
                                      pname, data.params_size,
                                      data.params);

                        error_test =
                                piglit_check_gl_error(GL_NO_ERROR);

                        if (!supported) {
                                value_test = check_params_zero(data);
                        } else {

                                value_test = check_possible_values(data, possible_values,
                                                                   ARRAY_SIZE(possible_values));
                                value_test = value_test &&
                                        check_params_against_get_tex_level_parameter(data,
                                                                                     targets[i],
                                                                                     equivalent_pname,
                                                                                     internalformats[j]);
                        }

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
check_textures_type(void)
{
        bool check_pass = true;
        struct test_data data;
        unsigned i;

        data.params = NULL;
        data.params_size = 1;

        for (i = 0; i < ARRAY_SIZE(pnames); i++) {
                bool pass = true;

                for (data.testing64 = 0; data.testing64 <= 1; data.testing64++) {
                        sync_test_data(&data);

                        pass = try_textures_type(texture_targets, ARRAY_SIZE(texture_targets),
                                                 valid_internalformats, ARRAY_SIZE(valid_internalformats),
                                                 pnames[i], equivalent_pnames[i],
                                                 data)
                                && pass;
                }
                piglit_report_subtest_result(pass ? PIGLIT_PASS : PIGLIT_FAIL,
                                             "%s", piglit_get_gl_enum_name(pnames[i]));

                check_pass = check_pass && pass;
        }

        clean_test_data(&data);

        return check_pass;
}

void
piglit_init(int argc, char **argv)
{
        bool pass = true;

        piglit_require_extension("GL_ARB_framebuffer_object");
        piglit_require_extension("GL_ARB_internalformat_query2");

        pass = check_textures_type()
                && pass;

        piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}
