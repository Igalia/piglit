/*
 * Copyright © 2013 Intel Corporation
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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

/**
 * \file api.c
 *
 * Basic AMD_performance_monitor infrastructure tests.  These test the
 * mechanism to retrieve counter and group information, string processing,
 * and various error conditions.  They do not actually activate monitoring.
 */

#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;
	config.window_visual = PIGLIT_GL_VISUAL_RGB;

PIGLIT_GL_TEST_CONFIG_END

/******************************************************************************/

/**
 * Get a list of group IDs.
 */
static void
get_groups(unsigned **groups, int *num_groups)
{
	glGetPerfMonitorGroupsAMD(num_groups, 0, NULL);
	*groups = calloc(*num_groups, sizeof(unsigned));
	glGetPerfMonitorGroupsAMD(NULL, *num_groups, *groups);
}

/**
 * Get a list of counter IDs in a given group.
 */
static void
get_counters(unsigned group, unsigned **counters, int *num_counters)
{
	glGetPerfMonitorCountersAMD(group, num_counters, NULL, 0, NULL);
	*counters = calloc(*num_counters, sizeof(unsigned));
	glGetPerfMonitorCountersAMD(group, NULL, NULL, *num_counters, *counters);
}

/**
 * Return true if x is in xs.
 */
static bool
in_list(int x, unsigned *xs, int elts)
{
	int i;
	for (i = 0; i < elts; i++) {
		if (x == xs[i])
			return true;
	}
	return false;
}

/**
 * Find an invalid group ID.
 */
static unsigned
find_invalid_group(unsigned *groups, int num_groups)
{
	unsigned invalid_group = ~0;

	/* Most implementations probably use small consecutive integers, so
	 * start at ~0 and work backwards.  Hopefully we shouldn't loop.
	 */
	while (in_list(invalid_group, groups, num_groups))
		--invalid_group;

	return invalid_group;
}

/**
 * Find an invalid counter ID.
 */
static unsigned
find_invalid_counter(unsigned *counters, int num_counters)
{
	unsigned invalid_counter = ~0;

	/* Most implementations probably use small consecutive integers, so
	 * start at ~0 and work backwards.  Hopefully we shouldn't loop.
	 */
	while (in_list(invalid_counter, counters, num_counters))
		--invalid_counter;

	return invalid_counter;
}

#define report(pass) \
	do { \
		piglit_report_subtest_result((pass) ? PIGLIT_PASS : PIGLIT_FAIL, __FUNCTION__); \
		return; \
	} while (0)

/******************************************************************************/

/**
 * Call glGetPerfMonitorGroupsAMD() with a NULL numGroups pointer.
 *
 * Verify that it doesn't attempt to write the number of groups and crash.
 */
static void
test_number_of_groups_null_num_groups_pointer(void)
{
	glGetPerfMonitorGroupsAMD(NULL, 0, NULL);
	report(piglit_check_gl_error(GL_NO_ERROR));
}


/**
 * Call glGetPerfMonitorGroupsAMD() with NULL for groups but non-zero groupSize.
 *
 * Verify that it returns the number of groups but doesn't try to write any
 * group IDs and crash.
 */
static void
test_number_of_groups_null_groups_pointer(void)
{
	bool pass = true;
	int num_groups = -1;

	glGetPerfMonitorGroupsAMD(&num_groups, 777, NULL);
	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;
	pass = num_groups >= 0 && pass;
	report(pass);
}

/**
 * Call glGetPerfMonitorGroupsAMD() with zero for groupSize.
 *
 * Verify that it doesn't write any group IDs.
 */
static void
test_number_of_groups_zero_size_array(void)
{
	bool pass = true;
	unsigned groups[2] = {0xd0d0d0d0, 0xd1d1d1d1};
	int num_groups = -1;

	glGetPerfMonitorGroupsAMD(&num_groups, 0, groups);
	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

	/* num_groups must have changed */
	pass = num_groups >= 0 && pass;

	/* The groups array should not have changed. */
	pass = groups[0] == 0xd0d0d0d0 && pass;
	pass = groups[1] == 0xd1d1d1d1 && pass;
	report(pass);
}

/**
 * Call glGetPerfMonitorGroupsAMD() with a groups array bigger than groupSize.
 *
 * Verify that it fills the correct number of array slots with group IDs.
 */
static void
test_number_of_groups_partial_array(void)
{
	bool pass = true;
	unsigned groups[] = {0xdddddddd, 0xdddddddd, 0xdddddddd, 0xdddddddd};

	/* Artificially low array size */
	const int groups_array_size = 2;
	int num_groups = -1;
	int i;

	/* This should return the number of groups.  It should not attempt to
	 * write any groups since the pointer is NULL.
	 */
	glGetPerfMonitorGroupsAMD(&num_groups, groups_array_size, groups);
	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

	/* num_groups must have changed */
	pass = num_groups >= 0 && pass;

	/* The first few elements should have changed. */
	for (i = 0; i < MIN2(num_groups, groups_array_size); i++) {
		pass = groups[i] != 0xdddddddd && pass;
	}

	/* Catalyst 13.10 on a Radeon 6870 appears to have a bug where this
	 * returns 3 elements instead of 2.  According to the spec,
	 * "The number of entries that will be returned in <groups> is
	 *  determined by <groupSize>."
	 *
	 * Technically, it does not say that N elements will be returned if
	 * groupSize is N, but that's the only reasonable assumption.
	 */

	/* The rest should remain untouched. */
	for (; i < ARRAY_SIZE(groups); i++) {
		pass = groups[i] == 0xdddddddd && pass;
	}

	report(pass);
}

/******************************************************************************/

/**
 * Call glGetPerfMonitorCountersAMD() with an invalid group ID.
 *
 * Verify that it produces INVALID_VALUE.
 */
static void
test_get_counters_invalid_group(unsigned invalid_group)
{
	glGetPerfMonitorCountersAMD(invalid_group, NULL, NULL, 0, NULL);
	report(piglit_check_gl_error(GL_INVALID_VALUE));
}

/**
 * Call glGetPerfMonitorCountersAMD() with a bunch of NULL pointers.
 *
 * Verify that it doesn't crash attempting to write numCounters,
 * maxActiveCounters, or the counters list.
 */
static void
test_get_counters_null_pointers(unsigned valid_group)
{
	glGetPerfMonitorCountersAMD(valid_group, NULL, NULL, 0, NULL);
	report(piglit_check_gl_error(GL_NO_ERROR));
}

/**
 * Call glGetPerfMonitorCountersAMD() with NULL for the array but non-zero size.
 *
 * Verify that it returns the number of groups but doesn't try to write any
 * group IDs and crash.
 */
static void
test_get_counters_null_pointer_non_zero_size(unsigned valid_group)
{
	glGetPerfMonitorCountersAMD(valid_group, NULL, NULL, 777, NULL);
	report(piglit_check_gl_error(GL_NO_ERROR));
}

/**
 * Call glGetPerfMonitorCountersAMD() with zero for countersSize.
 *
 * Verify that it doesn't write any IDs, but does return other data.
 */
static void
test_get_counters_zero_size_array(unsigned valid_group)
{
	bool pass = true;
	unsigned counters[2] = {0xd0d0d0d0, 0xd1d1d1d1};
	int num_counters = -1;
	int max_active_counters = -1;

	glGetPerfMonitorCountersAMD(valid_group, &num_counters,
				    &max_active_counters,
				    0, counters);
	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

	/* Expect a positive number of counters. */
	pass = num_counters >= 0 && pass;

	/* Expect a positive maximum active counters. */
	pass = max_active_counters >= 0 && pass;

	/* The counters array should not have changed. */
	pass = counters[0] == 0xd0d0d0d0 && pass;
	pass = counters[1] == 0xd1d1d1d1 && pass;
	report(pass);
}

/**
 * Call glGetPerfMonitorGroupsAMD() with a groups array bigger than groupSize.
 *
 * Verify that it fills the correct number of array slots with group IDs.
 */
static void
test_get_counters_partial_array(unsigned valid_group)
{
	bool pass = true;
	unsigned counters[] = {0xdddddddd, 0xdddddddd, 0xdddddddd, 0xdddddddd};

	/* Artificially low array size */
	const int counters_array_size = 2;
	int num_counters = -1;
	int i;

	/* This should return the number of groups.  It should not attempt to
	 * write any groups since the pointer is NULL.
	 */
	glGetPerfMonitorCountersAMD(valid_group, &num_counters, NULL,
				    counters_array_size, counters);
	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

	/* num_counters must have changed */
	pass = num_counters >= 0 && pass;

	/* The first few elements should have changed. */
	for (i = 0; i < MIN2(num_counters, counters_array_size); i++) {
		pass = counters[i] != 0xdddddddd && pass;
	}

	/* The rest should remain untouched. */
	for (; i < ARRAY_SIZE(counters); i++) {
		pass = counters[i] == 0xdddddddd && pass;
	}

	report(pass);
}

/******************************************************************************/

/**
 * Call glGetPerfMonitorGroupStringAMD() with an invalid group ID.
 *
 * Verify that it produces INVALID_VALUE.
 */
static void
test_group_string_invalid_group(unsigned invalid_group)
{
	glGetPerfMonitorGroupStringAMD(invalid_group, 0, NULL, NULL);
	report(piglit_check_gl_error(GL_INVALID_VALUE));
}

/**
 * Call glGetPerfMonitorGroupStringAMD() with a NULL length pointer.
 *
 * Verify that it doesn't crash.
 */
static void
test_group_string_null_length(unsigned valid_group)
{
	glGetPerfMonitorGroupStringAMD(valid_group, 0, NULL, NULL);
	report(piglit_check_gl_error(GL_NO_ERROR));
}

/**
 * Call glGetPerfMonitorGroupStringAMD() with a single character buffer.
 *
 * Verify that length is correct and no buffer overflows occur.
 */
static void
test_group_string_single_character_buffer(unsigned valid_group)
{
	bool pass = true;
	char name[3] = "```";
	GLsizei length = 0xd0d0d0d0;

	glGetPerfMonitorGroupStringAMD(valid_group, 1, &length, name);
	pass = piglit_check_gl_error(GL_NO_ERROR);

	/* Verify buffer contents: only the first character should change. */
	pass = name[0] != '`' && pass;
	pass = name[1] == '`' && pass;
	pass = name[2] == '`' && pass;

	/* length is the number of characters written excluding the null
	 * terminator.
	 */
	if (name[0] == '\0') {
		pass = length == 0 && pass;
	} else {
		/* AMD Catalyst 13.10 (Radeon 6870) does not write a null
		 * terminator.  Instead, it writes the first part of the name.
		 */
		pass = length == 1 && pass;
	}

	report(pass);
}

/**
 * Call glGetPerfMonitorGroupStringAMD() with a small buffer.
 *
 * Verify that a name is returned, length is valid, and no overflows occur.
 */
static void
test_group_string_small_buffer(unsigned valid_group)
{
	bool pass = true;
	char name[3] = "```";
	GLsizei length = 0xd0d0d0d0;
	int i;

	glGetPerfMonitorGroupStringAMD(valid_group, 3, &length, name);

	pass = length <= 3 && pass;

	/* Verify buffer contents: accept no null terminator. */
	for (i = 0; i < length; i++)
		pass = name[i] != '`' && pass;

	if (length < 3) {
		pass = name[length] == '\0';
		for (i = length + 1; i < 3; i++)
			pass = name[i] == '`' && pass;
	}

	report(pass);
}

/**
 * Call glGetPerfMonitorGroupStringAMD() with an appropriately sized buffer.
 *
 * Verify that a name is returned, length is valid, and no overflows occur.
 */
static void
test_group_string_normal_buffer(unsigned valid_group)
{
	bool pass = true;
	char *name;
	GLsizei length = 0xd0d0d0d0;
	int i;

	/* Get the length; bail if unwritten to avoid huge allocations. */
	glGetPerfMonitorGroupStringAMD(valid_group, 0, &length, NULL);
	pass = pass && piglit_check_gl_error(GL_NO_ERROR);
	if (length == 0xd0d0d0d0)
		report(false);

	name = malloc(length + 1);
	assert(name != NULL);

	/* Fill the buffer with a known character (` marks) */
	memset(name, '`', length + 1);

	/* Get the name; everything will fit. */
	glGetPerfMonitorGroupStringAMD(valid_group, length + 1, NULL, name);
	pass = pass && piglit_check_gl_error(GL_NO_ERROR);

	/* Indexes in the interval [0, length) must have been written, or
	 * else length is wrong.
	 */
	for (i = 0; i < length; i++)
		pass = name[i] != '`' && pass;

	/* The last character must be the null terminator. */
	pass = name[length] == '\0' && pass;

	report(pass);
}

/******************************************************************************/

/**
 * Call glGetPerfMonitorCounterStringAMD() with an invalid group ID.
 *
 * Verify that it produces INVALID_VALUE.
 */
static void
test_counter_string_invalid_group(unsigned invalid_group)
{
	glGetPerfMonitorCounterStringAMD(invalid_group, 0, 0, NULL, NULL);
	report(piglit_check_gl_error(GL_INVALID_VALUE));
}

/**
 * Call glGetPerfMonitorCounterStringAMD() with an invalid counter ID.
 *
 * Verify that it produces INVALID_VALUE.
 */
static void
test_counter_string_invalid_counter(unsigned group, unsigned invalid_counter)
{
	glGetPerfMonitorCounterStringAMD(group, invalid_counter, 0, NULL, NULL);
	report(piglit_check_gl_error(GL_INVALID_VALUE));
}

/**
 * Call glGetPerfMonitorCounterStringAMD() with a NULL length pointer.
 *
 * Verify that it doesn't crash.
 */
static void
test_counter_string_null_length(unsigned group, unsigned counter)
{
	glGetPerfMonitorCounterStringAMD(group, counter, 0, NULL, NULL);
	report(piglit_check_gl_error(GL_NO_ERROR));
}

/**
 * Call glGetPerfMonitorCounterStringAMD() with a single character buffer.
 *
 * Verify that length is correct and no buffer overflows occur.
 */
static void
test_counter_string_single_character_buffer(unsigned group, unsigned counter)
{
	bool pass = true;
	char name[3] = "```";
	GLsizei length = 0xd0d0d0d0;

	glGetPerfMonitorCounterStringAMD(group, counter, 1, &length, name);
	pass = piglit_check_gl_error(GL_NO_ERROR);

	/* Verify buffer contents */
	pass = name[0] != '`' && pass;
	pass = name[1] == '`' && pass;
	pass = name[2] == '`' && pass;

	/* length is the number of characters written excluding the null
	 * terminator.
	 */
	if (name[0] == '\0') {
		pass = length == 0 && pass;
	} else {
		/* AMD Catalyst 13.10 (Radeon 6870) does not write a null
		 * terminator.  Instead, it writes the first part of the name.
		 */
		pass = length == 1 && pass;
	}

	report(pass);
}

/**
 * Call glGetPerfMonitorCounterStringAMD() with a small buffer.
 *
 * Verify that a name is returned, length is valid, and no overflows occur.
 */
static void
test_counter_string_small_buffer(unsigned group, unsigned counter)
{
	bool pass = true;
	char name[3] = "```";
	GLsizei length = 0xd0d0d0d0;
	int i;

	glGetPerfMonitorCounterStringAMD(group, counter, 3, &length, name);

	pass = length <= 3 && pass;

	/* Verify buffer contents: accept no null terminator. */
	for (i = 0; i < length; i++)
		pass = name[i] != '`' && pass;

	if (length < 3) {
		pass = name[length] == '\0';
		for (i = length + 1; i < 3; i++)
			pass = name[i] == '`' && pass;
	}

	report(pass);
}

/**
 * Call glGetPerfMonitorCounterStringAMD() with an appropriately sized buffer.
 *
 * Verify that a name is returned, length is valid, and no overflows occur.
 */
static void
test_counter_string_normal_buffer(unsigned group, unsigned counter)
{
	bool pass = true;
	char *name;
	GLsizei length = 0xd0d0d0d0;
	int i;

	/* Get the length; bail if unwritten to avoid huge allocations. */
	glGetPerfMonitorCounterStringAMD(group, counter, 0, &length, NULL);
	pass = pass && piglit_check_gl_error(GL_NO_ERROR);
	if (length == 0xd0d0d0d0)
		report(false);

	name = malloc(length + 1);
	assert(name != NULL);

	/* Fill the buffer with a known character (` marks) */
	memset(name, '`', length + 1);

	/* Get the name; everything will fit. */
	glGetPerfMonitorCounterStringAMD(group, counter, length + 1, NULL, name);
	pass = pass && piglit_check_gl_error(GL_NO_ERROR);

	/* Indexes in the interval [0, length) must have been written, or
	 * else length is wrong.
	 */
	for (i = 0; i < length; i++)
		pass = name[i] != '`' && pass;

	/* The last character must be the null terminator. */
	pass = name[length] == '\0' && pass;

	report(pass);
}

/******************************************************************************/

/**
 * Call glGetPerfMonitorCounterInfoAMD() with an invalid group ID.
 *
 * Verify that it produces INVALID_VALUE.
 */
static void
test_counter_info_invalid_group(unsigned invalid_group)
{
	GLenum type;
	glGetPerfMonitorCounterInfoAMD(invalid_group, 0, GL_COUNTER_TYPE_AMD,
				       &type);
	report(piglit_check_gl_error(GL_INVALID_VALUE));
}

/**
 * Call glGetPerfMonitorCounterInfoAMD() with an invalid counter ID.
 *
 * Verify that it produces INVALID_VALUE.
 */
static void
test_counter_info_invalid_counter(unsigned group, unsigned invalid_counter)
{
	GLenum type;
	glGetPerfMonitorCounterInfoAMD(group, invalid_counter,
				       GL_COUNTER_TYPE_AMD, &type);
	report(piglit_check_gl_error(GL_INVALID_VALUE));
}

/**
 * Call glGetPerfMonitorCounterInfoAMD() on every group/counter and verify that:
 * - All counters must have a valid type.
 * - Percentage counters must have a range of [0.0f, 100.0f]
 * - Counter ranges should return a minimum strictly less than the maximum.
 * - The counter range query doesn't return too much data.
 */
static void
test_counter_info(unsigned *groups, int num_groups)
{
	int i;
	int j;

	for (i = 0; i < num_groups; i++) {
		unsigned *counters;
		int num_counters;
		get_counters(groups[i], &counters, &num_counters);

		for (j = 0; j < num_counters; j++) {
			GLenum type = GL_NONE;
			uint64_t data[3];
			uint64_t min_u, max_u;
			float min_f, max_f;
			uint32_t unchanged;
			bool is_unsigned = false;

			glGetPerfMonitorCounterInfoAMD(groups[i], counters[j],
						       GL_COUNTER_TYPE_AMD,
						       &type);

			/* Get the range */
			memset(data, 0xff, sizeof(uint64_t) * 3);
			glGetPerfMonitorCounterInfoAMD(groups[i], counters[j],
						       GL_COUNTER_RANGE_AMD,
						       data);

			/* Validate the type and use it to interpret the
			 * minimum/maximum information.
			 */
			switch (type) {
			case GL_UNSIGNED_INT:
				min_u = ((uint32_t *) data)[0];
				max_u = ((uint32_t *) data)[1];
				unchanged = ((uint32_t *) data)[2];
				is_unsigned = true;
				break;
			case GL_UNSIGNED_INT64_AMD:
				min_u = data[0];
				max_u = data[1];
				unchanged = ((uint32_t *) data)[4];
				is_unsigned = true;
				break;
			case GL_PERCENTAGE_AMD:
			case GL_FLOAT:
				min_f = ((float *) data)[0];
				max_f = ((float *) data)[1];
				unchanged = ((uint32_t *) data)[2];
				break;
			default:
				printf("Group %u/Counter %u has an invalid type: %x\n", groups[i], counters[j], type);
				report(false);
			}

			/* Make sure it didn't write too much data. */
			if (unchanged != 0xffffffff) {
				printf("COUNTER_RANGE_AMD query for group %u/Counter %u wrote too much data to the buffer.\n", groups[i], counters[j]);
				report(false);
			}

			/* "If type value returned is PERCENTAGE_AMD, then this
			 *  describes a float value that is in the range [0.0 ..
			 *  100.0]."  So we can check this.
			 */
			if (type == GL_PERCENTAGE_AMD) {
				if (min_f != 0.0f || max_f != 100.0f) {
					printf("Group %u/Counter %u's minimum (%f) and maximum (%f) must be 0.0f and 100.0f, respectively.\n", groups[i], counters[j], min_f, max_f);
					report(false);
				}
			} else if (is_unsigned) {
				/* The spec doesn't explicitly state it, but it
				 * makes sense for the minimum to be strictly
				 * less than the maximum.  Do a service to
				 * driver authors and validate that.
				 */
				if (min_u >= max_u) {
					printf("Group %u/Counter %u's minimum (%" PRIu64 ") is >= the maximum (%" PRIu64 ").\n", groups[i], counters[j], min_u, max_u);
					report(false);
				}
			} else if (type == GL_FLOAT) {
				if (min_f >= max_f) {
					printf("Group %u/Counter %u's minimum (%f) is >= the maximum (%f).\n", groups[i], counters[j], min_f, max_f);
					report(false);
				}
			}
		}

		free(counters);
	}
	report(true);
}

/******************************************************************************/


/**
 * Call glBeginPerfMonitorAMD() on an invalid monitor ID.
 * (Should be run before any Gen tests to ensure this ID is invalid.)
 *
 * XXX: This isn't actually specified, but it seems like it ought to be.
 */
void
test_begin_invalid_monitor(void)
{
	glBeginPerfMonitorAMD(777);
	report(piglit_check_gl_error(GL_INVALID_VALUE));
}

/**
 * Call glEndPerfMonitorAMD() on an invalid monitor ID.
 * (Should be run before any Gen tests to ensure this ID is invalid.)
 *
 * XXX: This isn't actually specified, but it seems like it ought to be.
 *
 * AMD Catalyst 13.10 (Radeon 6870) instead produces INVALID_OPERATION,
 * presumably because the (invalid) monitor hasn't been started.  (See
 * test_end_without_begin.)  So we allow either here.
 */
void
test_end_invalid_monitor(void)
{
	GLenum error;
	glEndPerfMonitorAMD(777);
	error = glGetError();
	report(error == GL_INVALID_VALUE || error == GL_INVALID_OPERATION);
}

/**
 * Call glGetPerfMonitorCounterDataAMD() with an invalid monitor ID.
 *
 * XXX: This isn't actually specified, but it seems like it ought to be.
 */
static void
test_get_counter_data_invalid_monitor(void)
{
	unsigned value;
	glGetPerfMonitorCounterDataAMD(777, GL_PERFMON_RESULT_AVAILABLE_AMD,
				       0, &value, NULL);
	report(piglit_check_gl_error(GL_INVALID_VALUE));
}

/**
 * Call glSelectPerfMonitorCountersAMD() with an invalid monitor ID.
 *
 * "If <monitor> is not a valid monitor created by GenPerfMonitorsAMD, then
 *  INVALID_VALUE will be generated."
 */
static void
test_select_counters_invalid_monitor(void)
{
	unsigned junk;
	glSelectPerfMonitorCountersAMD(777, false, 0, 0, &junk);
	report(piglit_check_gl_error(GL_INVALID_VALUE));
}

/**
 * Call glDeletePerfMonitorsAMD() on an invalid monitor ID.
 * (Should be run before any Gen tests to ensure this ID is invalid.)
 *
 * "If a monitor ID in the list <monitors> does not reference a previously
 *  generated performance monitor, an INVALID_VALUE error is generated."
 *
 * AMD Catalyst 13.10 (Radeon 6870) fails this test, producing NO_ERROR.
 */
static void
test_delete_monitor_invalid(void)
{
	unsigned monitor = 777;
	glDeletePerfMonitorsAMD(1, &monitor);
	report(piglit_check_gl_error(GL_INVALID_VALUE));
}

/**
 * Mean tests for glGetPerfMonitorCounterDataAMD()'s data return mechanism.
 *
 * AMD Catalyst 13.10 (Radeon 6870) fails this test.  It does not set
 * bytes_written, yet writes 0 for each of these queries.  It apparently
 * interprets these fields as only relevant to the PERFMON_RESULT_AMD query.
 */
static void
test_get_counter_data_byte_size(void)
{
	bool pass = true;
	unsigned monitor;
	unsigned value;
	GLsizei bytes_written;

	glGenPerfMonitorsAMD(1, &monitor);
	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

	/* "It is an INVALID_OPERATION error far <data> to be NULL." */
	glGetPerfMonitorCounterDataAMD(monitor, GL_PERFMON_RESULT_AVAILABLE_AMD,
				       0, NULL, NULL);
	pass = piglit_check_gl_error(GL_INVALID_OPERATION) && pass;

	/* "The argument <dataSize> specifies the number of bytes available in
	 *  the <data> buffer for writing."
	 *
	 * It would be easy to accidentally treat this as 4-byte units, so
	 * be mean and try < sizeof(int) sizes.
	 */

	/* dataSize = 0: Nothing should be written. */
	value = bytes_written = 0xd0d0d0d0;
	glGetPerfMonitorCounterDataAMD(monitor, GL_PERFMON_RESULT_AVAILABLE_AMD,
				       0, &value, &bytes_written);
	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;
	pass = value == 0xd0d0d0d0 && pass;
	pass = bytes_written == 0 && pass;

	/* dataSize = 1: Unclear.  Accept nothing or 1 byte written. */
	value = bytes_written = 0xd0d0d0d0;
	glGetPerfMonitorCounterDataAMD(monitor, GL_PERFMON_RESULT_AVAILABLE_AMD,
				       1, &value, &bytes_written);
	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;
	pass = value == 0xd0d0d0d0 && pass;
	if (bytes_written == 1) {
		pass = value == 0xd0d0d000 && pass;
	} else if (bytes_written == 0) {
		pass = value == 0xd0d0d0d0 && pass;
	} else {
		pass = false;
	}

	glDeletePerfMonitorsAMD(1, &monitor);
	report(pass);
}

static void
test_gen_initial_state(void)
{
	bool pass = true;
	unsigned monitor;
	unsigned value;

	glGenPerfMonitorsAMD(1, &monitor);
	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

	/* "The value of the PERFMON_RESULT_AVAILABLE_AMD, PERMON_RESULT_AMD,
	 *  and PERFMON_RESULT_SIZE queries will all initially be 0."
	 */
	value = 0xd0d0d0d0;
	glGetPerfMonitorCounterDataAMD(monitor, GL_PERFMON_RESULT_AVAILABLE_AMD,
				       4, &value, NULL);
	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;
	pass = value == 0 && pass;

	/* AMD Catalyst 13.10 (Radeon 6870) actually does write 0 for the
	 * PERFMON_RESULT query even though it isn't available.  This
	 * matches the spec, but is strange.
	 */
	value = 0xd0d0d0d0;
	glGetPerfMonitorCounterDataAMD(monitor, GL_PERFMON_RESULT_AMD,
				       4, &value, NULL);
	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;
	pass = value == 0 && pass;

	value = 0xd0d0d0d0;
	glGetPerfMonitorCounterDataAMD(monitor, GL_PERFMON_RESULT_SIZE_AMD,
				       4, &value, NULL);
	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;
	pass = value == 0 && pass;

	glDeletePerfMonitorsAMD(1, &monitor);
	report(pass);
}

/**
 * "INVALID_OPERATION error will be generated if EndPerfMonitorAMD is
 *  called when a performance monitor is not currently started."
 */
void
test_end_without_begin(void)
{
	unsigned monitor;
	glGenPerfMonitorsAMD(1, &monitor);
	glEndPerfMonitorAMD(monitor);
	glDeletePerfMonitorsAMD(1, &monitor);
	report(piglit_check_gl_error(GL_INVALID_OPERATION));
}

/**
 * "INVALID_OPERATION error will be generated if BeginPerfMonitorAMD is
 *  called when a performance monitor is already active."
 */
void
test_double_begin(void)
{
	GLenum error;
	bool pass;
	unsigned monitor;
	glGenPerfMonitorsAMD(1, &monitor);
	glBeginPerfMonitorAMD(monitor);

	error = glGetError();
	if (error != GL_NO_ERROR) {
		glDeletePerfMonitorsAMD(1, &monitor);
		/* Monitoring couldn't start for some reason; bail. */
		if (error == GL_INVALID_OPERATION)
			return;
		/* We weren't expecting this other error. */
		report(false);
	}

	/* Double begin */
	glBeginPerfMonitorAMD(monitor);
	pass = piglit_check_gl_error(GL_INVALID_OPERATION);

	glDeletePerfMonitorsAMD(1, &monitor);
	report(pass);
}

/******************************************************************************/

/**
 * Call glSelectPerfMonitorCountersAMD() with an invalid group ID.
 *
 * "If <group> is not a valid group, the INVALID_VALUE error will be generated."
 */
static void
test_select_counters_invalid_group(unsigned invalid_group)
{
	unsigned monitor;
	unsigned junk;
	bool pass;
	glGenPerfMonitorsAMD(1, &monitor);
	glSelectPerfMonitorCountersAMD(monitor, false, invalid_group, 0, &junk);
	pass = piglit_check_gl_error(GL_INVALID_VALUE);
	glDeletePerfMonitorsAMD(1, &monitor);
	report(pass);
}


/**
 * Call glSelectPerfMonitorCountersAMD() with numCounters < 0.
 *
 * "If <numCounters> is less than 0, an INVALID_VALUE error will be generated."
 */
static void
test_select_counters_invalid_num_counters(unsigned group)
{
	unsigned monitor;
	unsigned junk;
	bool pass;
	glGenPerfMonitorsAMD(1, &monitor);
	glSelectPerfMonitorCountersAMD(monitor, false, group, -1, &junk);
	pass = piglit_check_gl_error(GL_INVALID_VALUE);
	glDeletePerfMonitorsAMD(1, &monitor);
	report(pass);
}

/******************************************************************************/

enum piglit_result
piglit_display(void)
{
	return PIGLIT_FAIL;
}

/**
 * The main test program.
 */
void
piglit_init(int argc, char **argv)
{
	unsigned *groups;
	int num_groups;
	unsigned *g0_counters;
	int num_g0_counters;
	unsigned invalid_group;
	unsigned invalid_counter;

	piglit_require_extension("GL_AMD_performance_monitor");

	/* Basic glGetPerfMonitorGroupsAMD() tests */
	test_number_of_groups_null_num_groups_pointer();
	test_number_of_groups_null_groups_pointer();
	test_number_of_groups_zero_size_array();
	test_number_of_groups_partial_array();

	get_groups(&groups, &num_groups);
	invalid_group = find_invalid_group(groups, num_groups);

	test_get_counters_invalid_group(invalid_group);
	test_group_string_invalid_group(invalid_group);
	test_counter_string_invalid_group(invalid_group);
	test_counter_info_invalid_group(invalid_group);

	test_begin_invalid_monitor();
	test_end_invalid_monitor();
	test_delete_monitor_invalid();
	test_get_counter_data_invalid_monitor();
	test_select_counters_invalid_monitor();
	test_get_counter_data_byte_size();
	test_gen_initial_state();
	test_end_without_begin();
	test_double_begin();

	test_select_counters_invalid_group(invalid_group);

	/* If there are no groups, the rest of the tests can't run.  Bail. */
	if (num_groups == 0)
		exit(0);

	test_get_counters_null_pointers(groups[0]);
	test_get_counters_null_pointer_non_zero_size(groups[0]);
	test_get_counters_zero_size_array(groups[0]);
	test_get_counters_partial_array(groups[0]);
	test_group_string_null_length(groups[0]);
	test_group_string_single_character_buffer(groups[0]);
	test_group_string_small_buffer(groups[0]);
	test_group_string_normal_buffer(groups[0]);

	test_counter_info(groups, num_groups);

	test_select_counters_invalid_num_counters(groups[0]);

	get_counters(groups[0], &g0_counters, &num_g0_counters);
	invalid_counter = find_invalid_counter(g0_counters, num_g0_counters);

	test_counter_string_invalid_counter(groups[0], invalid_counter);
	test_counter_info_invalid_counter(groups[0], invalid_counter);

	/* If there are no counters, the rest of the tests can't run.  Bail. */
	if (num_g0_counters == 0)
		exit(0);

	test_counter_string_null_length(groups[0], g0_counters[0]);
	test_counter_string_single_character_buffer(groups[0], g0_counters[0]);
	test_counter_string_small_buffer(groups[0], g0_counters[0]);
	test_counter_string_normal_buffer(groups[0], g0_counters[0]);

	exit(0);
}
