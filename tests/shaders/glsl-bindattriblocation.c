/*
 * Copyright © 2009 Intel Corporation
 * Copyright © 2010 VMware, Inc.
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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

/**
 * \file glsl-bindattriblocation.c
 *
 * Check glBindAttribLocation().
 *
 * We create a simple vertex shader with a single user-defined vertex
 * attribute bound to location 3 (or anything non-zero).  Then try to
 * draw a polygon.  Mesa has (had) a draw-time validation check which
 * no-op'd the draw if vertex array #0 was not enabled.
 *
 * This test is based on the glsl-dlist-getattriblocation.c test written
 * by Ian.
 *
 * \author Ian Romanick <ian.d.romanick@intel.com>
 * \author Brian Paul
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

static const GLchar *vertShaderText =
	"attribute vec4 attrib;\n"
	"void main()\n"
	"{\n"
	"	gl_Position = gl_ModelViewProjectionMatrix * attrib;\n"
	"	gl_FrontColor = vec4(0.0, 1.0, 0.0, 1.0);\n"
	"} \n";


static const GLfloat Vcoords[4][2] = { {-1, -1}, {1, -1}, {1, 1}, {-1, 1}};


enum piglit_result
piglit_display(void)
{
	static const GLfloat expColor[4] = {0, 1, 0, 1};
	GLint vs;
	GLint prog;
	GLint orig_attrib_loc, attrib_loc;
	enum piglit_result result;

	vs = piglit_compile_shader_text(GL_VERTEX_SHADER, vertShaderText);
	if (vs == 0) {
                exit(1);
        }

	prog = piglit_link_simple_program(vs, 0);
	if (prog == 0) {
                exit(1);
        }

	orig_attrib_loc = glGetAttribLocation(prog, "attrib");
	if (!piglit_automatic)
		printf("original attrib_loc = %d\n", orig_attrib_loc);

	/* Bind "attrib" to location 3 and re-link */
	glBindAttribLocation(prog, 3, "attrib");
	glLinkProgram(prog);

	/* check that the bind worked */
	attrib_loc = glGetAttribLocation(prog, "attrib");
	if (!piglit_automatic)
		printf("new attrib_loc = %d\n", attrib_loc);
	if (attrib_loc != 3) {
		fprintf(stderr, "glsl-bindattriblocation: glBindAttribLocation failed\n");
		fprintf(stderr, "  expectec location 3, found location %d\n",
			attrib_loc);
		return PIGLIT_FAIL;
	}

	/* now draw something and check that it works */
	glUseProgram(prog);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-1.1, 1.1, -1.1, 1.1, -1, 1);

	glClear(GL_COLOR_BUFFER_BIT);

	glVertexAttribPointer(attrib_loc, 2, GL_FLOAT, GL_FALSE, 0, Vcoords);
	glEnableVertexAttribArray(attrib_loc);

	glDrawArrays(GL_POLYGON, 0, 4);

	result = piglit_probe_pixel_rgba(20, 20, expColor)
		? PIGLIT_PASS : PIGLIT_FAIL;

	glDisableVertexAttribArray(attrib_loc);

	piglit_present_results();

	return result;
}


void
piglit_init(int argc, char **argv)
{
	piglit_require_vertex_shader();

	piglit_require_gl_version(20);
}
