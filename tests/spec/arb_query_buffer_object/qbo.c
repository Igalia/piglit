/*
 * Copyright © 2015 Glenn Kennard
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
 *
 * Author: Glenn Kennard <glenn.kennard@gmail.com>
 */

/**
 * \file qbo.c
 * Tests ARB_query_buffer_object
 * - synchronous wait for result
 * - asynchrounous result, default value is left intact if result unavailable
 * - asynchrounous result, retrieve result to client memory before & after
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN
	config.supports_gl_compat_version = 32;
	config.supports_gl_core_version = 32;
	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE |
		PIGLIT_GL_VISUAL_DEPTH;

PIGLIT_GL_TEST_CONFIG_END

#define BUFFER_OFFSET(i) ((void *)((char *)NULL + i))

static const float green[] = {0, 1, 0, 1};

static unsigned query;
static unsigned qbo;

static int prog;
static int qbo_prog;
static int sync_mode_loc;
static int expect_exact_loc;
static int is_64bit_loc;
static int expected_loc;
static int expected_hi_loc;

enum sync_mode {
	QBO_SYNC,
	QBO_SYNC_CPU_READ_AFTER_CACHE_TEST,
	QBO_ASYNC,
	QBO_ASYNC_CPU_READ_BEFORE,
	QBO_ASYNC_CPU_READ_AFTER,
	NUM_QBO_SYNC_MODES,
};

static const char * const sync_mode_names[] = {
	"SYNC",
	"SYNC_CPU_READ_AFTER_CACHE_TEST",
	"ASYNC",
	"ASYNC_CPU_READ_BEFORE",
	"ASYNC_CPU_READ_AFTER",
};

static GLenum query_type;
static enum sync_mode sync_mode;
static GLenum result_type;

struct query_type_desc {
	GLenum type;
	const char *extensions[2];
};

/* Note: meaningful test cases (with non-zero values) for the following are
 * missing:
 *  - GL_COMPUTE_SHADER_INVOCATIONS_ARB
 *  - GL_GEOMETRY_SHADER_INVOCATIONS
 *  - GL_GEOMETRY_SHADER_PRIMITIVES_EMITTED_ARB
 *  - GL_TESS_CONTROL_SHADER_PATCHES_ARB
 *  - GL_TESS_EVALUATION_SHADER_INVOCATIONS_ARB
 *  - GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN
 */
static const struct query_type_desc query_types[] = {
	{ GL_ANY_SAMPLES_PASSED,			{ "GL_ARB_occlusion_query2", NULL } },
	{ GL_ANY_SAMPLES_PASSED_CONSERVATIVE,		{ "GL_ARB_ES3_compatibility", NULL } },
	{ GL_CLIPPING_INPUT_PRIMITIVES_ARB,		{ "GL_ARB_pipeline_statistics_query", NULL } },
	{ GL_CLIPPING_OUTPUT_PRIMITIVES_ARB,		{ "GL_ARB_pipeline_statistics_query", NULL } },
	{ GL_COMPUTE_SHADER_INVOCATIONS_ARB,		{ "GL_ARB_pipeline_statistics_query", "GL_ARB_compute_shader" } },
	{ GL_FRAGMENT_SHADER_INVOCATIONS_ARB,		{ "GL_ARB_pipeline_statistics_query", NULL } },
	{ GL_GEOMETRY_SHADER_INVOCATIONS,		{ "GL_ARB_pipeline_statistics_query", NULL } },
	{ GL_GEOMETRY_SHADER_PRIMITIVES_EMITTED_ARB,	{ "GL_ARB_pipeline_statistics_query", NULL } },
	{ GL_PRIMITIVES_GENERATED,			{ NULL, } },
	{ GL_PRIMITIVES_SUBMITTED_ARB,			{ "GL_ARB_pipeline_statistics_query", NULL } },
	{ GL_SAMPLES_PASSED_ARB,			{ NULL, } },
	{ GL_TESS_CONTROL_SHADER_PATCHES_ARB,		{ "GL_ARB_pipeline_statistics_query", "GL_ARB_tessellation_shader" } },
	{ GL_TESS_EVALUATION_SHADER_INVOCATIONS_ARB,	{ "GL_ARB_pipeline_statistics_query", "GL_ARB_tessellation_shader" } },
	{ GL_TIMESTAMP,					{ "GL_ARB_timer_query", NULL } },
	{ GL_TIME_ELAPSED,				{ "GL_ARB_timer_query", NULL } },
	{ GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN,	{ NULL, } },
	{ GL_VERTEX_SHADER_INVOCATIONS_ARB,		{ "GL_ARB_pipeline_statistics_query", NULL } },
	{ GL_VERTICES_SUBMITTED_ARB,			{ "GL_ARB_pipeline_statistics_query", NULL } },
};

static void
get_query_values(GLenum query_type, bool *exact, uint32_t *expected)
{
	*exact = true;

	switch (query_type) {
	case GL_ANY_SAMPLES_PASSED:
	case GL_ANY_SAMPLES_PASSED_CONSERVATIVE:
		*expected = 1;
		break;
	case GL_CLIPPING_INPUT_PRIMITIVES_ARB:
	case GL_CLIPPING_OUTPUT_PRIMITIVES_ARB:
		*exact = false;
		*expected = 1;
		break;
	case GL_FRAGMENT_SHADER_INVOCATIONS_ARB:
		*exact = false;
		*expected = 1;
		break;
	case GL_PRIMITIVES_GENERATED:
	case GL_PRIMITIVES_SUBMITTED_ARB:
		*exact = false;
		*expected = 1;
		break;
	case GL_SAMPLES_PASSED_ARB:
		*expected = piglit_width * piglit_height;
		break;
	case GL_TIMESTAMP:
	case GL_TIME_ELAPSED:
		*exact = false;
		*expected = 1;
		break;
	case GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN:
		*expected = 0;
		break;
	case GL_VERTEX_SHADER_INVOCATIONS_ARB:
	case GL_VERTICES_SUBMITTED_ARB:
		*exact = false;
		*expected = 1;
		break;
	case GL_COMPUTE_SHADER_INVOCATIONS_ARB:
	case GL_GEOMETRY_SHADER_INVOCATIONS:
	case GL_GEOMETRY_SHADER_PRIMITIVES_EMITTED_ARB:
	case GL_TESS_CONTROL_SHADER_PATCHES_ARB:
	case GL_TESS_EVALUATION_SHADER_INVOCATIONS_ARB:
		*expected = 0;
		break;
	default:
		abort();
	}
}

static enum piglit_result
cpu_gather_query(bool exact, uint32_t expected, uint64_t *cpu_result)
{
	*cpu_result = 0;

	glBindBuffer(GL_QUERY_BUFFER, 0);

	if (result_type == GL_INT)
		glGetQueryObjectiv(query, GL_QUERY_RESULT, (GLint*)cpu_result);
	else if (result_type == GL_UNSIGNED_INT)
		glGetQueryObjectuiv(query, GL_QUERY_RESULT, (GLuint*)cpu_result);
	else
		glGetQueryObjectui64v(query, GL_QUERY_RESULT, cpu_result);

	glBindBuffer(GL_QUERY_BUFFER, qbo);

	return (exact ? *cpu_result == expected : *cpu_result >= expected)
		? PIGLIT_PASS : PIGLIT_FAIL;
}

enum piglit_result
run_subtest(void)
{
	bool exact;
	uint32_t expected;
	uint64_t cpu_result;
	bool have_cpu_result = false;
	uint32_t default_value[4] = { 0xccccccccu, 0xccccccccu, 0xccccccccu, 0xccccccccu };
	bool is_sync =
		sync_mode == QBO_SYNC ||
		sync_mode == QBO_SYNC_CPU_READ_AFTER_CACHE_TEST;

	get_query_values(query_type, &exact, &expected);

	glClearColor(0.5, 0.5, 0.5, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	/* Load default value into buffer */
	glBindBuffer(GL_QUERY_BUFFER, qbo);
	glBufferData(GL_QUERY_BUFFER, 16, default_value, GL_DYNAMIC_COPY);

	/* Enable query, draw something that should pass */
	glEnable(GL_DEPTH_TEST);
	glUseProgram(prog);
	glGenQueries(1, &query);
	if (query_type != GL_TIMESTAMP)
		glBeginQuery(query_type, query);
	piglit_draw_rect_z(0.5, -1, -1, 2, 2);
	if (query_type != GL_TIMESTAMP)
		glEndQuery(query_type);
	else
		glQueryCounter(query, query_type);

	if (sync_mode == QBO_ASYNC_CPU_READ_BEFORE) {
		if (cpu_gather_query(exact, expected, &cpu_result))
			return PIGLIT_FAIL;
		have_cpu_result = true;
	}

	glBindBuffer(GL_QUERY_BUFFER, qbo);
	if (is_sync) {
		/* Special mode to test against a possible cache invalidation
		 * in case the wait-for-result is handled at a different place
		 * in the memory hierarchy than actually reading and
		 * summarizing the result.
		 */
		if (sync_mode == QBO_SYNC_CPU_READ_AFTER_CACHE_TEST)
			glGetQueryObjectivARB(query, GL_QUERY_RESULT_NO_WAIT, BUFFER_OFFSET(0));

		if (result_type == GL_INT)
			glGetQueryObjectivARB(query, GL_QUERY_RESULT, BUFFER_OFFSET(0));
		else if (result_type == GL_UNSIGNED_INT)
			glGetQueryObjectuivARB(query, GL_QUERY_RESULT, BUFFER_OFFSET(0));
		else
			glGetQueryObjectui64v(query, GL_QUERY_RESULT, BUFFER_OFFSET(0));
	} else {
		if (result_type == GL_INT) {
			glGetQueryObjectivARB(query, GL_QUERY_RESULT_AVAILABLE, BUFFER_OFFSET(8));
			glGetQueryObjectivARB(query, GL_QUERY_RESULT_NO_WAIT, BUFFER_OFFSET(0));
		} else if (result_type == GL_UNSIGNED_INT) {
			glGetQueryObjectuivARB(query, GL_QUERY_RESULT_AVAILABLE, BUFFER_OFFSET(8));
			glGetQueryObjectuivARB(query, GL_QUERY_RESULT_NO_WAIT, BUFFER_OFFSET(0));
		} else {
			glGetQueryObjectui64v(query, GL_QUERY_RESULT_AVAILABLE, BUFFER_OFFSET(8));
			glGetQueryObjectui64v(query, GL_QUERY_RESULT_NO_WAIT, BUFFER_OFFSET(0));
		}
	}

	if (sync_mode == QBO_SYNC_CPU_READ_AFTER_CACHE_TEST ||
	    sync_mode == QBO_ASYNC_CPU_READ_AFTER) {
		if (cpu_gather_query(exact, expected, &cpu_result))
			return PIGLIT_FAIL;
		have_cpu_result = true;
	}

	/* Make it available to shader as uniform buffer 0 */
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, qbo);

	glUseProgram(qbo_prog);

	/* Setup program uniforms */
	glUniform1ui(sync_mode_loc, is_sync ? GL_TRUE : GL_FALSE);
	glUniform1ui(expect_exact_loc, have_cpu_result || exact);
	glUniform1ui(is_64bit_loc, result_type == GL_UNSIGNED_INT64_ARB);
	glUniform1ui(expected_loc, have_cpu_result ? cpu_result : expected);
	glUniform1ui(expected_hi_loc, have_cpu_result ? (cpu_result >> 32) : 0);

	glDisable(GL_DEPTH_TEST);
	/* Draw green if query successful */
	piglit_draw_rect(-1, -1, 2, 2);

	glDeleteQueries(1, &query);

	if (!piglit_probe_rect_rgba(0, 0, piglit_width, piglit_height, green)) {
		unsigned *ptr = glMapBuffer(GL_QUERY_BUFFER, GL_READ_ONLY);

		printf("Expected: %u\n", expected);
		if (have_cpu_result)
			printf("CPU result: %lu\n", cpu_result);
		printf("QBO: %u %u %u %u\n", ptr[0], ptr[1], ptr[2], ptr[3]);
		glUnmapBuffer(GL_QUERY_BUFFER);

		return PIGLIT_FAIL;
	}

	return PIGLIT_PASS;
}

enum piglit_result
piglit_display(void)
{
	static const GLenum result_types[] = {
		GL_INT,
		GL_UNSIGNED_INT,
		GL_UNSIGNED_INT64_ARB
	};
	enum piglit_result r = PIGLIT_PASS;

	for (unsigned qnum = 0; qnum < ARRAY_SIZE(query_types); qnum++) {
		const struct query_type_desc *desc = &query_types[qnum];
		bool supported = true;

		query_type = desc->type;

		for (unsigned i = 0; i < ARRAY_SIZE(desc->extensions); ++i) {
			if (!desc->extensions[i])
				break;

			if (!piglit_is_extension_supported(desc->extensions[i])) {
				supported = false;
				break;
			}
		}

		for (sync_mode = QBO_SYNC;
		     sync_mode < NUM_QBO_SYNC_MODES;
		     sync_mode++) {
			for (unsigned ridx = 0; ridx < ARRAY_SIZE(result_types); ++ridx) {
				enum piglit_result subtest_result = PIGLIT_SKIP;

				result_type = result_types[ridx];

				if (supported) {
					subtest_result = run_subtest();
					if (subtest_result != PIGLIT_PASS)
						r = subtest_result;
				}

				piglit_report_subtest_result(subtest_result, "query-%s-%s-%s",
						piglit_get_gl_enum_name(query_type),
						sync_mode_names[sync_mode],
						piglit_get_gl_enum_name(result_type));
			}
		}
	}

	return r;
}

void
piglit_init(int argc, char **argv)
{
	char *vsCode;
	char *fsCode, *qboFsCode;

	piglit_require_extension("GL_ARB_query_buffer_object");
	piglit_require_extension("GL_ARB_uniform_buffer_object");

	glGenBuffers(1, &qbo);
	glBindBuffer(GL_QUERY_BUFFER, qbo);
	glBufferData(GL_QUERY_BUFFER, 4, NULL, GL_DYNAMIC_COPY);

	vsCode =
		"#version 150\n"
		"in vec4 pos_in;\n"
		"void main() {\n"
		"	gl_Position = pos_in;\n"
		"}\n";
	fsCode =
		"#version 150\n"
		"out vec4 color;\n"
		"void main() {\n"
		"	color = vec4(0.0, 0.0, 1.0, 1.0);\n"
		"}\n";
	qboFsCode =
		"#version 150\n"
		"#extension GL_ARB_uniform_buffer_object : require\n"
		"uniform query {\n"
		"	uint result;\n"
		"	uint result_hi;\n"
		"	uint available;\n"
		"	uint available_hi;\n"
		"};\n"
		"uniform bool sync_mode;\n"
		"uniform bool expect_exact;\n"
		"uniform bool is_64bit;\n"
		"uniform uint expected;\n"
		"uniform uint expected_hi;\n"
		"out vec4 color;\n"
		"void main() {\n"
		"	uint INIT = uint(0xcccccccc);\n"
		"	bool ready = sync_mode || available != 0u;\n"
		"	if (!is_64bit && (result_hi != INIT || available_hi != INIT)) {\n"
		"		color = vec4(1.0, 0.0, 0.25, 1.0);\n"
		"	} else if ((sync_mode && (available != INIT ||\n"
		"	                          available_hi != INIT)) ||\n"
		"	           (!sync_mode && ((available != 0u && available != 1u) ||\n"
		"	                           (is_64bit && available_hi != 0u) ||\n"
		"	                           (!is_64bit && available_hi != INIT)))) {\n"
		"		color = vec4(1.0, 0.0, 0.5, 1.0);\n"
		"	} else {\n"
		"		bool result_ok = false;\n"
		"		if (result == expected &&\n"
		"		    (!is_64bit || result_hi == expected_hi))\n"
		"			result_ok = true;\n"
		"		if (!expect_exact &&\n"
		"		    ((!is_64bit && result >= expected) ||\n"
		"		     (is_64bit && ((result_hi == expected_hi && result >= expected) ||\n"
		"		                   (result_hi > expected_hi)))))\n"
		"			result_ok = true;\n"
		"		if (!ready && result == INIT && result_hi == INIT)\n"
		"			result_ok = true;\n"
		"		if (result_ok) {\n"
		"			color = vec4(0.0, 1.0, 0.0, 1.0);\n"
		"		} else if (ready) {\n"
		"			color = vec4(1.0, 0.0, 0.0, 1.0);\n"
		"		} else {\n"
		"			color = vec4(1.0, 0.5, 0.0, 1.0);\n"
		"		}\n"
		"	}\n"
		"}\n";

	prog = piglit_build_simple_program(vsCode, fsCode);
	qbo_prog = piglit_build_simple_program(vsCode, qboFsCode);
	sync_mode_loc = glGetUniformLocation(qbo_prog, "sync_mode");
	expect_exact_loc = glGetUniformLocation(qbo_prog, "expect_exact");
	is_64bit_loc = glGetUniformLocation(qbo_prog, "is_64bit");
	expected_loc = glGetUniformLocation(qbo_prog, "expected");
	expected_hi_loc = glGetUniformLocation(qbo_prog, "expected_hi");
}
