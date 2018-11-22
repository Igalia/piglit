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
 *
 */

#include "piglit-framework-cl-custom.h"

PIGLIT_CL_CUSTOM_TEST_CONFIG_BEGIN

	config.name = "clCreateSubBuffer()";
	config.run_per_device = true;

PIGLIT_CL_CUSTOM_TEST_CONFIG_END

#define DATA_BYTE 0xabababab
#define BUFFER_SIZE 1000
#define PAD_SIZE 100
#define SUB_BUFFER_SIZE (BUFFER_SIZE - 2 * PAD_SIZE)
#define SUB_BUFFER_ITEMS (SUB_BUFFER_SIZE / 4)

#define STR(x) #x
#define STRINGIFY(x) STR(x)

char *source =
"kernel void test(global int *out) { int i; for (i = 0; i < " STRINGIFY(SUB_BUFFER_ITEMS) "; i++) { out[i] = " STRINGIFY(DATA_BYTE) ";  } }\n";


enum piglit_result
piglit_cl_test(const int argc,
	       const char **argv,
	       const struct piglit_cl_custom_test_config *config,
	       const struct piglit_cl_custom_test_env *env)
{
	piglit_cl_context piglit_cl_context = NULL;
	cl_command_queue queue = NULL;
	cl_mem buffer = NULL, sub_buffer = NULL;
	cl_program program = NULL;
	cl_kernel kernel = NULL;
	unsigned i;
	size_t global_size = 1, local_size = 1;
	cl_buffer_region region = {PAD_SIZE, SUB_BUFFER_SIZE };
	cl_int err;
	char *sub_data = malloc(BUFFER_SIZE);
	char *padding = malloc(PAD_SIZE);
	char data_byte = (char)DATA_BYTE;
	char pad_byte = 0xcd;
	char *out_data = malloc(BUFFER_SIZE);

	assert(SUB_BUFFER_SIZE % 4 == 0);
	memset(sub_data, data_byte, SUB_BUFFER_SIZE);
	memset(padding, pad_byte, PAD_SIZE);

	piglit_cl_context = piglit_cl_create_context(env->platform_id,
							&env->device_id, 1);
	queue = piglit_cl_context->command_queues[0];
	buffer = piglit_cl_create_buffer(piglit_cl_context, CL_MEM_READ_WRITE,
                                         BUFFER_SIZE);
	sub_buffer = clCreateSubBuffer(buffer, CL_MEM_READ_WRITE,
                                       CL_BUFFER_CREATE_TYPE_REGION,
                                       &region, &err);
	if (err != CL_SUCCESS) {
		fprintf(stderr, "clCreateSubBuffer() failed.");
		return PIGLIT_FAIL;
	}

	clEnqueueWriteBuffer(queue, buffer, CL_FALSE, 0, PAD_SIZE, padding,
                             0, NULL, NULL);
	clEnqueueWriteBuffer(queue, buffer, CL_FALSE, BUFFER_SIZE - PAD_SIZE,
                             PAD_SIZE, padding, 0, NULL, NULL);
	clFinish(queue);

	program = piglit_cl_build_program_with_source(piglit_cl_context, 1,
                                                      &source, "");
	kernel = piglit_cl_create_kernel(program, "test");

	if (!piglit_cl_set_kernel_arg(kernel, 0, sizeof(cl_mem), &sub_buffer)) {
		return PIGLIT_FAIL;
	}

	if (!piglit_cl_enqueue_ND_range_kernel(queue, kernel, 1, NULL,
	                                       &global_size, &local_size,
					       NULL)) {
		return PIGLIT_FAIL;
	}
	clFinish(queue);

	clEnqueueReadBuffer(queue, buffer, CL_TRUE, 0, BUFFER_SIZE, out_data,
                            0, NULL, NULL);
	for (i = 0; i < PAD_SIZE; i++) {
		if (!piglit_cl_probe_integer(out_data[i], pad_byte, 0)) {
			fprintf(stderr, "Failed at offset %u\n", i);
			return PIGLIT_FAIL;
		}
	}

	for (i = BUFFER_SIZE - PAD_SIZE; i < BUFFER_SIZE; i++) {
		if (!piglit_cl_probe_integer(out_data[i], pad_byte, 0)) {
			fprintf(stderr, "Failed at offset %u\n", i);
			return PIGLIT_FAIL;
		}
	}

	for (i = PAD_SIZE; i < BUFFER_SIZE - PAD_SIZE; i++) {
		if (!piglit_cl_probe_integer(out_data[i], data_byte, 0)) {
			fprintf(stderr, "Failed at offset %u\n", i);
			return PIGLIT_FAIL;
		}
	}
	return PIGLIT_PASS;
}
