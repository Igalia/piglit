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

/** \file pipeline_stats_vert.c
 *
 *  This test verifies that the vertex shader related tokens of
 *  ARB_pipeline_statistics_query() work as expected. OpenGL 4.4 Specification,
 *  Core Profile.
 *
 *  Section 11.1.3 as quoted below makes it sound like we can't actually
 *  reliably count on any of these values. Consider that information when
 *  investigating failures.
 *
 * 10.1
 *  When BeginQuery is called with a target of VERTICES_SUBMITTED_ARB, the
 *  submitted vertices count maintained by the GL is set to zero. When a
 *  vertices submitted query is active, the submitted vertices count is
 *  incremented every time a vertex is transferred to the GL (see sections
 *  10.3.4, and 10.5). In case of primitive types with adjacency information
 *  (see sections 10.1.11 through 10.1.14) only the vertices belonging to the
 *  main primitive are counted but not the adjacent vertices. In case of line
 *  loop primitives implementations are allowed to count the first vertex
 *  twice for the purposes of VERTICES_SUBMITTED_ARB queries.  Additionally,
 *  vertices corresponding to incomplete primitives may or may not be
 *  counted.
 *
 *  When BeginQuery is called with a target of PRIMITIVES_SUBMITTED_ARB, the
 *  submitted primitives count maintained by the GL is set to zero. When a
 *  primitives submitted query is active, the submitted primitives count is
 *  incremented every time a point, line, triangle, or patch primitive is
 *  transferred to the GL (see sections 10.1, 10.3.5, and 10.5). Restarting a
 *  primitive topology using the primitive restart index has no effect on the
 *  issued primitives count. Incomplete primitives may or may not be counted.
 *
 * 11.1.3 (the chicken clause)
 *  Implementations are allowed to skip the execution of certain shader
 *  invocations, and to execute additional shader invocations for any shader
 *  type during programmable vertex processing due to implementation dependent
 *  reasons, including the execution of shader invocations that don't have an
 *  active program object present for the particular shader stage, as long as
 *  the results of rendering otherwise remain unchanged.
 */

#include "piglit-util-gl.h"
#include "pipestat_help.h"

PIGLIT_GL_TEST_CONFIG_BEGIN
    config.supports_gl_compat_version = 30;
    config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;
PIGLIT_GL_TEST_CONFIG_END

static const char *vs_src =
	"#version 110                   \n"
	"                               \n"
	"void main()                    \n"
	"{                              \n"
	"   gl_Position = gl_Vertex;    \n"
	"}                              \n";

static struct query queries[] = {
	{
	 .query = GL_PRIMITIVES_SUBMITTED_ARB,
	 .name = "GL_PRIMITIVES_SUBMITTED_ARB",
	 .min = NUM_PRIMS},
	{
	 .query = GL_VERTICES_SUBMITTED_ARB,
	 .name = "GL_VERTICES_SUBMITTED_ARB",
	 .min = NUM_VERTS},
	{
	 .query = GL_VERTEX_SHADER_INVOCATIONS_ARB,
	 .name = "GL_VERTEX_SHADER_INVOCATIONS_ARB",
	 .min = NUM_VERTS}
};

/* Use DISPLAY for debug */
enum piglit_result
piglit_display(void)
{
	enum piglit_result ret = do_query(queries, ARRAY_SIZE(queries));
#ifdef DISPLAY
	piglit_present_results();
#endif
	return ret;
}

void
piglit_init(int argc, char *argv[])
{
	GLuint vs, prog;

	piglit_require_gl_version(11);
	piglit_require_GLSL();

#ifndef DISPLAY
	glEnable(GL_RASTERIZER_DISCARD);
#endif
	do_query_init(queries, ARRAY_SIZE(queries));

	/* Emit a very simply vertex shader just to make sure we actually go
	 * through the part of the pipeline we're trying to test. */
	vs = piglit_compile_shader_text(GL_VERTEX_SHADER, vs_src);
	prog = glCreateProgram();
	glAttachShader(prog, vs);
	glLinkProgram(prog);

	if (!piglit_link_check_status(prog)) {
		glDeleteProgram(prog);
		piglit_report_result(PIGLIT_FAIL);
	}

	glUseProgram(prog);
}
