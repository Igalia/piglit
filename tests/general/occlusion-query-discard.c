/*
 * Copyright © 2009-2010 Intel Corporation
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
 * Authors:
 *    Ian Romanick <ian.d.romanick@intel.com>
 *    Eric Anholt <eric@anholt.net>
 */

/**
 * \file occlusion-query-discard.c
 *
 * Simple test for GL_ARB_occlusion_query with a discard statement in GLSL.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

static char vs_code[] =
	"varying float do_discard;\n"
	"\n"
	"void main()\n"
	"{\n"
	"	gl_Position = gl_Vertex;\n"
	"	do_discard = gl_MultiTexCoord0.x;\n"
	"}\n";

static char fs_code[] =
	"uniform vec4 color;\n"
	"varying float do_discard;\n"
	"\n"
	"void main()\n"
	"{\n"
	"	if (do_discard != 0.0)\n"
	"		discard;\n"
	"	gl_FragColor = color;\n"
	"}\n";

GLuint prog;

static GLuint setup_shaders()
{
    GLuint vs, fs;

    vs = piglit_compile_shader_text(GL_VERTEX_SHADER, vs_code);
    fs = piglit_compile_shader_text(GL_FRAGMENT_SHADER, fs_code);
    prog = piglit_link_simple_program(vs, fs);

    glUseProgram(prog);
    return prog;
}

enum piglit_result
piglit_display(void)
{
	GLboolean pass = GL_TRUE;
	GLuint q;
	int location, passed;
	float red[4] =   {1, 0, 0, 0};
	float green[4] = {0, 1, 0, 0};

	setup_shaders();

	glGenQueries(1, &q);
	location = glGetUniformLocation(prog, "color");

	/* Drawn. */
	glUniform4fv(location, 1, green);
	glBeginQuery(GL_SAMPLES_PASSED, q);
	piglit_draw_rect_tex(-1, -1, 2, 2,
			     0, 0, 0, 0);
	glEndQuery(GL_SAMPLES_PASSED);
	glGetQueryObjectiv(q, GL_QUERY_RESULT, &passed);
	if (passed != piglit_width * piglit_height) {
		printf("Undiscarded draw covered %d pixels instead of %d\n",
		       passed, piglit_width * piglit_height);
		pass = GL_FALSE;
	}

	/* Discarded. */
	glUniform4fv(location, 1, red);
	glBeginQuery(GL_SAMPLES_PASSED, q);
	piglit_draw_rect_tex(-1, -1, 2, 2,
			     1, 0, 0, 0);
	glEndQuery(GL_SAMPLES_PASSED);
	glGetQueryObjectiv(q, GL_QUERY_RESULT, &passed);
	if (passed != 0) {
		printf("discarded draw covered %d pixels instead of 0\n",
		       passed);
		pass = GL_FALSE;
	}

	pass = pass && piglit_probe_rect_rgba(0, 0, piglit_width, piglit_height, green);

	piglit_present_results();

	glDeleteQueries(1, &q);
	glUseProgram(0);
	glDeleteProgram(prog);

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	GLint query_bits;

	piglit_require_gl_version(20);

	/* It is legal for a driver to support the query API but not have
	 * any query bits.  I wonder how many applications actually check for
	 * this case...
	 */
	glGetQueryiv(GL_SAMPLES_PASSED, GL_QUERY_COUNTER_BITS, &query_bits);
	if (query_bits == 0) {
		piglit_report_result(PIGLIT_SKIP);
	}
}
