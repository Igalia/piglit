/*
 * Copyright © 2012 Blaž Tomažič <blaz.tomazic@gmail.com>
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
 */

/**
 * @file get-mem-object-info.c
 *
 * Test API function:
 *
 *   cl_int clGetMemObjectInfo (cl_mem memobj,
 *                              cl_mem_info param_name,
 *                              size_t param_value_size,
 *                              void *param_value,
 *                              size_t *param_value_size_ret)
 */

#include "piglit-framework-cl-api.h"


PIGLIT_CL_API_TEST_CONFIG_BEGIN

	config.name = "clGetMemObjectInfo";
	config.version_min = 10;

	config.run_per_platform = true;
	config.create_context = true;

PIGLIT_CL_API_TEST_CONFIG_END

#define BUFFER_SIZE 512

static enum piglit_result
test_get_value(cl_mem memobj,
               cl_mem_info param_name,
               size_t *param_value_size,
               void **param_value) {
	cl_int errNo;

	errNo = clGetMemObjectInfo(memobj,
	                           param_name,
	                           0,
	                           NULL,
	                           param_value_size);
	if(!piglit_cl_check_error(errNo, CL_SUCCESS)) {
		fprintf(stderr,
		        "Failed (error code: %s): Get size of %s.\n",
		        piglit_cl_get_error_name(errNo),
		        piglit_cl_get_enum_name(param_name));
		*param_value = NULL;
		return PIGLIT_FAIL;
	}

	*param_value = malloc(*param_value_size);
	errNo = clGetMemObjectInfo(memobj,
	                           param_name,
	                           *param_value_size,
	                           *param_value,
	                           NULL);
	if(!piglit_cl_check_error(errNo, CL_SUCCESS)) {
		fprintf(stderr,
		        "Failed (error code: %s): Get value of %s.\n",
		        piglit_cl_get_error_name(errNo),
		        piglit_cl_get_enum_name(param_name));
		free(*param_value);
		*param_value = NULL;
		return PIGLIT_FAIL;
	}

	return PIGLIT_PASS;
}

static enum piglit_result
test(int n,
     cl_mem memobj,
     cl_mem_info param_name,
     cl_mem_object_type mem_type,
     cl_mem_flags mem_flags,
     size_t mem_size,
     void *mem_ptr,
     const struct piglit_cl_api_test_env *env,
     cl_mem mem_parent,
     size_t mem_offset) {
	size_t param_value_size;
	void* param_value;

	if (test_get_value(memobj,
	                   param_name,
	                   &param_value_size,
	                   &param_value) != PIGLIT_PASS) {
		fprintf(stderr,
		        "Buffer %d, test_get_value() failed.\n", n);
		return PIGLIT_FAIL;
	}

#define CHECK_SIZE(_type_) \
	if (param_value_size != sizeof(_type_)) { \
		fprintf(stderr, \
		        "Buffer %d, failed: the returned size doesn't match. Expected %lu, got %lu\n", \
		        n, sizeof(_type_), param_value_size); \
		return PIGLIT_FAIL; \
	}

#define CHECK_VALUE(_type_, _value_) \
	if (*(_type_*)param_value != _value_) { \
		fprintf(stderr, \
		        "Buffer %d, failed: the returned value doesn't match.\n", \
		        n); \
		return PIGLIT_FAIL; \
	}

	switch (param_name) {
		case CL_MEM_TYPE:
			CHECK_SIZE(cl_mem_object_type)
			CHECK_VALUE(cl_mem_object_type, mem_type)
			break;
		case CL_MEM_FLAGS:
			CHECK_SIZE(cl_mem_flags)
			CHECK_VALUE(cl_mem_flags, mem_flags)
			break;
		case CL_MEM_SIZE:
			CHECK_SIZE(size_t)
			CHECK_VALUE(size_t, mem_size)
			break;
		case CL_MEM_HOST_PTR:
			CHECK_SIZE(void *)
			CHECK_VALUE(void *, mem_ptr)
			break;
		case CL_MEM_MAP_COUNT:
			CHECK_SIZE(cl_uint)
			//stale
			break;
		case CL_MEM_REFERENCE_COUNT:
			CHECK_SIZE(cl_uint)
			//stale
			break;
		case CL_MEM_CONTEXT:
			CHECK_SIZE(cl_context)
			CHECK_VALUE(cl_context, env->context->cl_ctx)
			break;
#if defined(CL_VERSION_1_1)
		case CL_MEM_ASSOCIATED_MEMOBJECT:
			if (env->version >= 11) {
				CHECK_SIZE(cl_mem)
				CHECK_VALUE(cl_mem, mem_parent)
			}
			break;
		case CL_MEM_OFFSET:
			if (env->version >= 11) {
				CHECK_SIZE(size_t)
				CHECK_VALUE(size_t, mem_offset)
			}
			break;
#endif
#if defined(CL_VERSION_2_0)
		case CL_MEM_USES_SVM_POINTER:
			if (env->version >= 20) {
				CHECK_SIZE(cl_bool)
				CHECK_VALUE(cl_bool, CL_FALSE)
			}
			break;
#endif
		default:
			fprintf(stderr, "Warn: untested parameter %s\n",
			        piglit_cl_get_enum_name(param_name));
			return PIGLIT_WARN;
	}

#undef CHECK_SIZE
#undef CHECK_VALUE

	free(param_value);
	return PIGLIT_PASS;
}

enum piglit_result
piglit_cl_test(const int argc,
               const char** argv,
               const struct piglit_cl_api_test_config* config,
               const struct piglit_cl_api_test_env* env)
{
	enum piglit_result result = PIGLIT_PASS;

	int i;
	cl_int errNo;
	cl_mem memobj[3];
	char host_mem[BUFFER_SIZE] = {0};

	size_t param_value_size;
	void* param_value;

	int num_mem_infos = PIGLIT_CL_ENUM_NUM(cl_mem_info, env->version);
	const cl_mem_info* mem_infos = PIGLIT_CL_ENUM_ARRAY(cl_mem_info);
	
	memobj[0] = clCreateBuffer(env->context->cl_ctx,
	                           CL_MEM_READ_WRITE|CL_MEM_COPY_HOST_PTR,
	                           BUFFER_SIZE,
	                           host_mem,
	                           &errNo);
	if(!piglit_cl_check_error(errNo, CL_SUCCESS)) {
		fprintf(stderr,
		        "Failed (error code: %s): Create buffer 0.\n",
		        piglit_cl_get_error_name(errNo));
		return PIGLIT_FAIL;
	}

	memobj[1] = clCreateBuffer(env->context->cl_ctx,
	                           CL_MEM_READ_WRITE|CL_MEM_USE_HOST_PTR,
	                           BUFFER_SIZE,
	                           host_mem,
	                           &errNo);
	if(!piglit_cl_check_error(errNo, CL_SUCCESS)) {
		fprintf(stderr,
		        "Failed (error code: %s): Create buffer 1.\n",
		        piglit_cl_get_error_name(errNo));
		return PIGLIT_FAIL;
	}

#if defined(CL_VERSION_1_1)
	if (env->version >= 11) {
		cl_buffer_region region;
		region.origin = BUFFER_SIZE/2;
		region.size = BUFFER_SIZE/2;

		memobj[2] = clCreateSubBuffer(memobj[1],
		                              0,
		                              CL_BUFFER_CREATE_TYPE_REGION,
		                              &region,
		                              &errNo);
		if(!piglit_cl_check_error(errNo, CL_SUCCESS)) {
			fprintf(stderr,
			        "Failed (error code: %s): Create buffer 2.\n",
			        piglit_cl_get_error_name(errNo));
			return PIGLIT_FAIL;
		}
	}
#endif

	/*** Basic test ***/
	piglit_merge_result(&result,
	                    test_get_value(memobj[0],
	                                   CL_MEM_TYPE,
	                                   &param_value_size,
	                                   &param_value)
	                   );
	free(param_value);

	if (result != PIGLIT_PASS)
		return result;

	/*** Normal usage ***/
	for(i = 0; i < num_mem_infos; i++) {
		enum piglit_result tmp_result = PIGLIT_PASS;
		enum piglit_result sub_result = PIGLIT_PASS;
		void *copy_ptr = host_mem;

#if defined(CL_VERSION_1_2)
		if (env->version >= 12)
			copy_ptr = NULL;
#endif

		tmp_result = test(0,
		                  memobj[0],
		                  mem_infos[i],
		                  CL_MEM_OBJECT_BUFFER,
		                  CL_MEM_READ_WRITE|CL_MEM_COPY_HOST_PTR,
		                  BUFFER_SIZE,
		                  copy_ptr,
		                  env,
		                  NULL,
		                  0);
		piglit_merge_result(&sub_result, tmp_result);

		tmp_result = test(1,
		                  memobj[1],
		                  mem_infos[i],
		                  CL_MEM_OBJECT_BUFFER,
		                  CL_MEM_READ_WRITE|CL_MEM_USE_HOST_PTR,
		                  BUFFER_SIZE,
		                  host_mem,
		                  env,
		                  NULL,
		                  0);
		piglit_merge_result(&sub_result, tmp_result);

#if defined(CL_VERSION_1_1)
		if (env->version >= 11) {
			tmp_result = test(2,
			                  memobj[2],
			                  mem_infos[i],
			                  CL_MEM_OBJECT_BUFFER,
			                  CL_MEM_READ_WRITE|CL_MEM_USE_HOST_PTR,
			                  BUFFER_SIZE/2,
			                  host_mem + BUFFER_SIZE/2,
			                  env,
			                  memobj[1],
			                  BUFFER_SIZE/2);
			piglit_merge_result(&sub_result, tmp_result);
		}
#endif
		piglit_merge_result(&result, sub_result);
		piglit_report_subtest_result(sub_result, "%s",
		                             piglit_cl_get_enum_name(mem_infos[i]));

	}

	/*** Errors ***/

	/*
	 * CL_INVALID_VALUE if param_name is not one of the supported
	 * values or if size in bytes specified by param_value_size is
	 * less than size of return type and param_value is not a NULL
	 * value.
	 */
	errNo = clGetMemObjectInfo(memobj[0],
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

	param_value = malloc(sizeof(cl_mem_object_type));
	errNo = clGetMemObjectInfo(memobj[0],
	                           CL_MEM_TYPE,
	                           1,
	                           param_value,
	                           NULL);
	free(param_value);
	if(!piglit_cl_check_error(errNo, CL_INVALID_VALUE)) {
		fprintf(stderr,
		        "Failed (error code: %s): Trigger CL_INVALID_VALUE if size in bytes specified by param_value is less than size of return type and param_value is not a NULL value.\n",
		        piglit_cl_get_error_name(errNo));
		piglit_merge_result(&result, PIGLIT_FAIL);
	}
	
	/*
	 * CL_INVALID_MEM_OBJECT if memobj is a not a valid memory object.
	 */
	errNo = clGetMemObjectInfo(NULL,
	                           CL_MEM_TYPE,
	                           0,
	                           NULL,
	                           &param_value_size);
	if(!piglit_cl_check_error(errNo, CL_INVALID_MEM_OBJECT)) {
		fprintf(stderr,
		        "Failed (error code: %s): Trigger CL_INVALID_MEM_OBJECT if memobj is not a valid memory object.\n",
		        piglit_cl_get_error_name(errNo));
		piglit_merge_result(&result, PIGLIT_FAIL);
	}

	clReleaseMemObject(memobj[0]);
	clReleaseMemObject(memobj[1]);
#if defined(CL_VERSION_1_1)
	if (env->version >= 11)
		clReleaseMemObject(memobj[2]);
#endif

	return result;
}
