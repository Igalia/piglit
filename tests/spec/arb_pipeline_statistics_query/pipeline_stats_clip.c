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

/** \file pipeline_stats_clip.c
 *
 * This test verifies that the clipper related tokens of
 * ARB_pipeline_statistics_query() work as expected. I believe these values
 * are safe to use on all hardware but I am not certain.
 *
 * As with the vertex information, there seems to be a clause which allows
 * implementations to make non-deterministic values (13.5 quoted below).
 *
 * 13.5 (the chicken clause)
 *
 *  Implementations are allowed to pass incoming primitives unchanged and to
 *  output multiple primitives for an incoming primitive due to implementation
 *  dependent reasons as long as the results of rendering otherwise remain
 *  unchanged.
 *
 * 13.5.2
 *  When BeginQuery is called with a target of CLIPPING_INPUT_PRIMITIVES_ARB,
 *  the clipping input primitives count maintained by the GL is set to zero.
 *  When a clipping input primitives query is active, the counter is
 *  incremented every time a primitive reaches the primitive clipping stage
 *  (see section 13.5).
 *
 *  When BeginQuery is called with a target of CLIPPING_OUTPUT_PRIMITIVES_ARB,
 *  the clipping output primitives count maintained by the GL is set to zero.
 *  When a clipping output primitives query is active, the counter is
 *  incremented every time a primitive passes the primitive clipping stage.
 *  The actual number of primitives output by the primitive clipping stage for
 *  a particular input primitive is implementation dependent (see section 13.5)
 *  but must satisfy the following conditions.
 *
 * (Chicken clause 2)
 *  If RASTERIZER_DISCARD is enabled, implementations are allowed to
 *  discard primitives right after the optional transform feedback state
 *  (see Section 14.1). As a result, if RASTERIZER_DISCARD is enabled,
 *  the clipping input and output primitives count may not be
 *  incremented.
 *
 * To me, this makes it sound like this is impossible to test RASTERIZER_DISCARD
 * reliably, so I won't bother.
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
	 .query = GL_CLIPPING_INPUT_PRIMITIVES_ARB,
	 .name = "GL_CLIPPING_INPUT_PRIMITIVES_ARB",
	 .min = NUM_PRIMS},
	{
	 .query = GL_CLIPPING_OUTPUT_PRIMITIVES_ARB,
	 .name = "GL_CLIPPING_OUTPUT_PRIMITIVES_ARB",
	 .min = NUM_PRIMS}
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
	GLuint vs, prog;

	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);

	piglit_require_gl_version(11);
	piglit_require_GLSL();

	do_query_init(queries, ARRAY_SIZE(queries));

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
