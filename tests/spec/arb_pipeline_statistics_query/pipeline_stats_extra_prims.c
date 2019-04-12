/*
 * Copyright © 2014 Intel Corporation
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

/** \file pipeline_stats_extra_prims.c
 *
 * GL_ARB_pipeline_statistics_query says:
 *
 * (23) How do operations like Clear, TexSubImage, etc. affect the results of
 *      the newly introduced queries?
 *
 *   DISCUSSION: Implementations might require "helper" rendering commands be
 *   issued to implement certain operations like Clear, TexSubImage, etc.
 *
 *   RESOLVED: They don't. Only application submitted rendering commands
 *   should have an effect on the results of the queries.
 *
 * This test tries to provoke extra primitives.
 */

#include "piglit-util-gl.h"
#include "pipestat_help.h"

PIGLIT_GL_TEST_CONFIG_BEGIN
	config.supports_gl_compat_version = 30;
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;
PIGLIT_GL_TEST_CONFIG_END

static const char *vs_src =
	"#version 130                   \n"
	"                               \n"
	"in vec4 piglit_vertex;		\n"
	"void main()                    \n"
	"{                              \n"
	"   gl_Position = piglit_vertex;\n"
	"}                              \n";

static const char *fs_src =
	"#version 110			\n"
	"				\n"
	"void main()			\n"
	"{				\n"
	"   gl_FragColor = vec4(0, 1, 0, 1); \n"
	"}				\n";

static struct query queries[] = {
	{
		.query = GL_PRIMITIVES_SUBMITTED_ARB,
		.min = 3 /* Going to emit three lines */
	},
};

/* Some random line vertices.  The values really don't matter. */
static const float vertex_data[] = {
	0.2f, 0.5f, 0.0f, 1.0f, /* Vert 0 */
	0.8f, 0.5f, 0.0f, 1.0f, /* Vert 1 */
};

static void
draw(void)
{
	glDrawArrays(GL_LINES, 0, 2);

	/* Perform a partial overwrite of the vertex buffer used by the
	 * previous draw call, to try and provoke a staging blit which may
	 * emit an extra primitive.
	 */
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertex_data) / 2,
			vertex_data);

	glDrawArrays(GL_LINES, 0, 2);

	/* Ensure clears aren't counted. */
	glClear(GL_COLOR_BUFFER_BIT);

	glDrawArrays(GL_LINES, 0, 2);

#ifdef DISPLAY
	piglit_present_results();
#endif
}

enum piglit_result
piglit_display(void)
{
	return do_query_func(queries, ARRAY_SIZE(queries), draw);
}

void
piglit_init(int argc, char *argv[])
{
	GLuint prog, array, buf;

	glGenVertexArrays(1, &array);
	glBindVertexArray(array);
	glGenBuffers(1, &buf);
	glBindBuffer(GL_ARRAY_BUFFER, buf);

	prog = piglit_build_simple_program(vs_src, fs_src);

	glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_data), vertex_data,
		     GL_STREAM_DRAW);

	glVertexAttribPointer(0, /* index */
			      4, /* size */
			      GL_FLOAT, /* type */
			      GL_FALSE, /* normalized */
			      0, /* stride */
			      NULL /* pointer */);
	glEnableVertexAttribArray(0);

#ifndef DISPLAY
	glEnable(GL_RASTERIZER_DISCARD);
#endif

	do_query_init(queries, ARRAY_SIZE(queries));

	if (!piglit_link_check_status(prog)) {
		glDeleteProgram(prog);
		piglit_report_result(PIGLIT_FAIL);
	}

	glUseProgram(prog);
}
