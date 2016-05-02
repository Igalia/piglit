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
 * \file query2-info.c
 *
 * Not a real test. It allows to print all the values for all the
 * possible pname/target/internalformat combinations.
 *
 */

#include "common.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;
	config.window_visual = PIGLIT_GL_VISUAL_RGB;

PIGLIT_GL_TEST_CONFIG_END

bool filter_supported = false;
int only_64bit_query = 1;
bool just_one_pname = false;
GLenum global_pname = 0;

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

                        /* Some queries will not modify params if
                         * unsupported. Use -1 as the value to point
                         * it, as no query with supported combination
                         * will return it */
                        test_data_set_value_at_index(data, 0, -1);
                        test_data_execute(data, targets[i], internalformats[j], pname);

                        error_test =
                                piglit_check_gl_error(GL_NO_ERROR);

                        if (!error_test)
                                fprintf(stderr, "ERROR:");

                        print_case(targets[i], internalformats[j],
                                   pname, filter_supported, data);

                }
        }

}

static void
print_usage(void)
{
   printf("Usage: query2-info [-a] [-f] [-h] [-pname <pname>]\n");
   printf("\t-pname <pname>: Prints info for only that pname (numeric value).\n");
   printf("\t-b: Prints info using (b)oth 32 and 64 bit queries. "
          "By default it only uses the 64-bit one.\n");
   printf("\t-f: Print info (f)iltering out the unsupported internalformat.\n");
   printf("\t\tNOTE: the filtering is based on internalformat being supported"
          " or not,\n\t\tnot on the combination of pname/target/internalformat being "
          "supported or not.\n");
   printf("\t-h: This information.\n");
}

static bool
_check_pname(const GLenum pname)
{
        int i;
        for (i = 0; i < ARRAY_SIZE(valid_pnames); i++) {
                if (pname == valid_pnames[i])
                        return true;
        }
        return false;
}

static void
parse_args(int argc, char **argv)
{
        int i;

        for (i = 1; i < argc; i++) {
                if (strcmp(argv[i], "-pname") == 0 && i + 1 < argc) {
                        global_pname = atoi(argv[i + 1]);
                        if (!_check_pname(global_pname)) {
                                printf("Value `%i' is not a valid <pname> for "
                                       "GetInternalformati*v.\n", global_pname);
                                print_usage();
                                exit(0);
                        }
                        just_one_pname = true;
                        i++;
                } else if (strcmp(argv[i], "-f") == 0) {
                        filter_supported = true;
                } else if (strcmp(argv[i], "-b") == 0) {
                        only_64bit_query = 0;
                } else if (strcmp(argv[i], "-h") == 0) {
                        print_usage();
                        exit(0);
                } else {
                        printf("Unknown option `%s'\n", argv[i]);
                        print_usage();
                        exit(0);
                }
        }
}

void
piglit_init(int argc, char **argv)
{
        bool pass = true;
        test_data *data = test_data_new(0, 64);
        GLenum pname;
	int testing64;

	piglit_require_extension("GL_ARB_framebuffer_object");
	piglit_require_extension("GL_ARB_internalformat_query2");

	parse_args(argc, argv);

        for (unsigned i = 0; i < ARRAY_SIZE(valid_pnames); i++) {
                pname = valid_pnames[i];
                if (pname == global_pname)
                        printf("Bingo!\n");
                /* Not really the optimal, but do their work */
                if (just_one_pname && global_pname != pname)
                        continue;
                for (testing64 = only_64bit_query; testing64 <= 1; testing64++) {
                        test_data_set_testing64(data, testing64);
                        print_pname_values(valid_targets, ARRAY_SIZE(valid_targets),
                                           valid_internalformats, ARRAY_SIZE(valid_internalformats),
                                           pname, data);
                }
        }

        test_data_clear(&data);

        piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}
