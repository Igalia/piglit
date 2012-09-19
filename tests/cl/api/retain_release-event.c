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
 * @file retain_release-mem-object.c
 *
 * Test API functions:
 *
 *   cl_int clRetainEvent (cl_mem event)
 *   cl_int clReleaseEvent (cl_mem event)
 */

#include "piglit-framework-cl-api.h"


PIGLIT_CL_API_TEST_CONFIG_BEGIN

	config.name = "clRetainEvent and clReleaseEvent";
	config.version_min = 10;

	config.run_per_device = true;
	config.create_context = true;

PIGLIT_CL_API_TEST_CONFIG_END


enum piglit_result
piglit_cl_test(const int argc,
               const char** argv,
               const struct piglit_cl_api_test_config* config,
               const struct piglit_cl_api_test_env* env)
{
	int ref_count = 0;
	const int max_ref_count = 10;
	cl_int errNo;
	cl_uint* ref_count_ptr;
	cl_mem memobj;
	cl_event event;
	unsigned char buffer[1];

	/*** Normal usage ***/

	memobj = clCreateBuffer(env->context->cl_ctx,
	                        CL_MEM_READ_WRITE,
	                        512,
	                        NULL,
	                        &errNo);
	if(!piglit_cl_check_error(errNo, CL_SUCCESS)) {
		fprintf(stderr,
		        "Failed (error code: %s): Create buffer.\n",
		        piglit_cl_get_error_name(errNo));
		return PIGLIT_FAIL;
	}

	errNo = clEnqueueReadBuffer(env->context->command_queues[0], memobj, true,
	                            0, 1, buffer, 0, NULL, &event);
	if(!piglit_cl_check_error(errNo, CL_SUCCESS)) {
		fprintf(stderr,
		        "Failed (error code: %s): Create event by enqueueing buffer read.\n",
		        piglit_cl_get_error_name(errNo));
		return PIGLIT_FAIL;
	}

	ref_count_ptr = piglit_cl_get_event_info(event, CL_EVENT_REFERENCE_COUNT);
	if(*ref_count_ptr != 1) {
		free(ref_count_ptr);
		fprintf(stderr,
		        "CL_EVENT_REFERENCE_COUNT should be 1 after creating event.\n");
		return PIGLIT_FAIL;
	}
	free(ref_count_ptr);

	/* increase by two and decrease by one on each iteration */
	for(ref_count = 1; ref_count < max_ref_count; ref_count++) {
		errNo = clRetainEvent(event);
		if(!piglit_cl_check_error(errNo, CL_SUCCESS)) {
			fprintf(stderr,
			        "clRetainEvent: Failed (error code: %s): Retain event.\n",
			        piglit_cl_get_error_name(errNo));
			return PIGLIT_FAIL;
		}
		errNo = clReleaseEvent(event);
		if(!piglit_cl_check_error(errNo, CL_SUCCESS)){
			fprintf(stderr,
			        "clReleaseEvent: Failed (error code: %s): Release event.\n",
			        piglit_cl_get_error_name(errNo));
			return PIGLIT_FAIL;
		}
		errNo = clRetainEvent(event);
		if(!piglit_cl_check_error(errNo, CL_SUCCESS)){
			fprintf(stderr,
			        "clRetainEvent: Failed (error code: %s): Retain event.\n",
			        piglit_cl_get_error_name(errNo));
			return PIGLIT_FAIL;
		}

		/* check internal value of reference count */
		ref_count_ptr =
			piglit_cl_get_event_info(event, CL_EVENT_REFERENCE_COUNT);
		if(*ref_count_ptr != (ref_count+1)) {
			free(ref_count_ptr);
			fprintf(stderr,
			        "CL_EVENT_REFERENCE_COUNT is not changing accordingly.\n");
			return PIGLIT_FAIL;
		}
		free(ref_count_ptr);
	}
	/* Decrease reference count to 0 */
	for(ref_count = max_ref_count; ref_count > 0; ref_count--) {
		errNo = clReleaseEvent(event);
		if(!piglit_cl_check_error(errNo, CL_SUCCESS)){
			fprintf(stderr,
			        "clReleaseEvent: Failed (error code: %s): Release event.\n",
			        piglit_cl_get_error_name(errNo));
			return PIGLIT_FAIL;
		}

		/* check internal value of reference count */
		if(ref_count > 1) {
			ref_count_ptr =
				piglit_cl_get_event_info(event, CL_EVENT_REFERENCE_COUNT);
			if(*ref_count_ptr != (ref_count-1)) {
				free(ref_count_ptr);
				fprintf(stderr,
				        "CL_EVENT_REFERENCE_COUNT is not changing accordingly.\n");
				return PIGLIT_FAIL;
			}
			free(ref_count_ptr);
		}
	}

	/*** Errors ***/

	/*
	 * CL_INVALID_EVENT if event is not a valid event object.
	 */
	errNo = clReleaseEvent(event);
	if(!piglit_cl_check_error(errNo, CL_INVALID_EVENT)) {
			fprintf(stderr,
			        "clReleaseEvent: Failed (error code: %s): Trigger CL_INVALID_EVENT if event is not a valid event object (already released).\n",
			        piglit_cl_get_error_name(errNo));
		return PIGLIT_FAIL;
	}
	errNo = clReleaseEvent(NULL);
	if(!piglit_cl_check_error(errNo, CL_INVALID_EVENT)) {
			fprintf(stderr,
			        "clReleaseEvent: Failed (error code: %s): Trigger CL_INVALID_EVENT if event is not a valid event object (NULL).\n",
			        piglit_cl_get_error_name(errNo));
		return PIGLIT_FAIL;
	}

	clReleaseMemObject(memobj);

	return PIGLIT_PASS;
}
