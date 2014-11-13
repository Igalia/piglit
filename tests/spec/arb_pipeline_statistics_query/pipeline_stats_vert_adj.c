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

/** \file pipeline_stats_vert_adj.c
 *
 * This test verifies that the vertex shader related tokens of
 * ARB_pipeline_statistics_query() work as expected. Much of this was derived
 * from ignore-adjacent-vertices.c
 *
 * 10.11
 *  In case of primitive types with adjacency information (see sections 10.1.11
 *  through 10.1.14) only the vertices belonging to the main primitive are
 *  counted but not the adjacent vertices. In case of line loop primitives
 *  implementations are allowed to count the first vertex twice for the
 *  purposes of VERTICES_SUBMITTED_ARB queries. Additionally, vertices
 *  corresponding to incomplete primitives may or may not be counted.
 *
 * I read this as: the only definite thing we can test across implementation is
 * discarding adjacent vertices.
 */

#include "piglit-util-gl.h"
#include "pipestat_help.h"

PIGLIT_GL_TEST_CONFIG_BEGIN
/* I believe the adjacency tokens require 3.2 GS */
	config.supports_gl_compat_version = 32;
	config.supports_gl_core_version = 32;
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
	 .name = "GL_PRIMITIVES_SUBMITTED_ARB",
	 .min = 1}, /* Going to emit a line */
	{
	 .query = GL_VERTICES_SUBMITTED_ARB,
	 .name = "GL_VERTICES_SUBMITTED_ARB",
	 .min = 2,
	 .max = 4}, /*(26) Should VERTICES_SUBMITTED_ARB count adjacent
			   vertices in case of primitives with adjacency? */
	{
	 .query = GL_VERTEX_SHADER_INVOCATIONS_ARB,
	 .name = "GL_VERTEX_SHADER_INVOCATIONS_ARB",
	 .min = 2,
	 .max = 4}
};

static void
draw(void)
{
	/* 4 components, 2 verts for a line, and 1 vert for adjacency per vertex
	 * that makes up the line. The values really don't matter for this.
	 */
	static const float vertex_data[] = {
		0.0f, 0.0f, 0.0f, 1.0f, /* Adjacent vert */
		0.2f, 0.5f, 0.0f, 1.0f, /* Vert 0 */
		0.8f, 0.5f, 0.0f, 1.0f, /* Vert 1 */
		1.0f, 0.0f, 0.0f, 1.0f /* Adjacent vert */
	};

	glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_data), vertex_data, GL_STREAM_DRAW);
	glDrawArrays(GL_LINES_ADJACENCY, 0, 4);
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
