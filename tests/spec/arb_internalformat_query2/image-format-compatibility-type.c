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

PIGLIT_GL_TEST_CONFIG_END

enum piglit_result
piglit_display(void)
{
	return PIGLIT_FAIL;
}

static bool
check_format_compatibility_type(void)
{
        bool pass = true;
        struct test_data data;
        static const GLint possible_values[] = {
                GL_NONE,
                GL_IMAGE_FORMAT_COMPATIBILITY_BY_SIZE,
                GL_IMAGE_FORMAT_COMPATIBILITY_BY_CLASS
        };

        data.params = NULL;
        data.params_size = 1;
	/* From the spec:
         *
         * - IMAGE_FORMAT_COMPATIBILITY_TYPE: The matching criteria
         *   use for the resource when used as an image textures is
         *   returned in <params>. This is equivalent to calling
         *   GetTexParameter with <value> set to
         *   IMAGE_FORMAT_COMPATIBILITY_TYPE. Possible values are
         *   IMAGE_FORMAT_COMPATIBILITY_BY_SIZE or
         *   IMAGE_FORMAT_COMPATIBILITY_BY_CLASS.  If the resource is
         *   not supported for image textures, or if image textures
         *   are not supported, NONE is returned.
         */
        for (data.testing64 = 0; data.testing64 <= 1; data.testing64++) {
                sync_test_data(&data);

                pass = try_basic(valid_targets, ARRAY_SIZE(valid_targets),
                                 valid_internalformats, ARRAY_SIZE(valid_internalformats),
                                 GL_IMAGE_FORMAT_COMPATIBILITY_TYPE,
                                 possible_values, ARRAY_SIZE(possible_values),
                                 data)
                        && pass;
        }

        /**
         * FIXME: needs to test this part of the spec: "This is
         * equivalent to calling GetTexParameter with <value> set to
         * IMAGE_FORMAT_COMPATIBILITY_TYPE.
         */

        piglit_report_subtest_result(pass ? PIGLIT_PASS : PIGLIT_FAIL,
                                     "%s", piglit_get_gl_enum_name(GL_IMAGE_FORMAT_COMPATIBILITY_TYPE));

        clean_test_data(&data);

        return pass;
}

void
piglit_init(int argc, char **argv)
{
        bool pass = true;

        piglit_require_extension("GL_ARB_framebuffer_object");
        piglit_require_extension("GL_ARB_internalformat_query2");

        pass = check_format_compatibility_type() && pass;

        piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}
