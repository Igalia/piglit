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
#ifndef PIGLIT_UTIL_CL_ENUM_H
#define PIGLIT_UTIL_CL_ENUM_H

#define CL_USE_DEPRECATED_OPENCL_1_0_APIS
#define CL_USE_DEPRECATED_OPENCL_1_1_APIS
#define CL_USE_DEPRECATED_OPENCL_1_2_APIS
#define CL_USE_DEPRECATED_OPENCL_2_0_APIS

#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/opencl.h>
#endif
/**
 * \brief Convert a CL enum to a string.
 *
 * For example, given CL_PLATFORM_NAME, return "CL_PLATFORM_NAME".
 *
 * Return "(unrecognized enum)" if the enum is not recognized.
 *
 * \warning Not all enums are recognized. Bitfield and non-unique
 * enums will return "(unrecognized enum)"
 */
const char *piglit_cl_get_enum_name(cl_uint param);

/**
 * \brief Convert a CL error to a string.
 *
 * For example, given CL_DEVICE_NOT_FOUND, return "CL_DEVICE_NOT_FOUND".
 *
 * Return "(unrecognized error)" if the error is not recognized.
 */
const char* piglit_cl_get_error_name(cl_int error);


#define PIGLIT_CL_ENUM_NUM(name, version)   \
        version == 10 ?                     \
            piglit_##name##_num_1_0 :       \
        version == 11 ?                     \
            piglit_##name##_num_1_1 :       \
        version == 12 ?                     \
            piglit_##name##_num_1_2 :       \
        version == 20 ?                     \
            piglit_##name##_num_2_0 :       \
            0

#define PIGLIT_CL_ENUM_ARRAY(name)          \
        piglit_##name


#define PIGLIT_CL_DEFINE_ENUM_PROTOTYPE(type, name)    \
        extern const unsigned int piglit_##name##_num_1_0;    \
        extern const unsigned int piglit_##name##_num_1_1;    \
        extern const unsigned int piglit_##name##_num_1_2;    \
        extern const unsigned int piglit_##name##_num_2_0;    \
        extern const type* piglit_##name;

#define PIGLIT_CL_DEFINE_ENUM_PROTOTYPE_2(name)        \
        PIGLIT_CL_DEFINE_ENUM_PROTOTYPE(name, name)


PIGLIT_CL_DEFINE_ENUM_PROTOTYPE_2(cl_mem_flags);
PIGLIT_CL_DEFINE_ENUM_PROTOTYPE_2(cl_device_type);
PIGLIT_CL_DEFINE_ENUM_PROTOTYPE_2(cl_program_info);
PIGLIT_CL_DEFINE_ENUM_PROTOTYPE_2(cl_program_build_info);
PIGLIT_CL_DEFINE_ENUM_PROTOTYPE_2(cl_mem_info);
PIGLIT_CL_DEFINE_ENUM_PROTOTYPE_2(cl_kernel_info);
#ifdef CL_VERSION_1_2
PIGLIT_CL_DEFINE_ENUM_PROTOTYPE_2(cl_kernel_arg_info);
#endif
PIGLIT_CL_DEFINE_ENUM_PROTOTYPE_2(cl_kernel_work_group_info);
PIGLIT_CL_DEFINE_ENUM_PROTOTYPE_2(cl_event_info);
PIGLIT_CL_DEFINE_ENUM_PROTOTYPE_2(cl_image_info);
PIGLIT_CL_DEFINE_ENUM_PROTOTYPE_2(cl_command_queue_info);
PIGLIT_CL_DEFINE_ENUM_PROTOTYPE_2(cl_context_info);
PIGLIT_CL_DEFINE_ENUM_PROTOTYPE_2(cl_platform_info);
PIGLIT_CL_DEFINE_ENUM_PROTOTYPE_2(cl_device_info);
PIGLIT_CL_DEFINE_ENUM_PROTOTYPE_2(cl_command_queue_properties);

PIGLIT_CL_DEFINE_ENUM_PROTOTYPE(cl_mem_flags, cl_mem_flags_mutexes);
PIGLIT_CL_DEFINE_ENUM_PROTOTYPE(cl_command_queue_properties, cl_command_queue_properties_mutexes);

#undef PIGLIT_CL_DEFINE_ENUM_PROTOTYPE
#undef PIGLIT_CL_DEFINE_ENUM_PROTOTYPE_2

#endif //PIGLIT_UTIL_CL_ENUM_H
