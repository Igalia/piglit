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
 * \file api-errors.c
 * Verify a handful of error conditions required by the spec.
 *
 * None of these subtests is large enough to warrant a separate test case.
 *
 * Equivalent to internalformat_query-api-errors, with testing
 * GetInternalformati64v and in addition to test that a INVALID_ENUM
 * is returned with an invalid combination, it also tests that a valid
 * combination doesn't return an INVALID_ENUM.
 *
 * The rationale of this is that is really likely that the implementation
 * of arb_internal_format_query2 would reuse a lot of bits of
 * arb_internal_format_query, so we want to be sure that a combination
 * that was invalid with arb_internal_format_query is not considered
 * invalid by arb_internal_format_query2.
 *
 */

#include "common.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

        config.supports_gl_compat_version = 10;
        config.window_visual = PIGLIT_GL_VISUAL_RGB;
	config.khr_no_error_support = PIGLIT_HAS_ERRORS;

PIGLIT_GL_TEST_CONFIG_END

enum piglit_result
piglit_display(void)
{
	return PIGLIT_FAIL;
}

static bool
try(const GLenum *targets, unsigned num_targets,
    const GLenum *pnames, unsigned num_pnames,
    GLenum expected_error)
{
        GLint params[64];
        GLint64 params64[64];
        bool pass = true;
        unsigned i;
        unsigned j;

	for (i = 0; i < num_targets; i++) {
                for (j = 0; j < num_pnames; j++) {
                        bool this_test;
                        bool this_test64;

                        /* We can use any internalformat, as we are
                         * just checking that the pname/target
                         * combination is or not valid. Knowing if the
                         * internalformat is supported is done on
                         * query2 using INTERNALFORMAT_SUPPORTED
                         * pname*/

                        glGetInternalformativ(targets[i],
                                              GL_RGBA,
                                              pnames[j],
                                              ARRAY_SIZE(params),
                                              params);

                        this_test =
                                piglit_check_gl_error(expected_error);

                        glGetInternalformati64v(targets[i],
                                                GL_RGBA,
                                                pnames[j],
                                                ARRAY_SIZE(params),
                                                params64);

                        this_test64 =
                                piglit_check_gl_error(expected_error);

                        if (this_test && this_test64)
                                continue;

                        fprintf(stderr,
                                "    Failing case was "
                                "target = %s, pname = %s\n",
                                piglit_get_gl_enum_name(targets[i]),
                                piglit_get_gl_enum_name(pnames[j]));

                        if (!this_test)
                                fprintf(stderr,
                                        "    Calling glGetInternalformativ\n");
                        if (!this_test64)
                                fprintf(stderr,
                                        "    Calling glGetInternalformati64v\n");
                        pass = false;
                }
	}

	return pass;
}

void
piglit_init(int argc, char **argv)
{
	bool pass = true;

	piglit_require_extension("GL_ARB_framebuffer_object");
	piglit_require_extension("GL_ARB_internalformat_query2");

	/* The GL_ARB_internalformat_query2 spec says:
         *
         * The INVALID_ENUM error is generated if the <target> parameter to
         * GetInternalformati*v is not one of the targets listed in Table 6.xx.
         *
         * The INVALID_ENUM error is generated if the <pname> parameter is
         * not one of the listed possibilities.
         *
         * So we will test that with all the listed pname/targets, no
         * error is returned, and that without those, INVALID_ENUM is
         * returned.
	 */
        pass = try(valid_targets, ARRAY_SIZE(valid_targets),
                   valid_pnames, ARRAY_SIZE(valid_pnames),
                   GL_NO_ERROR)
                && pass;

        pass = try(invalid_targets, ARRAY_SIZE(invalid_targets),
                   valid_pnames, ARRAY_SIZE(valid_pnames),
                   GL_INVALID_ENUM)
                && pass;

        pass = try(valid_targets, ARRAY_SIZE(valid_targets),
                   invalid_pnames, ARRAY_SIZE(invalid_pnames),
                   GL_INVALID_ENUM)
                && pass;

        pass = try(invalid_targets, ARRAY_SIZE(invalid_targets),
                   invalid_pnames, ARRAY_SIZE(invalid_pnames),
                   GL_INVALID_ENUM)
                && pass;

	/* The GL_ARB_internalformat_query spec says:
	 *
	 *     "If the <bufSize> parameter to GetInternalformativ is negative,
	 *     then an INVALID_VALUE error is generated."
         *
         * Although not specified on query2 spec, we understand that
         * it should be the case, and is a missing on the query2
         * spec. It is properly checked on all the query2
         * implementations we tested so far.
	 */
	glGetInternalformativ(valid_targets[0],
			      GL_RGBA,
			      valid_pnames[0],
			      -1, NULL);
	pass = piglit_check_gl_error(GL_INVALID_VALUE)
		&& pass;

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}
