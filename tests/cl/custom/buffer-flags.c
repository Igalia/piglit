/*
 * Copyright 2013 Jan Vesely
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
 */

#include "piglit-framework-cl-custom.h"

PIGLIT_CL_CUSTOM_TEST_CONFIG_BEGIN

	config.name = "CL buffer memory flags";
	config.run_per_device = true;

PIGLIT_CL_CUSTOM_TEST_CONFIG_END


/* This is a simple copy-kernel, the purpose of this test is to test buffer
 * data availability, not specific compute function. */
char *source =
"__kernel void test (global float *out, global float *in) {\n"
"	int i = get_global_id(0);                          \n"
"	out[i] = in[i];                                    \n"
"}                                                         \n";

#define BUFFER_SIZE   16 /* not too big */

static enum piglit_result
buffer_test(piglit_cl_context *ctx,
	    cl_program *prg,
	    cl_mem_flags in_flags,
	    cl_mem_flags out_flags,
	    float data)
{
	float in_data[BUFFER_SIZE];
	float out_data[BUFFER_SIZE];
	float *result = out_data;

	cl_mem in_buffer = NULL, out_buffer = NULL;
	cl_kernel kernel = NULL;

	piglit_cl_context context = *ctx;

	cl_int errNo;
	unsigned i;
	size_t global = BUFFER_SIZE;
	size_t local = 1;
	enum piglit_result ret = PIGLIT_PASS;
	const char kernel_name[] = "test";

	printf("> Running kernel test: in-0x%x-out-0x%x\n",
	       (unsigned)in_flags, (unsigned)out_flags);
	for (i = 0; i < BUFFER_SIZE; ++i) {
		in_data[i] = data;
		out_data[i] = 0.0f;
	}
	printf("Using kernel %s\n", kernel_name);

	printf("Creating buffers...\n");
	/* Create input buffer */
	if ((in_flags & CL_MEM_USE_HOST_PTR) ||
	    (in_flags & CL_MEM_COPY_HOST_PTR)) {
		/* Use host side memory */
		in_buffer = clCreateBuffer(context->cl_ctx, in_flags,
			sizeof(in_data), in_data, &errNo);
	} else {
		/* Use device memory and copy */
		in_buffer = clCreateBuffer(context->cl_ctx, in_flags,
			sizeof(in_data), NULL, &errNo);
		if (errNo == CL_SUCCESS && !piglit_cl_write_buffer(
		           context->command_queues[0], in_buffer, 0,
		           sizeof(in_data), in_data)) {
			ret = PIGLIT_FAIL;
			goto cleanup;
		}
	}

	if(!piglit_cl_check_error(errNo, CL_SUCCESS)) {
		fprintf(stderr,
		        "Could not create in buffer with flags %x: %s\n",
		        (unsigned)in_flags, piglit_cl_get_error_name(errNo));
		ret = PIGLIT_FAIL;
		goto cleanup;
	}

	/* Create destination buffer */
	if ((out_flags & CL_MEM_USE_HOST_PTR) ||
	    (out_flags & CL_MEM_COPY_HOST_PTR)) {
		out_buffer = clCreateBuffer(context->cl_ctx, out_flags,
			sizeof(out_data), out_data, &errNo);
	} else {
		out_buffer = clCreateBuffer(context->cl_ctx, out_flags,
			sizeof(out_data), NULL, &errNo);
	}
	if(!piglit_cl_check_error(errNo, CL_SUCCESS)) {
		fprintf(stderr,
		        "Could not create out buffer with flags %x: %s\n",
		        (unsigned)out_flags, piglit_cl_get_error_name(errNo));
		ret = PIGLIT_FAIL;
		goto cleanup;
	}
	kernel = piglit_cl_create_kernel(*prg, kernel_name);

	printf("Setting kernel arguments...\n");
	if (!piglit_cl_set_kernel_arg(kernel, 0, sizeof(cl_mem), &out_buffer)) {
		ret = PIGLIT_FAIL;
		goto cleanup;
	}
	if (!piglit_cl_set_kernel_arg(kernel, 1, sizeof(cl_mem), &in_buffer)) {
		ret = PIGLIT_FAIL;
		goto cleanup;
	}

	printf("Running the kernel...\n");
	if (!piglit_cl_enqueue_ND_range_kernel(context->command_queues[0],
					kernel, 1, NULL, &global, &local, NULL)) {
		ret = PIGLIT_FAIL;
		goto cleanup;
	}

	clFlush(context->command_queues[0]);

	printf("Retrieving results...\n");
	if ((out_flags & CL_MEM_USE_HOST_PTR) ||
	     (out_flags & CL_MEM_ALLOC_HOST_PTR)) {
		/* buffer uses host side memory, map it here,
                 * map is also a synchronization point */
		result = clEnqueueMapBuffer(context->command_queues[0],
			out_buffer, true, CL_MAP_READ, 0, sizeof(out_data), 0,
			NULL, NULL, &errNo);
		if (!piglit_cl_check_error(errNo, CL_SUCCESS)) {
			fprintf(stderr,
				"Could not map out buffer with flags %x: %s\n",
				(unsigned)out_flags,
			        piglit_cl_get_error_name(errNo));
			ret = PIGLIT_FAIL;
			goto cleanup;
		}
	} else {
		/* Copy back from device */
		if (!piglit_cl_read_buffer(context->command_queues[0],
		                           out_buffer, 0, sizeof(out_data),
		                           out_data)) {
			ret = PIGLIT_FAIL;
			goto cleanup;
		}
	}

	for (i = 0; i < BUFFER_SIZE; ++i) {
		if (!piglit_cl_probe_floating(result[i], in_data[i], 0)) {
			printf("Error at float[%u]\n", i);
			ret = PIGLIT_FAIL;
			goto cleanup;
		}
	}

	/* cleanup */
cleanup:
	/* result = out_data is set at the beginning. It's modified only if
	 * 'result' is used to store clEnqueueMapBuffer pointer. */
	if (result != out_data)
		clEnqueueUnmapMemObject(context->command_queues[0], out_buffer,
		                        result, 0, NULL, NULL);
	clReleaseMemObject(in_buffer);
	clReleaseMemObject(out_buffer);
	clReleaseKernel(kernel);
	piglit_report_subtest_result(ret, "in-0x%x-out-0x%x",
	                             (unsigned)in_flags, (unsigned)out_flags);
	return ret;

};

enum piglit_result
piglit_cl_test(const int argc,
	       const char **argv,
	       const struct piglit_cl_custom_test_config *config,
	       const struct piglit_cl_custom_test_env *env)
{

	piglit_cl_context context = NULL;
	cl_program program = NULL;

	unsigned i, j;

	static const cl_mem_flags possibilities[] = {
#ifdef CL_VERSION_1_2
		0,
#endif
		CL_MEM_USE_HOST_PTR,
		CL_MEM_COPY_HOST_PTR,
		CL_MEM_ALLOC_HOST_PTR,
		CL_MEM_COPY_HOST_PTR | CL_MEM_ALLOC_HOST_PTR,
	};

	const size_t nump = ARRAY_SIZE(possibilities);
	enum piglit_result part_ret, ret = PIGLIT_PASS;
	float data = 10;

	context = piglit_cl_create_context(env->platform_id, &env->device_id, 1);

	program = piglit_cl_build_program_with_source(context, 1, &source, NULL);

	for (i = 0; i < nump; ++i)
		for (j = 0; j < nump; ++j) {
			part_ret = buffer_test(&context, &program,
			     possibilities[i], possibilities[j], ++data);
			piglit_merge_result(&ret, part_ret);
		}

	clReleaseProgram(program);
	piglit_cl_release_context(context);
	return ret;
}
