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
 * @file enqueue-fill-image.c
 *
 * Test API function:
 *
 *   cl_int
 *   clEnqueueFillImage(cl_command_queue command_queue, cl_mem image,
 *                      const void *fill_color, size_t *origin, size_t *region
 *                      cl_uint num_events_in_wait_list,
 *                      const cl_event *event_wait_list,
 *                      cl_event *event )
 */

#include "piglit-framework-cl-api.h"


PIGLIT_CL_API_TEST_CONFIG_BEGIN

	config.name = "clEnqueueFillImage";
	config.version_min = 12;

	config.run_per_device = true;
	config.create_context = true;

PIGLIT_CL_API_TEST_CONFIG_END


#if defined(CL_VERSION_1_2)
static bool
test(cl_command_queue queue, cl_mem image,
     const void *fill_color, size_t *origin, size_t *region,
     cl_uint num_events_in_wait_list,
     const cl_event *event_wait_list,
     cl_event *event,
     cl_int expected_error, enum piglit_result* result,
     const char* test_str) {
	cl_int errNo;

	errNo = clEnqueueFillImage(queue, image,
	                           fill_color, origin, region,
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
	cl_int err;

#define IMG_WIDTH 4
#define IMG_HEIGHT 4
#define IMG_DATA_SIZE 4
#define IMG_BUFFER_SIZE IMG_WIDTH * IMG_HEIGHT * IMG_DATA_SIZE

	unsigned char img_buf[IMG_BUFFER_SIZE] = {0};
	unsigned char dst_buf[IMG_BUFFER_SIZE] = {0};
	unsigned char exp_buf[IMG_BUFFER_SIZE] = {0};
	int pattern[4] = {129, 33, 77, 255};
	size_t origin[3] = {0, 0, 0};
	size_t region[3] = {2, 2, 1};
	size_t tmp;
	cl_event event;
	cl_mem image;
	cl_image_format img_format;
	cl_image_desc img_desc = {0};
	cl_command_queue queue = env->context->command_queues[0];
	int i;

	cl_bool *image_support =
		piglit_cl_get_device_info(env->context->device_ids[0],
		                          CL_DEVICE_IMAGE_SUPPORT);

	if (!*image_support) {
		fprintf(stderr, "No image support\n");
		free(image_support);
		return PIGLIT_SKIP;
	}

	img_format.image_channel_order = CL_RGBA;
	img_format.image_channel_data_type = CL_UNSIGNED_INT8;
	img_desc.image_type = CL_MEM_OBJECT_IMAGE2D;
	img_desc.image_width = IMG_WIDTH;
	img_desc.image_height = IMG_HEIGHT;
	img_desc.buffer = NULL;

/*** Normal usage ***/
	image = clCreateImage(env->context->cl_ctx,
	                      CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
	                      &img_format, &img_desc, &img_buf, &err);

	if(!piglit_cl_check_error(err, CL_SUCCESS)) {
		fprintf(stderr, "Failed (error code: %s): Creating an image\n",
		        piglit_cl_get_error_name(err));
		return PIGLIT_FAIL;
	}

	if (!test(queue, image, pattern, origin, region,
	          0, NULL, NULL,
	          CL_SUCCESS, &result, "Enqueuing the image to be filled")) {
		return PIGLIT_FAIL;
	}

	region[0] = IMG_WIDTH;
	region[1] = IMG_HEIGHT;
	err = clEnqueueReadImage(queue, image, 1, origin, region, 0, 0,
	                         dst_buf, 0, NULL, NULL);
	if(!piglit_cl_check_error(err, CL_SUCCESS)) {
		fprintf(stderr, "Failed (error code: %s): Reading image\n",
		        piglit_cl_get_error_name(err));
		return PIGLIT_FAIL;
	}

	/*
	 * fill the host buffer with the pattern
	 * for exemple : pattern == 1234
	 *
	 * 12341234abcdabcd
	 * 12341234abcdabcd
	 * abcdabcdabcdabcd
	 * abcdabcdabcdabcd
	 */
	exp_buf[0] = pattern[0];
	exp_buf[1] = pattern[1];
	exp_buf[2] = pattern[2];
	exp_buf[3] = pattern[3];
	memcpy(exp_buf + (IMG_DATA_SIZE * 1), exp_buf, IMG_DATA_SIZE);
	memcpy(exp_buf + (IMG_DATA_SIZE * 4), exp_buf, IMG_DATA_SIZE);
	memcpy(exp_buf + (IMG_DATA_SIZE * 5), exp_buf, IMG_DATA_SIZE);

	for (i = 0; i < sizeof(dst_buf) / sizeof(dst_buf[0]); ++i) {
		if (!piglit_cl_probe_integer(dst_buf[i], exp_buf[i], 0)) {
			fprintf(stderr, "Error at %d: got %d, expected %d\n",
			        i, dst_buf[i], exp_buf[i]);
			return PIGLIT_FAIL;
		}
	}

/*** Errors ***/

	/*
	 * CL_INVALID_COMMAND_QUEUE if command_queue is not a valid command-queue.
	 */
	test(NULL, image, pattern, origin, region,
		  0, NULL, NULL,
		  CL_INVALID_COMMAND_QUEUE, &result,
		  "CL_INVALID_COMMAND_QUEUE if command_queue is not a valid command-queue");

	/*
	 * CL_INVALID_CONTEXT if the context associated with command_queue and
	 * image are not the same or if the context associated with command_queue
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
					test(context->command_queues[0], image, pattern, origin, region,
					     0, NULL, NULL,
					     CL_INVALID_CONTEXT, &result,
					     "CL_INVALID_CONTEXT if the context associated with command_queue and image are not the same");

					test(queue, image, pattern, origin, region,
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
	 * CL_INVALID_MEM_OBJECT if image is not a valid buffer object.
	 */
	test(queue, NULL, pattern, origin, region,
	     0, NULL, NULL,
	     CL_INVALID_MEM_OBJECT, &result,
	     "CL_INVALID_MEM_OBJECT if image is not a valid buffer object");

	/*
	 * CL_INVALID_VALUE if fill_color is NULL.
	 */
	test(queue, image, NULL, origin, region,
	     0, NULL, NULL,
	     CL_INVALID_VALUE, &result,
	     "CL_INVALID_VALUE if fill_color is NULL");

	/*
	 * CL_INVALID_VALUE if the region being written specified by origin and
	 * region is out of bounds or if ptr is a NULL value.
	 */
	tmp = origin[0];
	origin[0] = IMG_WIDTH + 1;
	test(queue, image, pattern, origin, region,
	     0, NULL, NULL,
	     CL_INVALID_VALUE, &result,
	     "CL_INVALID_VALUE if the region being written specified by origin and region is out of bounds (origin)");
	origin[0] = tmp;

	tmp = region[0];
	region[0] = IMG_WIDTH + 1;
	test(queue, image, pattern, origin, region,
	     0, NULL, NULL,
	     CL_INVALID_VALUE, &result,
	     "CL_INVALID_VALUE if the region being written specified by origin and region is out of bounds (region)");
	region[0] = tmp;

	test(queue, image, pattern, NULL, region,
	     0, NULL, NULL,
	     CL_INVALID_VALUE, &result,
	     "CL_INVALID_VALUE if ptr is a NULL value (origin)");

	test(queue, image, pattern, origin, NULL,
	     0, NULL, NULL,
	     CL_INVALID_VALUE, &result,
	     "CL_INVALID_VALUE if ptr is a NULL value (region)");

	/*
	 * CL_INVALID_VALUE if values in origin and region do not follow rules
	 * described in the argument description for origin and region.
	 */
	tmp = origin[2];
	origin[2] = 1;
	test(queue, image, pattern, origin, region,
	     0, NULL, NULL,
	     CL_INVALID_VALUE, &result,
	     "CL_INVALID_VALUE if values in origin do not follow rules described in the argument description for origin");
	origin[2] = tmp;

	tmp = region[2];
	region[2] = 0;
	test(queue, image, pattern, origin, region,
		  0, NULL, NULL,
		CL_INVALID_VALUE, &result,
		"CL_INVALID_VALUE if values in region do not follow rules described in the argument description for region");
	region[2] = tmp;

	/*
	 * CL_INVALID_EVENT_WAIT_LIST if event_wait_list is NULL and
	 * num_events_in_wait_list > 0, or event_wait_list is not NULL and
	 * num_events_in_wait_list is 0, or if event objects in event_wait_list
	 * are not valid events.
	 */
	event = NULL;
	test(queue, image, pattern, origin, region,
	     1, NULL, NULL,
	     CL_INVALID_EVENT_WAIT_LIST, &result,
	     "CL_INVALID_EVENT_WAIT_LIST if event_wait_list is NULL and num_events_in_wait_list > 0");

	test(queue, image, pattern, origin, region,
	     0, &event, NULL,
	     CL_INVALID_EVENT_WAIT_LIST, &result,
	     "CL_INVALID_EVENT_WAIT_LIST if event_wait_list is not NULL and num_events_in_wait_list is 0");

	test(queue, image, pattern, origin, region,
	     1, &event, NULL,
	     CL_INVALID_EVENT_WAIT_LIST, &result,
	     "CL_INVALID_EVENT_WAIT_LIST if event objects in event_wait_list are not valid events");

	/*
	 * CL_INVALID_IMAGE_SIZE if image dimensions (image width, height, specified
	 * or compute row and/or slice pitch) for image are not supported by device
	 * associated with queue.
	 */
	/* This is a per device test, clCreateImage would have failed before */

	/*
	 * CL_INVALID_IMAGE_FORMAT if image format (image channel order and data type)
	 * for image are not supported by device associated with queue.
	 */
	/* This is a per device test, clCreateImage would have failed before */

	free(image_support);
	clReleaseMemObject(image);
	return result;
#else
	return PIGLIT_SKIP;
#endif
}
