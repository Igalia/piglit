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

#include "common.h"

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

static const struct query_type_desc *query_desc;
static enum sync_mode sync_mode;
static GLenum result_type;

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

	get_query_values(query_desc, &exact, &expected);

	glGenQueries(1, &query);
	run_query(query, query_desc);

	/* Load default value into buffer */
	glBindBuffer(GL_QUERY_BUFFER, qbo);
	glBufferData(GL_QUERY_BUFFER, 16, default_value, GL_DYNAMIC_COPY);

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

	for (unsigned qnum = 0; qnum < num_query_types(); qnum++) {
		query_desc = &query_types[qnum];

		bool supported = is_query_supported(query_desc);

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
						piglit_get_gl_enum_name(query_desc->type),
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
	char *qboFsCode;

	piglit_require_extension("GL_ARB_query_buffer_object");
	piglit_require_extension("GL_ARB_uniform_buffer_object");

	query_common_init();

	glGenBuffers(1, &qbo);
	glBindBuffer(GL_QUERY_BUFFER, qbo);
	glBufferData(GL_QUERY_BUFFER, 4, NULL, GL_DYNAMIC_COPY);

	vsCode =
		"#version 150\n"
		"in vec4 pos_in;\n"
		"void main() {\n"
		"	gl_Position = pos_in;\n"
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

	qbo_prog = piglit_build_simple_program(vsCode, qboFsCode);
	sync_mode_loc = glGetUniformLocation(qbo_prog, "sync_mode");
	expect_exact_loc = glGetUniformLocation(qbo_prog, "expect_exact");
	is_64bit_loc = glGetUniformLocation(qbo_prog, "is_64bit");
	expected_loc = glGetUniformLocation(qbo_prog, "expected");
	expected_hi_loc = glGetUniformLocation(qbo_prog, "expected_hi");
}
