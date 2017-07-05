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

/** @file buffer-binding.c
 *
 * Test that glBindBufferBase() and glBindBufferRange() have the
 * necessary error checking for atomic counter buffers and that they
 * update the buffer metadata correctly.
 */

#include "common.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

        config.supports_gl_core_version = 31;

        config.window_width = 1;
        config.window_height = 1;
        config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;
	config.khr_no_error_support = PIGLIT_NO_ERRORS;

PIGLIT_GL_TEST_CONFIG_END

static bool
run_test_bind_at(unsigned i)
{
        GLuint buffer;

        glGenBuffers(1, &buffer);

        glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, i, buffer);
        if (glGetError())
                return false;

        glBufferData(GL_ATOMIC_COUNTER_BUFFER, 4, NULL, GL_STATIC_DRAW);
        glDeleteBuffers(1, &buffer);

        return !glGetError();
}

static bool
run_test_bind_range(unsigned i)
{
        GLuint buffer;
        GLint binding;
        GLint start, size;

        glGenBuffers(1, &buffer);

        glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, i, buffer);
        if (!piglit_check_gl_error(GL_NO_ERROR)) {
                printf("Initial buffer binding failed.\n");
                return false;
        }

        glBufferData(GL_ATOMIC_COUNTER_BUFFER, 16, NULL, GL_STATIC_DRAW);

        if (!piglit_khr_no_error) {
                glBindBufferRange(GL_ATOMIC_COUNTER_BUFFER, i, buffer, 6, 5);
                if (!piglit_check_gl_error(GL_INVALID_VALUE)) {
                        printf("Misaligned buffer range binding didn't "
                               "generate a GL_INVALID_VALUE error.\n");
                        return false;
                }
        }

        glBindBufferRange(GL_ATOMIC_COUNTER_BUFFER, i, buffer, 8, 5);
        if (!piglit_check_gl_error(GL_NO_ERROR)) {
                printf("Buffer range binding failed.\n");
                return false;
        }

        glGetIntegerv(GL_ATOMIC_COUNTER_BUFFER_BINDING, &binding);
        if (!piglit_check_gl_error(GL_NO_ERROR) ||
            binding != buffer) {
                printf("Unexpected generic counter buffer binding: 0x%x.\n",
                       binding);
                return false;
        }

        glGetIntegeri_v(GL_ATOMIC_COUNTER_BUFFER_BINDING, i, &binding);
        if (!piglit_check_gl_error(GL_NO_ERROR) ||
            binding != buffer) {
                printf("Unexpected counter buffer binding %d: 0x%x.\n",
                       i, binding);
                return false;
        }

        glGetIntegeri_v(GL_ATOMIC_COUNTER_BUFFER_START, i, &start);
        if (!piglit_check_gl_error(GL_NO_ERROR) ||
            start != 8) {
                printf("Unexpected counter buffer offset 0x%x.\n",
                       (unsigned)start);
                return false;
        }

        glGetIntegeri_v(GL_ATOMIC_COUNTER_BUFFER_SIZE, i, &size);
        if (!piglit_check_gl_error(GL_NO_ERROR) ||
            size != 5) {
                printf("Unexpected counter buffer size: 0x%x.\n",
                       (unsigned)size);
                return false;
        }

        glDeleteBuffers(1, &buffer);

        return true;
}

void
piglit_init(int argc, char **argv)
{
        struct atomic_counters_limits ls = atomic_counters_get_limits();
        enum piglit_result status = PIGLIT_PASS;

        piglit_require_gl_version(31);
        piglit_require_extension("GL_ARB_shader_atomic_counters");

        atomic_counters_subtest(&status, GL_NONE,
                                "Atomic buffer binding below the "
                                "implementation limit",
                                run_test_bind_at, ls.bindings - 1);

        if (!piglit_khr_no_error) {
                atomic_counters_subtest(&status, GL_NONE,
                                        "Atomic buffer binding above the "
                                        "implementation limit",
                                        !run_test_bind_at, ls.bindings);
        }

        atomic_counters_subtest(&status, GL_NONE,
                                "Atomic buffer range binding",
                                run_test_bind_range, ls.bindings - 1);

        piglit_report_result(status);
}

enum piglit_result
piglit_display(void)
{
        return PIGLIT_PASS;
}
