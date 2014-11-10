/*
 * Copyright 2014 Advanced Micro Devices, Inc.
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
 * Authors: Tom Stellard <thomas.stellard@amd.com>
 *          Aaron Watry <awatry@gmail.com>
 *
 */

#include "piglit-framework-cl-api.h"
#include "piglit-util-cl.h"


PIGLIT_CL_API_TEST_CONFIG_BEGIN

	config.name = "clEnqueueMapBuffer";
	config.version_min = 10;

	config.run_per_platform = true;
	config.create_context = true;

PIGLIT_CL_API_TEST_CONFIG_END

enum piglit_result
piglit_cl_test(const int argc,
	const char **argv,
	const struct piglit_cl_api_test_config* config,
	const struct piglit_cl_api_test_env* env)
{
	int host_src_buffer[4] = {1, 2, 3, 4};
	int host_dst_buffer[4] = {0, 0, 0, 0};
	cl_mem device_src_buffer, device_dst_buffer;
	cl_command_queue queue = env->context->command_queues[0];
	cl_int err;
	cl_int *mapped_buffer;
	int i;

	device_src_buffer = piglit_cl_create_buffer(
		env->context, CL_MEM_READ_WRITE, sizeof(host_src_buffer));
	device_dst_buffer = piglit_cl_create_buffer(
		env->context, CL_MEM_READ_WRITE, sizeof(host_dst_buffer));

	//Map source buffer
	mapped_buffer = clEnqueueMapBuffer(queue, device_src_buffer, CL_TRUE,
		CL_MAP_WRITE, 0, sizeof(host_src_buffer), 0, NULL, NULL, &err);
	if (!piglit_cl_check_error(err, CL_SUCCESS)) {
		return PIGLIT_FAIL;
	}

	//memcpy to it
	memcpy(mapped_buffer, host_src_buffer, sizeof(host_src_buffer));

	//Unmap source buffer
	err = clEnqueueUnmapMemObject(queue, device_src_buffer, mapped_buffer,
			0, NULL, NULL);
	if (!piglit_cl_check_error(err, CL_SUCCESS)) {
		return PIGLIT_FAIL;
	}

	//enqueueCopyBuffer from device src to device dst
	err = clEnqueueCopyBuffer(queue, device_src_buffer, device_dst_buffer,
			0, 0, sizeof(host_src_buffer), 0, NULL, NULL);
	if (!piglit_cl_check_error(err, CL_SUCCESS)) {
		return PIGLIT_FAIL;
	}

	//Map dest buffer
	mapped_buffer = clEnqueueMapBuffer(queue, device_dst_buffer, CL_TRUE,
			CL_MAP_READ, 0, sizeof(host_src_buffer), 0, NULL, NULL, &err);
	if (!piglit_cl_check_error(err, CL_SUCCESS)) {
		return PIGLIT_FAIL;
	}

	//Memcpy back
	memcpy(host_dst_buffer, mapped_buffer, sizeof(host_dst_buffer));

	//Unmap
	err = clEnqueueUnmapMemObject(queue, device_dst_buffer, mapped_buffer,
			0, NULL, NULL);
	if (!piglit_cl_check_error(err, CL_SUCCESS)) {
		return PIGLIT_FAIL;
	}

	//Do comparison of host src/dest
	for (i = 0; i < sizeof(host_src_buffer) / sizeof(host_src_buffer[0]);
									i++) {
		if (!piglit_cl_probe_integer(host_dst_buffer[i],
						host_src_buffer[i], 0)) {
			fprintf(stderr, "Error at %d\n", i);
			return PIGLIT_FAIL;
		}
	}

	err = clReleaseMemObject(device_src_buffer);
	if (!piglit_cl_check_error(err, CL_SUCCESS)) {
		return PIGLIT_FAIL;
	}
	err = clReleaseMemObject(device_dst_buffer);
	if (!piglit_cl_check_error(err, CL_SUCCESS)) {
		return PIGLIT_FAIL;
	}

#if defined(CL_VERSION_1_2)
	/*
	 * CL_INVALID_OPERATION if buffer has been created with
	 * CL_MEM_HOST_WRITE_ONLY or CL_MEM_HOST_NO_ACCESS
	 * and CL_MAP_READ is set in map_flags
	 *
	 * CL_INVALID_OPERATION if buffer has been created with
	 * CL_MEM_HOST_READ_ONLY or CL_MEM_HOST_NO_ACCESS
	 * and CL_MAP_WRITE or CL_MAP_WRITE_INVALIDATE_REGION is set in map_flags.
	 *
	 * Version: 1.2
	 */
	if(env->version >= 12) {
		enum piglit_result result;
		cl_mem device_mem;
		cl_ulong alloc_size = 64;
		cl_int *host_mem;

		/* host write only buffer */

		device_mem = clCreateBuffer(env->context->cl_ctx, CL_MEM_HOST_WRITE_ONLY,
		                            alloc_size, NULL, NULL);

		host_mem = clEnqueueMapBuffer(queue, device_mem, CL_TRUE,
		                              CL_MAP_READ, 0, alloc_size,
		                              0, NULL, NULL, &err);

		if (!piglit_cl_check_error(err, CL_INVALID_OPERATION)) {
			fprintf(stderr, "clEnqueueMapBuffer CL_MAP_READ: Failed (error code: %s): %s.\n",
			        piglit_cl_get_error_name(err),
			        "Trigger CL_INVALID_OPERATION when buffer has been created with CL_MEM_HOST_READ_ONLY");
			piglit_merge_result(&result, PIGLIT_FAIL);
		}

		clReleaseMemObject(device_mem);

		/* host no access buffer */

		device_mem = clCreateBuffer(env->context->cl_ctx, CL_MEM_HOST_NO_ACCESS,
		                            alloc_size, NULL, NULL);

		host_mem = clEnqueueMapBuffer(queue, device_mem, CL_TRUE,
		                              CL_MAP_READ, 0, alloc_size,
		                              0, NULL, NULL, &err);

		if (!piglit_cl_check_error(err, CL_INVALID_OPERATION)) {
			fprintf(stderr, "clEnqueueMapBuffer CL_MAP_READ: Failed (error code: %s): %s.\n",
			        piglit_cl_get_error_name(err),
			        "Trigger CL_INVALID_OPERATION when buffer has been created with CL_MEM_HOST_NO_ACCESS");
			piglit_merge_result(&result, PIGLIT_FAIL);
		}

		host_mem = clEnqueueMapBuffer(queue, device_mem, CL_TRUE,
		                              CL_MAP_WRITE, 0, alloc_size,
		                              0, NULL, NULL, &err);

		if (!piglit_cl_check_error(err, CL_INVALID_OPERATION)) {
			fprintf(stderr, "clEnqueueMapBuffer CL_MAP_WRITE: Failed (error code: %s): %s.\n",
			        piglit_cl_get_error_name(err),
			        "Trigger CL_INVALID_OPERATION when buffer has been created with CL_MEM_HOST_NO_ACCESS");
			piglit_merge_result(&result, PIGLIT_FAIL);
		}

		host_mem = clEnqueueMapBuffer(queue, device_mem, CL_TRUE,
		                              CL_MAP_WRITE_INVALIDATE_REGION, 0, alloc_size,
		                              0, NULL, NULL, &err);

		if (!piglit_cl_check_error(err, CL_INVALID_OPERATION)) {
			fprintf(stderr, "clEnqueueMapBuffer CL_MAP_WRITE_INVALIDATE_REGION: Failed (error code: %s): %s.\n",
			        piglit_cl_get_error_name(err),
			        "Trigger CL_INVALID_OPERATION when buffer has been created with CL_MEM_HOST_NO_ACCESS");
			piglit_merge_result(&result, PIGLIT_FAIL);
		}

		clReleaseMemObject(device_mem);

		/* host read only buffer */

		device_mem = clCreateBuffer(env->context->cl_ctx, CL_MEM_HOST_READ_ONLY,
		                            alloc_size, NULL, NULL);

		host_mem = clEnqueueMapBuffer(queue, device_mem, CL_TRUE,
		                              CL_MAP_WRITE, 0, alloc_size,
		                              0, NULL, NULL, &err);

		if (!piglit_cl_check_error(err, CL_INVALID_OPERATION)) {
			fprintf(stderr, "clEnqueueMapBuffer CL_MAP_WRITE: Failed (error code: %s): %s.\n",
			        piglit_cl_get_error_name(err),
			        "Trigger CL_INVALID_OPERATION when buffer has been created with CL_MEM_HOST_NO_ACCESS");
			piglit_merge_result(&result, PIGLIT_FAIL);
		}

		host_mem = clEnqueueMapBuffer(queue, device_mem, CL_TRUE,
		                              CL_MAP_WRITE_INVALIDATE_REGION, 0, alloc_size,
		                              0, NULL, NULL, &err);

		if (!piglit_cl_check_error(err, CL_INVALID_OPERATION)) {
			fprintf(stderr, "clEnqueueMapBuffer CL_MAP_WRITE_INVALIDATE_REGION: Failed (error code: %s): %s.\n",
			        piglit_cl_get_error_name(err),
			        "Trigger CL_INVALID_OPERATION when buffer has been created with CL_MEM_HOST_NO_ACCESS");
			piglit_merge_result(&result, PIGLIT_FAIL);
		}

		(void)host_mem;
		clReleaseMemObject(device_mem);

		if (result == PIGLIT_FAIL)
			return PIGLIT_FAIL;
	}
#endif //CL_VERSION_1_2

	return PIGLIT_PASS;
}
