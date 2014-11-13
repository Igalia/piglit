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

/** \file pipeline_stats_geom.c
 *
 *  This test verifies that the vertex shader related tokens of
 *  ARB_pipeline_statistics_query() work as expected. OpenGL 4.4 Specification,
 *  Core Profile.
 *
 *  When BeginQuery is called with a target of GEOMETRY_SHADER_INVOCATIONS,
 *  the geometry shader invocations count maintained by the GL is set to zero.
 *  When a geometry shader invocations query is active, the counter is
 *  incremented every time the geometry shader is invoked (see section 11.3).
 *  In case of instanced geometry shaders (see section 11.3.4.2) the geometry
 *  shader invocations count is incremented for each separate instanced
 *  invocation.
 *
 *  When BeginQuery is called with a target of GEOMETRY_SHADER_PRIMITIVES_-
 *  EMITTED_ARB, the geometry shader output primitives count maintained by the
 *  GL is set to zero. When a geometry shader primitives emitted query is
 *  active, the counter is incremented every time the geometry shader emits
 *  a primitive to a vertex stream that is further processed by the GL (see
 *  section 11.3.2). Restarting primitive topology using the shading language
 *  built-in functions EndPrimitive or EndStreamPrimitive does not increment
 *  the geometry shader output primitives count.
 *
 *  (The chicken clause)
 *  The result of geometry shader queries may be implementation dependent due
 *  to reasons described in section 11.1.3.
 */

#include "piglit-util-gl.h"
#include "pipestat_help.h"

PIGLIT_GL_TEST_CONFIG_BEGIN
    config.supports_gl_core_version = 32;
    config.supports_gl_compat_version = 32;
    config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;
PIGLIT_GL_TEST_CONFIG_END

const char *vs_src =
	"#version 150			\n"
	"in vec4 piglit_vertex;		\n"
	"out vec4 vertex_to_gs;		\n"
	"void main()			\n"
	"{				\n"
	"  vertex_to_gs = piglit_vertex;\n"
	"}				\n";

const char *gs_src =
	"#version 150					\n"
	"layout(triangles) in;				\n"
	"layout(triangle_strip, max_vertices = 6) out;	\n"
	"in vec4 vertex_to_gs[3];			\n"
	"void main()					\n"
	"{						\n"
	"  for (int i = 0; i < 6; i++) {		\n"
	"    gl_Position = vertex_to_gs[i % 3];		\n"
	"    EmitVertex();				\n"
	"  }						\n"
	"}						\n";


#ifdef DISPLAY
const char *fs_src =
	"#version 150					\n"
	"out vec4 color;				\n"
	"void main()					\n"
	"{						\n"
	"    color = vec4(0.0, 1.0, 0.0, 1.0);		\n"
	"}						\n";
#endif

static struct query queries[] = {
	{
	 .query = GL_GEOMETRY_SHADER_INVOCATIONS,
	 .name = "GL_GEOMETRY_SHADER_INVOCATIONS",
	 .min = NUM_PRIMS,
	 .max = NUM_PRIMS},

	/* There are going to be NUM_PRIMS invocations, and for each invocation
	 * we're going to write 6 vertices in a tristrip, which is 4 triangles.
	 * So NUM_PRIMS * 4 is what we expect here */
	{
	 .query = GL_GEOMETRY_SHADER_PRIMITIVES_EMITTED_ARB,
	 .name = "GL_GEOMETRY_SHADER_PRIMITIVES_EMITTED_ARB",
	 .min = NUM_PRIMS * 4,
	 .max = NUM_PRIMS * 4}
};

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
	GLuint vs, gs, prog;

	piglit_require_gl_version(15);
	piglit_require_GLSL();

	do_query_init(queries, ARRAY_SIZE(queries));

	prog = glCreateProgram();

	vs = piglit_compile_shader_text(GL_VERTEX_SHADER, vs_src);
	gs = piglit_compile_shader_text(GL_GEOMETRY_SHADER, gs_src);

#ifndef DISPLAY
	glEnable(GL_RASTERIZER_DISCARD);
#else
	glAttachShader(prog,
			piglit_compile_shader_text(GL_FRAGMENT_SHADER, fs_src));
#endif

	glAttachShader(prog, vs);
	glAttachShader(prog, gs);
	glLinkProgram(prog);

	if (!piglit_link_check_status(prog)) {
		glDeleteProgram(prog);
		piglit_report_result(PIGLIT_FAIL);
	}

	glUseProgram(prog);
}
