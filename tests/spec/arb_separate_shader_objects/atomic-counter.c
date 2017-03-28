/*
 * Copyright © 2015 Intel Corporation
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
 * \file atomic-counter.c
 *
 * Test incrementing atomic counter in a separable program.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_core_version = 31;
	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

const char *vs_source =
	"#version 150\n"
	"in vec4 vertex;\n"
	"out gl_PerVertex { vec4 gl_Position; }; \n"
	"void main() {\n"
	"	gl_Position = vertex;\n"
	"}\n";

const char *fs_source =
	"#version 150\n"
	"#extension GL_ARB_shader_atomic_counters : enable\n"
	"layout(binding = 0, offset = 0) uniform atomic_uint counter;\n"
	"out vec4 color;\n"
	"void main() {\n"
	"	atomicCounterIncrement(counter);\n"
	"	uint c = atomicCounter(counter);\n"
	"	color = vec4(0.0, c, 0.0, 1.0);\n"
	"}\n";

GLuint buffer;
GLuint vs, fs, pipe;

enum piglit_result
piglit_display(void)
{
	bool pass = true;
	uint32_t *data;

	glViewport(0, 0, piglit_width, piglit_height);
	glClear(GL_COLOR_BUFFER_BIT);

	piglit_draw_rect(-1, -1, 2, 2);

	if (!piglit_check_gl_error(GL_NO_ERROR)) {
		fprintf(stderr, "error while drawing\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	/* Verify that all the pixels are green. */
	const float green[] = { 0.0, 1.0, 0.0};
	pass = pass &&
		piglit_probe_rect_rgb(0, 0, piglit_width, piglit_height, green);
	if (!pass)
		fprintf(stderr, "noise in rendering results\n");

	piglit_present_results();

	/* Check that counter was incremented. */
	glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, buffer);
	glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT);
	data = glMapBufferRange(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(uint32_t),
				GL_MAP_READ_BIT);
	pass = pass && *data == piglit_width * piglit_height;
	if (*data != piglit_width * piglit_height)
		fprintf(stderr, "atomic buffer data %u, expected %u\n",
			*data, piglit_width * piglit_height);
	glUnmapBuffer(GL_ATOMIC_COUNTER_BUFFER);
	glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	bool pass = true;
	uint32_t counter = 0;

	piglit_require_gl_version(31);
	piglit_require_GLSL_version(150);
	piglit_require_extension("GL_ARB_shader_atomic_counters");
	piglit_require_extension("GL_ARB_separate_shader_objects");

	/* Create program pipeline. */
	glGenProgramPipelines(1, &pipe);
	glBindProgramPipeline(pipe);

	vs = glCreateShaderProgramv(GL_VERTEX_SHADER, 1, &vs_source);
	pass = piglit_link_check_status(vs) && pass;

	fs = glCreateShaderProgramv(GL_FRAGMENT_SHADER, 1, &fs_source);
	pass = piglit_link_check_status(fs) && pass;

	glUseProgramStages(pipe, GL_VERTEX_SHADER_BIT, vs);
	glUseProgramStages(pipe, GL_FRAGMENT_SHADER_BIT, fs);

	glBindProgramPipeline(pipe);
	glValidateProgramPipeline(pipe);

	if (!pass || !piglit_check_gl_error(GL_NO_ERROR)) {
		fprintf(stderr, "error building program/pipeline\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	/* Create atomic counter buffer. */
	glGenBuffers(1, &buffer);
	glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, buffer);
	glBufferData(GL_ATOMIC_COUNTER_BUFFER,
		sizeof(uint32_t), &counter, GL_DYNAMIC_DRAW);

	if (!piglit_check_gl_error(GL_NO_ERROR)) {
		fprintf(stderr, "error creating atomic buffer\n");
		piglit_report_result(PIGLIT_FAIL);
	}
}
