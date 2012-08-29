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

#include "piglit-framework-cl-program.h"

PIGLIT_CL_PROGRAM_TEST_CONFIG_BEGIN

	config.name = "Run kernel with max work item sizes";
	config.clc_version_min = 10;

	config.run_per_device = true;

	config.program_source = "kernel void test(global uint* out){ \
	                             uint i = get_global_id(get_work_dim()-1); \
	                             out[i] = i+1;\
	                         }";
	config.kernel_name = "test";

PIGLIT_CL_PROGRAM_TEST_CONFIG_END

size_t get_group_size(size_t* item_sizes, cl_uint dimensions)
{
	cl_uint i;
	size_t size = 1;

	for(i = 0; i < dimensions; i++) {
		size *= item_sizes[i];
	}

	return size;
}

enum piglit_result
piglit_cl_test(const int argc,
               const char** argv,
               const struct piglit_cl_program_test_config* config,
               const struct piglit_cl_program_test_env* env)
{
	enum piglit_result result = PIGLIT_PASS;

	cl_uint i, j;

	cl_uint* dimensions = NULL;
	size_t* global_size = NULL;
	size_t* group_size = NULL;
	size_t* item_sizes = NULL;

	cl_uint* ptr_out = NULL;
	cl_mem mem_out = NULL;

	dimensions = piglit_cl_get_device_info(env->device_id,
	                                       CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS);
	group_size = piglit_cl_get_device_info(env->device_id,
	                                       CL_DEVICE_MAX_WORK_GROUP_SIZE);
	item_sizes = piglit_cl_get_device_info(env->device_id,
	                                       CL_DEVICE_MAX_WORK_ITEM_SIZES);

	global_size = malloc(*dimensions * sizeof(size_t));

	printf("Dimensions: %u\n", *dimensions);
	printf("Max group size: %zu\n", *group_size);
	for(i = 0; i < *dimensions; i++) {
		printf("Max item size dimension %u: %zu\n", i+1, item_sizes[i]);
	}

	/* Execute a kernel with max work item size in each dimension separately */
	for(i = 0; i < *dimensions; i++) {
		printf("Testing max item size in dimension %u:\n", i+1);

		// set global size
		for(j = 0; j < i; j++) {
			global_size[j] = 1;
		}
		global_size[i] = item_sizes[i] < *group_size ? item_sizes[i]
		                                             : *group_size;
		for(j = i+1; j < *dimensions; j++) {
			global_size[j] = 0;
		}

		printf("  Global work size and local work size is: {%zu",
		       global_size[0]);
		for(j = 1; j < *dimensions; j++) {
			printf(", %zu", global_size[j]);
		}
		printf("}\n");

		// create an out buffer and fill it with zeroes
		ptr_out = calloc(global_size[i], sizeof(cl_uint));
		mem_out = piglit_cl_create_buffer(env->context, CL_MEM_WRITE_ONLY,
		                                  global_size[i] * sizeof(cl_uint));
		piglit_cl_write_buffer(env->context->command_queues[0], mem_out, 0,
		                       global_size[i] * sizeof(cl_uint), ptr_out);

		// set kernel args and run the kernel
		piglit_cl_set_kernel_buffer_arg(env->kernel, 0, &mem_out);
		piglit_cl_execute_ND_range_kernel(env->context->command_queues[0],
		                                  env->kernel,
		                                  i+1,
		                                  global_size,
		                                  global_size);

		// read back out buffer and check its values
		piglit_cl_read_buffer(env->context->command_queues[0], mem_out, 0,
		                      global_size[i] * sizeof(cl_uint), ptr_out);
		for(j = 0; j < global_size[i]; j++) {
			if(ptr_out[j] != j+1) {
				printf("  At index %u, expecting %u, but got %u.\n",
				       j, j+1, ptr_out[j]);
				piglit_merge_result(&result, PIGLIT_FAIL);
				break;
			}
		}

		// free resources
		clReleaseMemObject(mem_out);
		free(ptr_out);
	}

	/* Release resources */
	free(global_size);
	free(dimensions);
	free(group_size);
	free(item_sizes);

	return result;
}
