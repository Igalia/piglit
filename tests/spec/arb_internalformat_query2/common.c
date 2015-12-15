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

#include "common.h"
#include <inttypes.h>  /* for PRIu64 macro */

/* Generic callback type, doing a cast of params to void*, to avoid
 * having two paths (32 and 64) for each check */
typedef void (*GetInternalformat)(GLenum target, GLenum internalformat,
                                  GLenum pname, GLsizei bufsize,
                                  void *params);

/* This struct is intended to abstract the fact that there are two
 * really similar methods, and two really similar params (just change
 * the type). All the castings and decision about which method should
 * be used would be done here, just to keep the code of the test
 * cleaner.
 */
struct _test_data {
        /* int instead of a bool to make easier iterate on the
         * possible values. */
        int testing64;
        int params_size;
        void *params;
        GetInternalformat callback;
};

/* Updates the callback and params based on current values of
 * testing64 and params_size */
static void
sync_test_data(test_data *data)
{
        if (data->params != NULL)
                free(data->params);

        if (data->testing64) {
                data->callback = (GetInternalformat) glGetInternalformati64v;
                data->params = malloc(sizeof(GLint64) * data->params_size);
        } else {
                data->callback = (GetInternalformat) glGetInternalformativ;
                data->params = malloc(sizeof(GLint) * data->params_size);
        }
}

test_data*
test_data_new(int testing64,
              int params_size)
{
        test_data *result;

        result = (test_data*) malloc(sizeof(test_data));
        result->testing64 = testing64;
        result->params_size = params_size;
        result->params = NULL;

        sync_test_data(result);

        return result;
}

/*
 * Frees @data, and sets its value to NULL.
 */
void
test_data_clear(test_data **data)
{
        test_data *_data = *data;

        if (_data == NULL)
                return;

        free(_data->params);
        _data->params = NULL;

        *data = NULL;
}

void
test_data_execute(test_data *data,
                  const GLenum target,
                  const GLenum internalformat,
                  const GLenum pname)
{
        data->callback(target, internalformat, pname,
                       data->params_size, data->params);
}

void
test_data_set_testing64(test_data *data,
                        const int testing64)
{
        if (data->testing64 == testing64)
                return;

        data->testing64 = testing64;
        sync_test_data(data);
}

GLint64
test_data_value_at_index(test_data *data,
                         const int index)
{
        if (index > data->params_size || index < 0) {
                fprintf(stderr, "ERROR: invalid index while retrieving"
                        " data from auxiliar test data\n");
                return -1;
        }

        return data->testing64 ?
                ((GLint64*)data->params)[index] :
                ((GLint*)data->params)[index];
}

/*
 * Returns if @target/@internalformat is supported using
 * INTERNALFORMAT_SUPPORTED for @target and @internalformat.
 *
 * @data is only used to known if we are testing the 32-bit or the
 * 64-bit query, so the content of @data will not be modified due this
 * call.
 */
bool
test_data_check_supported(const test_data *data,
                          const GLenum target,
                          const GLenum internalformat)
{
        bool result;
        test_data *local_data = test_data_new(data->testing64, 1);

        test_data_execute(local_data, target, internalformat,
                          GL_INTERNALFORMAT_SUPPORTED);

        if (!piglit_check_gl_error(GL_NO_ERROR))
                result = false;
        else
                result = !test_data_is_zero(local_data);

        test_data_clear(&local_data);

        return result;
}

bool
test_data_is_zero(test_data *data)
{
        return test_data_value_at_index(data, 0) == 0;
}

/* Returns if @value is one of the values among @set */
bool
value_on_set(const GLint *set,
       const unsigned set_size,
       GLint value)
{
        unsigned i;

        for (i = 0; i < set_size; i++) {
                if (set[i] == value)
                        return true;
        }

        return false;
}

bool
test_data_check_possible_values(test_data *data,
                                const GLint *possible_values,
                                const unsigned num_possible_values)
{
        return value_on_set(possible_values, num_possible_values,
                            test_data_value_at_index(data, 0));
}

/*
 * Prints the info of a failing case for a given pname.
 *
 * Note that it tries to get the name of the value at @data as if it
 * were a enum, as it is useful on that case. But there are several
 * pnames that returns a value. A possible improvement would be that
 * for those just printing the value.
 */
void
print_failing_case(const GLenum target, const GLenum internalformat,
                   const GLenum pname, test_data *data)
{
        print_failing_case_full(target, internalformat, pname, -1, data);
}

/*
 * Prints the info of a failing case. If expected_value is smaller
 * that 0, it is not printed.
*/
void print_failing_case_full(const GLenum target, const GLenum internalformat,
                             const GLenum pname, GLint64 expected_value,
                             test_data *data)
{
        /* Knowing if it is supported is interesting in order to know
         * if the test is being too restrictive */
        bool supported = test_data_check_supported(data, target, internalformat);
        GLint64 current_value = test_data_value_at_index(data, 0);

        if (data->testing64) {
                fprintf(stderr,  "    64 bit failing case: ");
        } else {
                fprintf(stderr,  "    32 bit failing case: ");
        }

        fprintf(stderr, "pname = %s, "
                "target = %s, internalformat = %s, ",
                piglit_get_gl_enum_name(pname),
                piglit_get_gl_enum_name(target),
                piglit_get_gl_enum_name(internalformat));

        if (expected_value >= 0)
                fprintf(stderr, "expected value = (%" PRIu64 "), ",
                        expected_value);

        fprintf(stderr, "params[0] = (%" PRIu64 ",%s), "
                "supported=%i\n",
                current_value,
                piglit_get_gl_enum_name(current_value),
                supported);
}

/*
 * The most basic condition. From spec, a lot of pnames has a
 * condition like this:
 *
 * "Possible values returned are <set>. If the resource is not
 *  supported, or if the operation is not supported, NONE is
 *  returned."
 *
 * So this method, calls the callback defined at @data (that should be
 * GetInternalformativ or GetInternalformati64v) using @pname, for all
 * @num_targets at @targets, and all @num_internalformats at
 * @internalformats, and checks the following conditions:
 *
 * * If @pname is not supported (calling INTERNALFORMAT_SUPPORTED),
 *   checks that the value returned is zero
 * * If @pname is supported, checks that the returned value is among
 *   one of the values defined at @possible_values
 *
 * @possible_values,@num_possible_values is allowed to be NULL,0, for
 * the cases where the set of returned values is not specified in
 * detail by the spec (like INTERNALFORMAT_PREFERRED). On that case,
 * it is not tested the returned value, and just tested that if not
 * suppported, the returned value is zero.
 *
 */
bool
try_basic(const GLenum *targets, unsigned num_targets,
          const GLenum *internalformats, unsigned num_internalformats,
          const GLenum pname,
          const GLint *possible_values, unsigned num_possible_values,
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

                        supported = test_data_check_supported(data, targets[i],
                                                              internalformats[j]);

                        if (supported && num_possible_values == 0)
                                continue;

                        test_data_execute(data, targets[i], internalformats[j], pname);

                        error_test =
                                piglit_check_gl_error(GL_NO_ERROR);

                        value_test = supported ?
                                test_data_check_possible_values(data, possible_values,
                                                                num_possible_values) :
                                test_data_is_zero(data);

                        if (error_test && value_test)
                                continue;

                        print_failing_case(targets[i], internalformats[j],
                                           pname, data);

                        pass = false;
                }
        }

	return pass;
}
