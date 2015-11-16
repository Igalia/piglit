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
 * \file texture-compressed-block.c
 *
 * Verify conditions defined on the spec for the following pnames:
 *  * TEXTURE_COMPRESSED_BLOCK_WIDTH
 *  * TEXTURE_COMPRESSED_BLOCK_HEIGHT
 *  * TEXTURE_COMPRESSED_BLOCK_SIZE
 *
 * In all those three the spec says :
 *
 * "If the internal format is not compressed, or the resource is not
 *  supported, 0 is returned."
 *
 * One could guess which internalformats are compressed, but
 * TEXTURE_COMPRESSED is already there to know that.
 *
 * So this test just verifies that if TEXTURE_COMPRESSED or
 * INTERNALFORMAT_SUPPORTED are false, all those pnames should return
 * 0.
 *
 * In that sense, this test is generic-pname-checks on those pnames,
 * plus a TEXTURE_COMPRESSED check.
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

static const GLenum pnames[] = {
        GL_TEXTURE_COMPRESSED_BLOCK_WIDTH,
        GL_TEXTURE_COMPRESSED_BLOCK_HEIGHT,
        GL_TEXTURE_COMPRESSED_BLOCK_SIZE,
};

static bool
check_compressed(const GLenum target, const GLenum internalformat,
                struct test_data data)
{
        data.callback(target, internalformat,
                      GL_TEXTURE_COMPRESSED,
                      data.params_size, data.params);

        if (!piglit_check_gl_error(GL_NO_ERROR))
                return false;

        return !check_params_zero(data);
}

/* try_local could be implemented as try_basic (at common.c) with
 * @possible_values==NULL, and testing that if TEXTURE_COMPRESSED is
 * false, it should returns zero. Candidate to be refactored */
bool
try_local(const GLenum *targets, unsigned num_targets,
          const GLenum *internalformats, unsigned num_internalformats,
          const GLenum pname,
          struct test_data data)
{
        bool pass = true;
        unsigned i;
        unsigned j;

	for (i = 0; i < num_targets; i++) {
                for (j = 0; j < num_internalformats; j++) {
                        bool error_test;
                        bool value_test = true;
                        bool supported;
                        bool compressed;

                        supported = check_supported(targets[i],
                                                    internalformats[j], data);
                        compressed = check_compressed(targets[i],
                                                      internalformats[j], data);

                        /* If it is supported and compressed, we don't
                         * have a way to verify at this point that the
                         * returned value is correct */
                        if (supported && compressed)
                                continue;

                        data.callback(targets[i], internalformats[j],
                                      pname, data.params_size,
                                      data.params);

                        error_test =
                                piglit_check_gl_error(GL_NO_ERROR);
                        /*
                         * From spec:
                         * "If the internal format is not compressed,
                         *  or the resource is not supported, 0 is
                         *  returned."
                         */
                        if (!supported || !compressed)
                                value_test = check_params_zero(data);

                        if (error_test && value_test)
                                continue;

                        /* If we are here, the test is failing */
                        print_failing_case(targets[i], internalformats[j],
                                           pname, data);
                        if (!supported && !value_test)
                                fprintf(stderr,"\tInternalformat is not supported, but returned value is not zero\n");

                        if (!compressed && !value_test)
                                fprintf(stderr,"\tInternalformat is not compressed, but returned value is not zero\n");

                        pass = false;
                }
        }

	return pass;
}

/*
 * Totally equal to check_basic (at generic-pname-checks) with
 * possible_values NULL and calling try_local instead of
 * try_basic. Candidate to be refactored.
 */
static bool
check_texture_compressed_block(const GLenum *pnames, unsigned num_pnames)
{
        bool check_pass = true;
        struct test_data data;
        unsigned i;

        data.params = NULL;
        data.params_size = 1;

        for (i = 0; i < num_pnames; i++) {
                bool pass = true;

                for (data.testing64 = 0; data.testing64 <= 1; data.testing64++) {
                        sync_test_data(&data);
                        pass = try_local(valid_targets, ARRAY_SIZE(valid_targets),
                                         valid_internalformats, ARRAY_SIZE(valid_internalformats),
                                         pnames[i], data)
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

        pass = check_texture_compressed_block(pnames, ARRAY_SIZE(pnames))
                && pass;

        piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}
