/*
 * Copyright © 2012 Intel Corporation
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

/** @file getactiveuniformsiv-uniform-block-index.c
 *
 * Tests that glGetActiveUniforms() returns the block index for a
 * uniform that the uniform block was given by
 * glGetUniformBlockIndex.()
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;

PIGLIT_GL_TEST_CONFIG_END

static const char vs_source[] =
	"#extension GL_ARB_uniform_buffer_object : require\n"
	"\n"
	"uniform ub_a { vec4 a; };\n"
	"uniform ub_b { vec4 b; };\n"
	"\n"
	"void main()\n"
	"{\n"
	"	gl_Position = a + b;\n"
	"}\n";

static const char fs_source[] =
	"#extension GL_ARB_uniform_buffer_object : require\n"
	"\n"
	"uniform ub_b { vec4 b; };\n"
	"uniform ub_c { vec4 c; };\n"
	"uniform vec4 d;\n"
	"\n"
	"void main()\n"
	"{\n"
	"	gl_FragColor = b + c + d;\n"
	"}\n";

void
piglit_init(int argc, char **argv)
{
	bool pass = true;
	GLuint prog;
	const char *uniform_block_names[3] = { "ub_a", "ub_b", "ub_c" };
	const char *uniform_names[4] = { "a", "b", "c", "d" };
	GLuint block_indices[3];
	GLuint uniform_indices[4];
	GLint uniform_block_indices[4];
	int i;

	piglit_require_extension("GL_ARB_uniform_buffer_object");

	prog = piglit_build_simple_program(vs_source, fs_source);

	for (i = 0; i < 3; i++) {
		block_indices[i] =
			glGetUniformBlockIndex(prog, uniform_block_names[i]);

		printf("Uniform block \"%s\" index: 0x%08x\n",
		       uniform_block_names[i], block_indices[i]);
	}

	if (block_indices[0] == block_indices[1] ||
	    block_indices[0] == block_indices[2] ||
	    block_indices[1] == block_indices[2]) {
		piglit_report_result(PIGLIT_FAIL);
	}

	glGetUniformIndices(prog, 4, uniform_names, uniform_indices);
	glGetActiveUniformsiv(prog, 4, uniform_indices,
			      GL_UNIFORM_BLOCK_INDEX, uniform_block_indices);
	for (i = 0; i < 4; i++) {
		int expected_index = (i == 3) ? -1 : block_indices[i];

		printf("Uniform \"%s\": index %d, block index %d",
		       uniform_names[i],
		       uniform_indices[i],
		       uniform_block_indices[i]);

		if (uniform_block_indices[i] != expected_index) {
			printf(" FAIL");
			pass = false;
		}

		printf("\n");
	}

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}

enum piglit_result piglit_display(void)
{
	/* UNREACHED */
	return PIGLIT_FAIL;
}
