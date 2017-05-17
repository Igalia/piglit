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

/** @file default-partition.c
 *
 *  Test that the following is met:
 *
 *  "Unlike other user-defined uniforms declared at global scope,
 *   [atomic counters] take NO storage from the default partition,
 *   they have NO location [...]"
 *
 * (from the ARB_shader_atomic_counters specification)
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
run_test(unsigned max_uniforms)
{
        const char *fs_template = "#version 140\n"
                "#extension GL_ARB_shader_atomic_counters : enable\n"
                "\n"
                "#define N %d\n"
                "\n"
                "out ivec4 fcolor;\n"
                "layout(binding=0) uniform atomic_uint x;\n"
                "uniform uint y[N];\n"
                "\n"
                "void main() {\n"
                "       int i;\n"
                "       uint z = atomicCounter(x);\n"
                "       \n"
                "       for (i = 0; i < N; ++i)\n"
                "               z += y[i];\n"
                "       \n"
                "       fcolor.x = int(z);\n"
                "}\n";
        char *fs_source;
        GLuint prog = glCreateProgram();
        int n = 0, ret;

        ret = asprintf(&fs_source, fs_template, max_uniforms);
        assert(ret > 0);

        /* This should fail to link if 'x' ended up being accounted in
         * the default uniform partition because 'y[]' uses up the
         * whole available uniform space. */
        atomic_counters_compile(prog, GL_FRAGMENT_SHADER, fs_source);
        glLinkProgram(prog);
        if (!piglit_check_gl_error(GL_NO_ERROR))
                return false;

        glGetProgramiv(prog, GL_ACTIVE_UNIFORMS, &n);
        if (n != 2) {
                printf("Unexpected number of active uniforms %d.\n", n);
                return false;
        }

        if (glGetUniformLocation(prog, "x") != -1) {
                printf("Atomic counter unexpectedly reported to have a"
                       " location.\n");
                return false;
        }

        free(fs_source);
        glDeleteProgram(prog);
        return true;
}

void
piglit_init(int argc, char **argv)
{
        struct atomic_counters_limits ls = atomic_counters_get_limits();
        enum piglit_result status = PIGLIT_PASS;

        piglit_require_gl_version(31);
        piglit_require_extension("GL_ARB_shader_atomic_counters");

        atomic_counters_subtest(&status, GL_FRAGMENT_SHADER,
                                "Atomic counter location",
                                run_test, ls.uniform_components);

        piglit_report_result(status);
}

enum piglit_result
piglit_display(void)
{
        return PIGLIT_PASS;
}
