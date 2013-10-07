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

/** @file minmax.c
 *
 * Test for the minimum maximum values described in the
 * ARB_shader_atomic_counters spec.
 */

#include "common.h"
#include "minmax-test.h"

#define L 1

PIGLIT_GL_TEST_CONFIG_BEGIN

        config.supports_gl_core_version = 31;
        config.window_width = L;
        config.window_height = L;
        config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;

PIGLIT_GL_TEST_CONFIG_END

enum piglit_result
piglit_display(void)
{
        return PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
        int ver = piglit_get_gl_version();

        piglit_require_extension("GL_ARB_shader_atomic_counters");

        piglit_print_minmax_header();

        piglit_test_min_int(GL_MAX_VERTEX_ATOMIC_COUNTERS, 0);
        piglit_test_min_int(GL_MAX_FRAGMENT_ATOMIC_COUNTERS, 8);
        if (ver >= 32)
                piglit_test_min_int(GL_MAX_GEOMETRY_ATOMIC_COUNTERS, 0);
        if (ver >= 40) {
                piglit_test_min_int(GL_MAX_TESS_CONTROL_ATOMIC_COUNTERS, 0);
                piglit_test_min_int(GL_MAX_TESS_EVALUATION_ATOMIC_COUNTERS, 0);
        }
        piglit_test_min_int(GL_MAX_COMBINED_ATOMIC_COUNTERS, 8);

        piglit_test_min_int(GL_MAX_VERTEX_ATOMIC_COUNTER_BUFFERS, 0);
        if (ver >= 32)
                piglit_test_min_int(GL_MAX_GEOMETRY_ATOMIC_COUNTER_BUFFERS, 0);
        if (ver >= 40) {
                piglit_test_min_int(GL_MAX_TESS_CONTROL_ATOMIC_COUNTER_BUFFERS, 0);
                piglit_test_min_int(GL_MAX_TESS_EVALUATION_ATOMIC_COUNTER_BUFFERS, 0);
        }
        piglit_test_min_int(GL_MAX_FRAGMENT_ATOMIC_COUNTER_BUFFERS, 1);
        piglit_test_min_int(GL_MAX_COMBINED_ATOMIC_COUNTER_BUFFERS, 1);
        piglit_test_min_int(GL_MAX_ATOMIC_COUNTER_BUFFER_SIZE, 32);

        piglit_test_min_int(GL_MAX_ATOMIC_COUNTER_BUFFER_BINDINGS, 1);

        if (!piglit_check_gl_error(GL_NO_ERROR))
                piglit_report_result(PIGLIT_FAIL);

        piglit_report_result(piglit_minmax_pass ? PIGLIT_PASS : PIGLIT_FAIL);
}
