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
 * @file create-buffer.c
 *
 * Test API function:
 *
 *   cl_mem clCreateBuffer (cl_context context,
 *                          cl_mem_flags flags,
 *                          size_t size,
 *                          void *host_ptr,
 *                          cl_int *errcode_ret)
 */

#include "piglit-framework-cl-api.h"


PIGLIT_CL_API_TEST_CONFIG_BEGIN

	config.name = "clCreateBuffer";
	config.version_min = 10;

	config.run_per_platform = true;
	config.create_context = true;

PIGLIT_CL_API_TEST_CONFIG_END


#define BUFFER_SIZE 512

static void
test(cl_context cl_ctx,
     cl_mem_flags flags,
     size_t size,
     void *host_ptr,
     cl_int expected_error,
     enum piglit_result* result,
     char* test_str) {
	cl_int errNo;
	cl_mem buffer;

	/* with errNo */
	buffer = clCreateBuffer(cl_ctx,
	                        flags,
	                        size,
	                        host_ptr,
	                        &errNo);
	
	if(!piglit_cl_check_error(errNo, expected_error)) {
		fprintf(stderr, "Failed (error code: %s): %s.\n",
		        piglit_cl_get_error_name(errNo), test_str);
		piglit_merge_result(result, PIGLIT_FAIL);
		return;
	}
	if(expected_error == CL_SUCCESS) {
		if(buffer == NULL) {
			printf("Expecting non-NULL cl_mem\n");
			fprintf(stderr, "Failed (NULL value returned): %s.\n", test_str);
			piglit_merge_result(result, PIGLIT_FAIL);
			return;
		}
		clReleaseMemObject(buffer);
	} else if (buffer != NULL) {
		printf("Expecting NULL cl_mem\n");
		fprintf(stderr, "Failed (non-NULL value returned): %s.\n", test_str);
		piglit_merge_result(result, PIGLIT_FAIL);
		return;
	}

	/* without errNo */
	buffer = clCreateBuffer(cl_ctx,
	                        flags,
	                        size,
	                        host_ptr,
	                        NULL);
	
	if(expected_error == CL_SUCCESS) {
		if(buffer == NULL) {
			printf("Expecting non-NULL cl_mem\n");
			fprintf(stderr, "Failed (NULL value returned): %s.\n", test_str);
			piglit_merge_result(result, PIGLIT_FAIL);
			return;
		}
		clReleaseMemObject(buffer);
	} else if (buffer != NULL) {
		printf("Expecting NULL cl_mem\n");
		fprintf(stderr, "Failed (non-NULL value returned): %s.\n", test_str);
		piglit_merge_result(result, PIGLIT_FAIL);
		return;
	}
}

/*
 * @param mask  Defines which fields to use
 */
static cl_mem_flags
get_mixed_mem_flags(int mask, const cl_mem_flags* mem_flags) {
	int i = 0;
	cl_mem_flags mixed_mem_flags = 0;
	
	while(mask > 0) {
		if(mask%2 == 1) {
			mixed_mem_flags |= mem_flags[i];
		}
		mask >>= 1;
		i++;
	}

	return mixed_mem_flags;
}

/*
 * Check if mem_flags are valid
 */
static bool
mem_flags_valid(cl_mem_flags mem_flags, int num_mutexes,
                const cl_mem_flags* mutexes) {
	int i;
	
	for(i = 0; i < num_mutexes; i++){
		/* check if mem_flags has the same bits set as mutexes[i] */
		if((mem_flags & mutexes[i]) == mutexes[i]) {
			return false;
		}
	}

	return true;
}


enum piglit_result
piglit_cl_test(const int argc,
               const char** argv,
               const struct piglit_cl_api_test_config* config,
               const struct piglit_cl_api_test_env* env)
{
	enum piglit_result result = PIGLIT_PASS;
	char test_str[1024];

	int i;
	int mask;
	cl_ulong alloc_size = BUFFER_SIZE; // max alloc size per device >= 128*1024*1024
	cl_ulong max_alloc;
	unsigned char host_buffer[BUFFER_SIZE];
	unsigned char host_buffer_read[BUFFER_SIZE];
	cl_mem_flags mixed_mem_flags;

	int num_mem_flags = PIGLIT_CL_ENUM_NUM(cl_mem_flags, env->version);
	const cl_mem_flags* mem_flags = PIGLIT_CL_ENUM_ARRAY(cl_mem_flags);

	int num_mutexes = PIGLIT_CL_ENUM_NUM(cl_mem_flags_mutexes, env->version);
	const cl_mem_flags* mutexes = PIGLIT_CL_ENUM_ARRAY(cl_mem_flags_mutexes);

	/*** Normal usage ***/

	for (i = 0; i < BUFFER_SIZE; i++){
		host_buffer[i] = (unsigned char)i;
	}

	/*
	 * For each memory flags mix.
	 * There are 2^(num_mem_flags)-1 possible options without
	 * excluding mutually exclusive options.
	 */
	for(mask = 1; mask < (1 << num_mem_flags); mask++) {
		mixed_mem_flags = get_mixed_mem_flags(mask, mem_flags);
		
		/* exclude invalid mixes */
		if(!mem_flags_valid(mixed_mem_flags, num_mutexes, mutexes)) {
			continue;
		}

		sprintf(test_str,
		        "Create buffer using 0x%X as memory flags",
		        (unsigned int)mixed_mem_flags);

		if(   (mixed_mem_flags & CL_MEM_USE_HOST_PTR)
		   || (mixed_mem_flags & CL_MEM_COPY_HOST_PTR)) {
			cl_mem buffer;

			/* test if function returns right values */
			test(env->context->cl_ctx,
			     mixed_mem_flags,
			     alloc_size,
			     host_buffer,
			     CL_SUCCESS,
			     &result,
			     test_str);

			/* test if buffer gets initialized properly */
			buffer = clCreateBuffer(env->context->cl_ctx,
			                        mixed_mem_flags,
			                        alloc_size,
			                        host_buffer,
			                        NULL);
			if(buffer) {
				cl_int errNo;
				
				errNo = clEnqueueReadBuffer(env->context->command_queues[0],
				                            buffer,
				                            true,
				                            0,
				                            alloc_size,
				                            host_buffer_read,
				                            0, NULL, NULL);
				
				if(errNo == CL_SUCCESS) {
					for(i = 0; i < BUFFER_SIZE; i++) {
						if(host_buffer[i] != host_buffer_read[i]) {
							printf("Buffer data was not initialized properly.\n");
							fprintf(stderr,
							        "Buffer data was not properly initialized using 0x%X as memory flags.\n",
							        (unsigned int)mixed_mem_flags);
							piglit_merge_result(&result, PIGLIT_FAIL);
							break;
						}
					}
				}
			}
		} else {
			test(env->context->cl_ctx, mixed_mem_flags, alloc_size, NULL,
			     CL_SUCCESS, &result, test_str);
		}
	}

#if defined(CL_VERSION_1_2)
	if(env->version >= 12) {
		test(env->context->cl_ctx,
		     0, // defaults to CL_MEM_READ_WRITE
		     alloc_size,
		     NULL,
		     CL_SUCCESS, &result,
		     "Create buffer using 0 (defaults to CL_MEM_READ_WRITE) as memory flags");
	}
#endif //CL_VERSION_1_2


	/*** Errors ***/

	/*
	 * CL_INVALID_CONTEXT if context is not a valid context.
	 */
	test(NULL, CL_MEM_READ_WRITE, alloc_size, NULL,
	     CL_INVALID_CONTEXT, &result,
	     "Trigger CL_INVALID_CONTEXT if context is not a valid context");

	/*
	 * CL_INVALID_VALUE if values specified in flags are not valid.
	 */
	for(mask = 1; mask < (1 << num_mem_flags); mask++) {
		mixed_mem_flags = get_mixed_mem_flags(mask, mem_flags);

		/* only invalid mixes */
		if(!mem_flags_valid(mixed_mem_flags, num_mutexes, mutexes)) {
			sprintf(test_str,
			        "Trigger CL_INVALID_VALUE if values specified in flags are not valid (using 0x%X as memory flags)",
			        (unsigned int)mixed_mem_flags);

			if(   (mixed_mem_flags & CL_MEM_USE_HOST_PTR)
			   || (mixed_mem_flags & CL_MEM_COPY_HOST_PTR)) {
				test(env->context->cl_ctx, mixed_mem_flags,
				     alloc_size, host_buffer,
				     CL_INVALID_VALUE, &result, test_str);
			} else {
				test(env->context->cl_ctx, mixed_mem_flags, alloc_size, NULL,
				     CL_INVALID_VALUE, &result, test_str);
			}
		}
	}

	/*
	 * CL_INVALID_BUFFER_SIZE if size is 0 or is greater than
	 * CL_DEVICE_MAX_MEM_ALLOC_SIZE value specified in table of
	 * OpenCL Device Queries for clGetDeviceInfo for all devices
	 * in context.
	 */
	test(env->context->cl_ctx, CL_MEM_READ_WRITE, 0, NULL,
	     CL_INVALID_BUFFER_SIZE, &result,
	     "Trigger CL_INVALID_BUFFER_SIZE if size is 0");
	
	max_alloc = 0;
	for(i = 0; i < env->context->num_devices; i++) {
		cl_ulong* max_device_alloc;

		max_device_alloc = piglit_cl_get_device_info(env->context->device_ids[i],
		                                             CL_DEVICE_MAX_MEM_ALLOC_SIZE);
		if(*max_device_alloc > max_alloc) {
			max_alloc = *max_device_alloc;
		}

		free(max_device_alloc);
	}
	
	test(env->context->cl_ctx,
	     CL_MEM_READ_WRITE,
	     max_alloc+1, // if we get to overflow, we're back at 0 and errNo must be the same
	     NULL,
	     CL_INVALID_BUFFER_SIZE, &result,
	     "Trigger CL_INVALID_BUFFER_SIZE if size is greater than CL_DEVICE_MAX_MEM_ALLOC_SIZE");

	/*
	 * CL_INVALID_HOST_PTR if host_ptr is NULL and CL_MEM_USE_HOST_PTR
	 * or CL_MEM_COPY_HOST_PTR are set in flags or if host_ptr is not
	 * NULL but CL_MEM_COPY_HOST_PTR or CL_MEM_USE_HOST_PTR are not
	 * set in flags.
	 */
	test(env->context->cl_ctx, CL_MEM_USE_HOST_PTR, alloc_size, NULL,
	     CL_INVALID_HOST_PTR, &result,
	     "Trigger CL_INVALID_HOST_PTR if host_ptr is NULL and CL_MEM_USE_HOST_PTR is set in flags");
	test(env->context->cl_ctx, CL_MEM_COPY_HOST_PTR, alloc_size, NULL,
	     CL_INVALID_HOST_PTR, &result,
	     "Trigger CL_INVALID_HOST_PTR if host_ptr is NULL and CL_MEM_COPY_HOST_PTR is set in flags");
	test(env->context->cl_ctx, CL_MEM_READ_WRITE, alloc_size, host_buffer,
	     CL_INVALID_HOST_PTR, &result,
	     "Trigger CL_INVALID_HOST_PTR if host_ptr is not NULL CL_MEM_USE_HOST_PTR or CL_MEM_COPY_HOST_PTR are not set in flags");

	return result;
}
