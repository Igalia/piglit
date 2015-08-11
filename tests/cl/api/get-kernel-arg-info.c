/*
 * Copyright © 2014 Serge Martin (EdB) <edb+piglit@sigluy.net>
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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * copied from get-kernel-info.c
 * Copyright © 2012 Blaž Tomažič <blaz.tomazic@gmail.com>
 */

/**
 * @file get-kernel-arg-info.c
 *
 * Test API function:
 *
 *   cl_int clGetKernelArgInfo (cl_kernel kernel,
 *                              cl_uint arg_indx,
 *                              cl_kernel_arg_info param_name,
 *                              size_t param_value_size,
 *                              void *param_value,
 *                              size_t *param_value_size_ret)
 */

#include "piglit-framework-cl-api.h"


PIGLIT_CL_API_TEST_CONFIG_BEGIN

       config.name = "clGetKernelArgInfo";
       config.version_min = 12;

       config.run_per_platform = true;
       config.create_context = true;

       config.program_source = "kernel void dummy_kernel(int param_1) {}";
       config.build_options = "-cl-kernel-arg-info";

PIGLIT_CL_API_TEST_CONFIG_END


enum piglit_result
piglit_cl_test(const int argc,
               const char** argv,
               const struct piglit_cl_api_test_config* config,
               const struct piglit_cl_api_test_env* env)
{
#if defined(CL_VERSION_1_2)
       enum piglit_result result = PIGLIT_PASS;

       int i;
       cl_int errNo;
       cl_kernel kernel;

       size_t param_value_size;
       size_t ret_value_size;
       size_t expected_size;
#define BUFFER_SIZE 8
       char param_value[BUFFER_SIZE];

       int num_kernel_arg_infos = PIGLIT_CL_ENUM_NUM(cl_kernel_arg_info, env->version);
       const cl_kernel_arg_info* kernel_arg_infos = PIGLIT_CL_ENUM_ARRAY(cl_kernel_arg_info);

       kernel = clCreateKernel(env->program,
                               "dummy_kernel",
                               &errNo);
       if(!piglit_cl_check_error(errNo, CL_SUCCESS)) {
               fprintf(stderr,
                       "Failed (error code: %s): Create kernel.\n",
                       piglit_cl_get_error_name(errNo));
               return PIGLIT_FAIL;
       }

       /*** Normal usage ***/
       for(i = 0; i < num_kernel_arg_infos; ++i) {
               printf("%s\n", piglit_cl_get_enum_name(kernel_arg_infos[i]));

               param_value_size = 0;
               ret_value_size = 0;

               errNo = clGetKernelArgInfo(kernel,
                                          0,
                                          kernel_arg_infos[i],
                                          0,
                                          NULL,
                                          &param_value_size);
               if(!piglit_cl_check_error(errNo, CL_SUCCESS)) {
                       fprintf(stderr,
                               "Failed (error code: %s): Get size of %s.\n",
                               piglit_cl_get_error_name(errNo),
                               piglit_cl_get_enum_name(kernel_arg_infos[i]));
                       piglit_merge_result(&result, PIGLIT_FAIL);
                       continue;
               }

               if (param_value_size > BUFFER_SIZE) {
                       fprintf(stderr,
                               "Failed: BUFFER_SIZE is too low\n");
                       piglit_merge_result(&result, PIGLIT_FAIL);
                       continue;
               }

               errNo = clGetKernelArgInfo(kernel,
                                          0,
                                          kernel_arg_infos[i],
                                          BUFFER_SIZE,
                                          &param_value,
                                          &ret_value_size);
               if(!piglit_cl_check_error(errNo, CL_SUCCESS)) {
                       fprintf(stderr,
                               "Failed (error code: %s): Get value of %s.\n",
                               piglit_cl_get_error_name(errNo),
                               piglit_cl_get_enum_name(kernel_arg_infos[i]));
                       piglit_merge_result(&result, PIGLIT_FAIL);
                       continue;
               }

               if (param_value_size != ret_value_size) {
                       fprintf(stderr,
                               "Failed: the returned size doesn't matches the queried one\n");
                       piglit_merge_result(&result, PIGLIT_FAIL);
                       continue;
               }

#define CASE(_enum_, _type_, _n_)             \
       case _enum_:                                \
               expected_size = sizeof(_type_) * ( _n_ ); \
               break;

               expected_size = 0;
               switch (kernel_arg_infos[i]) {
                       CASE(CL_KERNEL_ARG_ADDRESS_QUALIFIER,
                            cl_kernel_arg_address_qualifier, 1)
                       CASE(CL_KERNEL_ARG_ACCESS_QUALIFIER,
                            cl_kernel_arg_access_qualifier, 1)
                       CASE(CL_KERNEL_ARG_TYPE_NAME, char, 3 + 1)
                       CASE(CL_KERNEL_ARG_TYPE_QUALIFIER,
                            cl_kernel_arg_type_qualifier, 1)
                       CASE(CL_KERNEL_ARG_NAME, char, 7 + 1)
               }

#undef CASE

               if (ret_value_size != expected_size) {
                       fprintf(stderr,
                               "Failed: the returned size doesn't matches. Expected %lu, got %lu\n",
                               expected_size, ret_value_size);
                       piglit_merge_result(&result, PIGLIT_FAIL);
               }

          //TODO: test returned values
       }

       /*** Errors ***/

       /*
        * CL_INVALID_ARG_INDEX if arg_indx is not a valid argument index.
        */
       errNo = clGetKernelArgInfo(kernel,
                                  99,
                                  CL_KERNEL_ARG_NAME,
                                  0,
                                  NULL,
                                  &param_value_size);
       if(!piglit_cl_check_error(errNo, CL_INVALID_ARG_INDEX)) {
               fprintf(stderr,
                       "Failed (error code: %s): Trigger CL_INVALID_ARG_INDEX if arg_indx is not a valid argument index.\n",
                       piglit_cl_get_error_name(errNo));
               piglit_merge_result(&result, PIGLIT_FAIL);
       }

       /*
        * CL_INVALID_VALUE if param_name is not one of the supported
        * values or if size in bytes specified by param_value_size is
        * less than size of return type and param_value is not a NULL
        * value.
        */
       errNo = clGetKernelArgInfo(kernel,
                                  0,
                                  CL_DEVICE_NAME,
                                  0,
                                  NULL,
                                  &param_value_size);
       if(!piglit_cl_check_error(errNo, CL_INVALID_VALUE)) {
               fprintf(stderr,
                       "Failed (error code: %s): Trigger CL_INVALID_VALUE if param_name is not one of the supported values.\n",
                       piglit_cl_get_error_name(errNo));
               piglit_merge_result(&result, PIGLIT_FAIL);
       }

       errNo = clGetKernelArgInfo(kernel,
                                  0,
                                  CL_KERNEL_ARG_NAME,
                                  1,
                                  &param_value,
                                  NULL);
       if(!piglit_cl_check_error(errNo, CL_INVALID_VALUE)) {
               fprintf(stderr,
                       "Failed (error code: %s): Trigger CL_INVALID_VALUE if size in bytes specified by param_value is less than size of return type and param_value is not a NULL value.\n",
                       piglit_cl_get_error_name(errNo));
               piglit_merge_result(&result, PIGLIT_FAIL);
       }

       /*
        * CL_KERNEL_ARG_INFO_NOT_AVAILABLE
        * if the argument information is not available for kernel.
        */
/*
 * Note that PoCL, AMD and Intel libs always return arg info.
 */
{
       char* strings[] = { "kernel void dummy_kernel(int param_a) {}" };

       cl_program prog = piglit_cl_build_program_with_source(env->context,
                                                             1,
                                                             strings,
                                                             "");
       cl_kernel kern = piglit_cl_create_kernel(prog, "dummy_kernel");

       errNo = clGetKernelArgInfo(kern,
                                  0,
                                  CL_KERNEL_ARG_NAME,
                                  0,
                                  NULL,
                                  &param_value_size);
       if(!piglit_cl_check_error(errNo, CL_KERNEL_ARG_INFO_NOT_AVAILABLE)) {
               fprintf(stderr,
                       "Failed (error code: %s): Trigger CL_KERNEL_ARG_INFO_NOT_AVAILABLE if the argument information is not available for kernel.\n",
                       piglit_cl_get_error_name(errNo));
               piglit_merge_result(&result, PIGLIT_FAIL);
       }

       clReleaseKernel(kern);
       clReleaseProgram(prog);
}

       /*
        * CL_INVALID_KERNEL if kernel is not a valid kernel object.
        */
       errNo = clGetKernelArgInfo(NULL,
                                  0,
                                  CL_KERNEL_ARG_NAME,
                                  0,
                                  NULL,
                                  &param_value_size);
       if(!piglit_cl_check_error(errNo, CL_INVALID_KERNEL)) {
               fprintf(stderr,
                       "Failed (error code: %s): Trigger CL_INVALID_KERNEL if kernel is not a valid kernel object.\n",
                       piglit_cl_get_error_name(errNo));
               piglit_merge_result(&result, PIGLIT_FAIL);
       }

       clReleaseKernel(kernel);

       return result;
#else
       return PIGLIT_SKIP;
#endif
}
