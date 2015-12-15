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
 * \file max dimensions.c
 *
 * Handle checks for several pnames involving max dimensions:
 *
 * MAX_WIDTH: From spec: If the resource is unsupported, zero is
 * returned."
 *
 * MAX_HEIGHT: From spec "If the resource does not have at least two
 *   dimensions, or if the resource is unsupported, zero is returned."
 *
 * MAX_DEPTH: From spec "If the resource does not have at least three
 *   dimensions, or if the resource is unsupported, zero is returned."
 *
 * MAX_LAYERS: From spec "For 1D array targets, the value returned is
 *   the same as the MAX_HEIGHT. For 2D and cube array targets, the
 *   value returned is the same as the MAX_DEPTH."
 *
 */

#include "common.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;
	config.window_visual = PIGLIT_GL_VISUAL_RGB;

PIGLIT_GL_TEST_CONFIG_END

enum piglit_result
piglit_display(void)
{
	return PIGLIT_FAIL;
}

/*
 * Returns the number of dimensions of @target
 */
static int
num_dimensions(const GLenum target)
{
        switch(target) {
        case GL_TEXTURE_1D:
        case GL_TEXTURE_BUFFER:
                return 1;

        /* Although technically 1D_ARRAY holds a 1D dimension texture,
         * it uses the height as the number of layers, and it is
         * created using TexImage2D, additionally from MAX_LAYERS
         * description at spec:
         *
         * "For 1D array targets, the value returned is the same as the
         * MAX_HEIGHT."
         *
         * So here are considered as having 3 dimensions
         */
        case GL_TEXTURE_1D_ARRAY:
        case GL_TEXTURE_2D:
        case GL_TEXTURE_CUBE_MAP:
        case GL_TEXTURE_RECTANGLE:
        case GL_TEXTURE_2D_MULTISAMPLE:
        case GL_RENDERBUFFER:
                return 2;

        /* Although technically, CUBE_MAP_ARRAY and 2D_ARRAY holds 2D
         * dimension textures, they use the depth as the number of
         * layers, and it is created using TexImage3D, additionally
         * from MAX_LAYERS description at spec:
         *
         * "For 2D and cube array targets, the value returned is the
         * same as the MAX_DEPTH."
         *
         * So here are considered as having 3 dimensions
         */
        case GL_TEXTURE_CUBE_MAP_ARRAY:
        case GL_TEXTURE_2D_ARRAY:
        case GL_TEXTURE_2D_MULTISAMPLE_ARRAY:
        case GL_TEXTURE_3D:
                return 3;
        default:
                return 0;
        }
}

static bool
try(const GLenum *targets, unsigned num_targets,
    const GLenum *internalformats, unsigned num_internalformats,
    const GLenum pname,
    const unsigned min_dimensions,
    test_data *data)
{
        bool pass = true;
        unsigned i;
        unsigned j;

	for (i = 0; i < num_targets; i++) {
		for (j = 0; j < num_internalformats; j++) {
                        bool error_test;
                        bool value_test = true;
                        bool supported;

                        supported = test_data_check_supported(data, targets[i],
                                                              internalformats[j]);

                        test_data_execute(data, targets[i], internalformats[j],
                                          pname);

                        error_test =
                                piglit_check_gl_error(GL_NO_ERROR);

                        /* From the spec:
                         *
                         * MAX_WIDTH
                         * "If the resource is unsupported, zero is
                         *  returned."
                         *
                         * MAX_HEIGHT
                         * "If the resource does not have at least two
                         *  dimensions, or if the resource is
                         *  unsupported, zero is returned."
                         *
                         * MAX_DEPTH:
                         * "If the resource does not have at least
                         * three dimensions, or if the resource is
                         * unsupported, zero is returned."
                         *
                         * We can only check if on those cases the value is zero.
                         */
                        if (!supported || num_dimensions(targets[i]) < min_dimensions) {
                                value_test = test_data_is_zero(data);
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
check_max_dimension(const GLenum pname,
                    const unsigned min_dimensions)
{
        bool pass = true;
        test_data *data = test_data_new(0, 1);
        int testing64;

        for (testing64 = 0; testing64 <= 1; testing64++) {
                test_data_set_testing64(data, testing64);

                pass = try(valid_targets, ARRAY_SIZE(valid_targets),
                           valid_internalformats, ARRAY_SIZE(valid_internalformats),
                           pname, min_dimensions, data)
                        && pass;
        }

        piglit_report_subtest_result(pass ? PIGLIT_PASS : PIGLIT_FAIL,
                                     "%s", piglit_get_gl_enum_name(pname));
        return pass;
}

static bool
is_array(const GLenum target)
{
        switch(target) {
        case GL_TEXTURE_1D_ARRAY:
        case GL_TEXTURE_CUBE_MAP_ARRAY:
        case GL_TEXTURE_2D_ARRAY:
        case GL_TEXTURE_2D_MULTISAMPLE_ARRAY:
                return true;
        default:
                return false;
        }
}

static bool
is_1d_array(const GLenum target)
{
        switch(target) {
        case GL_TEXTURE_1D_ARRAY:
                return true;
        default:
                return false;
        }
}

/*
 * From the spec:
 *
 * "MAX_LAYERS: The maximum supported number of layers for the
 *  resource is returned in <params>. For 1D array targets, the value
 *  returned is the same as the MAX_HEIGHT. For 2D and cube array
 *  targets, the value returned is the same as the MAX_DEPTH. If the
 *  resource does not support layers, or if the resource is
 *  unsupported, zero is returned."
 *
 * This function is a check to ensure that the value is the same that
 * the other pnames. So this function calls GetInternalformat with the
 * pname MAX_HEIGHT or MAX_DEPTH (depends on @target), and compare it
 * against the value stored at @data, that is a test_data that have
 * just called GetInternalformat with MAX_LAYERS.
 */
static bool
check_params_against_dimension(test_data *data,
                               const GLenum target,
                               const GLenum internalformat)
{
        test_data *local_data = test_data_clone(data);
        GLenum dimension_pname;
        bool result = true;

        if (!is_array(target))
                return true;

        if (is_1d_array(target)) {
                dimension_pname = GL_MAX_HEIGHT;
        } else {
                dimension_pname = GL_MAX_DEPTH;
        }

        test_data_execute(local_data, target, internalformat,
                          dimension_pname);

        if (!piglit_check_gl_error(GL_NO_ERROR)) {
                return false;
        }

        result = test_data_equal_at_index(data, local_data, 0);

        test_data_clear(&local_data);

        return result;
}
/*
 * From the spec:
 *
 * "MAX_LAYERS: The maximum supported number of layers for the
 *  resource is returned in <params>. For 1D array targets, the value
 *  returned is the same as the MAX_HEIGHT. For 2D and cube array
 *  targets, the value returned is the same as the MAX_DEPTH. If the
 *  resource does not support layers, or if the resource is
 *  unsupported, zero is returned."
 */
static bool
try_max_layers(const GLenum *targets, unsigned num_targets,
               const GLenum *internalformats, unsigned num_internalformats,
               test_data *data)
{
        bool pass = true;
        unsigned i;
        unsigned j;

	for (i = 0; i < num_targets; i++) {
		for (j = 0; j < num_internalformats; j++) {
                        bool error_test;
                        bool value_test = true;
                        bool supported;

                        supported = test_data_check_supported(data, targets[i],
                                                              internalformats[j]);

                        test_data_execute(data, targets[i], internalformats[j],
                                          GL_MAX_LAYERS);

                        error_test =
                                piglit_check_gl_error(GL_NO_ERROR);

                        value_test = supported ?
                                check_params_against_dimension(data,
                                                               targets[i],
                                                               internalformats[i]) :
                                test_data_is_zero(data);

                        if (error_test && value_test)
                                continue;

                        print_failing_case(targets[i], internalformats[j],
                                           GL_MAX_LAYERS, data);

                        pass = false;
                }
        }

	return pass;
}

static bool
check_max_layers()
{
        bool pass = true;
        test_data *data = test_data_new(0, 1);
        int testing64;

        for (testing64 = 0; testing64 <= 1; testing64++) {
                test_data_set_testing64(data, testing64);

                pass = try_max_layers(valid_targets, ARRAY_SIZE(valid_targets),
                                      valid_internalformats, ARRAY_SIZE(valid_internalformats),
                                      data)
                        && pass;
        }

        piglit_report_subtest_result(pass ? PIGLIT_PASS : PIGLIT_FAIL,
                                     "%s", piglit_get_gl_enum_name(GL_MAX_LAYERS));
        return pass;
}

void
piglit_init(int argc, char **argv)
{
        bool pass = true;

        piglit_require_extension("GL_ARB_framebuffer_object");
        piglit_require_extension("GL_ARB_internalformat_query2");

        pass = check_max_dimension(GL_MAX_WIDTH, 1) && pass;
        pass = check_max_dimension(GL_MAX_HEIGHT, 2) && pass;
        pass = check_max_dimension(GL_MAX_DEPTH, 3) && pass;
        pass = check_max_layers() && pass;

        piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}
