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

/** @file multiple-defs.c
 *
 * Checks that atomic counters with the same name may be linked
 * together if and only if their layout specifications are equivalent.
 */

#include "common.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

        config.supports_gl_core_version = 31;

        config.window_width = 1;
        config.window_height = 1;
        config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;

PIGLIT_GL_TEST_CONFIG_END

const char *frag_src = "#version 140\n"
        "#extension GL_ARB_shader_atomic_counters : enable\n"
        "\n"
        "flat in ivec4 vcolor;\n"
        "out ivec4 fcolor;\n"
        "\n"
        "layout(binding=3, offset=4) uniform atomic_uint x0;\n"
        "layout(binding=2, offset=0) uniform atomic_uint x1;\n"
        "layout(binding=2) uniform atomic_uint x2;\n"
        "\n"
        "void main() {\n"
        "       fcolor.x = vcolor.x + int(atomicCounter(x0) +\n"
        "                  atomicCounter(x1) + atomicCounter(x2));\n"
        "}\n";

const char *vert_src_ok = "#version 140\n"
        "#extension GL_ARB_shader_atomic_counters : enable\n"
        "\n"
        "in vec4 position;\n"
        "flat out ivec4 vcolor;\n"
        "\n"
        "layout(binding=2) uniform atomic_uint x1;\n"
        "layout(binding=2, offset=4) uniform atomic_uint x2;\n"
        "layout(binding=3, offset=4) uniform atomic_uint x0;\n"
        "\n"
        "void main() {\n"
        "       vcolor.x = int(atomicCounter(x0) + atomicCounter(x1)\n"
        "                      + atomicCounter(x2));\n"
        "       gl_Position = position;\n"
        "}\n";

/* This should fail because 'x1' is redefined with a conflicting
 * binding specification. */
const char *vert_src_fail_1 = "#version 140\n"
        "#extension GL_ARB_shader_atomic_counters : enable\n"
        "\n"
        "in vec4 position;\n"
        "flat out ivec4 vcolor;\n"
        "\n"
        "layout(binding=0) uniform atomic_uint x1;\n"
        "layout(binding=2, offset=4) uniform atomic_uint x2;\n"
        "layout(binding=3, offset=4) uniform atomic_uint x0;\n"
        "\n"
        "void main() {\n"
        "       vcolor.x = int(atomicCounter(x0) + atomicCounter(x1)\n"
        "                      + atomicCounter(x2));\n"
        "       gl_Position = position;\n"
        "}\n";

/* This should fail because 'x0' is redefined with a conflicting
 * implicit offset specification. */
const char *vert_src_fail_2 = "#version 140\n"
        "#extension GL_ARB_shader_atomic_counters : enable\n"
        "\n"
        "in vec4 position;\n"
        "flat out ivec4 vcolor;\n"
        "\n"
        "layout(binding=2) uniform atomic_uint x1;\n"
        "layout(binding=2, offset=4) uniform atomic_uint x2;\n"
        "layout(binding=3) uniform atomic_uint x0;\n"
        "\n"
        "void main() {\n"
        "       vcolor.x = int(atomicCounter(x0) + atomicCounter(x1)\n"
        "                      + atomicCounter(x2));\n"
        "       gl_Position = position;\n"
        "}\n";

/* This should fail because 'x3' overlaps an already defined
 * counter. */
const char *vert_src_fail_3 = "#version 140\n"
        "#extension GL_ARB_shader_atomic_counters : enable\n"
        "\n"
        "in vec4 position;\n"
        "flat out ivec4 vcolor;\n"
        "\n"
        "layout(binding=2) uniform atomic_uint x1;\n"
        "layout(binding=2, offset=4) uniform atomic_uint x2;\n"
        "layout(binding=3, offset=0) uniform atomic_uint x3[2];\n"
        "\n"
        "void main() {\n"
        "       vcolor.x = int(atomicCounter(x1) + atomicCounter(x2)\n"
        "                      + atomicCounter(x3[0]));\n"
        "       gl_Position = position;\n"
        "}\n";

/* This should fail because 'x3' has the same location specification
 * as 'x0'. */
const char *vert_src_fail_4 = "#version 140\n"
        "#extension GL_ARB_shader_atomic_counters : enable\n"
        "\n"
        "in vec4 position;\n"
        "flat out ivec4 vcolor;\n"
        "\n"
        "layout(binding=2) uniform atomic_uint x1;\n"
        "layout(binding=2, offset=4) uniform atomic_uint x2;\n"
        "layout(binding=3, offset=4) uniform atomic_uint x3;\n"
        "\n"
        "void main() {\n"
        "       vcolor.x = int(atomicCounter(x1) + atomicCounter(x2)\n"
        "                      + atomicCounter(x3));\n"
        "       gl_Position = position;\n"
        "}\n";

bool
run_test(const char *fs_source, const char *vs_source)
{
        GLuint prog = glCreateProgram();
        bool ret =
                atomic_counters_compile(prog, GL_FRAGMENT_SHADER, fs_source) &&
                atomic_counters_compile(prog, GL_VERTEX_SHADER, vs_source) &&
                atomic_counters_link(prog);

        glDeleteProgram(prog);
        return ret;
}

void
piglit_init(int argc, char **argv)
{
        struct atomic_counters_limits ls = atomic_counters_get_limits();
        enum piglit_result status = PIGLIT_PASS;

        piglit_require_extension("GL_ARB_shader_atomic_counters");

        if (ls.fragment_counters < 3 || ls.vertex_counters < 4) {
                fprintf(stderr, "Insufficient number of supported atomic "
                        "counters.\n");
                piglit_report_result(PIGLIT_SKIP);
        }

        if (ls.fragment_buffers < 2 || ls.vertex_buffers < 3) {
                fprintf(stderr, "Insufficient number of supported atomic "
                        "buffers.\n");
                piglit_report_result(PIGLIT_SKIP);
        }

        atomic_counters_subtest(&status, GL_NONE,
                                "Multiple atomic counter definitions "
                                "(compatible defs)",
                                run_test, frag_src, vert_src_ok);

        atomic_counters_subtest(&status, GL_NONE,
                                "Multiple atomic counter definitions "
                                "(1: incompatible bindings)",
                                !run_test, frag_src, vert_src_fail_1);

        atomic_counters_subtest(&status, GL_NONE,
                                "Multiple atomic counter definitions "
                                "(2: incompatible offsets)",
                                !run_test, frag_src, vert_src_fail_2);

        atomic_counters_subtest(&status, GL_NONE,
                                "Multiple atomic counter definitions "
                                "(3: array overlap)",
                                !run_test, frag_src, vert_src_fail_3);

        atomic_counters_subtest(&status, GL_NONE,
                                "Multiple atomic counter definitions "
                                "(4: conflicting locations)",
                                !run_test, frag_src, vert_src_fail_4);

        piglit_report_result(status);
}

enum piglit_result
piglit_display(void)
{
        return PIGLIT_PASS;
}
