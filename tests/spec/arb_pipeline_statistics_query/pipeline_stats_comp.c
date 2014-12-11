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

/** \file pipeline_stats_comp.c
 *
 *  This test verifies that the vertex shader related tokens of
 *  ARB_pipeline_statistics_query() work as expected. OpenGL 4.4
 *  Specification, Core Profile.
 *
 *  When BeginQuery is called with a target of
 *  COMPUTE_SHADER_INVOCATIONS_ARB, the compute shader invocations
 *  count maintained by the GL is set to zero. When a compute shader
 *  invocations query is active, the counter is incremented every time
 *  the compute shader is invoked (see chapter 19).
 *
 */

#include "piglit-util-gl.h"
#include "pipestat_help.h"

PIGLIT_GL_TEST_CONFIG_BEGIN
    config.supports_gl_core_version = 32;
    config.supports_gl_compat_version = 32;
    config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;
PIGLIT_GL_TEST_CONFIG_END

#define NUM_ATOMIC_COUNTERS 1

static GLuint atomics_bo = 0;

static int sizes[] = {
	1, 2, 3, 4, 5, 7, 8, 9, 15, 16, 17, 31, 32, 33, 63, 64, 65,
	127, 128, 129, 255, 256, 257, 511, 512, 513, 1023, 1024
};

static struct query queries[] = {
	{
	 .query = GL_COMPUTE_SHADER_INVOCATIONS_ARB,
	 .name = "GL_COMPUTE_SHADER_INVOCATIONS_ARB",
	 .min = 0 /* Adjusted by confirm_size */},
};

static const char *compute_shader_template =
	"#version 330\n"
	"#extension GL_ARB_compute_shader: enable\n"
	"#extension GL_ARB_shader_atomic_counters: require\n"
	"\n"
	"layout(binding = 0) uniform atomic_uint atc;\n"
	"\n"
	"layout(local_size_x = %d, local_size_y = %d, local_size_z = %d) in;\n"
	"\n"
	"void main()\n"
	"{\n"
	"    atomicCounterIncrement(atc);\n"
	"}\n";


static void
dispatch_size(uint32_t x, uint32_t y, uint32_t z)
{
	char *compute_shader;
	GLint shader_string_size;
	GLuint shader = glCreateShader(GL_COMPUTE_SHADER);
	GLint ok;
	static GLint prog = 0;

	if (prog != 0) {
		glDeleteProgram(prog);
	}

	asprintf(&compute_shader, compute_shader_template, x, y, z);

	glShaderSource(shader, 1,
		       (const GLchar **) &compute_shader,
		       &shader_string_size);

	glCompileShader(shader);

	glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
	assert(ok);

	prog = glCreateProgram();

	glAttachShader(prog, shader);

	glLinkProgram(prog);

	glGetProgramiv(prog, GL_LINK_STATUS, &ok);
	assert(ok);

	glUseProgram(prog);

	glMemoryBarrier(GL_ALL_BARRIER_BITS);
	glDispatchCompute(1, 1, 1);
	glMemoryBarrier(GL_ALL_BARRIER_BITS);
}


static void
test_all_sizes_for_query(void)
{
	uint32_t xi, yi, zi;
	uint32_t x, y, z;

	for (zi = 0; zi <= ARRAY_SIZE(sizes); zi++) {
		z = sizes[zi];
		if (z > 64)
			break;
		for (yi = 0; yi <= ARRAY_SIZE(sizes); yi++) {
			y = sizes[yi];
			if ((y * z) > 1024)
				break;
			for (xi = 0; xi <= ARRAY_SIZE(sizes); xi++) {
				x = sizes[xi];
				if ((x * y * z) > 1024)
					break;
				dispatch_size(x, y, z);

				/* Adjust the expected count. This
				 * will be verified after we
				 * return. */
				queries[0].min += x * y * z;
			}
		}
	}
}


enum piglit_result
piglit_display(void)
{
	return PIGLIT_FAIL;
}


void
piglit_init(int argc, char **argv)
{
	enum piglit_result result;
	GLuint *atomics_buf;

	piglit_require_extension("GL_ARB_compute_shader");
	piglit_require_extension("GL_ARB_shader_atomic_counters");

	do_query_init(queries, ARRAY_SIZE(queries));

	atomics_buf = calloc(NUM_ATOMIC_COUNTERS, sizeof(GLuint));
	glGenBuffers(1, &atomics_bo);
	glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, atomics_bo);
	glBufferData(GL_ATOMIC_COUNTER_BUFFER,
		     sizeof(GLuint) * NUM_ATOMIC_COUNTERS,
		     atomics_buf, GL_STATIC_DRAW);
	free(atomics_buf);

	result = do_query_func(queries, ARRAY_SIZE(queries),
			       test_all_sizes_for_query);

	piglit_report_result(result);
}
