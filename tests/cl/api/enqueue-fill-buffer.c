/*
 * Copyright © 2015 Serge Martin (EdB) <edb+piglit@sigluy.net>
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
 * @file enqueue-fill-buffer.c
 *
 * Test API function:
 *
 *   cl_int
 *   clEnqueueFillBuffer(cl_command_queue command_queue, cl_mem buffer,
 *                       const void *pattern, size_t pattern_size,
 *                       size_t offset, size_t size,
 *                       cl_uint num_events_in_wait_list,
 *                       const cl_event *event_wait_list,
 *                       cl_event *event )
 */

#include "piglit-framework-cl-api.h"


PIGLIT_CL_API_TEST_CONFIG_BEGIN

	config.name = "clEnqueueFillBuffer";
	config.version_min = 12;

	config.run_per_device = true;
	config.create_context = true;

PIGLIT_CL_API_TEST_CONFIG_END


#if defined(CL_VERSION_1_2)
static bool
test(cl_command_queue queue, cl_mem buffer,
     const void *pattern, size_t pattern_size,
     size_t offset, size_t size,
     cl_uint num_events_in_wait_list,
     const cl_event *event_wait_list,
     cl_event *event ,
     cl_int expected_error, enum piglit_result* result,
     const char* test_str) {
	cl_int errNo;

	errNo = clEnqueueFillBuffer(queue, buffer,
	                            pattern, pattern_size, offset, size,
	                            num_events_in_wait_list, event_wait_list,
	                            event);

	if(!piglit_cl_check_error(errNo, expected_error)) {
		fprintf(stderr, "Failed (error code: %s): %s.\n",
		        piglit_cl_get_error_name(errNo), test_str);
		piglit_merge_result(result, PIGLIT_FAIL);
		return false;
	}

	return true;
}
#endif

enum piglit_result
piglit_cl_test(const int argc,
               const char **argv,
               const struct piglit_cl_api_test_config* config,
               const struct piglit_cl_api_test_env* env)
{
#if defined(CL_VERSION_1_2)
	enum piglit_result result = PIGLIT_PASS;
	cl_int src_buf[4] = {4, 5, 6, 7};
	cl_int dst_buf[4] = {0, 0, 0, 0};
	cl_int exp_buf[4] = {4, 9, 9, 7};
	cl_int pattern = 9;
	cl_event event;
	cl_mem device_buffer;
	cl_command_queue queue = env->context->command_queues[0];
	int i;

/*** Normal usage ***/
	device_buffer = piglit_cl_create_buffer(env->context, CL_MEM_READ_WRITE,
	                                        sizeof(src_buf));
	if (!piglit_cl_write_whole_buffer(queue, device_buffer, src_buf)) {
		return PIGLIT_FAIL;
	}

	if (!test(queue, device_buffer, &pattern, sizeof(pattern),
	          sizeof(pattern) * 1, sizeof(pattern) * 2,
	          0, NULL, NULL,
	          CL_SUCCESS, &result, "Enqueuing the buffer to fill.")) {
		return PIGLIT_FAIL;
	}

	if (!piglit_cl_read_whole_buffer(queue, device_buffer, dst_buf)) {
		return PIGLIT_FAIL;
	}

	for (i = 0; i < sizeof(dst_buf) / sizeof(dst_buf[0]); ++i) {
		if (!piglit_cl_probe_integer(dst_buf[i], exp_buf[i], 0)) {
			fprintf(stderr, "Error at %d\n", i);
			return PIGLIT_FAIL;
		}
	}

/*** Errors ***/

	/*
	 * CL_INVALID_COMMAND_QUEUE if command_queue is not a valid command-queue.
	 */
	test(NULL, device_buffer, &pattern, sizeof(pattern),
	     sizeof(pattern) * 1, sizeof(pattern) * 2,
	     0, NULL, NULL,
	     CL_INVALID_COMMAND_QUEUE, &result,
	     "CL_INVALID_COMMAND_QUEUE if command_queue is not a valid command-queue");

	/*
	 * CL_INVALID_CONTEXT if the context associated with command_queue and
	 * buffer are not the same or if the context associated with command_queue
	 * and events in event_wait_list are not the same.
	 */
	{
		piglit_cl_context context;
		cl_int err;
		context = piglit_cl_create_context(env->platform_id,
		                                   env->context->device_ids, 1);
		if (context) {
			event = clCreateUserEvent(context->cl_ctx, &err);
			if (err == CL_SUCCESS) {
				err = clSetUserEventStatus(event, CL_COMPLETE);
				if (err == CL_SUCCESS) {
					test(context->command_queues[0], device_buffer,
					     &pattern, sizeof(pattern),
					     sizeof(pattern) * 1, sizeof(pattern) * 2,
					     0, NULL, NULL,
					     CL_INVALID_CONTEXT, &result,
					     "CL_INVALID_CONTEXT if the context associated with command_queue and buffer are not the same");

					test(context->command_queues[0], device_buffer,
					     &pattern, sizeof(pattern),
					     sizeof(pattern) * 1, sizeof(pattern) * 2,
					     1, &event, NULL,
					     CL_INVALID_CONTEXT, &result,
					     "CL_INVALID_CONTEXT if the context associated with command_queue and events in event_wait_list are not the same");
				} else {
					fprintf(stderr, "Could not set event status.\n");
					piglit_merge_result(&result, PIGLIT_WARN);
				}
				clReleaseEvent(event);
			} else {
				fprintf(stderr, "Could not create user event.\n");
				piglit_merge_result(&result, PIGLIT_WARN);
			}

			piglit_cl_release_context(context);
		} else {
			fprintf(stderr, "Could not test triggering CL_INVALID_CONTEXT.\n");
			piglit_merge_result(&result, PIGLIT_WARN);
		}
	}

	/*
	 * CL_INVALID_MEM_OBJECT if buffer is not a valid buffer object.
	 */
	test(queue, NULL, &pattern, sizeof(pattern),
	     sizeof(pattern) * 1, sizeof(pattern) * 2,
	     0, NULL, NULL,
	     CL_INVALID_MEM_OBJECT, &result,
	     "CL_INVALID_MEM_OBJECT if buffer is not a valid buffer object");

	/*
	 * CL_INVALID_VALUE if offset or offset + size require accessing elements
	 * outside the buffer buffer object respectively.
	 */
	test(queue, device_buffer, &pattern, sizeof(pattern),
	     (sizeof(pattern) * 1 + sizeof(src_buf)), sizeof(pattern) * 2,
	     0, NULL, NULL,
	     CL_INVALID_VALUE, &result,
	     "CL_INVALID_VALUE if offset or offset + size require accessing elements outside the buffer buffer object respectively");

	/*
	 * CL_INVALID_VALUE if pattern is NULL or if pattern_size is 0
	 * or if pattern_size is not one of {1, 2, 4, 8, 16, 32, 64, 128}.
	 */
	test(queue, device_buffer, NULL, sizeof(pattern),
	     sizeof(pattern) * 1, sizeof(pattern) * 2,
	     0, NULL, NULL,
	     CL_INVALID_VALUE, &result,
	     "CL_INVALID_VALUE if pattern is NULL");

	test(queue, device_buffer, &pattern, 0,
	     sizeof(pattern) * 1, sizeof(pattern) * 2,
	     0, NULL, NULL,
	     CL_INVALID_VALUE, &result,
	     "CL_INVALID_VALUE if pattern_size is 0");

	test(queue, device_buffer, &pattern, 3,
	    sizeof(pattern) * 1, sizeof(pattern) * 2,
	    0, NULL, NULL,
	    CL_INVALID_VALUE, &result,
	    "CL_INVALID_VALUE if pattern_size is not one of {1, 2, 4, 8, 16, 32, 64, 128}");

	/*
	 * CL_INVALID_VALUE if offset and size are not a multiple of pattern_size.
	 */
	test(queue, device_buffer, &src_buf, sizeof(src_buf),
	     1, sizeof(pattern) * 2,
	     0, NULL, NULL,
	     CL_INVALID_VALUE, &result,
	     "CL_INVALID_VALUE if offset is not a multiple of pattern_size");

	test(queue, device_buffer, &src_buf, sizeof(src_buf),
	     sizeof(pattern) * 1, 1,
	     0, NULL, NULL,
	     CL_INVALID_VALUE, &result,
	     "CL_INVALID_VALUE if size is not a multiple of pattern_size");

	/*
	 * CL_INVALID_EVENT_WAIT_LIST if event_wait_list is NULL and
	 * num_events_in_wait_list > 0, or event_wait_list is not NULL and
	 * num_events_in_wait_list is 0, or if event objects in event_wait_list
	 * are not valid events.
	 */
	event = NULL;
	test(queue, device_buffer, &pattern, sizeof(pattern),
	     sizeof(pattern) * 1, sizeof(pattern) * 2,
	     1, NULL, NULL,
	     CL_INVALID_EVENT_WAIT_LIST, &result,
	     "CL_INVALID_EVENT_WAIT_LIST if event_wait_list is NULL and num_events_in_wait_list > 0");

	test(queue, device_buffer, &pattern, sizeof(pattern),
	     sizeof(pattern) * 1, sizeof(pattern) * 2,
	     0, &event, 0,
	     CL_INVALID_EVENT_WAIT_LIST, &result,
	     "CL_INVALID_EVENT_WAIT_LIST if event_wait_list is not NULL and num_events_in_wait_list is 0");

	test(queue, device_buffer, &pattern, sizeof(pattern),
	     sizeof(pattern) * 1, sizeof(pattern) * 2,
	     1, &event, 0,
	     CL_INVALID_EVENT_WAIT_LIST, &result,
	     "CL_INVALID_EVENT_WAIT_LIST if event objects in event_wait_list are not valid events");

	/*
	 * CL_MISALIGNED_SUB_BUFFER_OFFSET if buffer is a sub-buffer object and
	 * offset specified when the sub-buffer object is created is not aligned
	 * to CL_DEVICE_MEM_BASE_ADDR_ALIGN value for device associated with queue.
	 */

	clReleaseMemObject(device_buffer);
	return result;
#else
	return PIGLIT_SKIP;
#endif
}
