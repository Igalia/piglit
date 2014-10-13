/*
 * Copyright 2014 Intel Corporation
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

/** @file dsa-utils.c
 *
 * Contains some common functionality for writing arb_direct_state_access
 * Piglit tests.
 *
 * @author Laura Ekstrand (laura@jlekstrand.net)
 */

#include "dsa-utils.h"
#include "piglit-shader.h"

/*
 * You must use shaders in order to use different texture units.
 * These duplicate fixed-function gl 1.0 pipeline shading.
 * Adapted from arb_clear_texture/3d.c.
 */
static const char dsa_vs_source[] =
	"#version 120\n"
	"attribute vec4 piglit_vertex;\n"
	"attribute vec4 piglit_texcoord;\n"
	"varying vec2 tex_coord;\n"
	"\n"
	"void main()\n"
	"{\n"
	"        gl_Position = gl_ModelViewProjectionMatrix\n"
	"		* piglit_vertex;\n"
	"        tex_coord = piglit_texcoord.st;\n"
	"}\n";

static const char dsa_fs_source[] =
	"#version 120\n"
	"uniform sampler2D tex;\n"
	"varying vec2 tex_coord;\n"
	"\n"
	"void main()\n"
	"{\n"
	"        gl_FragColor = texture2D(tex, tex_coord);\n"
	"}\n";

static GLuint dsa_prog;
static GLuint dsa_uniform;

void
dsa_init_program(void)
{
	dsa_prog = piglit_build_simple_program(dsa_vs_source, dsa_fs_source);
	glUseProgram(dsa_prog);
	dsa_uniform = glGetUniformLocation(dsa_prog, "tex");
	glUniform1i(dsa_uniform, 0);
}

void
dsa_texture_with_unit(GLuint unit)
{
	glUniform1i(dsa_uniform, unit);
}
