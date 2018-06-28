/*
 * Copyright (c) 2015 Intel Corporation
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

/** @file common.h
 *
 * Common utility functions for the ARB_compute_shader tests.
 */

#ifndef __PIGLIT_ARB_COMPUTE_SHADER_COMMON_H__
#define __PIGLIT_ARB_COMPUTE_SHADER_COMMON_H__

#include "piglit-util-gl.h"

void
cs_ids_common_init(void);

void
cs_ids_common_destroy(void);

enum piglit_result
cs_ids_test_all_sizes(void);

void
cs_ids_verbose(void);

void
cs_ids_set_local_id_test(void);

void
cs_ids_set_global_id_test(void);

void
cs_ids_use_indirect_dispatch(void);

void
cs_ids_use_direct_dispatch(void);

enum piglit_result
cs_ids_set_local_size(uint32_t x, uint32_t y, uint32_t z);

enum piglit_result
cs_ids_set_global_size(uint32_t x, uint32_t y, uint32_t z);

enum piglit_result
cs_ids_run_test();

void
cs_ids_run_test_without_check();

void
cs_ids_setup_atomics_for_test();

enum piglit_result
cs_ids_confirm_initial_atomic_counters();

enum piglit_result
cs_ids_confirm_size();

#endif
