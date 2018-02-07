/*
 * Copyright © 2018 Broadcom
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
 * \file vc4.c
 *
 * Check consistency of some of the VC4 perf counters.
 */

#define __STDC_FORMAT_MACROS
#include <inttypes.h>
#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 20;
	config.window_visual = PIGLIT_GL_VISUAL_RGB;

PIGLIT_GL_TEST_CONFIG_END

#define verify(x)                                                            \
	if (!(x)) {                                                          \
		printf("%s:%i\n", __func__, __LINE__);			     \
		piglit_report_subtest_result(PIGLIT_FAIL, "%s", test->name); \
		return;                                                      \
	}

/******************************************************************************/

struct perfcounter_id {
	unsigned groupid;
	unsigned counterid;
};

struct perfmon_counter {
	const char *name;
	unsigned id;
	GLenum type;
};

struct perfmon_group {
	const char *name;
	unsigned id;
	int num_counters;
	int max_active_counters;
	struct perfmon_counter *counters;
};

struct perfmon_info {
	int num_groups;
	struct perfmon_group *groups;
};

static void
get_group_info(struct perfmon_group *group)
{
	unsigned *counterids;
	GLsizei length;
	char *name;
	int i;

	glGetPerfMonitorGroupStringAMD(group->id, 0, &length, NULL);
	name = calloc(length + 1, sizeof(char));
	group->name = name;
	glGetPerfMonitorGroupStringAMD(group->id, length + 1, NULL, name);

	glGetPerfMonitorCountersAMD(group->id, &group->num_counters,
				    NULL, 0, NULL);
	group->counters = calloc(group->num_counters, sizeof(*group->counters));
	counterids = calloc(group->num_counters, sizeof(*counterids));
	glGetPerfMonitorCountersAMD(group->id, NULL,
				    &group->max_active_counters,
				    group->num_counters, counterids);

	for (i = 0; i < group->num_counters; i++) {
		group->counters[i].id = counterids[i];
		glGetPerfMonitorCounterStringAMD(group->id, counterids[i], 0,
						 &length, NULL);
		name = calloc(length + 1, sizeof(char));
		group->counters[i].name = name;
		glGetPerfMonitorCounterStringAMD(group->id, counterids[i],
						 length + 1, NULL, name);
		glGetPerfMonitorCounterInfoAMD(group->id, counterids[i],
					       GL_COUNTER_TYPE_AMD,
					       &group->counters[i].type);
	}

	free(counterids);
}

static void
get_perfmon_info(struct perfmon_info *info)
{
	unsigned *groupids;
	int i;

	glGetPerfMonitorGroupsAMD(&info->num_groups, 0, NULL);
	info->groups = calloc(info->num_groups, sizeof(*info->groups));
	groupids = calloc(info->num_groups, sizeof(*groupids));

	glGetPerfMonitorGroupsAMD(NULL, info->num_groups, groupids);

	for (i = 0; i < info->num_groups; i++) {
		info->groups[i].id = groupids[i];
		get_group_info(&info->groups[i]);
	}

	free(groupids);
}

static bool
find_perfcounter(const struct perfmon_info *info, const char *group_name,
		 const char *counter_name, struct perfcounter_id *id)
{
	int i, j;

	for (i = 0; i < info->num_groups; i++) {
		struct perfmon_group *group = &info->groups[i];

		if (strcmp(group->name, group_name))
			continue;

		for (j = 0; j < group->num_counters; j++) {
			struct perfmon_counter *counter = &group->counters[j];

			if (strcmp(counter->name, counter_name))
				continue;

			id->groupid = i;
			id->counterid = j;
			return true;
		}
	}

	return false;
}

/******************************************************************************/

enum piglit_result
piglit_display(void)
{
	return PIGLIT_FAIL;
}

struct counter_res {
	unsigned group;
	unsigned counter;
	uint64_t val;
};

struct perfmon_test {
	const char *name;
	const char *group;
	const char *counter;
	void (*job)(const struct perfmon_test *test);
	bool (*check_res)(uint64_t res);
};

static void
do_perfmon_test(const struct perfmon_info *info,
		const struct perfmon_test *test)
{
	struct perfcounter_id counterid;
	unsigned perfmon, counter;
	struct counter_res res = { };
	unsigned avail = 0;
	int written = 0;

	if (!find_perfcounter(info, test->group, test->counter,
			      &counterid)) {
		piglit_report_subtest_result(PIGLIT_SKIP, "%s", test->name);
		return;
	}

	glGenPerfMonitorsAMD(1, &perfmon);
	verify(piglit_check_gl_error(GL_NO_ERROR));

	counter = counterid.counterid;
	glSelectPerfMonitorCountersAMD(perfmon, true, counterid.groupid, 1,
				       &counter);

	/* Start monitoring. */
	glBeginPerfMonitorAMD(perfmon);
	verify(piglit_check_gl_error(GL_NO_ERROR));

	test->job(test);

	/* Stop monitoring. */
	glEndPerfMonitorAMD(perfmon);
	verify(piglit_check_gl_error(GL_NO_ERROR));

	while (!avail) {
		glGetPerfMonitorCounterDataAMD(perfmon,
					       GL_PERFMON_RESULT_AVAILABLE_AMD,
					       sizeof(avail), &avail,
					       &written);
		verify(piglit_check_gl_error(GL_NO_ERROR));
		verify(written == sizeof(avail));
	}

	glGetPerfMonitorCounterDataAMD(perfmon, GL_PERFMON_RESULT_AMD,
				       sizeof(res), (GLuint *)&res,
				       &written);
	verify(piglit_check_gl_error(GL_NO_ERROR));
	verify(written == sizeof(res));
	verify(res.group == 0 && res.counter == counter);
	verify(test->check_res(res.val));

	piglit_report_subtest_result(PIGLIT_PASS, "%s", test->name);
}

#define FEP_VALID_QUADS_REF_VAL		6440

static void draw_rect(const struct perfmon_test *test)
{
	piglit_draw_rect(-1, -1, 3, 3);
}

static void draw_tex(const struct perfmon_test *test)
{
	GLuint tex;

	tex = piglit_rgbw_texture(GL_RGBA, 64, 64, false, true,
				  GL_UNSIGNED_BYTE);
	verify(piglit_check_gl_error(GL_NO_ERROR));

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, tex);
	piglit_draw_rect_tex(-1, -1, 2, 2, 0, 0, 1, 1);
	glDisable(GL_TEXTURE_2D);
	glDeleteTextures(1, &tex);
}

static bool fep_valid_quads_check_res(uint64_t res)
{
	return res == FEP_VALID_QUADS_REF_VAL;
}

static bool is_zero(uint64_t res)
{
	return !res;
}

static bool not_zero(uint64_t res)
{
	return res;
}

static const struct perfmon_test tests[] = {
	{
		.name = "fep-valid-quads",
		.group = "V3D counters",
		.counter = "FEP-valid-quads",
		.job = draw_rect,
		.check_res = fep_valid_quads_check_res,
	},
	{
		.name = "no-tex-qpu-wait-tmu-zero",
		.group = "V3D counters",
		.counter = "QPU-total-clk-cycles-waiting-TMU",
		.job = draw_rect,
		.check_res = is_zero,
	},
	{
		.name = "tex-qpu-wait-tmu-not-zero",
		.group = "V3D counters",
		.counter = "QPU-total-clk-cycles-waiting-TMU",
		.job = draw_tex,
		.check_res = not_zero,
	},
};

/**
 * The main test program.
 */
void
piglit_init(int argc, char **argv)
{
	struct perfmon_info info;
	int i;

	piglit_require_extension("GL_AMD_performance_monitor");

	get_perfmon_info(&info);

	for (i = 0; i < ARRAY_SIZE(tests); i++)
		do_perfmon_test(&info, &tests[i]);

	exit(0);
}
