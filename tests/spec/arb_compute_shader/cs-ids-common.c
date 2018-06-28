/*
 * Copyright (c) 2014 - 2015 Intel Corporation
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

#include "piglit-util-gl.h"
#include "piglit-shader.h"
#include "common.h"

#define NUM_ATOMIC_COUNTERS 8

static GLuint atomics_bo = 0;
static GLuint indirect_bo = 0;
static bool verbose = false;
static bool indirect_dispatch = false;
static bool global_id = false;
static GLint prog = 0;

static uint32_t global_x = 0, global_y = 0, global_z = 0;
static uint32_t local_x = 0, local_y = 0, local_z = 0;

static uint32_t sizes[] = {
	1, 2, 3, 4, 5, 7, 8, 9, 15, 16, 17, 31, 32, 33, 63, 64, 65,
	127, 128, 129, 255, 256, 257, 511, 512, 513, 1023, 1024
};

static const char *extensions =
	"#extension GL_ARB_shader_atomic_counters: require\n";

static const char *compute_shader_source =
	"layout(binding = 0) uniform atomic_uint a0;\n"
	"layout(binding = 0) uniform atomic_uint a1;\n"
	"layout(binding = 0) uniform atomic_uint a2;\n"
	"layout(binding = 0) uniform atomic_uint a3;\n"
	"layout(binding = 0) uniform atomic_uint a4;\n"
	"layout(binding = 0) uniform atomic_uint a5;\n"
	"layout(binding = 0) uniform atomic_uint a6;\n"
	"layout(binding = 0) uniform atomic_uint a7;\n"
	"\n"
	"#ifdef GLOBAL_ID_TEST\n"
	"#define ID_VAR gl_GlobalInvocationID\n"
	"#define ID_DIM(a) (gl_NumWorkGroups.a * gl_WorkGroupSize.a)\n"
	"#else\n"
	"#define ID_VAR gl_LocalInvocationID\n"
	"#define ID_DIM(a) (gl_WorkGroupSize.a)\n"
	"#endif\n"
	"\n"
	"void main()\n"
	"{\n"
	"    uint x = ID_VAR.x;\n"
	"    uint y = ID_VAR.y;\n"
	"    uint z = ID_VAR.z;\n"
	"    uint hx = ID_DIM(x) / 2u;\n"
	"    uint hy = ID_DIM(y) / 2u;\n"
	"    uint hz = ID_DIM(z) / 2u;\n"
	"\n"
	"    if (((x & y) & z) == 0u)\n"
	"	 atomicCounterIncrement(a0);\n"
	"    if (((x | y) | z) == 7u)\n"
	"	 atomicCounterIncrement(a1);\n"
	"    if (x == y && y == z)\n"
	"	 atomicCounterIncrement(a2);\n"
	"    if (x != y && y != z && x != z)\n"
	"	 atomicCounterIncrement(a3);\n"
	"    if (((x & y) & z) == 2u)\n"
	"	 atomicCounterIncrement(a4);\n"
	"    if (((x | y) | z) == 5u)\n"
	"	 atomicCounterIncrement(a5);\n"
	"    if (x < hx && y < hy && z < hz)\n"
	"	 atomicCounterIncrement(a6);\n"
	"    if (x >= hx || y >= hy || z >= hz)\n"
	"	 atomicCounterIncrement(a7);\n"
	"}\n";


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
compare_atomic_counters(uint32_t *values, uint32_t xs, uint32_t ys,
                        uint32_t zs)
{
	bool pass = true;
	uint32_t *p;

	glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, atomics_bo);
	p = glMapBufferRange(GL_ATOMIC_COUNTER_BUFFER,
			     0,
			     NUM_ATOMIC_COUNTERS * sizeof(uint32_t),
			     GL_MAP_READ_BIT);

	if (!p) {
		printf("Couldn't map atomic counter to verify expected value.\n");
		return PIGLIT_FAIL;
	}

	for (unsigned i = 0; i < NUM_ATOMIC_COUNTERS; i++) {
		uint32_t found = p[i];
		if (verbose)
			printf("Atomic counter %d\n"
			       "  Reference: %u\n"
			       "  Observed:  %u\n"
			       "  Result: %s\n",
			       i, values[i], found,
			       values[i] == found ? "pass" : "fail");
		if (values[i] != found) {
			printf("Atomic counter test %d failed for (%d, %d, %d)\n",
			       i, xs, ys, zs);
			printf("  Reference: %u\n", values[i]);
			printf("  Observed:  %u\n", found);
			pass = false;
			break;
		}
	}

	glUnmapBuffer(GL_ATOMIC_COUNTER_BUFFER);

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

enum piglit_result
cs_ids_confirm_initial_atomic_counters()
{
	uint32_t atomics_init[NUM_ATOMIC_COUNTERS] = { 0 };
	return compare_atomic_counters(atomics_init, 0, 0, 0);
}

enum piglit_result
cs_ids_confirm_size()
{
	uint32_t values[NUM_ATOMIC_COUNTERS];
	uint32_t i, x, y, z;
	uint32_t xs, ys, zs;
	uint32_t hx, hy, hz;

	xs = local_x;
	ys = local_y;
	zs = local_z;

	if (global_id) {
		xs *= global_x;
		ys *= global_y;
		zs *= global_z;
	}

	hx = xs / 2u;
	hy = ys / 2u;
	hz = zs / 2u;

	memset(&values, 0, sizeof values);

	const bool no_work = global_x == 0 || global_y == 0 || global_z == 0;
	for (z = 0; z < zs && !no_work; z++) {
		for (y = 0; y < ys; y++) {
			for (x = 0; x < xs; x++) {
				if (((x & y) & z) == 0u)
					values[0]++;
				if (((x | y) | z) == 7u)
					values[1]++;
				if (x == y && y == z)
					values[2]++;
				if (x != y && y != z && x != z)
					values[3]++;
				if (((x & y) & z) == 2u)
					values[4]++;
				if (((x | y) | z) == 5u)
					values[5]++;
				if (x < hx && y < hy && z < hz)
					values[6]++;
				if (x >= hx || y >= hy || z >= hz)
					values[7]++;
			}
		}
	}

	if (!global_id) {
		for (i = 0; i < NUM_ATOMIC_COUNTERS; i++)
			values[i] *= global_x * global_y * global_z;
	}

	return compare_atomic_counters(values, xs, ys, zs);
}


static enum piglit_result
build_program_for_size(uint32_t x, uint32_t y, uint32_t z)
{
	char *src;

	if (local_x == x && local_y == y &&
	    local_z == z && prog != 0) {
		return PIGLIT_PASS;
	}

	clear_program();

	if (global_id) {
		src = concat(hunk("#define GLOBAL_ID_TEST\n"),
			     hunk(compute_shader_source),
			     NULL);
	} else {
		src = hunk(compute_shader_source);
	}

	prog = generate_cs_prog(x, y, z, hunk(extensions), src);

	if (!prog)
		return PIGLIT_FAIL;

	local_x = x;
	local_y = y;
	local_z = z;

	return PIGLIT_PASS;
}


enum piglit_result
cs_ids_set_local_size(uint32_t x, uint32_t y, uint32_t z)
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


enum piglit_result
cs_ids_set_global_size(uint32_t x, uint32_t y, uint32_t z)
{
	GLuint indirect_buf[3] = { x, y, z };

	global_x = x;
	global_y = y;
	global_z = z;

	glBindBuffer(GL_DISPATCH_INDIRECT_BUFFER, indirect_bo);
	glBufferData(GL_DISPATCH_INDIRECT_BUFFER,
		     sizeof(indirect_buf),
		     indirect_buf, GL_STREAM_READ);

	return PIGLIT_PASS;
}


void
cs_ids_setup_atomics_for_test()
{
	uint32_t atomics_init[NUM_ATOMIC_COUNTERS] = { 0 };

	glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 0, atomics_bo);
	glBufferData(GL_ATOMIC_COUNTER_BUFFER,
		     sizeof(atomics_init),
		     atomics_init, GL_STATIC_DRAW);
}


/* Running the test without checking the result is useful for creating display
 * list tests.
 */
void
cs_ids_run_test_without_check()
{
	if (verbose)
		printf("Testing local dim = %dx%dx%d; "
		       "global dim = %dx%dx%d\n",
		       local_x, local_y, local_z,
		       global_x, global_y, global_z);

	if (local_x == 0 || local_y == 0 || local_z == 0) {
		fprintf(stderr, "Internal error: local size not set\n");
		return;
	}

	glUseProgram(prog);

	glMemoryBarrier(GL_ALL_BARRIER_BITS);
	if (indirect_dispatch) {
		glBindBuffer(GL_DISPATCH_INDIRECT_BUFFER, indirect_bo);
		glDispatchComputeIndirect(0);
	} else {
		glDispatchCompute(global_x, global_y, global_z);
	}
	glMemoryBarrier(GL_ALL_BARRIER_BITS);
}


enum piglit_result
cs_ids_run_test()
{
	enum piglit_result result;

	cs_ids_setup_atomics_for_test();
	cs_ids_run_test_without_check();

	result = cs_ids_confirm_size();
	if (result != PIGLIT_PASS)
		piglit_report_result(result);

	return result;
}


static enum piglit_result
test_size(uint32_t x, uint32_t y, uint32_t z)
{
	enum piglit_result result;

	result = cs_ids_set_local_size(x, y, z);
	if (result != PIGLIT_PASS)
		piglit_report_result(result);

	result = cs_ids_run_test();
	if (result != PIGLIT_PASS)
		piglit_report_result(result);

	return result;
}


enum piglit_result
cs_ids_test_all_sizes()
{
	enum piglit_result result = PIGLIT_PASS;
	uint32_t xi, yi, zi;
	uint32_t x, y, z;


	for (zi = 0; zi < ARRAY_SIZE(sizes); zi++) {
		z = sizes[zi];
		if (z > 64)
			break;
		for (yi = 0; yi < ARRAY_SIZE(sizes); yi++) {
			y = sizes[yi];
			if ((y * z) > 1024)
				break;
			for (xi = 0; xi < ARRAY_SIZE(sizes); xi++) {
				x = sizes[xi];
				if ((x * y * z) > 1024)
					break;
				result = test_size(x, y, z);
				if (result != PIGLIT_PASS)
					return result;
			}
		}
	}

	return result;
}

void
cs_ids_common_init(void)
{
	piglit_require_extension("GL_ARB_compute_shader");
	piglit_require_extension("GL_ARB_shader_atomic_counters");

	glGenBuffers(1, &atomics_bo);
	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	glGenBuffers(1, &indirect_bo);
	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);
	cs_ids_set_global_size(1, 1, 1);

}


void
cs_ids_common_destroy(void)
{
	if (atomics_bo != 0)
		glDeleteBuffers(1, &atomics_bo);
	if (indirect_bo != 0)
		glDeleteBuffers(1, &indirect_bo);
}


void
cs_ids_set_local_id_test(void)
{
	if (global_id) {
		uint32_t x = local_x, y = local_y, z = local_z;
		clear_program();
		global_id = false;
		cs_ids_set_local_size(x, y, z);
	}
}


void
cs_ids_set_global_id_test(void)
{
	if (!global_id) {
		uint32_t x = local_x, y = local_y, z = local_z;
		clear_program();
		global_id = true;
		cs_ids_set_local_size(x, y, z);
	}
}


void
cs_ids_use_indirect_dispatch(void)
{
	indirect_dispatch = true;
}


void
cs_ids_use_direct_dispatch(void)
{
	indirect_dispatch = false;
}

void
cs_ids_verbose(void)
{
	verbose = true;
}
