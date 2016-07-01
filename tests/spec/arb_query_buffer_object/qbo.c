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

#define BUFFER_OFFSET(i) ((GLint *)((unsigned char*)NULL + (i)))

static const float green[] = {0, 1, 0, 1};

static unsigned query;
static unsigned qbo;

static int prog;
static int qbo_prog;
static int sync_mode_loc;
static int original_count_loc;
static int expect_exact_loc;
static int expected_count_loc;
static bool has_pipeline_stats;

enum sync_mode {
	QBO_SYNC,
	QBO_ASYNC,
	QBO_ASYNC_CPU_READ_BEFORE,
	QBO_ASYNC_CPU_READ_AFTER,
	NUM_QBO_SYNC_MODES,
};

static char* sync_mode_names[] = {
	"SYNC",
	"ASYNC",
	"ASYNC_CPU_READ_BEFORE",
	"ASYNC_CPU_READ_AFTER",
};

static enum sync_mode sync_mode;

static bool
is_pipeline_stats_query(GLenum q)
{
	switch (q) {
	case GL_VERTICES_SUBMITTED_ARB:
	case GL_PRIMITIVES_SUBMITTED_ARB:
	case GL_VERTEX_SHADER_INVOCATIONS_ARB:
	case GL_TESS_CONTROL_SHADER_PATCHES_ARB:
	case GL_TESS_EVALUATION_SHADER_INVOCATIONS_ARB:
	case GL_GEOMETRY_SHADER_INVOCATIONS:
	case GL_GEOMETRY_SHADER_PRIMITIVES_EMITTED_ARB:
	case GL_FRAGMENT_SHADER_INVOCATIONS_ARB:
	case GL_COMPUTE_SHADER_INVOCATIONS_ARB:
	case GL_CLIPPING_INPUT_PRIMITIVES_ARB:
	case GL_CLIPPING_OUTPUT_PRIMITIVES_ARB:
		return true;
	default:
		return false;
	}
}

static GLenum query_types[] = {
	GL_ANY_SAMPLES_PASSED,
	GL_ANY_SAMPLES_PASSED_CONSERVATIVE,
	GL_CLIPPING_INPUT_PRIMITIVES_ARB,
	GL_CLIPPING_OUTPUT_PRIMITIVES_ARB,
	/* GL_COMPUTE_SHADER_INVOCATIONS_ARB, */
	GL_FRAGMENT_SHADER_INVOCATIONS_ARB,
	/* GL_GEOMETRY_SHADER_INVOCATIONS, */
	/* GL_GEOMETRY_SHADER_PRIMITIVES_EMITTED_ARB, */
	GL_PRIMITIVES_GENERATED,
	GL_PRIMITIVES_SUBMITTED_ARB,
	GL_SAMPLES_PASSED_ARB,
	/* GL_TESS_CONTROL_SHADER_PATCHES_ARB, */
	/* GL_TESS_EVALUATION_SHADER_INVOCATIONS_ARB, */
	GL_TIMESTAMP,
	GL_TIME_ELAPSED,
	GL_TRANSFORM_FEEDBACK_PRIMITIVES_WRITTEN,
	GL_VERTEX_SHADER_INVOCATIONS_ARB,
	GL_VERTICES_SUBMITTED_ARB,
};

static GLenum query_type;

static void
get_query_values(GLenum query_type, uint32_t *original,
		 bool *exact, uint32_t *expected)
{
	*original = 0xffffffff;
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
	default:
		abort();
	}
}

static enum piglit_result
cpu_gather_query(bool exact, uint32_t expected)
{
	GLint qresult;

	glBindBuffer(GL_QUERY_BUFFER, 0);

	glGetQueryObjectiv(query, GL_QUERY_RESULT, &qresult);

	glBindBuffer(GL_QUERY_BUFFER, qbo);

	return (exact ? qresult == expected : qresult >= expected)
		? PIGLIT_PASS : PIGLIT_FAIL;
}

enum piglit_result
run_subtest(void)
{
	uint32_t original;
	bool exact;
	uint32_t expected;
	uint32_t default_value[2] = { 0u, 0u };
	bool is_sync = sync_mode == QBO_SYNC;

	get_query_values(query_type, &original, &exact, &expected);
	default_value[0] = original;

	glClearColor(0.5, 0.5, 0.5, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	/* Load default value into buffer */
	glBindBuffer(GL_QUERY_BUFFER, qbo);
	glBufferData(GL_QUERY_BUFFER, 8, default_value, GL_DYNAMIC_COPY);

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

	if (sync_mode == QBO_ASYNC_CPU_READ_BEFORE &&
	    cpu_gather_query(exact, expected))
		return PIGLIT_FAIL;

	glBindBuffer(GL_QUERY_BUFFER, qbo);
	if (is_sync) {
		/* Stuff query result into qbo */
		glGetQueryObjectivARB(query, GL_QUERY_RESULT,
				      BUFFER_OFFSET(0));
	} else {
		/* Stuff query result into qbo */
		glGetQueryObjectivARB(query, GL_QUERY_RESULT_NO_WAIT, BUFFER_OFFSET(0));
		/* Stuff query availability into qbo */
		glGetQueryObjectivARB(query, GL_QUERY_RESULT_AVAILABLE, BUFFER_OFFSET(4));
	}

	if (sync_mode == QBO_ASYNC_CPU_READ_AFTER &&
	    cpu_gather_query(exact, expected))
		return PIGLIT_FAIL;

	/* Make it available to shader as uniform buffer 0 */
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, qbo);

	glUseProgram(qbo_prog);

	/* Setup program uniforms */
	glUniform1ui(sync_mode_loc, is_sync ? GL_TRUE : GL_FALSE);
	glUniform1ui(original_count_loc, original);
	glUniform1ui(expect_exact_loc, exact ? GL_TRUE : GL_FALSE);
	glUniform1ui(expected_count_loc, expected);

	glDisable(GL_DEPTH_TEST);
	/* Draw green if query successful */
	piglit_draw_rect(-1, -1, 2, 2);

	glDeleteQueries(1, &query);

	return piglit_probe_rect_rgba(0, 0, piglit_width,
				      piglit_height, green)
		? PIGLIT_PASS : PIGLIT_FAIL;
}

enum piglit_result
run_subtest_and_present(void)
{
	char *subtest_name;
	enum piglit_result r = run_subtest();
	piglit_present_results();
	(void)!asprintf(&subtest_name, "query-%s-%s",
			piglit_get_gl_enum_name(query_type),
			sync_mode_names[sync_mode]);
	piglit_report_subtest_result(r, "%s", subtest_name);
	free(subtest_name);
	return r;
}

enum piglit_result
piglit_display(void)
{
	enum piglit_result r = PIGLIT_PASS;
	enum piglit_result subtest_result;
	int qnum;

	for (qnum = 0; qnum < ARRAY_SIZE(query_types); qnum++) {
		query_type = query_types[qnum];
		for (sync_mode = QBO_SYNC;
		     sync_mode < NUM_QBO_SYNC_MODES;
		     sync_mode++) {
			if (!has_pipeline_stats &&
			    is_pipeline_stats_query(query_type))
				continue;
			subtest_result = run_subtest_and_present();
			r = MAX2(r, subtest_result);
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
	has_pipeline_stats =
		piglit_is_extension_supported("GL_ARB_pipeline_statistics_query");

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
		"	uint available;\n"
		"};\n"
		"uniform bool sync_mode;\n"
		"uniform uint original_count;\n"
		"uniform bool expect_exact;\n"
		"uniform uint expected_count;\n"
		"out vec4 color;\n"
		"void main() {\n"
		"	bool ready = sync_mode || available != 0u;\n"
		"	if (!ready && result == original_count) {\n"
		"		color = vec4(0.0, 1.0, 0.0, 1.0);\n"
		"	} else if (ready &&\n"
		"	           (expect_exact ? result == expected_count :\n"
		"	                           result >= expected_count)) {\n"
		"		color = vec4(0.0, 1.0, 0.0, 1.0);\n"
		"	} else {\n"
		"		color = vec4(1.0, 0.0, 0.0, 1.0);\n"
		"	}\n"
		"}\n";

	prog = piglit_build_simple_program(vsCode, fsCode);
	qbo_prog = piglit_build_simple_program(vsCode, qboFsCode);
	sync_mode_loc = glGetUniformLocation(qbo_prog, "sync_mode");
	original_count_loc = glGetUniformLocation(qbo_prog, "original_count");
	expect_exact_loc = glGetUniformLocation(qbo_prog, "expect_exact");
	expected_count_loc = glGetUniformLocation(qbo_prog, "expected_count");
}
