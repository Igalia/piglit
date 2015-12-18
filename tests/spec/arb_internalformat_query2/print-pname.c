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
 *  a) That if not supported, the returned value is zero
 *  b) If supported, the returned value is among a fixed set of
 *     possible values.
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
 * Print all the values for a given pname, for the cases where it is
 * supported.
 *
 * This is an utility in order to print what other driver returns, in
 * order to have a reference.
 */

static void
print_pname_values(const GLenum *targets, unsigned num_targets,
                   const GLenum *internalformats, unsigned num_internalformats,
                   const GLenum pname, test_data *data)
{
        unsigned i;
        unsigned j;

	for (i = 0; i < num_targets; i++) {
                for (j = 0; j < num_internalformats; j++) {
                        bool error_test;
                        bool supported;

                        supported = test_data_check_supported(data, targets[i],
                                                              internalformats[j]);

                        test_data_execute(data, targets[i], internalformats[j], pname);

                        error_test =
                                piglit_check_gl_error(GL_NO_ERROR);

                        if (!error_test)
                                fprintf(stderr, "ERROR\n");

                        if (supported || !error_test)
                                print_case(targets[i], internalformats[j],
                                           pname, data);

                }
        }

}

void
piglit_init(int argc, char **argv)
{
        bool pass = true;
        test_data *data = test_data_new(0, 1);
        GLenum pname;

	piglit_require_extension("GL_ARB_framebuffer_object");
	piglit_require_extension("GL_ARB_internalformat_query2");

        /* FIXME: as an utility executable, how about getting it from argc/argv? */
        pname = GL_FILTER;
        print_pname_values(valid_targets, ARRAY_SIZE(valid_targets),
                           valid_internalformats, ARRAY_SIZE(valid_internalformats),
                           pname, data);

        test_data_clear(&data);

        piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}
