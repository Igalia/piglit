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

#include "piglit-util-cl.h"

const char *
piglit_cl_get_enum_name(cl_uint param)
{
#define CASE(x) case x: return #x;

	/* Enums are copied from cl.h header and then the bitfield
	 * and non-unique enums are commented out (only one group
	 * of intersecting groups of enums is left uncommented).
	 */

	switch (param) {

/* Error Codes */
	/*
	CASE(CL_SUCCESS)                                  // 0
	CASE(CL_DEVICE_NOT_FOUND)                         // -1
	CASE(CL_DEVICE_NOT_AVAILABLE)                     // -2
	CASE(CL_COMPILER_NOT_AVAILABLE)                   // -3
	CASE(CL_MEM_OBJECT_ALLOCATION_FAILURE)            // -4
	CASE(CL_OUT_OF_RESOURCES)                         // -5
	CASE(CL_OUT_OF_HOST_MEMORY)                       // -6
	CASE(CL_PROFILING_INFO_NOT_AVAILABLE)             // -7
	CASE(CL_MEM_COPY_OVERLAP)                         // -8
	CASE(CL_IMAGE_FORMAT_MISMATCH)                    // -9
	CASE(CL_IMAGE_FORMAT_NOT_SUPPORTED)               // -10
	CASE(CL_BUILD_PROGRAM_FAILURE)                    // -11
	CASE(CL_MAP_FAILURE)                              // -12
#ifdef CL_VERSION_1_1
	CASE(CL_MISALIGNED_SUB_BUFFER_OFFSET)             // -13
	CASE(CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST) // -14
#endif //CL_VERSION_1_1
#ifdef CL_VERSION_1_2
	CASE(CL_COMPILE_PROGRAM_FAILURE)                  // -15
	CASE(CL_LINKER_NOT_AVAILABLE)                     // -16
	CASE(CL_LINK_PROGRAM_FAILURE)                     // -17
	CASE(CL_DEVICE_PARTITION_FAILED)                  // -18
	CASE(CL_KERNEL_ARG_INFO_NOT_AVAILABLE)            // -19
#endif //CL_VERSION_1_2

	CASE(CL_INVALID_VALUE)                            // -30
	CASE(CL_INVALID_DEVICE_TYPE)                      // -31
	CASE(CL_INVALID_PLATFORM)                         // -32
	CASE(CL_INVALID_DEVICE)                           // -33
	CASE(CL_INVALID_CONTEXT)                          // -34
	CASE(CL_INVALID_QUEUE_PROPERTIES)                 // -35
	CASE(CL_INVALID_COMMAND_QUEUE)                    // -36
	CASE(CL_INVALID_HOST_PTR)                         // -37
	CASE(CL_INVALID_MEM_OBJECT)                       // -38
	CASE(CL_INVALID_IMAGE_FORMAT_DESCRIPTOR)          // -39
	CASE(CL_INVALID_IMAGE_SIZE)                       // -40
	CASE(CL_INVALID_SAMPLER)                          // -41
	CASE(CL_INVALID_BINARY)                           // -42
	CASE(CL_INVALID_BUILD_OPTIONS)                    // -43
	CASE(CL_INVALID_PROGRAM)                          // -44
	CASE(CL_INVALID_PROGRAM_EXECUTABLE)               // -45
	CASE(CL_INVALID_KERNEL_NAME)                      // -46
	CASE(CL_INVALID_KERNEL_DEFINITION)                // -47
	CASE(CL_INVALID_KERNEL)                           // -48
	CASE(CL_INVALID_ARG_INDEX)                        // -49
	CASE(CL_INVALID_ARG_VALUE)                        // -50
	CASE(CL_INVALID_ARG_SIZE)                         // -51
	CASE(CL_INVALID_KERNEL_ARGS)                      // -52
	CASE(CL_INVALID_WORK_DIMENSION)                   // -53
	CASE(CL_INVALID_WORK_GROUP_SIZE)                  // -54
	CASE(CL_INVALID_WORK_ITEM_SIZE)                   // -55
	CASE(CL_INVALID_GLOBAL_OFFSET)                    // -56
	CASE(CL_INVALID_EVENT_WAIT_LIST)                  // -57
	CASE(CL_INVALID_EVENT)                            // -58
	CASE(CL_INVALID_OPERATION)                        // -59
	CASE(CL_INVALID_GL_OBJECT)                        // -60
	CASE(CL_INVALID_BUFFER_SIZE)                      // -61
	CASE(CL_INVALID_MIP_LEVEL)                        // -62
	CASE(CL_INVALID_GLOBAL_WORK_SIZE)                 // -63
#ifdef CL_VERSION_1_1
	CASE(CL_INVALID_PROPERTY)                         // -64
#endif //CL_VERSION_1_1
#ifdef CL_VERSION_1_2
	CASE(CL_INVALID_IMAGE_DESCRIPTOR)                 // -65
	CASE(CL_INVALID_COMPILER_OPTIONS)                 // -66
	CASE(CL_INVALID_LINKER_OPTIONS)                   // -67
	CASE(CL_INVALID_DEVICE_PARTITION_COUNT)           // -68
#endif //CL_VERSION_1_2
	*/

/* OpenCL Version */
	/*
	CASE(CL_VERSION_1_0)                              // 1
	CASE(CL_VERSION_1_1)                              // 1
	CASE(CL_VERSION_1_2)                              // 1
	*/

/* cl_bool */
	/*
	CASE(CL_FALSE)                                    // 0
	CASE(CL_TRUE)                                     // 1
	CASE(CL_BLOCKING)                                 // CL_TRUE
	CASE(CL_NON_BLOCKING)                             // CL_FALSE
	*/

/* cl_platform_info */
	CASE(CL_PLATFORM_PROFILE)                         // 0x0900
	CASE(CL_PLATFORM_VERSION)                         // 0x0901
	CASE(CL_PLATFORM_NAME)                            // 0x0902
	CASE(CL_PLATFORM_VENDOR)                          // 0x0903
	CASE(CL_PLATFORM_EXTENSIONS)                      // 0x0904

/* cl_device_type - bitfield */
	/*
	CASE(CL_DEVICE_TYPE_DEFAULT)                      // (1 << 0)
	CASE(CL_DEVICE_TYPE_CPU)                          // (1 << 1)
	CASE(CL_DEVICE_TYPE_GPU)                          // (1 << 2)
	CASE(CL_DEVICE_TYPE_ACCELERATOR)                  // (1 << 3)
	CASE(CL_DEVICE_TYPE_CUSTOM)                       // (1 << 4)
	CASE(CL_DEVICE_TYPE_ALL)                          // 0xFFFFFFFF
	*/

/* cl_device_info */
	CASE(CL_DEVICE_TYPE)                              // 0x1000
	CASE(CL_DEVICE_VENDOR_ID)                         // 0x1001
	CASE(CL_DEVICE_MAX_COMPUTE_UNITS)                 // 0x1002
	CASE(CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS)          // 0x1003
	CASE(CL_DEVICE_MAX_WORK_GROUP_SIZE)               // 0x1004
	CASE(CL_DEVICE_MAX_WORK_ITEM_SIZES)               // 0x1005
	CASE(CL_DEVICE_PREFERRED_VECTOR_WIDTH_CHAR)       // 0x1006
	CASE(CL_DEVICE_PREFERRED_VECTOR_WIDTH_SHORT)      // 0x1007
	CASE(CL_DEVICE_PREFERRED_VECTOR_WIDTH_INT)        // 0x1008
	CASE(CL_DEVICE_PREFERRED_VECTOR_WIDTH_LONG)       // 0x1009
	CASE(CL_DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT)      // 0x100A
	CASE(CL_DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE)     // 0x100B
	CASE(CL_DEVICE_MAX_CLOCK_FREQUENCY)               // 0x100C
	CASE(CL_DEVICE_ADDRESS_BITS)                      // 0x100D
	CASE(CL_DEVICE_MAX_READ_IMAGE_ARGS)               // 0x100E
	CASE(CL_DEVICE_MAX_WRITE_IMAGE_ARGS)              // 0x100F
	CASE(CL_DEVICE_MAX_MEM_ALLOC_SIZE)                // 0x1010
	CASE(CL_DEVICE_IMAGE2D_MAX_WIDTH)                 // 0x1011
	CASE(CL_DEVICE_IMAGE2D_MAX_HEIGHT)                // 0x1012
	CASE(CL_DEVICE_IMAGE3D_MAX_WIDTH)                 // 0x1013
	CASE(CL_DEVICE_IMAGE3D_MAX_HEIGHT)                // 0x1014
	CASE(CL_DEVICE_IMAGE3D_MAX_DEPTH)                 // 0x1015
	CASE(CL_DEVICE_IMAGE_SUPPORT)                     // 0x1016
	CASE(CL_DEVICE_MAX_PARAMETER_SIZE)                // 0x1017
	CASE(CL_DEVICE_MAX_SAMPLERS)                      // 0x1018
	CASE(CL_DEVICE_MEM_BASE_ADDR_ALIGN)               // 0x1019
	CASE(CL_DEVICE_MIN_DATA_TYPE_ALIGN_SIZE)          // 0x101A
	CASE(CL_DEVICE_SINGLE_FP_CONFIG)                  // 0x101B
	CASE(CL_DEVICE_GLOBAL_MEM_CACHE_TYPE)             // 0x101C
	CASE(CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE)         // 0x101D
	CASE(CL_DEVICE_GLOBAL_MEM_CACHE_SIZE)             // 0x101E
	CASE(CL_DEVICE_GLOBAL_MEM_SIZE)                   // 0x101F
	CASE(CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE)          // 0x1020
	CASE(CL_DEVICE_MAX_CONSTANT_ARGS)                 // 0x1021
	CASE(CL_DEVICE_LOCAL_MEM_TYPE)                    // 0x1022
	CASE(CL_DEVICE_LOCAL_MEM_SIZE)                    // 0x1023
	CASE(CL_DEVICE_ERROR_CORRECTION_SUPPORT)          // 0x1024
	CASE(CL_DEVICE_PROFILING_TIMER_RESOLUTION)        // 0x1025
	CASE(CL_DEVICE_ENDIAN_LITTLE)                     // 0x1026
	CASE(CL_DEVICE_AVAILABLE)                         // 0x1027
	CASE(CL_DEVICE_COMPILER_AVAILABLE)                // 0x1028
	CASE(CL_DEVICE_EXECUTION_CAPABILITIES)            // 0x1029
	CASE(CL_DEVICE_QUEUE_PROPERTIES)                  // 0x102A
	CASE(CL_DEVICE_NAME)                              // 0x102B
	CASE(CL_DEVICE_VENDOR)                            // 0x102C
	CASE(CL_DRIVER_VERSION)                           // 0x102D
	CASE(CL_DEVICE_PROFILE)                           // 0x102E
	CASE(CL_DEVICE_VERSION)                           // 0x102F
	CASE(CL_DEVICE_EXTENSIONS)                        // 0x1030
	CASE(CL_DEVICE_PLATFORM)                          // 0x1031
#ifdef CL_VERSION_1_2
	CASE(CL_DEVICE_DOUBLE_FP_CONFIG)                  // 0x1032
#endif //CL_VERSION_1_2
	/* 0x1033 reserved for CL_DEVICE_HALF_FP_CONFIG */
#ifdef CL_VERSION_1_1
	CASE(CL_DEVICE_PREFERRED_VECTOR_WIDTH_HALF)       // 0x1034
	CASE(CL_DEVICE_HOST_UNIFIED_MEMORY)               // 0x1035
	CASE(CL_DEVICE_NATIVE_VECTOR_WIDTH_CHAR)          // 0x1036
	CASE(CL_DEVICE_NATIVE_VECTOR_WIDTH_SHORT)         // 0x1037
	CASE(CL_DEVICE_NATIVE_VECTOR_WIDTH_INT)           // 0x1038
	CASE(CL_DEVICE_NATIVE_VECTOR_WIDTH_LONG)          // 0x1039
	CASE(CL_DEVICE_NATIVE_VECTOR_WIDTH_FLOAT)         // 0x103A
	CASE(CL_DEVICE_NATIVE_VECTOR_WIDTH_DOUBLE)        // 0x103B
	CASE(CL_DEVICE_NATIVE_VECTOR_WIDTH_HALF)          // 0x103C
	CASE(CL_DEVICE_OPENCL_C_VERSION)                  // 0x103D
#endif //CL_VERSION_1_1
#ifdef CL_VERSION_1_2
	CASE(CL_DEVICE_LINKER_AVAILABLE)                  // 0x103E
	CASE(CL_DEVICE_BUILT_IN_KERNELS)                  // 0x103F
	CASE(CL_DEVICE_IMAGE_MAX_BUFFER_SIZE)             // 0x1040
	CASE(CL_DEVICE_IMAGE_MAX_ARRAY_SIZE)              // 0x1041
	CASE(CL_DEVICE_PARENT_DEVICE)                     // 0x1042
	CASE(CL_DEVICE_PARTITION_MAX_SUB_DEVICES)         // 0x1043
	CASE(CL_DEVICE_PARTITION_PROPERTIES)              // 0x1044
	CASE(CL_DEVICE_PARTITION_AFFINITY_DOMAIN)         // 0x1045
	CASE(CL_DEVICE_PARTITION_TYPE)                    // 0x1046
	CASE(CL_DEVICE_REFERENCE_COUNT)                   // 0x1047
	CASE(CL_DEVICE_PREFERRED_INTEROP_USER_SYNC)       // 0x1048
	CASE(CL_DEVICE_PRINTF_BUFFER_SIZE)                // 0x1049
#endif //CL_VERSION_1_2

/* cl_device_fp_config - bitfield */
	/*
	CASE(CL_FP_DENORM)                                // (1 << 0)
	CASE(CL_FP_INF_NAN)                               // (1 << 1)
	CASE(CL_FP_ROUND_TO_NEAREST)                      // (1 << 2)
	CASE(CL_FP_ROUND_TO_ZERO)                         // (1 << 3)
	CASE(CL_FP_ROUND_TO_INF)                          // (1 << 4)
	CASE(CL_FP_FMA)                                   // (1 << 5)
	CASE(CL_FP_SOFT_FLOAT)                            // (1 << 6)
	CASE(CL_FP_CORRECTLY_ROUNDED_DIVIDE_SQRT)         // (1 << 7)
	*/

/* cl_device_mem_cache_type */
	/*
	CASE(CL_NONE)                                     // 0x0
	CASE(CL_READ_ONLY_CACHE)                          // 0x1
	CASE(CL_READ_WRITE_CACHE)                         // 0x2
	*/

/* cl_device_local_mem_type */
	/*
	CASE(CL_LOCAL)                                    // 0x1
	CASE(CL_GLOBAL)                                   // 0x2
	*/

/* cl_device_exec_capabilities - bitfield */
	/*
	CASE(CL_EXEC_KERNEL)                              // (1 << 0)
	CASE(CL_EXEC_NATIVE_KERNEL)                       // (1 << 1)
	*/

/* cl_command_queue_properties - bitfield */
	/*
	CASE(CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE)      // (1 << 0)
	CASE(CL_QUEUE_PROFILING_ENABLE)                   // (1 << 1)
	*/

/* cl_context_info  */
	CASE(CL_CONTEXT_REFERENCE_COUNT)                  // 0x1080
	CASE(CL_CONTEXT_DEVICES)                          // 0x1081
	CASE(CL_CONTEXT_PROPERTIES)                       // 0x1082
#ifdef CL_VERSION_1_1
	CASE(CL_CONTEXT_NUM_DEVICES)                      // 0x1083
#endif //CL_VERSION_1_1

/* cl_context_properties */
	CASE(CL_CONTEXT_PLATFORM)                         // 0x1084
#ifdef CL_VERSION_1_2
	CASE(CL_CONTEXT_INTEROP_USER_SYNC)                // 0x1085
#endif //CL_VERSION_1_2

/* cl_device_partition_property */
#ifdef CL_VERSION_1_2
	CASE(CL_DEVICE_PARTITION_EQUALLY)                 // 0x1086
	CASE(CL_DEVICE_PARTITION_BY_COUNTS)               // 0x1087
	CASE(CL_DEVICE_PARTITION_BY_COUNTS_LIST_END)      // 0x0
	CASE(CL_DEVICE_PARTITION_BY_AFFINITY_DOMAIN)      // 0x1088
#endif //CL_VERSION_1_2

/* cl_device_affinity_domain */
	/*
	CASE(CL_DEVICE_AFFINITY_DOMAIN_NUMA)                     // (1 << 0)
	CASE(CL_DEVICE_AFFINITY_DOMAIN_L4_CACHE)                 // (1 << 1)
	CASE(CL_DEVICE_AFFINITY_DOMAIN_L3_CACHE)                 // (1 << 2)
	CASE(CL_DEVICE_AFFINITY_DOMAIN_L2_CACHE)                 // (1 << 3)
	CASE(CL_DEVICE_AFFINITY_DOMAIN_L1_CACHE)                 // (1 << 4)
	CASE(CL_DEVICE_AFFINITY_DOMAIN_NEXT_PARTITIONABLE)       // (1 << 5)
	*/

/* cl_command_queue_info */
	CASE(CL_QUEUE_CONTEXT)                            // 0x1090
	CASE(CL_QUEUE_DEVICE)                             // 0x1091
	CASE(CL_QUEUE_REFERENCE_COUNT)                    // 0x1092
	CASE(CL_QUEUE_PROPERTIES)                         // 0x1093

/* cl_mem_flags - bitfield */
	/*
	CASE(CL_MEM_READ_WRITE)                           // (1 << 0)
	CASE(CL_MEM_WRITE_ONLY)                           // (1 << 1)
	CASE(CL_MEM_READ_ONLY)                            // (1 << 2)
	CASE(CL_MEM_USE_HOST_PTR)                         // (1 << 3)
	CASE(CL_MEM_ALLOC_HOST_PTR)                       // (1 << 4)
	CASE(CL_MEM_COPY_HOST_PTR)                        // (1 << 5)
	// reserved                                         (1 << 6)
	CASE(CL_MEM_HOST_WRITE_ONLY)                      // (1 << 7)
	CASE(CL_MEM_HOST_READ_ONLY)                       // (1 << 8)
	CASE(CL_MEM_HOST_NO_ACCESS)                       // (1 << 9)
	*/

/* cl_mem_migration_flags - bitfield */
	/*
	CASE(CL_MIGRATE_MEM_OBJECT_HOST)                  // (1 << 0)
	CASE(CL_MIGRATE_MEM_OBJECT_CONTENT_UNDEFINED)     // (1 << 1)
	*/

/* cl_channel_order */
	CASE(CL_R)                                        // 0x10B0
	CASE(CL_A)                                        // 0x10B1
	CASE(CL_RG)                                       // 0x10B2
	CASE(CL_RA)                                       // 0x10B3
	CASE(CL_RGB)                                      // 0x10B4
	CASE(CL_RGBA)                                     // 0x10B5
	CASE(CL_BGRA)                                     // 0x10B6
	CASE(CL_ARGB)                                     // 0x10B7
	CASE(CL_INTENSITY)                                // 0x10B8
	CASE(CL_LUMINANCE)                                // 0x10B9
#ifdef CL_VERSION_1_1
	CASE(CL_Rx)                                       // 0x10BA
	CASE(CL_RGx)                                      // 0x10BB
	CASE(CL_RGBx)                                     // 0x10BC
#endif //CL_VERSION_1_1

/* cl_channel_type */
	CASE(CL_SNORM_INT8)                               // 0x10D0
	CASE(CL_SNORM_INT16)                              // 0x10D1
	CASE(CL_UNORM_INT8)                               // 0x10D2
	CASE(CL_UNORM_INT16)                              // 0x10D3
	CASE(CL_UNORM_SHORT_565)                          // 0x10D4
	CASE(CL_UNORM_SHORT_555)                          // 0x10D5
	CASE(CL_UNORM_INT_101010)                         // 0x10D6
	CASE(CL_SIGNED_INT8)                              // 0x10D7
	CASE(CL_SIGNED_INT16)                             // 0x10D8
	CASE(CL_SIGNED_INT32)                             // 0x10D9
	CASE(CL_UNSIGNED_INT8)                            // 0x10DA
	CASE(CL_UNSIGNED_INT16)                           // 0x10DB
	CASE(CL_UNSIGNED_INT32)                           // 0x10DC
	CASE(CL_HALF_FLOAT)                               // 0x10DD
	CASE(CL_FLOAT)                                    // 0x10DE

/* cl_mem_object_type */
	CASE(CL_MEM_OBJECT_BUFFER)                        // 0x10F0
	CASE(CL_MEM_OBJECT_IMAGE2D)                       // 0x10F1
	CASE(CL_MEM_OBJECT_IMAGE3D)                       // 0x10F2
#ifdef CL_VERSION_1_2
	CASE(CL_MEM_OBJECT_IMAGE2D_ARRAY)                 // 0x10F3
	CASE(CL_MEM_OBJECT_IMAGE1D)                       // 0x10F4
	CASE(CL_MEM_OBJECT_IMAGE1D_ARRAY)                 // 0x10F5
	CASE(CL_MEM_OBJECT_IMAGE1D_BUFFER)                // 0x10F6
#endif //CL_VERSION_1_2

/* cl_mem_info */
	CASE(CL_MEM_TYPE)                                 // 0x1100
	CASE(CL_MEM_FLAGS)                                // 0x1101
	CASE(CL_MEM_SIZE)                                 // 0x1102
	CASE(CL_MEM_HOST_PTR)                             // 0x1103
	CASE(CL_MEM_MAP_COUNT)                            // 0x1104
	CASE(CL_MEM_REFERENCE_COUNT)                      // 0x1105
	CASE(CL_MEM_CONTEXT)                              // 0x1106
#ifdef CL_VERSION_1_1
	CASE(CL_MEM_ASSOCIATED_MEMOBJECT)                 // 0x1107
	CASE(CL_MEM_OFFSET)                               // 0x1108
#endif //CL_VERSION_1_1

/* cl_image_info */
	CASE(CL_IMAGE_FORMAT)                             // 0x1110
	CASE(CL_IMAGE_ELEMENT_SIZE)                       // 0x1111
	CASE(CL_IMAGE_ROW_PITCH)                          // 0x1112
	CASE(CL_IMAGE_SLICE_PITCH)                        // 0x1113
	CASE(CL_IMAGE_WIDTH)                              // 0x1114
	CASE(CL_IMAGE_HEIGHT)                             // 0x1115
	CASE(CL_IMAGE_DEPTH)                              // 0x1116
#ifdef CL_VERSION_1_2
	CASE(CL_IMAGE_ARRAY_SIZE)                         // 0x1117
	CASE(CL_IMAGE_BUFFER)                             // 0x1118
	CASE(CL_IMAGE_NUM_MIP_LEVELS)                     // 0x1119
	CASE(CL_IMAGE_NUM_SAMPLES)                        // 0x111A
#endif //CL_VERSION_1_2

/* cl_addressing_mode */
	CASE(CL_ADDRESS_NONE)                             // 0x1130
	CASE(CL_ADDRESS_CLAMP_TO_EDGE)                    // 0x1131
	CASE(CL_ADDRESS_CLAMP)                            // 0x1132
	CASE(CL_ADDRESS_REPEAT)                           // 0x1133
#ifdef CL_VERSION_1_1
	CASE(CL_ADDRESS_MIRRORED_REPEAT)                  // 0x1134
#endif //CL_VERSION_1_1

/* cl_filter_mode */
	CASE(CL_FILTER_NEAREST)                           // 0x1140
	CASE(CL_FILTER_LINEAR)                            // 0x1141

/* cl_sampler_info */
	CASE(CL_SAMPLER_REFERENCE_COUNT)                  // 0x1150
	CASE(CL_SAMPLER_CONTEXT)                          // 0x1151
	CASE(CL_SAMPLER_NORMALIZED_COORDS)                // 0x1152
	CASE(CL_SAMPLER_ADDRESSING_MODE)                  // 0x1153
	CASE(CL_SAMPLER_FILTER_MODE)                      // 0x1154

/* cl_map_flags - bitfield */
	/*
	CASE(CL_MAP_READ)                                 // (1 << 0)
	CASE(CL_MAP_WRITE)                                // (1 << 1)
	CASE(CL_MAP_WRITE_INVALIDATE_REGION)              // (1 << 2)
	*/

/* cl_program_info */
	CASE(CL_PROGRAM_REFERENCE_COUNT)                  // 0x1160
	CASE(CL_PROGRAM_CONTEXT)                          // 0x1161
	CASE(CL_PROGRAM_NUM_DEVICES)                      // 0x1162
	CASE(CL_PROGRAM_DEVICES)                          // 0x1163
	CASE(CL_PROGRAM_SOURCE)                           // 0x1164
	CASE(CL_PROGRAM_BINARY_SIZES)                     // 0x1165
	CASE(CL_PROGRAM_BINARIES)                         // 0x1166
#ifdef CL_VERSION_1_2
	CASE(CL_PROGRAM_NUM_KERNELS)                      // 0x1167
	CASE(CL_PROGRAM_KERNEL_NAMES)                     // 0x1168
#endif //CL_VERSION_1_2

/* cl_program_build_info */
	CASE(CL_PROGRAM_BUILD_STATUS)                     // 0x1181
	CASE(CL_PROGRAM_BUILD_OPTIONS)                    // 0x1182
	CASE(CL_PROGRAM_BUILD_LOG)                        // 0x1183
#ifdef CL_VERSION_1_2
	CASE(CL_PROGRAM_BINARY_TYPE)                      // 0x1184
#endif //CL_VERSION_1_2

/* cl_program_binary_type */
	/*
	CASE(CL_PROGRAM_BINARY_TYPE_NONE)                 // 0x0
	CASE(CL_PROGRAM_BINARY_TYPE_COMPILED_OBJECT)      // 0x1
	CASE(CL_PROGRAM_BINARY_TYPE_LIBRARY)              // 0x2
	CASE(CL_PROGRAM_BINARY_TYPE_EXECUTABLE)           // 0x4
	*/

/* cl_build_status */
	/*
	CASE(CL_BUILD_SUCCESS)                            // 0
	CASE(CL_BUILD_NONE)                               // -1
	CASE(CL_BUILD_ERROR)                              // -2
	CASE(CL_BUILD_IN_PROGRESS)                        // -3
	*/

/* cl_kernel_info */
	CASE(CL_KERNEL_FUNCTION_NAME)                     // 0x1190
	CASE(CL_KERNEL_NUM_ARGS)                          // 0x1191
	CASE(CL_KERNEL_REFERENCE_COUNT)                   // 0x1192
	CASE(CL_KERNEL_CONTEXT)                           // 0x1193
	CASE(CL_KERNEL_PROGRAM)                           // 0x1194
#ifdef CL_VERSION_1_2
	CASE(CL_KERNEL_ATTRIBUTES)                        // 0x1195
#endif //CL_VERSION_1_2

/* cl_kernel_arg_info */
#ifdef CL_VERSION_1_2
	CASE(CL_KERNEL_ARG_ADDRESS_QUALIFIER)             // 0x1196
	CASE(CL_KERNEL_ARG_ACCESS_QUALIFIER)              // 0x1197
	CASE(CL_KERNEL_ARG_TYPE_NAME)                     // 0x1198
	CASE(CL_KERNEL_ARG_TYPE_QUALIFIER)                // 0x1199
	CASE(CL_KERNEL_ARG_NAME)                          // 0x119A
#endif //CL_VERSION_1_2

/* cl_kernel_arg_address_qualifier */
#ifdef CL_VERSION_1_2
	CASE(CL_KERNEL_ARG_ADDRESS_GLOBAL)                // 0x119B
	CASE(CL_KERNEL_ARG_ADDRESS_LOCAL)                 // 0x119C
	CASE(CL_KERNEL_ARG_ADDRESS_CONSTANT)              // 0x119D
	CASE(CL_KERNEL_ARG_ADDRESS_PRIVATE)               // 0x119E
#endif //CL_VERSION_1_2

/* cl_kernel_arg_access_qualifier */
#ifdef CL_VERSION_1_2
	CASE(CL_KERNEL_ARG_ACCESS_READ_ONLY)              // 0x11A0
	CASE(CL_KERNEL_ARG_ACCESS_WRITE_ONLY)             // 0x11A1
	CASE(CL_KERNEL_ARG_ACCESS_READ_WRITE)             // 0x11A2
	CASE(CL_KERNEL_ARG_ACCESS_NONE)                   // 0x11A3
#endif //CL_VERSION_1_2

/* cl_kernel_arg_type_qualifer */
	/*
	CASE(CL_KERNEL_ARG_TYPE_NONE)                     // 0
	CASE(CL_KERNEL_ARG_TYPE_CONST)                    // (1 << 0)
	CASE(CL_KERNEL_ARG_TYPE_RESTRICT)                 // (1 << 1)
	CASE(CL_KERNEL_ARG_TYPE_VOLATILE)                 // (1 << 2)
	*/

/* cl_kernel_work_group_info */
	CASE(CL_KERNEL_WORK_GROUP_SIZE)                   // 0x11B0
	CASE(CL_KERNEL_COMPILE_WORK_GROUP_SIZE)           // 0x11B1
	CASE(CL_KERNEL_LOCAL_MEM_SIZE)                    // 0x11B2
#ifdef CL_VERSION_1_1
	CASE(CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE) // 0x11B3
	CASE(CL_KERNEL_PRIVATE_MEM_SIZE)                  // 0x11B4
#endif //CL_VERSION_1_1
#ifdef CL_VERSION_1_2
	CASE(CL_KERNEL_GLOBAL_WORK_SIZE)                  // 0x11B5
#endif //CL_VERSION_1_2

/* cl_event_info  */
	CASE(CL_EVENT_COMMAND_QUEUE)                      // 0x11D0
	CASE(CL_EVENT_COMMAND_TYPE)                       // 0x11D1
	CASE(CL_EVENT_REFERENCE_COUNT)                    // 0x11D2
	CASE(CL_EVENT_COMMAND_EXECUTION_STATUS)           // 0x11D3
#ifdef CL_VERSION_1_1
	CASE(CL_EVENT_CONTEXT)                            // 0x11D4
#endif //CL_VERSION_1_1

/* cl_command_type */
	CASE(CL_COMMAND_NDRANGE_KERNEL)                   // 0x11F0
	CASE(CL_COMMAND_TASK)                             // 0x11F1
	CASE(CL_COMMAND_NATIVE_KERNEL)                    // 0x11F2
	CASE(CL_COMMAND_READ_BUFFER)                      // 0x11F3
	CASE(CL_COMMAND_WRITE_BUFFER)                     // 0x11F4
	CASE(CL_COMMAND_COPY_BUFFER)                      // 0x11F5
	CASE(CL_COMMAND_READ_IMAGE)                       // 0x11F6
	CASE(CL_COMMAND_WRITE_IMAGE)                      // 0x11F7
	CASE(CL_COMMAND_COPY_IMAGE)                       // 0x11F8
	CASE(CL_COMMAND_COPY_IMAGE_TO_BUFFER)             // 0x11F9
	CASE(CL_COMMAND_COPY_BUFFER_TO_IMAGE)             // 0x11FA
	CASE(CL_COMMAND_MAP_BUFFER)                       // 0x11FB
	CASE(CL_COMMAND_MAP_IMAGE)                        // 0x11FC
	CASE(CL_COMMAND_UNMAP_MEM_OBJECT)                 // 0x11FD
	CASE(CL_COMMAND_MARKER)                           // 0x11FE
	CASE(CL_COMMAND_ACQUIRE_GL_OBJECTS)               // 0x11FF
	CASE(CL_COMMAND_RELEASE_GL_OBJECTS)               // 0x1200
#ifdef CL_VERSION_1_1
	CASE(CL_COMMAND_READ_BUFFER_RECT)                 // 0x1201
	CASE(CL_COMMAND_WRITE_BUFFER_RECT)                // 0x1202
	CASE(CL_COMMAND_COPY_BUFFER_RECT)                 // 0x1203
	CASE(CL_COMMAND_USER)                             // 0x1204
#endif //CL_VERSION_1_1
#ifdef CL_VERSION_1_2
	CASE(CL_COMMAND_BARRIER)                          // 0x1205
	CASE(CL_COMMAND_MIGRATE_MEM_OBJECTS)              // 0x1206
	CASE(CL_COMMAND_FILL_BUFFER)                      // 0x1207
	CASE(CL_COMMAND_FILL_IMAGE)                       // 0x1208
#endif //CL_VERSION_1_2

/* command execution status */
	/*
	CASE(CL_COMPLETE)                                 // 0x0
	CASE(CL_RUNNING)                                  // 0x1
	CASE(CL_SUBMITTED)                                // 0x2
	CASE(CL_QUEUED)                                   // 0x3
	*/

/* cl_buffer_create_type  */
#ifdef CL_VERSION_1_1
	CASE(CL_BUFFER_CREATE_TYPE_REGION)                // 0x1220
#endif //CL_VERSION_1_1

/* cl_profiling_info  */
	CASE(CL_PROFILING_COMMAND_QUEUED)                 // 0x1280
	CASE(CL_PROFILING_COMMAND_SUBMIT)                 // 0x1281
	CASE(CL_PROFILING_COMMAND_START)                  // 0x1282
	CASE(CL_PROFILING_COMMAND_END)                    // 0x1283

	default:
		return "(unrecognized enum)";
	}

#undef CASE
}

const char* piglit_cl_get_error_name(cl_int error) {
#define CASE(x) case x: return #x;

    switch (error) {
	CASE(CL_SUCCESS)                                  // 0
	CASE(CL_DEVICE_NOT_FOUND)                         // -1
	CASE(CL_DEVICE_NOT_AVAILABLE)                     // -2
	CASE(CL_COMPILER_NOT_AVAILABLE)                   // -3
	CASE(CL_MEM_OBJECT_ALLOCATION_FAILURE)            // -4
	CASE(CL_OUT_OF_RESOURCES)                         // -5
	CASE(CL_OUT_OF_HOST_MEMORY)                       // -6
	CASE(CL_PROFILING_INFO_NOT_AVAILABLE)             // -7
	CASE(CL_MEM_COPY_OVERLAP)                         // -8
	CASE(CL_IMAGE_FORMAT_MISMATCH)                    // -9
	CASE(CL_IMAGE_FORMAT_NOT_SUPPORTED)               // -10
	CASE(CL_BUILD_PROGRAM_FAILURE)                    // -11
	CASE(CL_MAP_FAILURE)                              // -12
#ifdef CL_VERSION_1_1
	CASE(CL_MISALIGNED_SUB_BUFFER_OFFSET)             // -13
	CASE(CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST) // -14
#endif //CL_VERSION_1_1
#ifdef CL_VERSION_1_2
	CASE(CL_COMPILE_PROGRAM_FAILURE)                  // -15
	CASE(CL_LINKER_NOT_AVAILABLE)                     // -16
	CASE(CL_LINK_PROGRAM_FAILURE)                     // -17
	CASE(CL_DEVICE_PARTITION_FAILED)                  // -18
	CASE(CL_KERNEL_ARG_INFO_NOT_AVAILABLE)            // -19
#endif //CL_VERSION_1_2

	CASE(CL_INVALID_VALUE)                            // -30
	CASE(CL_INVALID_DEVICE_TYPE)                      // -31
	CASE(CL_INVALID_PLATFORM)                         // -32
	CASE(CL_INVALID_DEVICE)                           // -33
	CASE(CL_INVALID_CONTEXT)                          // -34
	CASE(CL_INVALID_QUEUE_PROPERTIES)                 // -35
	CASE(CL_INVALID_COMMAND_QUEUE)                    // -36
	CASE(CL_INVALID_HOST_PTR)                         // -37
	CASE(CL_INVALID_MEM_OBJECT)                       // -38
	CASE(CL_INVALID_IMAGE_FORMAT_DESCRIPTOR)          // -39
	CASE(CL_INVALID_IMAGE_SIZE)                       // -40
	CASE(CL_INVALID_SAMPLER)                          // -41
	CASE(CL_INVALID_BINARY)                           // -42
	CASE(CL_INVALID_BUILD_OPTIONS)                    // -43
	CASE(CL_INVALID_PROGRAM)                          // -44
	CASE(CL_INVALID_PROGRAM_EXECUTABLE)               // -45
	CASE(CL_INVALID_KERNEL_NAME)                      // -46
	CASE(CL_INVALID_KERNEL_DEFINITION)                // -47
	CASE(CL_INVALID_KERNEL)                           // -48
	CASE(CL_INVALID_ARG_INDEX)                        // -49
	CASE(CL_INVALID_ARG_VALUE)                        // -50
	CASE(CL_INVALID_ARG_SIZE)                         // -51
	CASE(CL_INVALID_KERNEL_ARGS)                      // -52
	CASE(CL_INVALID_WORK_DIMENSION)                   // -53
	CASE(CL_INVALID_WORK_GROUP_SIZE)                  // -54
	CASE(CL_INVALID_WORK_ITEM_SIZE)                   // -55
	CASE(CL_INVALID_GLOBAL_OFFSET)                    // -56
	CASE(CL_INVALID_EVENT_WAIT_LIST)                  // -57
	CASE(CL_INVALID_EVENT)                            // -58
	CASE(CL_INVALID_OPERATION)                        // -59
	CASE(CL_INVALID_GL_OBJECT)                        // -60
	CASE(CL_INVALID_BUFFER_SIZE)                      // -61
	CASE(CL_INVALID_MIP_LEVEL)                        // -62
	CASE(CL_INVALID_GLOBAL_WORK_SIZE)                 // -63
#ifdef CL_VERSION_1_1
	CASE(CL_INVALID_PROPERTY)                         // -64
#endif //CL_VERSION_1_1
#ifdef CL_VERSION_1_2
	CASE(CL_INVALID_IMAGE_DESCRIPTOR)                 // -65
	CASE(CL_INVALID_COMPILER_OPTIONS)                 // -66
	CASE(CL_INVALID_LINKER_OPTIONS)                   // -67
	CASE(CL_INVALID_DEVICE_PARTITION_COUNT)           // -68
#endif //CL_VERSION_1_2

    default:
        return "(unrecognized error)";
    }

#undef CASE
}


#define PIGLIT_CL_DEFINE_ENUM(type, name, count10, count11, count12)  \
        const unsigned int piglit_##name##_num_1_0 = count10;         \
        const unsigned int piglit_##name##_num_1_1 = count11;         \
        const unsigned int piglit_##name##_num_1_2 = count12;         \
        const type _piglit_##name[]

#define PIGLIT_CL_DEFINE_ENUM_2(name, count10, count11, count12)      \
        PIGLIT_CL_DEFINE_ENUM(name, name, count10, count11, count12)

#define PIGLIT_CL_DEFINE_ENUM_PTR(type, name)                         \
        const type* piglit_##name = _piglit_##name;

#define PIGLIT_CL_DEFINE_ENUM_PTR_2(name)                             \
        PIGLIT_CL_DEFINE_ENUM_PTR(name, name)


PIGLIT_CL_DEFINE_ENUM_2(cl_mem_flags, 6, 6, 9) = {
	CL_MEM_READ_WRITE,
	CL_MEM_READ_ONLY,
	CL_MEM_WRITE_ONLY,
	CL_MEM_USE_HOST_PTR,
	CL_MEM_ALLOC_HOST_PTR, // 5
	CL_MEM_COPY_HOST_PTR,
#if defined(CL_VERSION_1_2)
	CL_MEM_HOST_WRITE_ONLY,
	CL_MEM_HOST_READ_ONLY,
	CL_MEM_HOST_NO_ACCESS,
#endif //CL_VERSION_1_2
};
PIGLIT_CL_DEFINE_ENUM_PTR_2(cl_mem_flags);

PIGLIT_CL_DEFINE_ENUM_2(cl_device_type, 5, 5, 6) = {
	CL_DEVICE_TYPE_CPU,
	CL_DEVICE_TYPE_GPU,
	CL_DEVICE_TYPE_ACCELERATOR,
	CL_DEVICE_TYPE_DEFAULT,
	CL_DEVICE_TYPE_ALL, // 5
#if defined(CL_VERSION_1_2)
	CL_DEVICE_TYPE_CUSTOM,
#endif
};
PIGLIT_CL_DEFINE_ENUM_PTR_2(cl_device_type);

PIGLIT_CL_DEFINE_ENUM_2(cl_program_info, 7, 7, 9) = {
	CL_PROGRAM_REFERENCE_COUNT,
	CL_PROGRAM_CONTEXT,
	CL_PROGRAM_NUM_DEVICES,
	CL_PROGRAM_DEVICES,
	CL_PROGRAM_SOURCE, // 5
	CL_PROGRAM_BINARY_SIZES,
	CL_PROGRAM_BINARIES,
#ifdef CL_VERSION_1_2
	CL_PROGRAM_NUM_KERNELS,
	CL_PROGRAM_KERNEL_NAMES,
#endif //CL_VERSION_1_2
};
PIGLIT_CL_DEFINE_ENUM_PTR_2(cl_program_info);

PIGLIT_CL_DEFINE_ENUM_2(cl_program_build_info, 3, 3, 4) = {
	CL_PROGRAM_BUILD_STATUS,
	CL_PROGRAM_BUILD_OPTIONS,
	CL_PROGRAM_BUILD_LOG,
#ifdef CL_VERSION_1_2
	CL_PROGRAM_BINARY_TYPE,
#endif //CL_VERSION_1_2
};
PIGLIT_CL_DEFINE_ENUM_PTR_2(cl_program_build_info);

PIGLIT_CL_DEFINE_ENUM_2(cl_mem_info, 7, 9, 9) = {
	CL_MEM_TYPE,
	CL_MEM_FLAGS,
	CL_MEM_SIZE,
	CL_MEM_HOST_PTR,
	CL_MEM_MAP_COUNT, // 5
	CL_MEM_REFERENCE_COUNT,
	CL_MEM_CONTEXT,
#ifdef CL_VERSION_1_1
	CL_MEM_ASSOCIATED_MEMOBJECT,
	CL_MEM_OFFSET,
#endif //CL_VERSION_1_1
};
PIGLIT_CL_DEFINE_ENUM_PTR_2(cl_mem_info);

PIGLIT_CL_DEFINE_ENUM_2(cl_kernel_info, 5, 5, 6) = {
	CL_KERNEL_FUNCTION_NAME,
	CL_KERNEL_NUM_ARGS,
	CL_KERNEL_REFERENCE_COUNT,
	CL_KERNEL_CONTEXT,
	CL_KERNEL_PROGRAM, // 5
#ifdef CL_VERSION_1_2
	CL_KERNEL_ATTRIBUTES,
#endif //CL_VERSION_1_2
};
PIGLIT_CL_DEFINE_ENUM_PTR_2(cl_kernel_info);

PIGLIT_CL_DEFINE_ENUM_2(cl_kernel_work_group_info, 3, 5, 6) = {
	CL_KERNEL_WORK_GROUP_SIZE,
	CL_KERNEL_COMPILE_WORK_GROUP_SIZE,
	CL_KERNEL_LOCAL_MEM_SIZE,
#ifdef CL_VERSION_1_1
	CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE,
	CL_KERNEL_PRIVATE_MEM_SIZE, // 5
#endif //CL_VERSION_1_1
#ifdef CL_VERSION_1_2
	CL_KERNEL_GLOBAL_WORK_SIZE,
#endif //CL_VERSION_1_2
};
PIGLIT_CL_DEFINE_ENUM_PTR_2(cl_kernel_work_group_info);

PIGLIT_CL_DEFINE_ENUM_2(cl_event_info, 4, 5, 5) = {
	CL_EVENT_COMMAND_QUEUE,
	CL_EVENT_COMMAND_TYPE,
	CL_EVENT_REFERENCE_COUNT,
	CL_EVENT_COMMAND_EXECUTION_STATUS,
#ifdef CL_VERSION_1_1
	CL_EVENT_CONTEXT, // 5
#endif //CL_VERSION_1_1
};
PIGLIT_CL_DEFINE_ENUM_PTR_2(cl_event_info);

PIGLIT_CL_DEFINE_ENUM_2(cl_image_info, 7, 7, 11) = {
	CL_IMAGE_FORMAT,
	CL_IMAGE_ELEMENT_SIZE,
	CL_IMAGE_ROW_PITCH,
	CL_IMAGE_SLICE_PITCH,
	CL_IMAGE_WIDTH, // 5
	CL_IMAGE_HEIGHT,
	CL_IMAGE_DEPTH,
#ifdef CL_VERSION_1_2
	CL_IMAGE_ARRAY_SIZE,
	CL_IMAGE_BUFFER,
	CL_IMAGE_NUM_MIP_LEVELS, // 10
	CL_IMAGE_NUM_SAMPLES,
#endif //CL_VERSION_1_2
};
PIGLIT_CL_DEFINE_ENUM_PTR_2(cl_image_info);

PIGLIT_CL_DEFINE_ENUM_2(cl_command_queue_info, 4, 4, 4) = {
	CL_QUEUE_CONTEXT,
	CL_QUEUE_DEVICE,
	CL_QUEUE_REFERENCE_COUNT,
	CL_QUEUE_PROPERTIES,
};
PIGLIT_CL_DEFINE_ENUM_PTR_2(cl_command_queue_info);

PIGLIT_CL_DEFINE_ENUM_2(cl_context_info, 3, 4, 4) = {
	CL_CONTEXT_REFERENCE_COUNT,
	CL_CONTEXT_DEVICES,
	CL_CONTEXT_PROPERTIES,
#if defined(CL_VERSION_1_1)
	CL_CONTEXT_NUM_DEVICES,
#endif //CL_VERSION_1_1
};
PIGLIT_CL_DEFINE_ENUM_PTR_2(cl_context_info);

PIGLIT_CL_DEFINE_ENUM_2(cl_platform_info, 5, 5, 5) = {
	CL_PLATFORM_PROFILE,
	CL_PLATFORM_VERSION,
	CL_PLATFORM_NAME,
	CL_PLATFORM_VENDOR,
	CL_PLATFORM_EXTENSIONS // 5
};
PIGLIT_CL_DEFINE_ENUM_PTR_2(cl_platform_info);

PIGLIT_CL_DEFINE_ENUM_2(cl_device_info, 50, 60, 73) = {
	CL_DEVICE_TYPE,
	CL_DEVICE_VENDOR_ID,
	CL_DEVICE_MAX_COMPUTE_UNITS,
	CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS,
	CL_DEVICE_MAX_WORK_GROUP_SIZE, // 5
	CL_DEVICE_MAX_WORK_ITEM_SIZES,
	CL_DEVICE_PREFERRED_VECTOR_WIDTH_CHAR,
	CL_DEVICE_PREFERRED_VECTOR_WIDTH_SHORT,
	CL_DEVICE_PREFERRED_VECTOR_WIDTH_INT,
	CL_DEVICE_PREFERRED_VECTOR_WIDTH_LONG, // 10
	CL_DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT,
	CL_DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE,
	CL_DEVICE_MAX_CLOCK_FREQUENCY,
	CL_DEVICE_ADDRESS_BITS,
	CL_DEVICE_MAX_READ_IMAGE_ARGS, // 15
	CL_DEVICE_MAX_WRITE_IMAGE_ARGS,
	CL_DEVICE_MAX_MEM_ALLOC_SIZE,
	CL_DEVICE_IMAGE2D_MAX_WIDTH,
	CL_DEVICE_IMAGE2D_MAX_HEIGHT,
	CL_DEVICE_IMAGE3D_MAX_WIDTH, // 20
	CL_DEVICE_IMAGE3D_MAX_HEIGHT,
	CL_DEVICE_IMAGE3D_MAX_DEPTH,
	CL_DEVICE_IMAGE_SUPPORT,
	CL_DEVICE_MAX_PARAMETER_SIZE,
	CL_DEVICE_MAX_SAMPLERS, // 25
	CL_DEVICE_MEM_BASE_ADDR_ALIGN,
	CL_DEVICE_MIN_DATA_TYPE_ALIGN_SIZE,
	CL_DEVICE_SINGLE_FP_CONFIG,
	CL_DEVICE_GLOBAL_MEM_CACHE_TYPE,
	CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE, // 30
	CL_DEVICE_GLOBAL_MEM_CACHE_SIZE,
	CL_DEVICE_GLOBAL_MEM_SIZE,
	CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE,
	CL_DEVICE_MAX_CONSTANT_ARGS,
	CL_DEVICE_LOCAL_MEM_TYPE, // 35
	CL_DEVICE_LOCAL_MEM_SIZE,
	CL_DEVICE_ERROR_CORRECTION_SUPPORT,
	CL_DEVICE_PROFILING_TIMER_RESOLUTION,
	CL_DEVICE_ENDIAN_LITTLE,
	CL_DEVICE_AVAILABLE, // 40
	CL_DEVICE_COMPILER_AVAILABLE,
	CL_DEVICE_EXECUTION_CAPABILITIES,
	CL_DEVICE_QUEUE_PROPERTIES,
	CL_DEVICE_NAME,
	CL_DEVICE_VENDOR, // 45
	CL_DRIVER_VERSION,
	CL_DEVICE_PROFILE,
	CL_DEVICE_VERSION,
	CL_DEVICE_EXTENSIONS,
	CL_DEVICE_PLATFORM, // 50
#if defined(CL_VERSION_1_1)
	CL_DEVICE_PREFERRED_VECTOR_WIDTH_HALF,
	CL_DEVICE_HOST_UNIFIED_MEMORY,
	CL_DEVICE_NATIVE_VECTOR_WIDTH_CHAR,
	CL_DEVICE_NATIVE_VECTOR_WIDTH_SHORT,
	CL_DEVICE_NATIVE_VECTOR_WIDTH_INT, // 55
	CL_DEVICE_NATIVE_VECTOR_WIDTH_LONG,
	CL_DEVICE_NATIVE_VECTOR_WIDTH_FLOAT,
	CL_DEVICE_NATIVE_VECTOR_WIDTH_DOUBLE,
	CL_DEVICE_NATIVE_VECTOR_WIDTH_HALF,
	CL_DEVICE_OPENCL_C_VERSION, // 60
#endif //CL_VERSION_1_1
#if defined(CL_VERSION_1_2)
	CL_DEVICE_DOUBLE_FP_CONFIG,
	CL_DEVICE_LINKER_AVAILABLE,
	CL_DEVICE_BUILT_IN_KERNELS,
	CL_DEVICE_IMAGE_MAX_BUFFER_SIZE,
	CL_DEVICE_IMAGE_MAX_ARRAY_SIZE, // 65
	CL_DEVICE_PARENT_DEVICE,
	CL_DEVICE_PARTITION_MAX_SUB_DEVICES,
	CL_DEVICE_PARTITION_PROPERTIES,
	CL_DEVICE_PARTITION_AFFINITY_DOMAIN,
	CL_DEVICE_PARTITION_TYPE, // 70
	CL_DEVICE_REFERENCE_COUNT,
	CL_DEVICE_PREFERRED_INTEROP_USER_SYNC,
	CL_DEVICE_PRINTF_BUFFER_SIZE, // 73
#endif //CL_VERSION_1_2
};
PIGLIT_CL_DEFINE_ENUM_PTR_2(cl_device_info);

PIGLIT_CL_DEFINE_ENUM_2(cl_command_queue_properties, 2, 2, 2) = {
	CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE,
	CL_QUEUE_PROFILING_ENABLE
};
PIGLIT_CL_DEFINE_ENUM_PTR_2(cl_command_queue_properties);


PIGLIT_CL_DEFINE_ENUM(cl_mem_flags, cl_mem_flags_mutexes, 5, 5, 8) = {
	CL_MEM_READ_WRITE | CL_MEM_READ_ONLY,
	CL_MEM_READ_WRITE | CL_MEM_WRITE_ONLY,
	CL_MEM_READ_ONLY | CL_MEM_WRITE_ONLY,
	CL_MEM_ALLOC_HOST_PTR | CL_MEM_USE_HOST_PTR,
	CL_MEM_COPY_HOST_PTR | CL_MEM_USE_HOST_PTR, // 5
#if defined(CL_VERSION_1_2)
	CL_MEM_HOST_WRITE_ONLY | CL_MEM_HOST_READ_ONLY,
	CL_MEM_HOST_WRITE_ONLY | CL_MEM_HOST_NO_ACCESS,
	CL_MEM_HOST_READ_ONLY | CL_MEM_HOST_NO_ACCESS,
#endif //CL_VERSION_1_2
};
PIGLIT_CL_DEFINE_ENUM_PTR(cl_mem_flags, cl_mem_flags_mutexes);



#undef PIGLIT_CL_DEFINE_ENUM
#undef PIGLIT_CL_DEFINE_ENUM_2
#undef PIGLIT_CL_DEFINE_ENUM_PTR
#undef PIGLIT_CL_DEFINE_ENUM_PTR_2
