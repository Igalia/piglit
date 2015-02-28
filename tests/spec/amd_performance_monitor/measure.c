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
 * \file measure.c
 *
 * Some AMD_performance_monitor tests that actually measure things.
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
get_counters(unsigned group, unsigned **counters, int *num_counters,
	     int *max_active_counters)
{
	glGetPerfMonitorCountersAMD(group, num_counters, NULL, 0, NULL);
	*counters = calloc(*num_counters, sizeof(unsigned));
	glGetPerfMonitorCountersAMD(group, NULL, max_active_counters,
				    *num_counters, *counters);
}

#define verify(x)                                                           \
	if (!(x)) {                                                         \
		piglit_report_subtest_result(PIGLIT_FAIL, "%s", test_name); \
		return;                                                     \
	}

/******************************************************************************/

/**
 * Poll until PERFMON_RESULT_AVAILABLE returns 1; glFinish() on each iteration.
 *
 * Only loop for 5 times to guard against implementations that never finish.
 */
static bool
wait_until_available(unsigned monitor)
{
	int i;
	unsigned available = 0;
	for (i = 0; !available && i < 5; i++) {
		glFinish();
		glGetPerfMonitorCounterDataAMD(monitor,
					       GL_PERFMON_RESULT_AVAILABLE_AMD,
					       sizeof(unsigned), &available,
					       NULL);
	}
	return available;
}

/**
 * Basic functional test: enable all the counters in the first group
 * (up to the maximum that can be active at a time), begin monitoring,
 * end monitoring, make sure results are available, sanity check the
 * result size, and get the results.
 */
static void
test_basic_measurement(unsigned group)
{
	unsigned monitor;
	unsigned *counters;
	int num_counters;
	int max_active_counters;
	unsigned usable_counters;
	unsigned result_size = 0;
	GLsizei bytes_written = 0;
	unsigned *data;

	uint32_t *p;
	unsigned value;

	const char *test_name;

	/**
	 * Test #1: Basic Measurement.
	 *
	 * Enable all the counters in the first group (up to the maximum that
	 * can be active at a time), begin monitoring, end monitoring, make
	 * sure results are available, sanity check the result size, and get
	 * the results.
	 */
	test_name = "basic measurement";

	get_counters(group, &counters, &num_counters,
		     &max_active_counters);
	verify(max_active_counters >= 0);
	verify(piglit_check_gl_error(GL_NO_ERROR));

	usable_counters = MIN2(num_counters, max_active_counters);

	glGenPerfMonitorsAMD(1, &monitor);
	verify(piglit_check_gl_error(GL_NO_ERROR));

	/* Enable counters 0 .. usable_counters from the list. */
	glSelectPerfMonitorCountersAMD(monitor, true, group, usable_counters,
				       counters);
	verify(piglit_check_gl_error(GL_NO_ERROR));

	/* Start monitoring */
	glBeginPerfMonitorAMD(monitor);
	verify(piglit_check_gl_error(GL_NO_ERROR));

	/* Drawing...meh */
	glFinish();

	/* End monitoring */
	glEndPerfMonitorAMD(monitor);
	verify(piglit_check_gl_error(GL_NO_ERROR));

	/* Wait for the result to be available. */
	verify(wait_until_available(monitor));
	verify(piglit_check_gl_error(GL_NO_ERROR));

	/* Get the result size. */
	glGetPerfMonitorCounterDataAMD(monitor, GL_PERFMON_RESULT_SIZE_AMD,
				       sizeof(unsigned), &result_size, NULL);
	verify(piglit_check_gl_error(GL_NO_ERROR));

	/* Make sure the size is in bytes. */
	verify(result_size % sizeof(unsigned) == 0);

	/* The format is <Group ID, Group, Value>.  The first two are
	 * uint32_ts.  Value is either a float, uint32_t, or uint64_t.
	 * As a sanity check, make sure the result size is within
	 * reasonable limits.  Don't bother checking the actual types
	 * since that's a bunch of work.
	 */
	verify(result_size >= 3 * sizeof(uint32_t) * usable_counters)
	verify(result_size <=
	       (2 * sizeof(uint32_t) + sizeof(uint64_t)) * usable_counters);

	/* Get the results. */
	data = calloc(1, result_size);
	glGetPerfMonitorCounterDataAMD(monitor, GL_PERFMON_RESULT_AMD,
				       result_size, data, &bytes_written);

	verify(bytes_written == result_size);

	piglit_report_subtest_result(PIGLIT_PASS, "%s", test_name);

	/**
	 * Test #2: Verify counter results against specified range.
	 */
	test_name = "counters in range";
	p = data;
	while ((char *) p < ((char *) data) + bytes_written) {
		uint32_t group_id = p[0];
		uint32_t counter_id = p[1];

		/* Counter values */
		uint32_t u32 = p[2];
		float f = ((float *) p)[2];

		/* Query results */
		GLenum counter_type = GL_NONE;
		uint64_t range[2];

		/* There's only one group, so it better match */
		verify(group_id == group);

		/* Getting the counter data also validates the counter ID
		 * without having to walk through the whole list of counters.
		 */
		glGetPerfMonitorCounterInfoAMD(group_id, counter_id,
					       GL_COUNTER_TYPE_AMD,
					       &counter_type);
		verify(piglit_check_gl_error(GL_NO_ERROR));

		glGetPerfMonitorCounterInfoAMD(group_id, counter_id,
					       GL_COUNTER_RANGE_AMD,
					       range);
		verify(piglit_check_gl_error(GL_NO_ERROR));

		/* Make sure it falls within the proper range */
		switch (counter_type) {
		case GL_UNSIGNED_INT: {
			uint32_t min = ((uint32_t *) range)[0];
			uint32_t max = ((uint32_t *) range)[1];
			verify(u32 >= min);
			verify(u32 <= max);
			break;
		}
		case GL_UNSIGNED_INT64_AMD: {
			uint64_t u64 = ((uint64_t *) p)[1];
			verify(u64 >= range[0]);
			verify(u64 <= range[1]);
			break;
		}
		case GL_PERCENTAGE_AMD:
		case GL_FLOAT: {
			float min = ((float *) range)[0];
			float max = ((float *) range)[1];
			verify(f >= min);
			verify(f <= max);
			break;
		}
		}

		p += (counter_type == GL_UNSIGNED_INT64_AMD) ? 4 : 3;
	}
	verify(result_size == ((char *) p - (char *) data));

	piglit_report_subtest_result(PIGLIT_PASS, "%s", test_name);

	free(data);

	/**
	 * Test #3: Changing the set of active counters resets queries.
	 *
	 * "When SelectPerfMonitorCountersAMD is called on a monitor, any
	 *  outstanding results for that monitor become invalidated and the
	 *  result queries PERFMON_RESULT_SIZE_AMD and
	 *  PERFMON_RESULT_AVAILABLE_AMD are reset to 0."
	 */
	test_name = "selecting counters resets queries";

	/* Turn off the first counter. */
	glSelectPerfMonitorCountersAMD(monitor, false, group, 1, counters);
	verify(piglit_check_gl_error(GL_NO_ERROR));

	/* Results should no longer be available.  All queries should
	 * return 0.
	 */
	value = 0xd0d0d0d0;
	glGetPerfMonitorCounterDataAMD(monitor, GL_PERFMON_RESULT_AVAILABLE_AMD,
				       sizeof(unsigned), &value, NULL);
	verify(piglit_check_gl_error(GL_NO_ERROR));
	verify(value == 0);

	value = 0xd0d0d0d0;
	glGetPerfMonitorCounterDataAMD(monitor, GL_PERFMON_RESULT_SIZE_AMD,
				       sizeof(unsigned), &value, NULL);
	verify(piglit_check_gl_error(GL_NO_ERROR));
	verify(value == 0);

	piglit_report_subtest_result(PIGLIT_PASS, "%s", test_name);

	glDeletePerfMonitorsAMD(1, &monitor);
}


/**
 * Make sure that calling SelectPerfMonitorCountersAMD on an active monitor
 * is possible, resets active queries, and restarts monitoring (so it remains
 * active).
 *
 * This is not actually specified, but matches the behavior of AMD's driver.
 * Being an AMD extension, other implementations should probably match theirs.
 */
static void
test_change_counters_while_active(unsigned group)
{
	unsigned monitor;
	unsigned *counters;
	int num_counters;
	int max_active_counters;
	unsigned usable_counters;
	unsigned data;
	const char *test_name = "change counters while active";

	get_counters(group, &counters, &num_counters,
		     &max_active_counters);
	verify(max_active_counters >= 0);
	verify(piglit_check_gl_error(GL_NO_ERROR));

	usable_counters = MIN2(num_counters, max_active_counters);

	if (usable_counters == 0)
		return; /* skip */

	glGenPerfMonitorsAMD(1, &monitor);
	verify(piglit_check_gl_error(GL_NO_ERROR));

	/* Enable counters 0 .. usable_counters from the list. */
	glSelectPerfMonitorCountersAMD(monitor, true, group, usable_counters,
				       counters);
	verify(piglit_check_gl_error(GL_NO_ERROR));

	/* Start monitoring */
	glBeginPerfMonitorAMD(monitor);
	verify(piglit_check_gl_error(GL_NO_ERROR));

	/* Turn off the first counter.  The specification is unclear whether
	 * or not this should be allowed while monitoring is active, but it
	 * apparently is (Catalyst 12.06 on a Radeon 3650).
	 */
	glSelectPerfMonitorCountersAMD(monitor, false, group, 1, counters);
	verify(piglit_check_gl_error(GL_NO_ERROR));

	/* Verify that all queries have been reset to 0 */
	data = 0xd0d0d0d0;
	glGetPerfMonitorCounterDataAMD(monitor, GL_PERFMON_RESULT_AVAILABLE_AMD,
				       sizeof(unsigned), &data, NULL);
	verify(piglit_check_gl_error(GL_NO_ERROR));
	verify(data == 0);

	data = 0xd0d0d0d0;
	glGetPerfMonitorCounterDataAMD(monitor, GL_PERFMON_RESULT_SIZE_AMD,
				       sizeof(unsigned), &data, NULL);
	verify(piglit_check_gl_error(GL_NO_ERROR));
	verify(data == 0);

	/* The spec doesn't explicitly mention whether or not monitoring
	 * is still active, but apparently it is.
	 */
	glEndPerfMonitorAMD(monitor);
	verify(piglit_check_gl_error(GL_NO_ERROR));

	glDeletePerfMonitorsAMD(1, &monitor);
	verify(piglit_check_gl_error(GL_NO_ERROR));

	piglit_report_subtest_result(PIGLIT_PASS, "%s", test_name);
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

	piglit_require_extension("GL_AMD_performance_monitor");

	/* Basic glGetPerfMonitorGroupsAMD() tests */
	get_groups(&groups, &num_groups);

	/* If there are no groups, the rest of the tests can't run.  Bail. */
	if (num_groups == 0)
		exit(0);

	test_basic_measurement(groups[0]);
	test_change_counters_while_active(groups[0]);

	exit(0);
}
