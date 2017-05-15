/*
 * Copyright (c) 2016 Samuel Pitoiset
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

/** \file
 *
 * Checks gl_LocalGroupSizeARB at various sizes up to the implementation
 * maximums using atomic counters.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 33;
	config.supports_gl_core_version = 33;
	config.khr_no_error_support = PIGLIT_NO_ERRORS;

PIGLIT_GL_TEST_CONFIG_END

static GLuint atomics_bo = 0;
static GLint prog = 0;

static uint32_t global_x = 1, global_y = 1, global_z = 1;
static uint32_t local_x = 0, local_y = 0, local_z = 0;
static int32_t max_local_x = 0, max_local_y = 0, max_local_z = 0;
static int32_t max_variable_invocations = 0;

static uint32_t sizes[] = {
	1, 2, 3, 4, 5, 7, 8, 9, 15, 16, 17, 31, 32, 33, 63, 64, 65,
	127, 128, 129, 255, 256, 257, 511, 512, 513, 1023, 1024
};

static const char *compute_shader_source =
	"#version 330\n"
	"#extension GL_ARB_compute_shader: enable\n"
	"#extension GL_ARB_compute_variable_group_size: enable\n"
	"#extension GL_ARB_shader_atomic_counters: require\n"
	"\n"
	"layout(binding = 0) uniform atomic_uint a;\n"
	"layout(local_size_variable) in;\n"
	"\n"
	"void main()\n"
	"{\n"
	"	if (gl_LocalGroupSizeARB.x == %du &&\n"
	"	    gl_LocalGroupSizeARB.y == %du &&\n"
	"	    gl_LocalGroupSizeARB.z == %du)\n"
	"		atomicCounterIncrement(a);\n"
	"}\n";

static GLuint
generate_cs_prog(uint32_t x, uint32_t y, uint32_t z, char *src)
{
	char *source = NULL;

	(void)!asprintf(&source, src, x, y, z);
	free(src);

	GLuint prog = glCreateProgram();

	GLuint shader =
		piglit_compile_shader_text_nothrow(GL_COMPUTE_SHADER, source);

	if (!shader) {
		glDeleteProgram(prog);
		return 0;
	}

	glAttachShader(prog, shader);

	glLinkProgram(prog);

	glDeleteShader(shader);

	if (!piglit_link_check_status(prog)) {
		glDeleteProgram(prog);
		return 0;
	}

	return prog;
}

static enum piglit_result
check_result()
{
	uint32_t expected = local_x * local_y * local_z;
	bool pass = true;
	uint32_t *p;

	glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, atomics_bo);
	p = glMapBufferRange(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(uint32_t),
			     GL_MAP_READ_BIT);
	if (!p) {
		printf("Couldn't map atomic counter to verify expected value.\n");
		return PIGLIT_FAIL;
	}

	if (p[0] != expected) {
		printf("Atomic counter test failed for (%d, %d, %d)\n",
		       local_x, local_y, local_z);
		printf("  Reference: %u\n", expected);
		printf("  Observed: %u\n", p[0]);
		pass = false;
	}

	glUnmapBuffer(GL_ATOMIC_COUNTER_BUFFER);

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

static enum piglit_result
run_test()
{
	enum piglit_result result;
	uint32_t atomics_init = 0;

	if (local_x == 0 || local_y == 0 || local_z == 0)
		return PIGLIT_FAIL;

	glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, atomics_bo);
	glBufferData(GL_ATOMIC_COUNTER_BUFFER, sizeof(uint32_t), &atomics_init,
		     GL_STATIC_DRAW);

	glUseProgram(prog);

	glMemoryBarrier(GL_ALL_BARRIER_BITS);
	glDispatchComputeGroupSizeARB(global_x, global_y, global_z,
				      local_x, local_y, local_z);
	if (!piglit_check_gl_error(GL_NO_ERROR))
		return PIGLIT_FAIL;
	glMemoryBarrier(GL_ALL_BARRIER_BITS);

	result = check_result();
	if (result != PIGLIT_PASS)
		piglit_report_result(result);

	return result;
}

static void
clear_program()
{
	if (prog != 0) {
		local_x = 0;
		local_y = 0;
		local_z = 0;
		glDeleteProgram(prog);
		prog = 0;
	}
}

static enum piglit_result
build_program_for_size(uint32_t x, uint32_t y, uint32_t z)
{
	if (local_x == x && local_y == y &&
	    local_z == z && prog != 0) {
		return PIGLIT_PASS;
	}

	clear_program();

	prog = generate_cs_prog(x, y, z, strdup(compute_shader_source));

	if (!prog)
		return PIGLIT_FAIL;

	local_x = x;
	local_y = y;
	local_z = z;

	return PIGLIT_PASS;
}

enum piglit_result
set_local_size(uint32_t x, uint32_t y, uint32_t z)
{
	enum piglit_result result = PIGLIT_PASS;

	if (x == 0 || y == 0 || z == 0) {
		clear_program();
		return PIGLIT_FAIL;
	}

	result = build_program_for_size(x, y, z);
	if (result != PIGLIT_PASS)
		piglit_report_result(result);

	return result;
}

static enum piglit_result
test_size(uint32_t x, uint32_t y, uint32_t z)
{
	enum piglit_result result;

	result = set_local_size(x, y, z);
	if (result != PIGLIT_PASS)
		piglit_report_result(result);

	result = run_test();
	if (result != PIGLIT_PASS)
		piglit_report_result(result);

	return result;
}

static enum piglit_result
test_all_sizes()
{
	enum piglit_result result = PIGLIT_PASS;
	uint32_t xi, yi, zi;
	uint32_t x, y, z;


	for (zi = 0; zi < ARRAY_SIZE(sizes); zi++) {
		z = sizes[zi];
		if (z > max_local_z)
			break;
		for (yi = 0; yi < ARRAY_SIZE(sizes); yi++) {
			y = sizes[yi];
			if (y > max_local_y ||
			    (y * z) > max_variable_invocations)
				break;
			for (xi = 0; xi < ARRAY_SIZE(sizes); xi++) {
				x = sizes[xi];
				if (x > max_local_x ||
				    (x * y * z) > max_variable_invocations)
					break;
				result = test_size(x, y, z);
				if (result != PIGLIT_PASS)
					return result;
			}
		}
	}

	return result;
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

	piglit_require_extension("GL_ARB_compute_variable_group_size");
	piglit_require_extension("GL_ARB_shader_atomic_counters");

	glGenBuffers(1, &atomics_bo);
	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	glGetIntegeri_v(GL_MAX_COMPUTE_VARIABLE_GROUP_SIZE_ARB,
			0, &max_local_x);
	glGetIntegeri_v(GL_MAX_COMPUTE_VARIABLE_GROUP_SIZE_ARB,
			1, &max_local_y);
	glGetIntegeri_v(GL_MAX_COMPUTE_VARIABLE_GROUP_SIZE_ARB,
			2, &max_local_z);
	glGetIntegerv(GL_MAX_COMPUTE_VARIABLE_GROUP_INVOCATIONS_ARB,
		      &max_variable_invocations);

	result = test_all_sizes();

	glDeleteBuffers(1, &atomics_bo);

	piglit_report_result(result);
}
