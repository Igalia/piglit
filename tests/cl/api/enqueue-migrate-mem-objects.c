/*
 * Copyright © 2015 Serge Martin <edb+piglit@sigluy.net>
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
 * @file enqueue-migrate-mem-objects.c
 *
 * Test API function:
 *
 *   cl_int
 *   clEnqueueMigrateMemObjects(cl_command_queue command_queue,
 *                              cl_uint num_mem_objects,
 *                              const cl_mem *mem_objects,
 *                              cl_mem_migration_flags flags,
 *                              cl_uint num_events_in_wait_list,
 *                              const cl_event *event_wait_list,
 *                              cl_event *event)
 */

#include "piglit-framework-cl-api.h"


PIGLIT_CL_API_TEST_CONFIG_BEGIN

	config.name = "clEnqueueMigrateMemObjects";
	config.version_min = 12;

	config.run_per_platform = true;
	config.create_context = true;

PIGLIT_CL_API_TEST_CONFIG_END


#if defined(CL_VERSION_1_2)
static bool
test(cl_command_queue queue, cl_uint num_mem_objects,
     const cl_mem *mem_objects, cl_mem_migration_flags flags,
     cl_uint num_events_in_wait_list, const cl_event *event_wait_list,
     cl_event *event,
     cl_int expected_error, enum piglit_result* result,
     const char* test_str) {
	cl_int errNo;

	errNo = clEnqueueMigrateMemObjects(queue, num_mem_objects,
	                                   mem_objects, flags,
	                                   num_events_in_wait_list,
	                                   event_wait_list,
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
	cl_mem device_buffer;
	cl_mem invalid_buffer = NULL;
	cl_command_queue queue;
	cl_event event;
	int i;

/*** Normal usage ***/
	device_buffer = piglit_cl_create_buffer(env->context,
	                                        CL_MEM_READ_WRITE|
	                                        CL_MEM_ALLOC_HOST_PTR, 32);

	for(i = 0; i < env->context->num_devices; ++i) {
		fprintf(stderr, "Testing on device %d\n", i);
		queue = env->context->command_queues[i];
		if (!test(queue, 1, &device_buffer, 0,
		          0, NULL, NULL,
		          CL_SUCCESS, &result, "Migrating the buffer")) {
			return PIGLIT_FAIL;
		}

		if (!test(queue, 1, &device_buffer,
		          CL_MIGRATE_MEM_OBJECT_HOST|
		          CL_MIGRATE_MEM_OBJECT_CONTENT_UNDEFINED,
		          0, NULL, NULL,
		          CL_SUCCESS, &result,
		          "Migrating the buffer with flags CL_MIGRATE_MEM_OBJECT_HOST|CL_MIGRATE_MEM_OBJECT_CONTENT_UNDEFINED")) {
			return PIGLIT_FAIL;
		}

		if (!test(queue, 1, &device_buffer,
		          CL_MIGRATE_MEM_OBJECT_CONTENT_UNDEFINED,
		          0, NULL, NULL,
		          CL_SUCCESS, &result,
		          "Migrating the buffer with flag CL_MIGRATE_MEM_OBJECT_CONTENT_UNDEFINED")) {
			return PIGLIT_FAIL;
		}

		if (!test(queue, 1, &device_buffer,
		          CL_MIGRATE_MEM_OBJECT_HOST,
		          0, NULL, NULL,
		          CL_SUCCESS, &result,
		          "Migrating the buffer with flag CL_MIGRATE_MEM_OBJECT_HOST")) {
			return PIGLIT_FAIL;
		}
	}

/*** Errors ***/

	/*
	 * CL_INVALID_COMMAND_QUEUE if command_queue is not a valid command-queue.
	 */
	test(NULL, 1, &device_buffer, 0,
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
			test(context->command_queues[0], 1, &device_buffer, 0,
			     0, NULL, NULL,
			     CL_INVALID_CONTEXT, &result,
			     "CL_INVALID_CONTEXT if the context associated with command_queue and buffer are not the same");

			event = clCreateUserEvent(context->cl_ctx, &err);
			if (err == CL_SUCCESS) {
				err = clSetUserEventStatus(event, CL_COMPLETE);
				if (err == CL_SUCCESS) {
					test(queue, 1, &device_buffer, 0,
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
	 * CL_INVALID_MEM_OBJECT if any of the memory objects in mem_objs is not a
	 * valid memory object.
	 */
	test(queue, 1, &invalid_buffer, 0,
	     0, NULL, NULL,
	     CL_INVALID_MEM_OBJECT, &result,
	     "CL_INVALID_MEM_OBJECT if any of the memory objects in mem_objs is not a valid memory object");

	/*
	 * CL_INVALID_VALUE if num_mem_objects is zero or if mem_objects is NULL.
	 */
	test(queue, 0, &device_buffer, 0,
	     0, NULL, NULL,
	     CL_INVALID_VALUE, &result,
	     "CL_INVALID_VALUE if num_mem_objects is zero");

	test(queue, 1, NULL, 0,
	     0, NULL, NULL,
	     CL_INVALID_VALUE, &result,
	     "CL_INVALID_VALUE if num_mem_objects is NULL");

	/*
	 * CL_INVALID_VALUE if flags is not 0 or is not any of the values
	 * described in the table above.
	 */
	test(queue, 1, &device_buffer, -1,
	     0, NULL, NULL,
	     CL_INVALID_VALUE, &result,
	     "CL_INVALID_VALUE if flags is not 0 or is not any of the values described in the table above");

	/*
	 * CL_INVALID_EVENT_WAIT_LIST if event_wait_list is NULL and
	 * num_events_in_wait_list > 0, or event_wait_list is not NULL and
	 * num_events_in_wait_list is 0, or if event objects in event_wait_list
	 * are not valid events.
	 */
	event = NULL;
	test(queue, 1, &device_buffer, 0,
	     1, NULL, NULL,
	     CL_INVALID_EVENT_WAIT_LIST, &result,
	     "CL_INVALID_EVENT_WAIT_LIST if event_wait_list is NULL and num_events_in_wait_list > 0");

	test(queue, 1, &device_buffer, 0,
	     0, &event, 0,
	     CL_INVALID_EVENT_WAIT_LIST, &result,
	     "CL_INVALID_EVENT_WAIT_LIST if event_wait_list is not NULL and num_events_in_wait_list is 0");

	test(queue, 1, &device_buffer, 0,
	     1, &event, 0,
	     CL_INVALID_EVENT_WAIT_LIST, &result,
	     "CL_INVALID_EVENT_WAIT_LIST if event objects in event_wait_list are not valid events");


	clReleaseMemObject(device_buffer);
	return result;
#else
	return PIGLIT_SKIP;
#endif
}
