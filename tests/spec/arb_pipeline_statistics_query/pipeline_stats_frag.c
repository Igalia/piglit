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

/** \file pipeline_stats_frag.c
 *
 * This test verifies that the fragment shader related tokens of
 * ARB_pipeline_statistics_query() work as expected. I believe these values
 * are safe to use on all hardware but I am not certain. One again we get the
 * beloved, "can't rely on values clause." For the most part this makes senes
 * since implementations can very well process too many vertices - but the
 * clause also allows too few. The former case is accounted for within this code.
 *
 * 15.2 (Another chicken clause)
 *  Implementations are allowed to skip the execution of certain fragment
 *  shader invocations, and to execute additional fragment shader invocations
 *  during programmable fragment processing due to implementation dependent
 *  reasons, including the execution of fragment shader invocations when there
 *  isn't an active program object present for the fragment shader stage, as
 *  long as the results of rendering otherwise remain unchanged.
 *
 * 15.3
 *  When BeginQuery is called with a target of FRAGMENT_SHADER_INVOCATIONS_ARB,
 *  the fragment shader invocations count maintained by the GL is set to zero.
 *  When a fragment shader invocations query is active, the counter is
 *  incremented every time the fragment shader is invoked (see section 15.2).
 *  The result of fragment shader queries may be implementation dependent due
 *  to reasons described in section 15.2.
 */

#include "piglit-util-gl.h"
#include "pipestat_help.h"

PIGLIT_GL_TEST_CONFIG_BEGIN
    config.supports_gl_compat_version = 30;
    config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;
    config.window_width = TEST_WIDTH;
    config.window_height = TEST_HEIGHT;
PIGLIT_GL_TEST_CONFIG_END

static const char *vs_src =
	"#version 110                   \n"
	"                               \n"
	"void main()                    \n"
	"{                              \n"
	"   gl_Position = gl_Vertex;    \n"
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
	 .query = GL_FRAGMENT_SHADER_INVOCATIONS_ARB,
	 .name = "GL_FRAGMENT_SHADER_INVOCATIONS_ARB",
	 .min = TEST_WIDTH * TEST_HEIGHT,
	 .max = TEST_WIDTH * TEST_HEIGHT * 3 / 2},
	/* XXX:
	 * Intel hardware has some very unpredictable results for fragment
	 * shader invocations. After a day of head scratching, I've given up.
	 * Generating a real min, or max is not possible. The spec allows this.
	 * This will also help variance across vendors.
	 */
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
	GLuint prog;

	piglit_require_gl_version(11);
	piglit_require_GLSL();

	do_query_init(queries, ARRAY_SIZE(queries));

	prog = piglit_build_simple_program(vs_src, fs_src);

	glUseProgram(prog);
}
