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
 * @file get-device-info.c
 *
 * Test API function:
 *
 *   cl_int clGetDeviceInfo(cl_device_id device,
 *                          cl_device_info param_name,
 *                          size_t param_value_size,
 *                          void *param_value,
 *                          size_t *param_value_size_ret)
 */

#include "piglit-framework-cl-api.h"


PIGLIT_CL_API_TEST_CONFIG_BEGIN

	config.name = "clGetDeviceInfo";
	config.version_min = 10;

	config.run_per_device = true;

PIGLIT_CL_API_TEST_CONFIG_END

struct device_config {
	cl_uint max_unit;
	cl_ulong mem_size;
	bool is_full_profile;
	cl_device_type type;
	bool has_double;
	bool has_image;
} device_config_t;

static bool
check_size(size_t expected_size, size_t actual_size, enum piglit_result *result) {
	if (expected_size != actual_size) {
		printf(": failed, expected and actual size differ. Expect %lu, got %lu",
		       expected_size, actual_size);
		piglit_merge_result(result, PIGLIT_FAIL);
		return false;
	}

	return true;
}

static bool
check_fp_config(cl_device_info kind, cl_device_fp_config value,
		enum piglit_result *result) {
	cl_device_fp_config allowed_flags  = CL_FP_DENORM|
	              CL_FP_INF_NAN|CL_FP_ROUND_TO_NEAREST|
	              CL_FP_ROUND_TO_ZERO|CL_FP_ROUND_TO_INF|
	              CL_FP_FMA|CL_FP_SOFT_FLOAT;

	if (kind == CL_DEVICE_SINGLE_FP_CONFIG) {
		allowed_flags |= CL_FP_CORRECTLY_ROUNDED_DIVIDE_SQRT;
	}

	if (value & ~(allowed_flags)) {
		printf(": failed, expected a combination of CL_FP_*. Got %lx", value);
		piglit_merge_result(result, PIGLIT_FAIL);
		printf("\n %lx", value);
		return false;
	}

	return true;
}

static bool
check_string_not_empty(size_t actual_size, char* actual_value, enum piglit_result *result) {
	if (actual_size && !actual_value) {
		printf(": failed, actual size is %zu, but string is NULL", actual_size);
		piglit_merge_result(result, PIGLIT_FAIL);
		return false;
	}

	if (!actual_size || !actual_value || actual_value[0] == '\0') {
		printf(": failed, string should not be empty");
		piglit_merge_result(result, PIGLIT_FAIL);
		return false;
	}

	return true;
}

static bool
check_sub_string(char* expected_sub, char* actual_value, enum piglit_result *result) {
	if (strstr(actual_value, expected_sub) == 0) {
		printf(": failed, '%s' should contains '%s'", actual_value, expected_sub);
		piglit_merge_result(result, PIGLIT_FAIL);
		return false;
	}

	return true;
}

static bool
check_min_int(size_t expected, size_t actual_value, enum piglit_result *result) {
	if (actual_value < expected) {
		printf(": failed, expected at least %lu, got %lu ", expected, actual_value);
		piglit_merge_result(result, PIGLIT_FAIL);
		return false;
	}

	return true;
}

static bool
check_max_int(size_t expected, size_t actual_value, enum piglit_result *result) {
	if (actual_value > expected) {
		printf(": failed, expected at most %lu, got %lu ", expected, actual_value);
		piglit_merge_result(result, PIGLIT_FAIL);
		return false;
	}

	return true;
}

static void
print_s(const char* str) {
	if (str) {
		printf(": '%s'", str);
	} else {
		printf(": (null)");
	}
}

static void
print_u(size_t i) {
	printf(": %zu", i);
}

static void
print_b(cl_bool b) {
	if (b) {
		printf(": CL_TRUE");
	} else {
		printf(": CL_FALSE");
	}
}

static void
check_info(const struct piglit_cl_api_test_env* env,
	   struct device_config* device_config,
	   cl_device_info kind, void* param_value, size_t param_value_size,
	   enum piglit_result *result) {
	bool valid = true;

	switch (kind) {
		case CL_DEVICE_NAME:
		case CL_DEVICE_VENDOR:
		case CL_DRIVER_VERSION:
		case CL_DEVICE_BUILT_IN_KERNELS:
			print_s(param_value);
			break;
		case CL_DEVICE_PROFILE:
			if (check_string_not_empty(param_value_size, param_value, result)) {
				if (strcmp("FULL_PROFILE", param_value) &&
			            strcmp("EMBEDDED_PROFILE", param_value)) {
					printf(": failed, expected and actual string differ. Expect '%s', got '%s'",
					"FULL_PROFILE or EMBEDDED_PROFILE", (char*)param_value);
					piglit_merge_result(result, PIGLIT_FAIL);
				} else {
					device_config->is_full_profile = strcmp("FULL_PROFILE", param_value) == 0;
					print_s(param_value);
				}
			}
			break;
		case CL_DEVICE_VERSION:
			if (check_string_not_empty(param_value_size, param_value, result) &&
			    ((env->version == 12 && check_sub_string("OpenCL 1.2", param_value, result)) ||
			      check_sub_string("OpenCL 1.1", param_value, result))) {
				print_s(param_value);
			}
			break;
		case CL_DEVICE_OPENCL_C_VERSION:
			if (check_string_not_empty(param_value_size, param_value, result) &&
			    ((env->version == 12 && check_sub_string("OpenCL C 1.2", param_value, result)) ||
			      check_sub_string("OpenCL C 1.1", param_value, result))) {
				print_s(param_value);
			}
			break;
		case CL_DEVICE_IMAGE_SUPPORT:
			if (check_size(sizeof(cl_bool), param_value_size, result)) {
				device_config->has_image = *(cl_bool*)param_value;
				print_b(*(cl_bool*)param_value);
			}
			break;
		case CL_DEVICE_EXTENSIONS:
			if (env->version == 12) {
				valid = check_sub_string("cl_khr_global_int32_base_atomics", param_value, result);
				valid &= check_sub_string("cl_khr_global_int32_extended_atomics", param_value, result);
				valid &= check_sub_string("cl_khr_local_int32_base_atomics", param_value, result);
				valid &= check_sub_string("cl_khr_local_int32_extended_atomics", param_value, result);
				valid &= check_sub_string("cl_khr_byte_addressable_store", param_value, result);
				if (device_config->has_double) {
					valid &= check_sub_string("cl_khr_fp64", param_value, result);
				}
			}

			if (valid) {
				print_s(param_value);
			}
			break;
		case CL_DEVICE_TYPE:
			if (check_size(sizeof(cl_device_type), param_value_size, result)) {
				valid = false;

				int num_type = PIGLIT_CL_ENUM_NUM(cl_device_type, env->version);
				const cl_device_type *type = PIGLIT_CL_ENUM_ARRAY(cl_device_type);
				for(int i = 0; i < num_type; i++) {
					valid |= type[i] & *(cl_device_type*)param_value;
				}

				if (!valid) {
					printf(": failed, unexpected value");
					piglit_merge_result(result, PIGLIT_FAIL);
				}
			}
			break;
		case CL_DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE:
			if (check_size(sizeof(cl_uint), param_value_size, result)) {
				if (*(cl_uint*)param_value) {
					device_config->has_double = true;
				}
				print_u(*(cl_uint*)param_value);
			}
			break;
		case CL_DEVICE_VENDOR_ID:
		case CL_DEVICE_ADDRESS_BITS:
		case CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS:
		case CL_DEVICE_MIN_DATA_TYPE_ALIGN_SIZE:
		case CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE:
		case CL_DEVICE_NATIVE_VECTOR_WIDTH_CHAR:
		case CL_DEVICE_NATIVE_VECTOR_WIDTH_SHORT:
		case CL_DEVICE_NATIVE_VECTOR_WIDTH_INT:
		case CL_DEVICE_NATIVE_VECTOR_WIDTH_LONG:
		case CL_DEVICE_NATIVE_VECTOR_WIDTH_HALF:
		case CL_DEVICE_NATIVE_VECTOR_WIDTH_FLOAT:
		case CL_DEVICE_NATIVE_VECTOR_WIDTH_DOUBLE:
		case CL_DEVICE_PREFERRED_VECTOR_WIDTH_CHAR:
		case CL_DEVICE_PREFERRED_VECTOR_WIDTH_SHORT:
		case CL_DEVICE_PREFERRED_VECTOR_WIDTH_INT:
		case CL_DEVICE_PREFERRED_VECTOR_WIDTH_LONG:
		case CL_DEVICE_PREFERRED_VECTOR_WIDTH_HALF:
		case CL_DEVICE_PREFERRED_VECTOR_WIDTH_FLOAT:
			if (check_size(sizeof(cl_uint), param_value_size, result)) {
				print_u(*(cl_uint*)param_value);
			}
			break;
		case CL_DEVICE_MAX_WORK_GROUP_SIZE:
		case CL_DEVICE_MAX_PARAMETER_SIZE:
		case CL_DEVICE_PROFILING_TIMER_RESOLUTION:
			if (check_size(sizeof(size_t), param_value_size, result)) {
				print_u(*(size_t*)param_value);
			}
			break;
		case CL_DEVICE_MAX_WORK_ITEM_SIZES:
			if (check_size(sizeof(size_t) * 3, param_value_size, result)) {
				printf(": (%zu, %zu, %zu)",
				       ((size_t*)param_value)[0],
				       ((size_t*)param_value)[1],
				       ((size_t*)param_value)[2]);
			}
			break;
		case CL_DEVICE_MAX_CLOCK_FREQUENCY:
			if (check_size(sizeof(cl_uint), param_value_size, result)) {
				printf(": %u MHz", *(cl_uint*)param_value);
			}
			break;
		case CL_DEVICE_MAX_COMPUTE_UNITS:
			valid = check_size(sizeof(cl_uint), param_value_size, result) &&
			        check_min_int(1, *(cl_uint*)param_value, result);

			if (valid) {
				device_config->max_unit = *(cl_uint*)param_value;
				print_u(*(cl_uint*)param_value);
			}
			break;
		case CL_DEVICE_MAX_WRITE_IMAGE_ARGS:
			valid = check_size(sizeof(cl_uint), param_value_size, result);
			if (valid && device_config->has_image) {
				valid = check_min_int(8, *(cl_uint*)param_value, result);
			}

			if (valid) {
				print_u(*(cl_uint*)param_value);
			}
			break;
		case CL_DEVICE_MAX_MEM_ALLOC_SIZE:
			valid = check_size(sizeof(cl_ulong), param_value_size, result);
			if (valid && device_config->type != CL_DEVICE_TYPE_CUSTOM) {
				valid = check_min_int(MAX2(device_config->mem_size/4, 128*1024*1024),
					               *(cl_ulong*)param_value,
					               result);
			}

			if (valid) {
				print_u(*(cl_ulong*)param_value);
			}
			break;
		case CL_DEVICE_IMAGE_MAX_BUFFER_SIZE:
			valid = check_size(sizeof(size_t), param_value_size, result);
			if (valid && device_config->has_image) {
				check_min_int(65536, *(size_t*)param_value, result);
			}

			if (valid) {
				print_u(*(size_t*)param_value);
			}
			break;
		case CL_DEVICE_IMAGE2D_MAX_WIDTH:
		case CL_DEVICE_IMAGE2D_MAX_HEIGHT:
			valid = check_size(sizeof(size_t), param_value_size, result);
			if (valid && device_config->has_image) {
				check_min_int(8192, *(size_t*)param_value, result);
			}

			if (valid) {
				print_u(*(size_t*)param_value);
			}
			break;
		case CL_DEVICE_IMAGE_MAX_ARRAY_SIZE:
		case CL_DEVICE_IMAGE3D_MAX_WIDTH:
		case CL_DEVICE_IMAGE3D_MAX_HEIGHT:
		case CL_DEVICE_IMAGE3D_MAX_DEPTH:
			valid = check_size(sizeof(size_t), param_value_size, result);
			if (valid && device_config->has_image) {
				check_min_int(2048, *(size_t*)param_value, result);
			}

			if (valid) {
				print_u(*(size_t*)param_value);
			}
			break;
		case CL_DEVICE_MAX_READ_IMAGE_ARGS:
			valid = check_size(sizeof(cl_uint), param_value_size, result);
			if (valid && device_config->has_image) {
				valid = check_min_int(128, *(cl_uint*)param_value, result);
			}

			if (valid) {
				print_u(*(cl_uint*)param_value);
			}
			break;
		case CL_DEVICE_MAX_SAMPLERS:
			valid = check_size(sizeof(cl_uint), param_value_size, result);
			if (valid && device_config->has_image) {
				valid = check_min_int(16, *(cl_uint*)param_value, result);
			}

			if (valid) {
				print_u(*(cl_uint*)param_value);
			}
			break;
		case CL_DEVICE_MEM_BASE_ADDR_ALIGN:
			valid = check_size(sizeof(cl_uint), param_value_size, result);
			if (valid && device_config->is_full_profile) {
				valid = check_min_int(sizeof(cl_long16), *(cl_uint*)param_value, result);
			}

			if (valid) {
				print_u(*(cl_uint*)param_value);
			}
			break;
		case CL_DEVICE_SINGLE_FP_CONFIG:
			if (check_size(sizeof(cl_device_fp_config), param_value_size, result) &&
			    check_fp_config(kind, *(cl_device_fp_config*)param_value, result)) {
				cl_device_fp_config value = *(cl_device_fp_config*)param_value;
				if (device_config->type != CL_DEVICE_TYPE_CUSTOM &&
				    !(value & (CL_FP_ROUND_TO_ZERO|CL_FP_ROUND_TO_NEAREST))) {
					printf(": failed, expected at least CL_FP_ROUND_TO_ZERO or CL_FP_ROUND_TO_NEAREST. Got %lx", value);
					piglit_merge_result(result, PIGLIT_FAIL);
				} else {
					printf(": %lx", value);
				}
			}
			break;
		case CL_DEVICE_DOUBLE_FP_CONFIG:
			if (check_size(sizeof(cl_device_fp_config), param_value_size, result) &&
			    check_fp_config(kind, *(cl_device_fp_config*)param_value, result)) {
				cl_device_fp_config value = *(cl_device_fp_config*)param_value;
				if (device_config->type != CL_DEVICE_TYPE_CUSTOM &&
				    device_config->has_double &&
				    !(value & (CL_FP_FMA|CL_FP_ROUND_TO_NEAREST|CL_FP_ROUND_TO_ZERO|
				               CL_FP_ROUND_TO_INF|CL_FP_INF_NAN|CL_FP_DENORM))) {
					printf(": failed, expected CL_FP_FMA|CL_FP_ROUND_TO_NEAREST|CL_FP_ROUND_TO_ZERO|" \
					       "CL_FP_ROUND_TO_INF|CL_FP_INF_NAN|CL_FP_DENORM. Got %lx", value);
					piglit_merge_result(result, PIGLIT_FAIL);
				} else {
					printf(": %lx", value);
				}
			}
			break;
		case CL_DEVICE_GLOBAL_MEM_CACHE_TYPE:
			if (check_size(sizeof(cl_device_mem_cache_type), param_value_size, result)) {
				if (*(cl_device_mem_cache_type*)param_value & ~(CL_READ_ONLY_CACHE|CL_READ_WRITE_CACHE)) {
					printf(": failed, expected CL_READ_ONLY_CACHE or CL_READ_WRITE_CACHE. Got %x",
					*(cl_device_mem_cache_type*)param_value);
					piglit_merge_result(result, PIGLIT_FAIL);
				} else {
					printf(": %x", *(cl_device_mem_cache_type*)param_value);
				}
			}
			break;
		case CL_DEVICE_GLOBAL_MEM_CACHE_SIZE:
			if (check_size(sizeof(cl_ulong), param_value_size, result)) {
				print_u(*(cl_ulong*)param_value);
			}
			break;
		case CL_DEVICE_GLOBAL_MEM_SIZE:
			if (check_size(sizeof(cl_ulong), param_value_size, result)) {
				device_config->mem_size = *(cl_ulong*)param_value;
				print_u(*(cl_ulong*)param_value);
			}
			break;
		case CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE:
			valid = check_size(sizeof(cl_ulong), param_value_size, result);
			if (valid && device_config->type != CL_DEVICE_TYPE_CUSTOM) {
				valid = check_min_int(64 * 1024, *(cl_ulong*)param_value, result);
			}

			if (valid) {
				print_u(*(cl_ulong*)param_value);
			}
			break;
		case CL_DEVICE_MAX_CONSTANT_ARGS:
			valid = check_size(sizeof(cl_uint), param_value_size, result);
			if (valid && device_config->type != CL_DEVICE_TYPE_CUSTOM) {
				valid = check_min_int(8, *(cl_uint*)param_value, result);
			}

			if (valid) {
				print_u(*(cl_uint*)param_value);
			}
			break;
		case CL_DEVICE_LOCAL_MEM_TYPE:
			if (check_size(sizeof(cl_device_local_mem_type), param_value_size, result)) {
				if (*(cl_device_local_mem_type*)param_value & ~(CL_LOCAL|CL_GLOBAL)) {
					printf(": failed, expected CL_LOCAL or CL_GLOBAL. Got %x",
					*(cl_device_local_mem_type*)param_value);
					piglit_merge_result(result, PIGLIT_FAIL);
				}
			}
			break;
		case CL_DEVICE_LOCAL_MEM_SIZE:
			valid = check_size(sizeof(cl_ulong), param_value_size, result);
			if (valid && device_config->type != CL_DEVICE_TYPE_CUSTOM) {
				valid = check_min_int(32 * 1024, *(cl_ulong*)param_value, result);
			}

			if (valid) {
				print_u(*(cl_ulong*)param_value);
			}
			break;
		case CL_DEVICE_PREFERRED_INTEROP_USER_SYNC:
		case CL_DEVICE_ERROR_CORRECTION_SUPPORT:
		case CL_DEVICE_HOST_UNIFIED_MEMORY:
		case CL_DEVICE_ENDIAN_LITTLE:
		case CL_DEVICE_AVAILABLE:
			if (check_size(sizeof(cl_bool), param_value_size, result)) {
				print_b(*(cl_bool*)param_value);
			}
			break;
		case CL_DEVICE_COMPILER_AVAILABLE:
		case CL_DEVICE_LINKER_AVAILABLE:
			valid = check_size(sizeof(cl_bool), param_value_size, result);
			if (valid && device_config->is_full_profile) {
				valid = check_min_int(1, *(cl_bool*)param_value, result);
			}

			if (valid) {
				print_b(*(cl_bool*)param_value);
			}
			break;
		case CL_DEVICE_EXECUTION_CAPABILITIES:
			if (check_size(sizeof(cl_device_exec_capabilities), param_value_size, result)) {
				cl_device_exec_capabilities value =
					*(cl_device_exec_capabilities*)param_value;

				if ((value & ~(CL_EXEC_KERNEL|CL_EXEC_NATIVE_KERNEL)) ||
				    !(value & CL_EXEC_KERNEL)) {
					printf(": failed, expected CL_EXEC_KERNEL and optional CL_EXEC_NATIVE_KERNEL");
					piglit_merge_result(result, PIGLIT_FAIL);
				} else {
					printf (": CL_EXEC_KERNEL");
					if (value & CL_EXEC_NATIVE_KERNEL) {
						printf(", CL_EXEC_NATIVE_KERNEL");
					}
				}
			}
			break;
		case CL_DEVICE_QUEUE_ON_HOST_PROPERTIES:
			if (check_size(sizeof(cl_command_queue_properties), param_value_size, result)) {
				cl_command_queue_properties value =
					*(cl_command_queue_properties*)param_value;

				if ((value & ~(CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE|CL_QUEUE_PROFILING_ENABLE)) ||
				    !(value &  CL_QUEUE_PROFILING_ENABLE)) {
					printf(": failed, expected  CL_QUEUE_PROFILING_ENABLE and optional CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE");
					piglit_merge_result(result, PIGLIT_FAIL);
				} else {
					printf (": CL_QUEUE_PROFILING_ENABLE");
					if (value & CL_EXEC_NATIVE_KERNEL) {
						printf(", CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE");
					}
				}
			}
			break;
		case CL_DEVICE_PLATFORM:
			if (check_size(sizeof(cl_platform_id), param_value_size, result)) {
				if (*(cl_platform_id*)param_value != env->platform_id) {
					printf(": failed, unexpected cl_platform_id value");
					piglit_merge_result(result, PIGLIT_FAIL);
				} else {
					printf(": %p", param_value);
				}
			}
			break;
		case CL_DEVICE_PARENT_DEVICE:
			if (check_size(sizeof(cl_device_id), param_value_size, result)) {
				if (*(cl_device_id*)param_value) {
					printf(": failed, parent device should be null, got %p", *(cl_device_id*)param_value);
					piglit_merge_result(result, PIGLIT_FAIL);
				} else {
					printf(": %p", *(cl_device_id*)param_value);
				}
			}
			break;
		case CL_DEVICE_PARTITION_MAX_SUB_DEVICES:
			if (check_size(sizeof(cl_uint), param_value_size, result) &&
			    check_max_int(device_config->max_unit, *(cl_uint*)param_value, result)) {
				print_u(*(cl_uint*)param_value);
			}
			break;
		case CL_DEVICE_PARTITION_PROPERTIES:
			if (param_value_size < sizeof(cl_device_partition_property)) {
				printf(": failed, expected and actual size differ. Expect >=%lu, got %lu",
				       sizeof(cl_device_partition_property), param_value_size);
				piglit_merge_result(result, PIGLIT_FAIL);
			} else {
				// TODO this a array of cl_device_partition_property
				// Only the first value is checked here.
				if ((*(cl_device_partition_property*)param_value &
				     ~(CL_DEVICE_PARTITION_EQUALLY|
				       CL_DEVICE_PARTITION_BY_COUNTS|
				       CL_DEVICE_PARTITION_BY_AFFINITY_DOMAIN))) {
					printf(": failed, expected  a cl_device_partition_property valid value. Got %lx",
					       *(cl_device_partition_property*)param_value);
					piglit_merge_result(result, PIGLIT_FAIL);
				} else {
					printf(": %lx", *(cl_device_partition_property*)param_value);
				}
			}
			break;
		case CL_DEVICE_PARTITION_AFFINITY_DOMAIN:
			if ((*(cl_device_affinity_domain*)param_value &
			     ~(CL_DEVICE_AFFINITY_DOMAIN_NUMA|
			       CL_DEVICE_AFFINITY_DOMAIN_L4_CACHE|
			       CL_DEVICE_AFFINITY_DOMAIN_L3_CACHE|
			       CL_DEVICE_AFFINITY_DOMAIN_L2_CACHE|
			       CL_DEVICE_AFFINITY_DOMAIN_L1_CACHE|
			       CL_DEVICE_AFFINITY_DOMAIN_NEXT_PARTITIONABLE))) {
				printf(": failed, expected  a cl_device_affinity_domain valid value. Got %lx",
				       *(cl_device_affinity_domain*)param_value);
				piglit_merge_result(result, PIGLIT_FAIL);
			} else {
				printf(": %lx", *(cl_device_affinity_domain*)param_value);
			}
			break;
		case CL_DEVICE_PARTITION_TYPE:
			if (param_value_size) {
				if (param_value_size < sizeof(cl_device_affinity_domain)) {
					printf(": failed, expected and actual size differ. Expect >=%lu, got %lu",
					       sizeof(cl_device_affinity_domain), param_value_size);
					piglit_merge_result(result, PIGLIT_FAIL);
				} else {
					printf(": %lx", *(cl_device_affinity_domain*)param_value);
				}
			} else {
				printf(": (empty)");
			}
			break;
		case CL_DEVICE_REFERENCE_COUNT:
			if (check_size(sizeof(cl_uint), param_value_size, result) &&
			    check_min_int(1, *(cl_uint*)param_value, result)) {
					print_u(*(cl_uint*)param_value);
			}
			break;
		case CL_DEVICE_PRINTF_BUFFER_SIZE:
			valid = check_size(sizeof(size_t), param_value_size, result);
			if (valid && device_config->is_full_profile) {
				valid = check_min_int(1024 * 1024, *(cl_uint*)param_value, result);
			} else {
				valid = check_min_int(1024, *(cl_uint*)param_value, result);
			}

			if (valid) {
				print_u(*(size_t*)param_value);
			}
			break;

		default:
			printf(": WARN unchecked value");
			piglit_merge_result(result, PIGLIT_WARN);
	}
}

enum piglit_result
piglit_cl_test(const int argc,
               const char** argv,
               const struct piglit_cl_api_test_config* config,
               const struct piglit_cl_api_test_env* env)
{
	enum piglit_result result = PIGLIT_PASS;

	struct device_config device_config;

	int i;
	cl_int errNo;

	size_t param_value_size;
	void* param_value;

	int num_device_infos = PIGLIT_CL_ENUM_NUM(cl_device_info, env->version);
	const cl_device_info *device_infos_enum = PIGLIT_CL_ENUM_ARRAY(cl_device_info);

	const cl_device_info *device_infos = device_infos_enum;

	if (env->version >= 12) {
		// for some check we need some infos to be read before others
		// piglit-util-cl-enum could be re ordered partialy
		// the first 10 elements are ok,
		// after that we want to start with dependant stuff
		int sorted_idx;
		cl_device_info *sorted_infos = malloc(sizeof(cl_device_info) * num_device_infos);
		memcpy(sorted_infos, device_infos_enum, sizeof(cl_device_info) * 10);

		sorted_infos[10] = CL_DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE;
		sorted_idx = 11;

		for(i = 10; i < num_device_infos; i++) {
			if (device_infos_enum[i] == CL_DEVICE_PREFERRED_VECTOR_WIDTH_DOUBLE) {
				continue;
			}
			sorted_infos[sorted_idx] = device_infos_enum[i];
			sorted_idx++;
		}
		device_infos = sorted_infos;
	}

	/*** Normal usage ***/

	for(i = 0; i < num_device_infos; i++) {
		printf("%s", piglit_cl_get_enum_name(device_infos[i]));

		errNo = clGetDeviceInfo(env->device_id,
		                        device_infos[i],
		                        0,
		                        NULL,
		                        &param_value_size);
		if(!piglit_cl_check_error(errNo, CL_SUCCESS)) {
			fprintf(stderr,
			        ": failed (error code: %s): Get size of %s.\n",
			        piglit_cl_get_error_name(errNo),
			        piglit_cl_get_enum_name(device_infos[i]));
			piglit_merge_result(&result, PIGLIT_FAIL);
			continue;
		}

		param_value = malloc(param_value_size);
		errNo = clGetDeviceInfo(env->device_id,
		                        device_infos[i],
		                        param_value_size,
		                        param_value,
		                        NULL);
		if(!piglit_cl_check_error(errNo, CL_SUCCESS)) {
			fprintf(stderr,
			        ": failed (error code: %s): Get value of %s.\n",
			        piglit_cl_get_error_name(errNo),
			        piglit_cl_get_enum_name(device_infos[i]));
			piglit_merge_result(&result, PIGLIT_FAIL);
		}

		check_info(env, &device_config,
		           device_infos[i], param_value, param_value_size, &result);

		printf("\n");
		free(param_value);
	}

	/*** Errors ***/

	/*
	 * CL_INVALID_VALUE if param_name is not one of the supported
	 * values or if size in bytes specified by param_value_size is
	 * less than size of return type and param_value is not a NULL
	 * value.
	 */
	errNo = clGetDeviceInfo(env->device_id,
	                        CL_DEVICE_VERSION,
	                        1,
	                        param_value,
	                        NULL);
	if(!piglit_cl_check_error(errNo, CL_INVALID_VALUE)) {
		fprintf(stderr,
		        "Failed (error code: %s): Trigger CL_INVALID_VALUE if param_name is not one of the supported values.\n",
		        piglit_cl_get_error_name(errNo));
		piglit_merge_result(&result, PIGLIT_FAIL);
	}
	
	errNo = clGetDeviceInfo(env->device_id,
	                        CL_PLATFORM_NAME,
	                        0,
	                        NULL,
	                        &param_value_size);
	if(!piglit_cl_check_error(errNo, CL_INVALID_VALUE)) {
		fprintf(stderr,
		        "Failed (error code: %s): Trigger CL_INVALID_VALUE if size in bytes specified by param_value is less than size of return type and param_value is not a NULL value.\n",
		        piglit_cl_get_error_name(errNo));
		piglit_merge_result(&result, PIGLIT_FAIL);
	}

	/*
	 * CL_INVALID_DEVICE if device is not a valid device.
	 */
	errNo = clGetDeviceInfo(NULL,
	                        CL_DEVICE_NAME,
	                        0,
	                        NULL,
	                        &param_value_size);
	if(!piglit_cl_check_error(errNo, CL_INVALID_DEVICE)) {
		fprintf(stderr,
		        "Failed (error code: %s): Trigger CL_INVALID_DEVICE if device is not a valid device.\n",
		        piglit_cl_get_error_name(errNo));
		piglit_merge_result(&result, PIGLIT_FAIL);
	}

	if (env->version >= 12) {
		free((void*)device_infos);
	}

	return result;
}
