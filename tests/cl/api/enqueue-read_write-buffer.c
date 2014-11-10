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
 * @file enqueue-read_write-buffer.c
 *
 * Test API functions:
 *
 *   cl_int clEnqueueReadBuffer (cl_command_queue command_queue,
 *                               cl_mem buffer,
 *                               cl_bool blocking_read,
 *                               size_t offset,
 *                               size_t cb,
 *                               void *ptr,
 *                               cl_uint num_events_in_wait_list,
 *                               const cl_event *event_wait_list,
 *                               cl_event *event)
 *   cl_int clEnqueueWriteBuffer (cl_command_queue command_queue,
 *                                cl_mem buffer,
 *                                cl_bool blocking_write,
 *                                size_t offset,
 *                                size_t cb,
 *                                const void *ptr,
 *                                cl_uint num_events_in_wait_list,
 *                                const cl_event *event_wait_list,
 *                                cl_event *event)
 */

#include "piglit-framework-cl-api.h"


PIGLIT_CL_API_TEST_CONFIG_BEGIN

	config.name = "clEnqueueReadBuffer and clEnqueueWriteBuffer";
	config.version_min = 10;

	config.run_per_platform = true;
	config.create_context = true;

PIGLIT_CL_API_TEST_CONFIG_END


#define BUFFER_SIZE 512

static bool
test_read(cl_command_queue command_queue,
          cl_mem buffer,
          cl_bool blocking_read,
          size_t offset,
          size_t cb,
          void *ptr,
          cl_uint num_events_in_wait_list,
          const cl_event *event_wait_list,
          cl_event *event,
          cl_int expected_error,
          enum piglit_result* result,
          const char* test_str) {
	cl_int errNo;

	errNo = clEnqueueReadBuffer(command_queue,
	                            buffer,
	                            blocking_read,
	                            offset,
	                            cb,
	                            ptr,
	                            num_events_in_wait_list,
	                            event_wait_list,
	                            event);

	if(!piglit_cl_check_error(errNo, expected_error)) {
		fprintf(stderr, "clEnqueueReadBuffer: Failed (error code: %s): %s.\n",
		        piglit_cl_get_error_name(errNo), test_str);
		piglit_merge_result(result, PIGLIT_FAIL);
		return false;
	}

	return true;
}

static bool
test_write(cl_command_queue command_queue,
           cl_mem buffer,
           cl_bool blocking_write,
           size_t offset,
           size_t cb,
           const void *ptr,
           cl_uint num_events_in_wait_list,
           const cl_event *event_wait_list,
           cl_event *event,
           cl_int expected_error,
           enum piglit_result* result,
           const char* test_str) {
	cl_int errNo;

	errNo = clEnqueueWriteBuffer(command_queue,
	                             buffer,
	                             blocking_write,
	                             offset,
	                             cb,
	                             ptr,
	                             num_events_in_wait_list,
	                             event_wait_list,
	                             event);

	if(!piglit_cl_check_error(errNo, expected_error)) {
		fprintf(stderr, "clEnqueueWriteBuffer: Failed (error code: %s): %s.\n",
		        piglit_cl_get_error_name(errNo), test_str);
		piglit_merge_result(result, PIGLIT_FAIL);
		return false;
	}

	return true;
}

/*
 * @param mask  Defines which fields to use
 */
static cl_mem_flags
get_mixed_mem_flags(int mask, const cl_mem_flags mem_flags[]) {
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
                const cl_mem_flags mutexes[]) {
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
	char test_str_read[1024];
	char test_str_write[1024];

	int i;
	int mask;
	piglit_cl_context context;
	cl_ulong alloc_size = BUFFER_SIZE;
	unsigned char host_buffer_init[BUFFER_SIZE];
	unsigned char host_buffer_read[BUFFER_SIZE];
	unsigned char host_buffer_write[BUFFER_SIZE];
	cl_mem mem;
	cl_mem_flags mixed_mem_flags;

	cl_event valid_event;
	cl_event invalid_event = NULL;

	int num_mem_flags = PIGLIT_CL_ENUM_NUM(cl_mem_flags, env->version);
	const cl_mem_flags* mem_flags = PIGLIT_CL_ENUM_ARRAY(cl_mem_flags);

	int num_mutexes = PIGLIT_CL_ENUM_NUM(cl_mem_flags_mutexes, env->version);
	const cl_mem_flags* mutexes = PIGLIT_CL_ENUM_ARRAY(cl_mem_flags_mutexes);

	/*** Normal usage ***/

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
		
		sprintf(test_str_read,
		        "Read from buffer using 0x%X as memory flags",
		        (unsigned int)mixed_mem_flags);
		sprintf(test_str_write,
		        "Write to buffer using 0x%X as memory flags",
		        (unsigned int)mixed_mem_flags);

		if(   (mixed_mem_flags & CL_MEM_USE_HOST_PTR)
		   || (mixed_mem_flags & CL_MEM_COPY_HOST_PTR)) {
			mem = clCreateBuffer(env->context->cl_ctx,
			                     mixed_mem_flags,
			                     alloc_size,
			                     host_buffer_init,
			                     NULL);
		} else {
			mem = clCreateBuffer(env->context->cl_ctx,
			                     mixed_mem_flags,
			                     alloc_size,
			                     NULL,
			                     NULL);
		}

		for(i = 0; i < env->context->num_devices; i++) {
			bool break_loop = false;
			int offset, cb;
			int step = BUFFER_SIZE/4;

			char* device_name =
				piglit_cl_get_device_info(env->context->device_ids[i],
				                          CL_DEVICE_NAME);

			for(offset = 0; offset < BUFFER_SIZE; offset += step) {
				for(cb = step; cb <= BUFFER_SIZE-offset; cb += step) {
					int j;

					for(j = offset; j < cb; j++) {
						host_buffer_write[j] += 1;
					}

#if defined(CL_VERSION_1_2)
					if(!(   mixed_mem_flags & CL_MEM_HOST_READ_ONLY
					     || mixed_mem_flags & CL_MEM_HOST_NO_ACCESS))
#endif //CL_VERSION_1_2
					if(!test_write(env->context->command_queues[i],
					               mem,
					               true,
					               offset,
					               cb,
					               &host_buffer_write[offset],
					               0,
					               NULL,
					               NULL,
					               CL_SUCCESS, &result, test_str_write)) {
						printf("  Device: %s\n    mem_flags: 0x%x, offset: %d, bytes: %d \n",
						       device_name,
						       (unsigned int)mixed_mem_flags,
						       offset,
						       cb);
						printf("    Could not write to buffer.\n");
						break_loop = true;
						break;
					}

#if defined(CL_VERSION_1_2)
					if(!(   mixed_mem_flags & CL_MEM_HOST_WRITE_ONLY
					     || mixed_mem_flags & CL_MEM_HOST_NO_ACCESS))
#endif //CL_VERSION_1_2
					if(!test_read(env->context->command_queues[i],
					              mem,
					              true,
					              offset,
					              cb,
					              &host_buffer_read[offset],
					              0,
					              NULL,
					              NULL,
					              CL_SUCCESS, &result, test_str_read)) {
						printf("  Device: %s\n    mem_flags: 0x%x, offset: %d, bytes: %d \n",
						       device_name,
						       (unsigned int)mixed_mem_flags,
						       offset,
						       cb);
						printf("    Could not read from buffer.\n");
						break_loop = true;
						break;
					}

#if defined(CL_VERSION_1_2)
					/* skip if we didn't write and read */
					if(   mixed_mem_flags & CL_MEM_HOST_WRITE_ONLY
					   || mixed_mem_flags & CL_MEM_HOST_READ_ONLY
					   || mixed_mem_flags & CL_MEM_HOST_NO_ACCESS)
						continue;
#endif //CL_VERSION_1_2

					for(j = offset; j < cb; j++) {
						if(host_buffer_read[j] != host_buffer_write[j]) {
							int k;

							sprintf(test_str_read,
							        "Data read from buffer is not the same as data written to buffer using 0x%X as memory flags",
							        (unsigned int)mixed_mem_flags);
							fprintf(stderr, "%s", test_str_read);

							printf("  Device: %s\n    mem_flags: 0x%x, offset: %d, bytes: %d \n",
							       device_name,
							       (unsigned int)mixed_mem_flags,
							       offset,
							       cb);

							printf("    Data written:");
							for(k = offset; k < cb; k++) {
								printf(" 0x%x", host_buffer_write[k]);
							}
							printf("\n");
							printf("    Data read:   ");
							for(k = offset; k < cb; k++) {
								printf(" 0x%x", host_buffer_read[k]);
							}
							printf("\n");

							break_loop = true;
							piglit_merge_result(&result, PIGLIT_FAIL);
							break;
						}
					}

					if(break_loop) break;
				}
				
				if(break_loop) break;
			}
			
			free(device_name);
		}

		clReleaseMemObject(mem);
	}

	/*** Errors ***/

	/* create buffer */
	mem = clCreateBuffer(env->context->cl_ctx,
	                     CL_MEM_READ_WRITE,
	                     alloc_size,
	                     NULL,
	                     NULL);
	
	/*
	 * CL_INVALID_COMMAND_QUEUE if command_queue is not a valid command-queue.
	 */
	test_write(NULL, mem, true, 0, BUFFER_SIZE, host_buffer_write,
	           0, NULL, NULL,
	           CL_INVALID_COMMAND_QUEUE, &result,
	           "Trigger CL_INVALID_COMMAND_QUEUE when command_queue is not a valid command-queue");
	test_read(NULL, mem, true, 0, BUFFER_SIZE, host_buffer_read, 0, NULL, NULL,
	          CL_INVALID_COMMAND_QUEUE, &result,
	          "Trigger CL_INVALID_COMMAND_QUEUE when command_queue is not a valid command-queue");
	
	/*
	 * CL_INVALID_CONTEXT if the context associated with command_queue and
	 * buffer are not the same or if the context associated with command_queue
	 * and events in event_wait_list are not the same.
	 *
	 * TODO: events
	 */
	context = piglit_cl_create_context(env->platform_id,
	                                   env->context->device_ids,
	                                   1);
	if(context != NULL){
		test_write(context->command_queues[0], mem, true, 0,
		           BUFFER_SIZE, host_buffer_write,
		           0, NULL, NULL,
		           CL_INVALID_CONTEXT, &result,
		           "Trigger CL_INVALID_CONTEXT when context associated with command_queue and buffer are not the same");
		test_read(context->command_queues[0], mem, true, 0,
		          BUFFER_SIZE, host_buffer_read,
		          0, NULL, NULL,
		          CL_INVALID_CONTEXT, &result,
		          "Trigger CL_INVALID_CONTEXT when context associated with command_queue and buffer are not the same");

		piglit_cl_release_context(context);
	} else {
		fprintf(stderr, "Could not test triggering CL_INVALID_CONTEXT.\n");
		piglit_merge_result(&result, PIGLIT_FAIL);
	}

	/*
	 * CL_INVALID_MEM_OBJECT if buffer is not a valid buffer object.
	 */
	test_write(env->context->command_queues[0], NULL, true, 0,
	           BUFFER_SIZE, host_buffer_write,
	           0, NULL, NULL,
	           CL_INVALID_MEM_OBJECT, &result,
	           "Trigger CL_INVALID_MEM_OBJECT when buffer is not a valid buffer object");
	test_read(env->context->command_queues[0], NULL, true, 0,
	          BUFFER_SIZE, host_buffer_read,
	          0, NULL, NULL,
	          CL_INVALID_MEM_OBJECT, &result,
	          "Trigger CL_INVALID_MEM_OBJECT when buffer is not a valid buffer object");

	/*
	 * CL_INVALID_VALUE if the region being read specified by (offset, cb)
	 * is out of bounds or if ptr is a NULL value.
	 */
	test_write(env->context->command_queues[0], mem, true, 0,
	           BUFFER_SIZE+1, host_buffer_write,
	           0, NULL, NULL,
	           CL_INVALID_VALUE, &result,
	           "Trigger CL_INVALID_VALUE when the region being read spcified by (offset, cb) is out of bounds");
	test_read(env->context->command_queues[0], mem, true, 0,
	          BUFFER_SIZE+1, host_buffer_read,
	          0, NULL, NULL,
	          CL_INVALID_VALUE, &result,
	          "Trigger CL_INVALID_VALUE when the region being read spcified by (offset, cb) is out of bounds");
	test_write(env->context->command_queues[0], mem, true, 1,
	           BUFFER_SIZE, host_buffer_write,
	           0, NULL, NULL,
	           CL_INVALID_VALUE, &result,
	           "Trigger CL_INVALID_VALUE when the region being read spcified by (offset, cb) is out of bounds");
	test_read(env->context->command_queues[0], mem, true, 1,
	          BUFFER_SIZE, host_buffer_read,
	          0, NULL, NULL,
	          CL_INVALID_VALUE, &result,
	          "Trigger CL_INVALID_VALUE when the region being read spcified by (offset, cb) is out of bounds");
	test_write(env->context->command_queues[0], mem, true, 0,
	           BUFFER_SIZE, NULL,
	           0, NULL, NULL,
	           CL_INVALID_VALUE, &result,
	           "Trigger CL_INVALID_VALUE when ptr is NULL value");
	test_read(env->context->command_queues[0], mem, true, 0, BUFFER_SIZE, NULL,
	          0, NULL, NULL,
	          CL_INVALID_VALUE, &result,
	          "Trigger CL_INVALID_VALUE when ptr is NULL value");
	
	/*
	 * CL_INVALID_EVENT_WAIT_LIST if event_wait_list is NULL and
	 * num_events_in_wait_list greater than 0, or event_wait_list is
	 * not NULL and num_events_in_wait_list is 0, or if event objects
	 * in event_wait_list are not valid events.
	 */
	
	/* create a valid event */
	test_write(env->context->command_queues[0], mem, true, 0,
	           BUFFER_SIZE, host_buffer_write,
	           0, NULL, &valid_event,
	           CL_SUCCESS, &result,
	           "Create an event");
	
	test_write(env->context->command_queues[0], mem, true, 0,
	           BUFFER_SIZE, host_buffer_write,
	           1, NULL, NULL,
	           CL_INVALID_EVENT_WAIT_LIST, &result,
	           "Trigger CL_INVALID_EVENT_WAIT_LIST when event_wait_list is NULL and num_events_in_wait_list is greater than 0");
	test_read(env->context->command_queues[0], mem, true, 0,
	          BUFFER_SIZE, host_buffer_read,
	          1, NULL, NULL,
	          CL_INVALID_EVENT_WAIT_LIST, &result,
	          "Trigger CL_INVALID_EVENT_WAIT_LIST when event_wait_list is NULL and num_events_in_wait_list is greater than 0");
	test_write(env->context->command_queues[0], mem, true, 0,
	           BUFFER_SIZE, host_buffer_write,
	           0, &valid_event, NULL,
	           CL_INVALID_EVENT_WAIT_LIST, &result,
	           "Trigger CL_INVALID_EVENT_WAIT_LIST when event_wait_list is not NULL and num_events_in_wait_list is 0");
	test_read(env->context->command_queues[0], mem, true, 0,
	          BUFFER_SIZE, host_buffer_read,
	          0, &valid_event, NULL,
	          CL_INVALID_EVENT_WAIT_LIST, &result,
	          "Trigger CL_INVALID_EVENT_WAIT_LIST when event_wait_list is not NULL and num_events_in_wait_list is 0");
	test_write(env->context->command_queues[0], mem, true, 0,
	           BUFFER_SIZE, host_buffer_write,
	           1, &invalid_event, NULL,
	           CL_INVALID_EVENT_WAIT_LIST, &result,
	           "Trigger CL_INVALID_EVENT_WAIT_LIST when event objects in event_wait_list are not valid events");
	test_read(env->context->command_queues[0], mem, true, 0,
	          BUFFER_SIZE, host_buffer_read,
	          1, &invalid_event, NULL,
	          CL_INVALID_EVENT_WAIT_LIST, &result,
	          "Trigger CL_INVALID_EVENT_WAIT_LIST when event objects in event_wait_list are not valid events");

	clReleaseEvent(valid_event);

	/*
	 * CL_MISALIGNED_SUB_BUFFER_OFFSET if buffer is a sub-buffer
	 * object and offset specified when the sub-buffer object is
	 * created is not aligned to CL_DEVICE_MEM_BASE_ADDR_ALIGN
	 * value for device associated with queue.
	 *
	 * Version: 1.1
	 * TODO
	 */
	/*
	 * CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST if the read
	 * and write operations are blocking and the execution status
	 * of any of the events in event_wait_list is a negative
	 * integer value.
	 *
	 * Version: 1.1
	 * TODO
	 */

	clReleaseMemObject(mem);

#if defined(CL_VERSION_1_2)
	/*
	 * CL_INVALID_OPERATION if clEnqueueReadBuffer is called on
	 * buffer which has been created with CL_MEM_HOST_WRITE_ONLY
	 * or CL_MEM_HOST_NO_ACCESS.
	 *
	 * CL_INVALID_OPERATION if clEnqueueWriteBuffer is called on
	 * buffer which has been created with CL_MEM_HOST_READ_ONLY
	 * or CL_MEM_HOST_NO_ACCESS.
	 *
	 * Version: 1.2
	 */
	if(env->version >= 12) {
		/* create host write only buffer */
		mem = clCreateBuffer(env->context->cl_ctx, CL_MEM_HOST_WRITE_ONLY, alloc_size,
		                     NULL, NULL);

		test_read(env->context->command_queues[0], mem, true, 0,
		          BUFFER_SIZE, host_buffer_read, 0, NULL, NULL,
		          CL_INVALID_OPERATION, &result,
		          "Trigger CL_INVALID_OPERATION when clEnqueueReadBuffer is called on buffer which has been created with CL_MEM_HOST_WRITE_ONLY");

		clReleaseMemObject(mem);

		/* create host no access buffer */
		mem = clCreateBuffer(env->context->cl_ctx, CL_MEM_HOST_NO_ACCESS, alloc_size,
		                     NULL, NULL);

		test_read(env->context->command_queues[0], mem, true, 0,
		          BUFFER_SIZE, host_buffer_read, 0, NULL, NULL,
		          CL_INVALID_OPERATION, &result,
		          "Trigger CL_INVALID_OPERATION when clEnqueueReadBuffer is called on buffer which has been created with CL_MEM_HOST_NO_ACCESS");

		test_write(env->context->command_queues[0], mem, true, 0,
		           BUFFER_SIZE, host_buffer_write, 0, NULL, NULL,
		           CL_INVALID_OPERATION, &result,
		           "Trigger CL_INVALID_OPERATION when clEnqueueWriteBuffer is called on buffer which has been created with CL_MEM_HOST_NO_ACCESS");

		clReleaseMemObject(mem);

		/* create host read only buffer */
		mem = clCreateBuffer(env->context->cl_ctx, CL_MEM_HOST_READ_ONLY, alloc_size,
		                     NULL, NULL);

		test_write(env->context->command_queues[0], mem, true, 0,
		           BUFFER_SIZE, host_buffer_write, 0, NULL, NULL,
		           CL_INVALID_OPERATION, &result,
		           "Trigger CL_INVALID_OPERATION when clEnqueueWriteBuffer is called on buffer which has been created with CL_MEM_HOST_READ_ONLY");

		clReleaseMemObject(mem);
	}
#endif //CL_VERSION_1_2

	return result;
}
