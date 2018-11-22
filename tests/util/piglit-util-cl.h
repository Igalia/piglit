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

#pragma once
#ifndef PIGLIT_UTIL_CL_H
#define PIGLIT_UTIL_CL_H

#include "piglit-util.h"
#include "piglit-util-cl-enum.h"


/* Define with which version of OpenCL Piglit was compiled */

#if defined(CL_VERSION_2_0)
#  define PIGLIT_CL_VERSION 20
#elif defined(CL_VERSION_1_2)
#  define PIGLIT_CL_VERSION 12
#elif defined(CL_VERSION_1_1)
#  define PIGLIT_CL_VERSION 11
#else
#  define PIGLIT_CL_VERSION 10
#endif


#ifdef __cplusplus
extern "C" {
#endif

#define DIRECT_CONVERT(to, from) \
static inline from convert_##to(from in) \
{ \
	return (to)in; \
}

DIRECT_CONVERT(cl_char, int64_t);
DIRECT_CONVERT(cl_uchar, uint64_t);
DIRECT_CONVERT(cl_short, int64_t);
DIRECT_CONVERT(cl_ushort, uint64_t);
DIRECT_CONVERT(cl_int, int64_t);
DIRECT_CONVERT(cl_uint, uint64_t);
DIRECT_CONVERT(cl_long, int64_t);
DIRECT_CONVERT(cl_ulong, uint64_t);
DIRECT_CONVERT(cl_float, double);
DIRECT_CONVERT(cl_double, double);

cl_half convert_cl_half(double in);


/* Runtime independent */

/**
 * \brief Probe integer \c value if it compares equal to \c expect with
 *        tolerance \c tolerance.
 */
bool piglit_cl_probe_integer(int64_t value, int64_t expect, uint64_t tolerance);

/**
 * \brief Probe unsigned integer \c value if it compares equal to \c expect with
 *        tolerance \c tolerance.
 */
bool piglit_cl_probe_uinteger(uint64_t value,
                              uint64_t expect,
                              uint64_t tolerance);

/**
 * \brief Probe half-floating-point \c value if it compares equal to \c expect with
 *        tolerance \c ulp.
 */
bool piglit_cl_probe_half(cl_half value, cl_half expect, uint32_t ulp);

/**
 * \brief Probe floating-point \c value if it compares equal to \c expect with
 *        tolerance \c ulp.
 */
bool piglit_cl_probe_floating(float value, float expect, uint32_t ulp);

/**
 * \brief Probe double \c value if it compares equal to \c expect with
 *        tolerance \c ulp.
 */
bool piglit_cl_probe_double(double value, double expect, uint64_t ulp);

/**
 * \brief Check for unexpected GL error and report it.
 *
 * If \c error is other than \c expected_error, then print a diagnostic.
 *
 * If you expect no error, then set \code expected_error = CL_SUCCESS \endcode.
 *
 * Returns true if \c error and \c expected_error are the same,
 * else it returns false.
 */
bool piglit_cl_check_error(cl_int error, cl_int expected_error);

/**
 * \brief Check for unexpected GL error and possibly terminate the test.
 *
 * If \c error is other than \c expected_error, then print a diagnostic
 * and terminate the test with the given \c result.
 *
 * If you expect no error, then set \code expected_error = CL_SUCCESS \endcode.
 */
void piglit_cl_expect_error(cl_int error,
                            cl_int expected_error,
                            enum piglit_result result);

/* Runtime dependent */

/* Info functions */

/**
 * \brief Get version of OpenCL API for \c platform.
 *
 * Returned version is multiplied by 10 to make it an integer. For
 * example, if the CL version is 1.1, the returned value is 11.
 */
int piglit_cl_get_platform_version(cl_platform_id platform);

/**
 * \brief Check for required OpenCL version and possibly terminate the test.
 *
 * \c required_version_times_10 must be an OpenCL version multiplied by 10.
 * For example, if the required CL version is 1.1, then the
 * \c required_version_times_10 should be 11.
 */
void piglit_cl_require_platform_version(cl_platform_id platform,
                                        int required_version_times_10);

/**
 * \brief Get version of OpenCL API for \c device.
 *
 * Returned version is multiplied by 10 to make it an integer. For
 * example, if the CL version is 1.1, the returned value is 11.
 */
int piglit_cl_get_device_version(cl_device_id platform);

/**
 * \brief Check for required OpenCL version and possibly terminate the test.
 *
 * \c required_version_times_10 must be an OpenCL version multiplied by 10.
 * For example, if the required CL version is 1.1, then the
 * \c required_version_times_10 should be 11.
 */
void piglit_cl_require_device_version(cl_device_id device,
                                      int required_version_times_10);

/**
 * \brief Get version of OpenCL C for \c device.
 *
 * Returned version is multiplied by 10 to make it an integer. For
 * example, if the CL C version is 1.1, the returned value is 11.
 */
int piglit_cl_get_device_cl_c_version(cl_device_id device);

/**
 * \brief Check for required OpenCL C version and possibly terminate the test.
 *
 * \c required_version_times_10 must be an OpenCL C version multiplied by 10.
 * For example, if the required CL C version is 1.1, then the
 * \c required_version_times_10 should be 11.
 */
void piglit_cl_require_device_cl_c_version(cl_device_id device,
                                           int required_version_times_10);

/**
 * \brief Get platform information.
 *
 * \warning Returned data must be freed by the caller.
 *
 * \note
 * Although the returned types of all params are of type char all through
 * the last version of OpenCL (1.2 as of time of writing), the return type
 * of this function is void* for future compatibility.
 */
void* piglit_cl_get_platform_info(cl_platform_id platform,
                                  cl_platform_info param);

/**
 * \brief Get device information.
 *
 * \warning Returned data must be freed by the caller.
 */
void* piglit_cl_get_device_info(cl_device_id device, cl_device_info param);

/**
 * \brief Get context information.
 *
 * \warning Returned data must be freed by the caller.
 */
void* piglit_cl_get_context_info(cl_context context, cl_context_info param);

/**
 * \brief Get command queue information.
 *
 * \warning Returned data must be freed by the caller.
 */
void* piglit_cl_get_command_queue_info(cl_command_queue command_queue,
                                       cl_command_queue_info param);

/**
 * \brief Get memory object information.
 *
 * \warning Returned data must be freed by the caller.
 */
void* piglit_cl_get_mem_object_info(cl_mem mem_obj, cl_mem_info param);

/**
 * \brief Get image information.
 *
 * \warning Returned data must be freed by the caller.
 */
void* piglit_cl_get_image_info(cl_mem image, cl_image_info param);

/**
 * \brief Get sampler information.
 *
 * \warning Returned data must be freed by the caller.
 */
void* piglit_cl_get_sampler_info(cl_sampler sampler, cl_sampler_info param);

/**
 * \brief Get program information.
 *
 * \warning Returned data must be freed by the caller.
 */
void* piglit_cl_get_program_info(cl_program program, cl_program_info param);

/**
 * \brief Get program build information.
 *
 * \warning Returned data must be freed by the caller.
 */
void* piglit_cl_get_program_build_info(cl_program program,
                                       cl_device_id device,
                                       cl_program_build_info param);

/**
 * \brief Get kernel information.
 *
 * \warning Returned data must be freed by the caller.
 */
void* piglit_cl_get_kernel_info(cl_kernel kernel, cl_mem_info param);

/**
 * \brief Get kernel work group information.
 *
 * \warning Returned data must be freed by the caller.
 */
void* piglit_cl_get_kernel_work_group_info(cl_kernel kernel,
                                           cl_device_id device,
                                           cl_mem_info param);

/**
 * \brief Get event information.
 *
 * \warning Returned data must be freed by the caller.
 */
void* piglit_cl_get_event_info(cl_event event, cl_event_info param);

/**
 * \brief Get profiling information.
 *
 * \warning Returned data must be freed by the caller.
 */
void* piglit_cl_get_event_profiling_info(cl_event event,
                                         cl_profiling_info param);

/* Extensions */

/**
 * \brief Check if platform extension is supported
 *
 * \pre name is not null
 */
bool piglit_cl_is_platform_extension_supported(cl_platform_id platform,
                                               const char *name);

/**
 * \brief Check for required OpenCL platform extension and possibly
 * terminate the test.
 */
void piglit_cl_require_platform_extension(cl_platform_id platform,
                                          const char *name);

/**
 * \brief Check for not required OpenCL platform extension and possibly
 * terminate the test.
 */
void piglit_cl_require_not_platform_extension(cl_platform_id platform,
                                              const char *name);

/**
 * \brief Check if device extension is supported
 *
 * \pre name is not null
 */
bool piglit_cl_is_device_extension_supported(cl_device_id device,
                                             const char *name);

/**
 * \brief Check for required OpenCL device extension and possibly
 * terminate the test.
 */
void piglit_cl_require_device_extension(cl_device_id device, const char *name);

/**
 * \brief Check for not required OpenCL device extension and possibly
 * terminate the test.
 */
void piglit_cl_require_not_device_extension(cl_device_id device,
                                            const char *name);

/* Helper functions */

/**
 * \brief Get all available platforms.
 *
 * \warning Caller must free the allocated platform array.
 *
 * @param platform_ids  Address to store a pointer to platform ids list.
 * @return              Number of stored platform ids.
 */
unsigned int piglit_cl_get_platform_ids(cl_platform_id** platform_ids);

/**
 * \brief Get all available devices on platform \c platform_id.
 *
 * \warning Caller must free the allocated device array.
 *
 * @param platform_id   Platform from which to get platforms.
 * @param device_type   A bitfield to filter device types.
 * @param device_ids    Address to store a pointer to device ids list.
 * @return              Number of stored device ids.
 */
unsigned int piglit_cl_get_device_ids(cl_platform_id platform_id,
                                      cl_device_type device_type,
                                      cl_device_id** device_ids);

/**
 * \brief Helper context.
 *
 * Helper context struct for easier OpenCL context manipulation.
 */
struct _piglit_cl_context {
	cl_platform_id platform_id; /**< Platform used to create context. */
	cl_context cl_ctx; /**< OpenCL context. */

	unsigned int num_devices; /**< Number of members in \c device_ids and \c
	                               command_queues */

	cl_device_id* device_ids; /**< Device ids available in the context. */
	cl_command_queue* command_queues; /**< Command queues available in the
	                                       context.  Each command queue is
	                                       assigned to device id in \c
	                                       device_ids with the same index. */
};
typedef struct _piglit_cl_context* piglit_cl_context;

/**
 * \brief Create \c piglit_cl_context
 *
 * Create a helper context from platform id \c platform_id and
 * device ids \c device_ids.
 *
 * @param context      Context struct to fill.
 * @param platform_id  Platform from which to create context.
 * @param device_ids   Device ids to add to context.
 * @param num_devices  Number of members in \c device_ids.
 * @return             Return \c true on success.
 */
piglit_cl_context
piglit_cl_create_context(cl_platform_id platform_id,
                         const cl_device_id device_ids[],
                         unsigned int num_devices);

/**
 * \brief Release \c piglit_cl_context
 *
 * Free memory used by \c context and release the generated context
 * and memory queues.
 *
 * @param context  Context to release.
 */
void
piglit_cl_release_context(piglit_cl_context context);

/**
 * \brief Create and build a program with source.
 *
 * Create and build a program with source for all devices in
 * \c piglit_cl_context.
 *
 * @param context      Context on which to create and build program.
 * @param count        Number of strings in \c strings.
 * @param string       Array of pointers to NULL-terminated source strings.
 * @param options      NULL-terminated string that describes build options.
 * @return             Built program or NULL on fail.
 */
cl_program
piglit_cl_build_program_with_source(piglit_cl_context context,
                                    cl_uint count,
                                    char** strings,
                                    const char* options);

/**
 * \brief Create and try to build a program with invalid source.
 *
 * Create and try to build a program with invalid source for all devices
 * in \c piglit_cl_context.
 *
 * @param context      Context on which to create and build program.
 * @param count        Number of strings in \c strings.
 * @param string       Array of pointers to NULL-terminated source strings.
 * @param options      NULL-terminated string that describes build options.
 * @return             Unsuccessfully built program or NULL on fail.
 */
cl_program
piglit_cl_fail_build_program_with_source(piglit_cl_context context,
                                         cl_uint count,
                                         char** strings,
                                         const char* options);

/**
 * \brief Create and build a program with binary.
 *
 * Create and build aprogram with binary for all devices in
 * \c piglit_cl_context.
 *
 * @param context      Context on which to create and build program.
 * @param lenghts      Lenghts of binaries in \c binaries.
 * @param binaries     Array of pointers to binaries.
 * @param options      NULL-terminated string that describes build options.
 * @return             Built program or NULL on fail.
 */
cl_program
piglit_cl_build_program_with_binary(piglit_cl_context context,
                                    size_t* lenghts,
                                    unsigned char** binaries,
                                    const char* options);

/**
 * \brief Create and try to build a program with invalid binary.
 *
 * Create and try to build a program with invalid binary for all devices
 * in \c piglit_cl_context.
 *
 * @param context      Context on which to create and build program.
 * @param lenghts      Lenghts of binaries in \c binaries.
 * @param binaries     Array of pointers to binaries.
 * @param options      NULL-terminated string that describes build options.
 * @return             Unsuccessfully built program or NULL on fail.
 */
cl_program
piglit_cl_fail_build_program_with_binary(piglit_cl_context context,
                                         size_t* lenghts,
                                         unsigned char** binaries,
                                         const char* options);

/**
 * \brief Create a buffer.
 *
 * @param context      Context on which to create buffer.
 * @param flags        Memory flags.
 * @param size         Size of created buffer.
 * @return             Created buffer or NULL on fail.
 */
cl_mem
piglit_cl_create_buffer(piglit_cl_context context,
                        cl_mem_flags flags,
                        size_t size);

/**
 * \brief Blocking write to a buffer.
 *
 * @param command_queue  Command queue to enqueue operation on.
 * @param buffer         Memory buffer to write to.
 * @param offset         Offset in buffer.
 * @param cb             Size of data in bytes.
 * @param ptr            Pointer to data to be written to buffer.
 * @return               \c true on succes, \c false otherwise.
 */
bool
piglit_cl_write_buffer(cl_command_queue command_queue,
                       cl_mem buffer,
                       size_t offset,
                       size_t cb,
                       const void *ptr);

/**
 * \brief Blocking write to a whole buffer.
 *
 * \warning \c ptr must point to memory space which is equal or larger
 * in size than \c buffer.
 *
 * @param command_queue  Command queue to enqueue operation on.
 * @param buffer         Memory buffer to write to.
 * @param ptr            Pointer to data to be written to buffer.
 * @return               \c true on succes, \c false otherwise.
 */
bool
piglit_cl_write_whole_buffer(cl_command_queue command_queue,
                             cl_mem buffer,
                             const void *ptr);

/**
 * \brief Blocking read from a buffer.
 *
 * @param command_queue  Command queue to enqueue operation on.
 * @param buffer         Memory buffer to read from.
 * @param offset         Offset in buffer.
 * @param cb             Size of data in bytes.
 * @param ptr            Pointer to data to be written from buffer.
 * @return               \c true on succes, \c false otherwise.
 */
bool
piglit_cl_read_buffer(cl_command_queue command_queue,
                      cl_mem buffer,
                      size_t offset,
                      size_t cb,
                      void *ptr);

/**
 * \brief Blocking read from a whole buffer.
 *
 * \warning \c ptr must point to memory space which is equal or larger
 * in size than \c buffer.
 *
 * @param command_queue  Command queue to enqueue operation on.
 * @param buffer         Memory buffer to read from.
 * @param ptr            Pointer to data to be written from buffer.
 * @return               \c true on succes, \c false otherwise.
 */
bool
piglit_cl_read_whole_buffer(cl_command_queue command_queue,
                            cl_mem buffer,
                            void *ptr);
#ifdef CL_VERSION_1_2
typedef cl_image_desc piglit_image_desc;
#else
/** Taken from OpenCL 1.2 specs 5.3.1.2 */
typedef struct {
	cl_mem_object_type image_type;
	size_t             image_width;
	size_t             image_height;
	size_t             image_depth;
	size_t             image_array_size;
	size_t             image_row_pitch;
	size_t             image_slice_pitch;
	cl_uint            num_mip_levels;
	cl_uint            num_samples;
	cl_mem             buffer;
} piglit_image_desc;
#endif

/**
 * \brief Get context image support.
 *
 * @param context      Context on which to create image.
 * @return             True if there is one device with image support.
 */
bool
piglit_cl_get_context_image_support(const piglit_cl_context context);

/**
 * \brief Get device image support.
 *
 * @param device       Context on which to create image.
 * @return             True if there the device has image support.
 */
bool
piglit_cl_get_device_image_support(cl_device_id device);


/**
 * \brief Create an image.
 *
 * @param context      Context on which to create image.
 * @param flags        Memory flags.
 * @param format       Image format.
 * @param desc         Image descriptor.
 * @return             Created image or NULL on fail.
 */
cl_mem
piglit_cl_create_image(piglit_cl_context context,
                       cl_mem_flags flags,
                       const cl_image_format *format,
                       const piglit_image_desc *desc);

/**
 * \brief Blocking write to an image.
 *
 * @param command_queue  Command queue to enqueue operation on.
 * @param image          Image to write to.
 * @param origin         (x, y, z) offset in pixels.
 * @param region         (width, height, depht) size in pixels.
 * @param ptr            Pointer to data to be written to image.
 * @return               \c true on succes, \c false otherwise.
 */
bool
piglit_cl_write_image(cl_command_queue command_queue,
                      cl_mem image,
                      const size_t *origin,
                      const size_t *region,
                      const void *ptr);

/**
 * \brief Blocking write to the entire area of an image.
 *
 * \warning \c ptr must point to memory space which is equal or larger
 * in size than \c image.
 *
 * @param command_queue  Command queue to enqueue operation on.
 * @param image          Image to write to.
 * @param ptr            Pointer to data to be written to image.
 * @return               \c true on succes, \c false otherwise.
 */
bool
piglit_cl_write_whole_image(cl_command_queue command_queue,
                            cl_mem image,
                            const void *ptr);

/**
 * \brief Blocking read from an image.
 *
 * @param command_queue  Command queue to enqueue operation on.
 * @param image          Image to read from.
 * @param origin         (x, y, z) offset in pixels.
 * @param region         (width, height, depht) size in pixels.
 * @param ptr            Pointer to data read from image.
 * @return               \c true on succes, \c false otherwise.
 */
bool
piglit_cl_read_image(cl_command_queue command_queue,
                     cl_mem image,
                     const size_t *origin,
                     const size_t *region,
                     void *ptr);

/**
 * \brief Blocking read of the full contents of an image.
 *
 * \warning \c ptr must point to memory space which is equal or larger
 * in size than \c image.
 *
 * @param command_queue  Command queue to enqueue operation on.
 * @param image          Image to read from.
 * @param ptr            Pointer to data read from image.
 * @return               \c true on succes, \c false otherwise.
 */
bool
piglit_cl_read_whole_image(cl_command_queue command_queue,
                           cl_mem image,
                           void *ptr);

/**
 * \brief Create a sampler.
 *
 * @param context            Context on which to create image.
 * @param normalized_coords  Use normalized coords if true.
 * @param addressing_mode    Addressing mode.
 * @param filter_mode        Filter mode.
 * @return                   Created sampler or NULL on fail.
 */
cl_sampler
piglit_cl_create_sampler(piglit_cl_context context,
                         cl_bool normalized_coords,
                         cl_addressing_mode addressing_mode,
                         cl_filter_mode filter_mode);

/**
 * \brief Create a kernel.
 *
 * @param context      Program on which to create a kernel.
 * @param kernel_name  Kernel name.
 * @return             Created kernel or NULL on fail.
 */
cl_kernel
piglit_cl_create_kernel(cl_program program, const char* kernel_name);

/**
 * \brief Set kernel argument.
 *
 * @param kernel       Kernel for which to set an argument.
 * @param arg_index    Argument index.
 * @param size         Size of argument.
 * @param arg_value    Pointer to argument value.
 * @return             \c true on succes, \c false otherwise.
 */
bool
piglit_cl_set_kernel_arg(cl_kernel kernel,
                         cl_uint arg_index,
                         size_t size,
                         const void* arg_value);

/**
 * \brief Set kernel buffer argument.
 *
 * @param kernel       Kernel for which to set an buffer argument.
 * @param arg_index    Argument index.
 * @param buffer       Buffer to be set as argument.
 * @return             \c true on succes, \c false otherwise.
 */
bool
piglit_cl_set_kernel_buffer_arg(cl_kernel kernel,
                                cl_uint arg_index,
                                cl_mem *buffer);

/**
 * \brief Enqueue ND-range kernel.
 *
 * @param command_queue     Command queue to enqueue operation on.
 * @param kernel            Kernel to be enqueued.
 * @param work_dim          Work dimensions.
 * @param global_offset     Global offset.
 * @param global_work_size  Global work sizes.
 * @param local_work_size   Local work sizes.
 * @param ev                Location to store execution event.
 * @return                  \c true on succes, \c false otherwise.
 */
bool
piglit_cl_enqueue_ND_range_kernel(cl_command_queue command_queue,
                                  cl_kernel kernel,
                                  cl_uint work_dim,
                                  const size_t* global_offset,
                                  const size_t* global_work_size,
                                  const size_t* local_work_size,
				  cl_event *ev);

/**
 * \brief Enqueue ND-range kernel and wait it to complete.
 *
 * @param command_queue     Command queue to enqueue operation on.
 * @param kernel            Kernel to be enqueued.
 * @param work_dim          Work dimensions.
 * @param global_offset     Global offset.
 * @param global_work_size  Global work sizes.
 * @param local_work_size   Local work sizes.
 * @return                  \c true on succes, \c false otherwise.
 */
bool
piglit_cl_execute_ND_range_kernel(cl_command_queue command_queue,
                                  cl_kernel kernel,
                                  cl_uint work_dim,
                                  const size_t* global_offset,
                                  const size_t* global_work_size,
                                  const size_t* local_work_size);

/**
 * \brief Enqueue kernel task.
 *
 * @param command_queue     Command queue to enqueue operation on.
 * @param kernel            Kernel to be enqueued.
 * @return                  \c true on succes, \c false otherwise.
 */
bool
piglit_cl_enqueue_task(cl_command_queue command_queue, cl_kernel kernel);

/**
 * \brief Enqueue kernel task and wait it to complete.
 *
 * @param command_queue     Command queue to enqueue operation on.
 * @param kernel            Kernel to be enqueued.
 * @return                  \c true on succes, \c false otherwise.
 */
bool
piglit_cl_execute_task(cl_command_queue command_queue, cl_kernel kernel);

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif //PIGLIT_UTIL_CL_H
