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
 * \file image-format-compatibility-type.c
 *
 * Verify conditions defined for IMAGE_FORMAT_COMPATIBILITY_TYPE pname.
 *
 */

#include "common.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;
	config.window_visual = PIGLIT_GL_VISUAL_RGB;
	config.khr_no_error_support = PIGLIT_NO_ERRORS;

PIGLIT_GL_TEST_CONFIG_END

/*
 * On this test we use GetTexParameteriv to check the correct
 * value. We can't use texture_targets because TEXTURE_BUFFER is not a
 * valid enum for GetTexParameteriv
 */
static const GLenum get_tex_parameter_targets[] = {
        GL_TEXTURE_1D,
        GL_TEXTURE_1D_ARRAY,
        GL_TEXTURE_2D,
        GL_TEXTURE_2D_ARRAY,
        GL_TEXTURE_3D,
        GL_TEXTURE_CUBE_MAP,
        GL_TEXTURE_CUBE_MAP_ARRAY,
        GL_TEXTURE_RECTANGLE,
        GL_TEXTURE_2D_MULTISAMPLE,
        GL_TEXTURE_2D_MULTISAMPLE_ARRAY,
};

enum piglit_result
piglit_display(void)
{
	return PIGLIT_FAIL;
}

static GLint
get_tex_parameter_value(const GLenum target,
                        const GLenum internalformat)
{
        bool has_texture;
        GLuint tex;
        GLuint buffer;
        GLint param;

        has_texture = create_texture(target, internalformat, &tex, &buffer);
        if (!has_texture) {
                return GL_NONE;
        }

        glGetTexParameteriv(target, GL_IMAGE_FORMAT_COMPATIBILITY_TYPE, &param);

        glDeleteTextures(1, &tex);
        glDeleteBuffers(1, &buffer);

        return param;
}

/* From the spec:
 *
 * "- IMAGE_FORMAT_COMPATIBILITY_TYPE: The matching criteria use for
 *   the resource when used as an image textures is returned in
 *   <params>. This is equivalent to calling GetTexParameter with
 *   <value> set to IMAGE_FORMAT_COMPATIBILITY_TYPE. Possible values
 *   are IMAGE_FORMAT_COMPATIBILITY_BY_SIZE or
 *   IMAGE_FORMAT_COMPATIBILITY_BY_CLASS.  If the resource is not
 *   supported for image textures, or if image textures are not
 *   supported, NONE is returned."
 *
 * So try_local is equivalent to try_basic, except that instead of
 * checking against a list of possible value, we test against the
 * value returned by GetTexParameter, or against GL_NONE if not
 * supported of if it is not a texture.
 */
bool
try_local(const GLenum *targets, unsigned num_targets,
          const GLenum *internalformats, unsigned num_internalformats,
          const GLenum pname,
          test_data *data)
{
        bool pass = true;
        unsigned i;
        unsigned j;
        GLint param;

	for (i = 0; i < num_targets; i++) {
                for (j = 0; j < num_internalformats; j++) {
                        bool error_test;
                        bool value_test;
                        bool supported;
                        bool is_valid_target;

                        supported = check_query2_dependencies(pname, targets[i])
                                && test_data_check_supported(data, targets[i],
                                                             internalformats[j]);

                        test_data_execute(data, targets[i], internalformats[j], pname);

                        error_test =
                                piglit_check_gl_error(GL_NO_ERROR);

                        is_valid_target = value_on_set((const GLint*)get_tex_parameter_targets,
                                                       ARRAY_SIZE(get_tex_parameter_targets),
                                                       (GLint) targets[i]);

                        if (is_valid_target && supported) {
                                param = get_tex_parameter_value(targets[i], internalformats[j]);
                                error_test = error_test &&
                                        piglit_check_gl_error(GL_NO_ERROR);

                                value_test = test_data_value_at_index(data, 0) == param;
                        } else {
                                value_test = test_data_is_unsupported_response(data, pname);
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
check_format_compatibility_type(void)
{
        bool pass = true;
        test_data *data = test_data_new(0, 1);
        int testing64;

        for (testing64 = 0; testing64 <= 1; testing64++) {
                test_data_set_testing64(data, testing64);

                pass = try_local(valid_targets, ARRAY_SIZE(valid_targets),
                                 valid_internalformats, num_valid_internalformats,
                                 GL_IMAGE_FORMAT_COMPATIBILITY_TYPE,
                                 data)
                        && pass;
        }

        piglit_report_subtest_result(pass ? PIGLIT_PASS : PIGLIT_FAIL,
                                     "%s", piglit_get_gl_enum_name(GL_IMAGE_FORMAT_COMPATIBILITY_TYPE));

        test_data_clear(&data);

        return pass;
}

void
piglit_init(int argc, char **argv)
{
        bool pass = true;

        piglit_require_extension("GL_ARB_internalformat_query2");
        initialize_valid_internalformats();

        pass = check_format_compatibility_type() && pass;

        piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}
