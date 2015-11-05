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
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN
	config.supports_gl_compat_version = 30;
//	config.supports_gl_core_version = 32;
	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE |
		PIGLIT_GL_VISUAL_DEPTH;

PIGLIT_GL_TEST_CONFIG_END

#define BUFFER_OFFSET(i) ((GLint *)((unsigned char*)NULL + (i)))

static const float green[] = {0, 1, 0, 1};
static const float red[] = {1, 0, 0, 1};

static unsigned query;
static unsigned qbo;

static int prog;
static int qbo_prog;
static int expected_count_loc;
static int qbo_async_prog;

enum piglit_result
synchronous_query(void)
{
	GLboolean pass;

	glClearColor(0.5, 0.5, 0.5, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// enable query, draw something that should pass
	glEnable(GL_DEPTH_TEST);
	glUseProgram(prog);
	glBeginQuery(GL_SAMPLES_PASSED, query);
	piglit_draw_rect_z(0.5, -1, -1, 2, 2);
	glEndQuery(GL_SAMPLES_PASSED);

	// Stuff query result into qbo
	glBindBuffer(GL_QUERY_BUFFER, qbo);
	glGetQueryObjectivARB(query, GL_QUERY_RESULT, BUFFER_OFFSET(0));
	// Make it available to shader as uniform buffer 0
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, qbo);

	glUseProgram(qbo_prog);

	// expected count of samples passed
	glUniform1ui(expected_count_loc, piglit_width * piglit_height);

	glDisable(GL_DEPTH_TEST);
	// draw green if query successful
	piglit_draw_rect(-1, -1, 2, 2);

	pass = piglit_probe_rect_rgba(0, 0, piglit_width, piglit_height, green);

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

enum piglit_result
asynchronous_query_with_default(void)
{
	enum piglit_result result;
	unsigned default_value[2] = { 42u, 0u };
	unsigned char *buffer;
	unsigned redcount, greencount, bluecount;
	size_t n;

	glClearColor(0.5, 0.5, 0.5, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Load default value into buffer
	glBindBuffer(GL_QUERY_BUFFER, qbo);
	glBufferData(GL_QUERY_BUFFER, 8, default_value, GL_DYNAMIC_COPY);

	// enable query, draw something that should pass
	glEnable(GL_DEPTH_TEST);
	glUseProgram(prog);
	glBeginQuery(GL_SAMPLES_PASSED, query);
	piglit_draw_rect_z(0.5, -1, -1, 2, 2);
	glEndQuery(GL_SAMPLES_PASSED);

	glBindBuffer(GL_QUERY_BUFFER, qbo);
	// Stuff query result into qbo
	glGetQueryObjectivARB(query, GL_QUERY_RESULT_NO_WAIT, BUFFER_OFFSET(0));
	// Stuff query availability into qbo
	glGetQueryObjectivARB(query, GL_QUERY_RESULT_AVAILABLE, BUFFER_OFFSET(4));
	// Make it available to shader as uniform buffer 0
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, qbo);

	glUseProgram(qbo_async_prog);

	glDisable(GL_DEPTH_TEST);
	// draw green if query successful
	piglit_draw_rect(-1, -1, 2, 2);

	// Any red values -> fail
	// No blue, we don't know if async result not available works -> warn
	buffer = malloc(piglit_width * piglit_height * 4);
	glReadPixels(0, 0, piglit_width, piglit_height, GL_RGBA, GL_UNSIGNED_BYTE, buffer);
	redcount = 0;
	bluecount = 0;
	greencount = 0;
	for (n = 0; n < piglit_width * piglit_height; n++) {
		unsigned char r = buffer[n*4 + 0];
		unsigned char g = buffer[n*4 + 1];
		unsigned char b = buffer[n*4 + 2];
		if (r)
			redcount++;
		if (b)
			bluecount++;
		if (g)
			greencount++;
	}
	free(buffer);

	if (redcount || greencount != piglit_width * piglit_height) {
		result = PIGLIT_FAIL;
	}
	else if (bluecount == 0) {
		result = PIGLIT_WARN;
		printf("no pixels where async query result unavailable, uncertain test result\n");
	}
	else {
		result = PIGLIT_PASS;
	}

	piglit_present_results();

	return result;
}

enum piglit_result
piglit_display(void)
{
	enum piglit_result r, r2;

	r = synchronous_query();
	piglit_report_subtest_result(r, "synchronous_query");

	r2 = asynchronous_query_with_default();
	piglit_report_subtest_result(r2, "asynchronous_query_with_default");

	return MAX2(r, r2);
}

void
piglit_init(int argc, char **argv)
{
	char *vsCode;
	char *fsCode, *qboFsCode, *qboAsyncFsCode;

	piglit_require_extension("GL_ARB_query_buffer_object");
	piglit_require_extension("GL_ARB_uniform_buffer_object");

	glGenQueries(1, &query);

	glGenBuffers(1, &qbo);
	glBindBuffer(GL_QUERY_BUFFER, qbo);
	glBufferData(GL_QUERY_BUFFER, 4, NULL, GL_DYNAMIC_COPY);

	vsCode =
		"#version 130\n"
		"void main() {\n"
		"	gl_Position = gl_Vertex;\n"
		"}\n";
	fsCode =
		"#version 130\n"
		"void main() {\n"
		"	gl_FragColor = vec4(0.0, 0.0, 1.0, 1.0);\n"
		"}\n";
	qboFsCode =
		"#version 130\n"
		"#extension GL_ARB_uniform_buffer_object : require\n"
		"uniform query {\n"
		"	uint result;\n"
		"};\n"
		"uniform uint expected_count;\n"
		"void main() {\n"
		"   if (result == expected_count) {\n"
		"		gl_FragColor = vec4(0.0, 1.0, 0.0, 1.0);\n"
		"	} else {\n"
		"		gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);\n"
		"	}\n"
		"}\n";
	qboAsyncFsCode =
		"#version 130\n"
		"#extension GL_ARB_uniform_buffer_object : require\n"
		"uniform query {\n"
		"	uint result;\n"
		"	uint available;\n"
		"};\n"
		"void main() {\n"
		"	if (available == 0u && result == 42u) {\n"
		"		gl_FragColor = vec4(0.0, 1.0, 1.0, 1.0);\n"
		"   } else if (available == 0u && result != 42u) {\n"
		"		gl_FragColor = vec4(1.0, 0.0, 1.0, 1.0);\n"
		"	} else if (available != 0u && result > 0u) {\n"
		"		gl_FragColor = vec4(0.0, 1.0, 0.0, 1.0);\n"
		"	} else {\n"
		"		gl_FragColor = vec4(1.0, 1.0, 0.0, 1.0);\n"
		"	}\n"
		"}\n";

	prog = piglit_build_simple_program(vsCode, fsCode);
	qbo_prog = piglit_build_simple_program(vsCode, qboFsCode);
	expected_count_loc = glGetUniformLocation(qbo_prog, "expected_count");
	qbo_async_prog = piglit_build_simple_program(vsCode, qboAsyncFsCode);
}
