/*
 * Copyright © 2013 Intel Corporation
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

/** @file common.c
 *
 * Common utility functions for the ARB_shader_atomic_counters tests.
 */

#ifndef __PIGLIT_ARB_SHADER_ATOMIC_COUNTERS_COMMON_H__
#define __PIGLIT_ARB_SHADER_ATOMIC_COUNTERS_COMMON_H__

#include "piglit-util-gl.h"

bool
atomic_counters_probe_buffer(unsigned base, unsigned count,
                             const uint32_t *expected);

bool
atomic_counters_compile(GLuint prog, GLuint stage, const char *src);

bool
atomic_counters_link(GLuint prog);

bool
atomic_counters_draw_point(GLuint prog, unsigned buf_size,
                           const uint32_t *buf);

bool
atomic_counters_draw_rect(GLuint prog, unsigned buf_size,
                          const uint32_t *buf);

bool
atomic_counters_draw_patch(GLuint prog, unsigned buf_size,
                           const uint32_t *buf);

bool
atomic_counters_supported(GLenum shader_stage);

char *
atomic_counters_generate_source(const char *src_template, const char *decl_template,
                                const char *insn_template, unsigned n);

struct atomic_counters_limits {
        int fragment_counters;
        int vertex_counters;
        int combined_counters;
        int fragment_buffers;
        int vertex_buffers;
        int combined_buffers;
        int bindings;
        int uniform_components;
};

struct atomic_counters_limits
atomic_counters_get_limits();

#define atomic_counters_subtest(status, shader_stage, name, func, ...) do { \
                if (atomic_counters_supported(shader_stage)) {          \
                        if (func(__VA_ARGS__)) {                        \
                                piglit_report_subtest_result(PIGLIT_PASS, name); \
                        } else {                                        \
                                piglit_report_subtest_result(PIGLIT_FAIL, name); \
                                *status = PIGLIT_FAIL;                  \
                        }                                               \
                } else {                                                \
                        piglit_report_subtest_result(PIGLIT_SKIP, name); \
                }                                                       \
        } while (0)

#endif
