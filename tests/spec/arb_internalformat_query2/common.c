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

bool
check_supported(const GLenum target, const GLenum internalformat,
                struct test_data data)
{
        bool result = false;
        void *supported;
        GLint supported32;
        GLint64 supported64;

        if (data.testing64)
                supported = &supported64;
        else
                supported = &supported32;

        data.callback(target, internalformat,
                      GL_INTERNALFORMAT_SUPPORTED,
                      1, supported);

        if (!piglit_check_gl_error(GL_NO_ERROR))
                return false;

        result = data.testing64 ? (*(GLint64*)supported != 0) :
                (*(GLint*)supported != 0);

        return result;
}

bool
check_params_zero(struct test_data data)
{
        return data.testing64 ?
                ((GLint64*)data.params)[0] == 0 :
                ((GLint*)data.params)[0] == 0;
}

bool
check_possible_values(struct test_data data,
                      const GLint *possible_values,
                      unsigned num_possible_values)
{
        unsigned i;

        for (i = 0; i < num_possible_values; i++) {
                if (data.testing64) {
                        if (possible_values[i] == ((GLint64*)data.params)[0])
                                return true;
                } else {
                        if (possible_values[i] == ((GLint*)data.params)[0])
                                return true;
                }
        }

        return false;
}

/* Updates the callback and params based on current values of
 * testing64 and params_size */
void
sync_test_data(struct test_data *data)
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

void
clean_test_data(struct test_data *data)
{
        if (data->params != NULL)
                free(data->params);

        data->params = NULL;
}

void
print_failing_case(const GLenum target, const GLenum internalformat,
                   const GLenum pname, struct test_data data)
{
        /* Knowing if it is supported is interesting in order to know
         * if the test is being too restrictive */
        bool supported = check_supported(target, internalformat, data);

        /* Note that it also tries to get the enum name from the
         * returned value: it is true that the returned value is not
         * always a enum, but having it printed is useful on the cases
         * it is */
        /* @FIXME: We can improve the readability of the failing cases,
         * printing enums only  if the pname is supposed to return enums,
         * integers otherwise.
         * Some pnames return booleans, for those we can print
         * (GL_TRUE/GL_FALSE) or just threat them like the pnames that
         * return integer.
         */
        if (data.testing64) {
                fprintf(stderr,
                        "    64 bit failing case: "
                        "pname = %s, "
                        "target = %s, internalformat = %s, "
                        "params[0] = (%" PRIu64 ",%s), "
                        "supported=%i\n",
                        piglit_get_gl_enum_name(pname),
                        piglit_get_gl_enum_name(target),
                        piglit_get_gl_enum_name(internalformat),
                        ((GLint64*)data.params)[0],
                        piglit_get_gl_enum_name(((GLint64*)data.params)[0]),
                        supported);
        } else {
                fprintf(stderr,
                        "    32 bit failing case: "
                        "pname = %s, "
                        "target = %s, internalformat = %s, "
                        "params[0] = (%i,%s) supported=%i\n",
                        piglit_get_gl_enum_name(pname),
                        piglit_get_gl_enum_name(target),
                        piglit_get_gl_enum_name(internalformat),
                        ((GLint*)data.params)[0],
                        piglit_get_gl_enum_name(((GLint*)data.params)[0]),
                        supported);
        }
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

                        if (supported && num_possible_values == 0)
                                continue;

                        data.callback(targets[i], internalformats[j],
                                      pname, data.params_size,
                                      data.params);

                        error_test =
                                piglit_check_gl_error(GL_NO_ERROR);

                        value_test = supported ?
                                check_possible_values(data, possible_values,
                                                      num_possible_values) :
                                check_params_zero(data);

                        if (error_test && value_test)
                                continue;

                        print_failing_case(targets[i], internalformats[j],
                                           pname, data);

                        pass = false;
                }
        }

	return pass;
}

/*
 * Sets the value of @data params at @index to @value.
*/
void
set_params_at_index(struct test_data *data,
                    unsigned index,
                    GLint64 value)
{
        if (index > data->params_size || index < 0) {
                fprintf(stderr, "ERROR: invalid index while setting"
                        " auxiliar test data\n");
                return;
        }

        if (data->testing64) {
                ((GLint64*)data->params)[index] = value;
        } else {
                ((GLint*)data->params)[index] = value;
        }
}

bool equal_at_index(struct test_data data,
                    struct test_data data_copy,
                    unsigned index)
{
        if (data.testing64 != data_copy.testing64) {
                fprintf(stderr, "ERROR: trying to compare imcompatible"
                        " auxiliar test data structures\n");
                return false;
        }
        if (data.params_size != data_copy.params_size) {
                fprintf(stderr, "ERROR: trying to compare imcompatible"
                        " auxiliar test data structures\n");
                return false;
        }
        if (index > data.params_size || index < 0) {
                fprintf(stderr, "ERROR: invalid index while setting"
                        " auxiliar test data\n");
                return false;
        }

        return (data.testing64 ?
                ((GLint64*)data.params)[index] == ((GLint64*)data_copy.params)[index] :
                ((GLint*)data.params)[index] == ((GLint*)data_copy.params)[index]);
}
