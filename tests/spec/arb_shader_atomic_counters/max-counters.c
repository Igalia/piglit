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

/** @file max-counters.c
 *
 * Test that using more than the maximum number of suported atomic
 * counters, buffers or bindings fails with a linking error.
 */

#include "common.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

        config.supports_gl_core_version = 31;

        config.window_width = 1;
        config.window_height = 1;
        config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;

PIGLIT_GL_TEST_CONFIG_END

static char *
iterate_template(const char *template, unsigned n)
{
        char *ss;
        int i, ret;

        ss = strdup("");
        assert(ss);

        for (i = 0; i < n; ++i) {
                char *s, *tmp = ss;

                ret = asprintf(&s, template, i);
                assert(ret >= 0);

                ret = asprintf(&ss, "%s%s", tmp, s);
                assert(ret >= 0);

                free(tmp);
                free(s);
        }

        return ss;
}

/**
 * Generate source code by substituting the first occurrence of "%s"
 * in \a src_template with \a n copies of \a decl_template and the
 * second occurrence of "%s" with \a n copies of \a insn_template.
 */
static char *
generate_source(const char *src_template,
                const char *decl_template, const char *insn_template,
                unsigned n)
{
        char *decls = iterate_template(decl_template, n);
        char *insns = iterate_template(insn_template, n);
        char *src;
        int ret;

        ret = asprintf(&src, src_template, decls, insns);
        assert(ret);

        free(decls);
        free(insns);

        return src;
}

static bool
run_test_vertex_max_counters(unsigned num_counters)
{
        /* Generate a shader with 'num_counters' counters. */
        char *vs_source = generate_source(
                "#version 140\n"
                "#extension GL_ARB_shader_atomic_counters : enable\n"
                "\n"
                "%s\n"
                "\n"
                "in vec4 position;\n"
                "flat out ivec4 vcolor;\n"
                "\n"
                "void main() {\n"
                "       uint y = 0u;\n"
                "       %s\n"
                "       vcolor.x = int(y);\n"
                "       gl_Position = position;\n"
                "}\n",
                "layout(binding=0) uniform atomic_uint x%d;\n",
                "       y += atomicCounterDecrement(x%d);\n",
                num_counters);
        GLuint prog = glCreateProgram();
        bool ret = atomic_counters_compile(prog, GL_VERTEX_SHADER, vs_source) &&
                atomic_counters_link(prog);

        glDeleteProgram(prog);
        free(vs_source);
        return ret;
}

static bool
run_test_fragment_max_counters(unsigned num_counters)
{
        /* Generate a shader with 'num_counters' counters. */
        char *fs_source = generate_source(
                "#version 140\n"
                "#extension GL_ARB_shader_atomic_counters : enable\n"
                "\n"
                "%s\n"
                "\n"
                "out ivec4 fcolor;\n"
                "\n"
                "void main() {\n"
                "       uint y = 0u;\n"
                "       %s\n"
                "       fcolor.x = int(y);\n"
                "}\n",
                "layout(binding=0) uniform atomic_uint x%d;\n",
                "       y += atomicCounterDecrement(x%d);\n",
                num_counters);
        GLuint prog = glCreateProgram();
        bool ret = atomic_counters_compile(prog, GL_FRAGMENT_SHADER, fs_source) &&
                atomic_counters_link(prog);

        glDeleteProgram(prog);
        free(fs_source);
        return ret;
}

static bool
run_test_combined_max_counters(unsigned num_fragment_counters,
                               unsigned num_vertex_counters)
{
        /* Generate a shader with 'num_fragment_counters' counters. */
        char *fs_source = generate_source(
                "#version 140\n"
                "#extension GL_ARB_shader_atomic_counters : enable\n"
                "\n"
                "%s\n"
                "\n"
                "flat in ivec4 vcolor;\n"
                "out ivec4 fcolor;\n"
                "\n"
                "void main() {\n"
                "       uint y = uint(vcolor.x);\n"
                "       %s\n"
                "       fcolor.x = int(y);\n"
                "}\n",
                "layout(binding=0) uniform atomic_uint fx%d;\n",
                "       y += atomicCounterDecrement(fx%d);\n",
                num_fragment_counters);
        /* Generate a shader with 'num_vertex_counters' counters. */
        char *vs_source = generate_source(
                "#version 140\n"
                "#extension GL_ARB_shader_atomic_counters : enable\n"
                "\n"
                "%s\n"
                "\n"
                "in vec4 position;\n"
                "flat out ivec4 vcolor;\n"
                "\n"
                "void main() {\n"
                "       uint y = 0u;\n"
                "       %s\n"
                "       vcolor.x = int(y);\n"
                "       gl_Position = position;\n"
                "}\n",
                "layout(binding=1) uniform atomic_uint vx%d;\n",
                "       y += atomicCounterDecrement(vx%d);\n",
                num_vertex_counters);
        GLuint prog = glCreateProgram();
        bool ret = atomic_counters_compile(prog, GL_FRAGMENT_SHADER, fs_source) &&
                atomic_counters_compile(prog, GL_VERTEX_SHADER, vs_source) &&
                atomic_counters_link(prog);

        glDeleteProgram(prog);
        free(fs_source);
        free(vs_source);
        return ret;
}

static bool
run_test_fragment_max_buffers(unsigned num_buffers)
{
        /* Generate a shader with 'num_buffers' buffers. */
        char *src = generate_source(
                "#version 140\n"
                "#extension GL_ARB_shader_atomic_counters : enable\n"
                "#define PASTE(A,B) A ## B\n"
                "#define Y(I) PASTE(y, I)\n"
                "\n"
                "out ivec4 fcolor;\n"
                "\n"
                "%s"
                "\n"
                "void main() {\n"
                "       uint x = 0u;\n"
                "       %s\n"
                "       fcolor.x = int(x);\n"
                "}\n",
                "#define I %d\n"
                "layout(binding=I, offset=0) uniform atomic_uint Y(I);\n"
                "#undef I\n",
                "       x += atomicCounterDecrement(y%d);\n",
                num_buffers);
        GLuint prog = glCreateProgram();
        bool ret = atomic_counters_compile(prog, GL_FRAGMENT_SHADER, src) &&
                atomic_counters_link(prog);

        glDeleteProgram(prog);
        free(src);
        return ret;
}

static bool
run_test_vertex_max_buffers(unsigned num_buffers)
{
        /* Generate a shader with 'num_buffers' buffers. */
        char *src = generate_source(
                "#version 140\n"
                "#extension GL_ARB_shader_atomic_counters : enable\n"
                "#define PASTE(A,B) A ## B\n"
                "#define X(I) PASTE(x, I)\n"
                "\n"
                "in vec4 position;\n"
                "flat out ivec4 vcolor;\n"
                "\n"
                "%s"
                "\n"
                "void main() {\n"
                "       uint x = 0u;\n"
                "       %s\n"
                "       vcolor.x = int(x);\n"
                "       gl_Position = position;\n"
                              "}\n",
                "#define I %d\n"
                "layout(binding=I, offset=0) uniform atomic_uint X(I);\n"
                "#undef I\n",
                "       x += atomicCounterDecrement(x%d);\n",
                num_buffers);
        GLuint prog = glCreateProgram();
        bool ret = atomic_counters_compile(prog, GL_VERTEX_SHADER, src) &&
                atomic_counters_link(prog);

        glDeleteProgram(prog);
        free(src);
        return ret;
}

static bool
run_test_combined_max_buffers(unsigned num_fragment_buffers,
                              unsigned num_vertex_buffers)
{
        /* Generate a shader with 'num_fragment_buffers' buffers. */
        char *fs_source = generate_source(
                "#version 140\n"
                "#extension GL_ARB_shader_atomic_counters : enable\n"
                "#define PASTE(A,B) A ## B\n"
                "#define Y(I) PASTE(y, I)\n"
                "\n"
                "out ivec4 fcolor;\n"
                "\n"
                "%s"
                "\n"
                "void main() {\n"
                "       uint x = 0u;\n"
                "       %s\n"
                "       fcolor.x = int(x);\n"
                "}\n",
                "#define I %d\n"
                "layout(binding=I, offset=0) uniform atomic_uint Y(I);\n"
                "#undef I\n",
                "       x += atomicCounterDecrement(y%d);\n",
                num_fragment_buffers);
        /* Generate a shader with 'num_vertex_buffers' buffers. */
        char *vs_source = generate_source(
                "#version 140\n"
                "#extension GL_ARB_shader_atomic_counters : enable\n"
                "#define PASTE(A,B) A ## B\n"
                "#define X(I) PASTE(x, I)\n"
                "\n"
                "in vec4 position;\n"
                "flat out ivec4 vcolor;\n"
                "\n"
                "%s"
                "\n"
                "void main() {\n"
                "       uint x = 0u;\n"
                "       %s\n"
                "       vcolor.x = int(x);\n"
                "       gl_Position = position;\n"
                "}\n",
                "#define I %d\n"
                "layout(binding=I, offset=0) uniform atomic_uint X(I);\n"
                "#undef I\n",
                "       x += atomicCounterDecrement(x%d);\n",
                num_vertex_buffers);
        GLuint prog = glCreateProgram();
        bool ret =
                atomic_counters_compile(prog, GL_FRAGMENT_SHADER, fs_source) &&
                atomic_counters_compile(prog, GL_VERTEX_SHADER, vs_source) &&
                atomic_counters_link(prog);

        glDeleteProgram(prog);
        free(fs_source);
        free(vs_source);
        return ret;
}

static bool
run_test_fragment_max_bindings(unsigned binding)
{
        const char *src_template = "#version 140\n"
                "#extension GL_ARB_shader_atomic_counters : enable\n"
                "\n"
                "layout(binding=%d) uniform atomic_uint x;"
                "\n"
                "void main() {\n"
                "}\n";
        char *src;
        GLuint prog = glCreateProgram();
        int ret;

        ret = asprintf(&src, src_template, binding);
        assert(ret);

        ret = atomic_counters_compile(prog, GL_FRAGMENT_SHADER, src);

        glDeleteProgram(prog);
        free(src);
        return ret;
}

static bool
run_test_vertex_max_bindings(unsigned binding)
{
        const char *src_template = "#version 140\n"
                "#extension GL_ARB_shader_atomic_counters : enable\n"
                "\n"
                "in vec4 position;\n"
                "layout(binding=%d) uniform atomic_uint x;"
                "\n"
                "void main() {\n"
                "       gl_Position = position;\n"
                "}\n";
        char *src;
        GLuint prog = glCreateProgram();
        int ret;

        ret = asprintf(&src, src_template, binding);
        assert(ret);

        ret = atomic_counters_compile(prog, GL_VERTEX_SHADER, src);

        glDeleteProgram(prog);
        free(src);
        return ret;
}

void
piglit_init(int argc, char **argv)
{
        enum piglit_result status = PIGLIT_PASS;
        struct atomic_counters_limits ls = atomic_counters_get_limits();

        piglit_require_gl_version(31);
        piglit_require_extension("GL_ARB_shader_atomic_counters");

        printf("Max combined: %d\n", ls.combined_counters);
        printf("Max VS: %d\n", ls.vertex_counters);
        printf("Max FS: %d\n", ls.fragment_counters);

        atomic_counters_subtest(&status, GL_VERTEX_SHADER,
                                "Vertex shader test above maximum "
                                "number of atomic counters",
                                !run_test_vertex_max_counters,
                                ls.vertex_counters + 1);

        atomic_counters_subtest(&status, GL_FRAGMENT_SHADER,
                                "Fragment shader test above maximum "
                                "number of atomic counters",
                                !run_test_fragment_max_counters,
                                ls.fragment_counters + 1);

        if (ls.vertex_counters + ls.fragment_counters > ls.combined_counters) {
                atomic_counters_subtest(&status, GL_NONE,
                                        "Combined test above maximum number "
                                        "of atomic counters",
                                        !run_test_combined_max_counters,
                                        ls.fragment_counters,
                                        ls.combined_counters
                                        - ls.fragment_counters + 1);
        } else {
                piglit_report_subtest_result(
                        PIGLIT_SKIP, "Combined test above maximum number "
                        "of atomic counters");
        }

        atomic_counters_subtest(&status, GL_FRAGMENT_SHADER,
                                "Fragment shader test under maximum "
                                "number of atomic counter buffers",
                                run_test_fragment_max_buffers,
                                ls.fragment_buffers);

        atomic_counters_subtest(&status, GL_FRAGMENT_SHADER,
                                "Fragment shader test above maximum "
                                "number of atomic counter buffers",
                                !run_test_fragment_max_buffers,
                                ls.fragment_buffers + 1);

        atomic_counters_subtest(&status, GL_VERTEX_SHADER,
                                "Vertex shader test under maximum "
                                "number of atomic counter buffers",
                                run_test_vertex_max_buffers,
                                ls.vertex_buffers);

        atomic_counters_subtest(&status, GL_VERTEX_SHADER,
                                "Vertex shader test above maximum "
                                "number of atomic counter buffers",
                                !run_test_vertex_max_buffers,
                                ls.vertex_buffers + 1);

        if (ls.vertex_buffers + ls.fragment_buffers > ls.combined_buffers) {
                int max_safe_vs = MIN2(ls.vertex_buffers,
				       ls.combined_buffers -
				       ls.fragment_buffers);
                atomic_counters_subtest(&status, GL_NONE,
                                        "Combined test under maximum "
                                        "number of atomic counter buffers",
                                        run_test_combined_max_buffers,
                                        ls.fragment_buffers,
                                        max_safe_vs);

                atomic_counters_subtest(&status, GL_NONE,
                                        "Combined test above maximum "
                                        "number of atomic counter buffers",
                                        !run_test_combined_max_buffers,
                                        ls.fragment_buffers,
                                        max_safe_vs + 1);

        } else {
                piglit_report_subtest_result(
                        PIGLIT_SKIP, "Combined test under maximum "
                        "number of atomic counter buffers");
                piglit_report_subtest_result(
                        PIGLIT_SKIP, "Combined test above maximum "
                        "number of atomic counter buffers");
        }

        atomic_counters_subtest(&status, GL_FRAGMENT_SHADER,
                                "Fragment shader test above maximum "
                                "number of atomic counter bindings",
                                !run_test_fragment_max_bindings,
                                ls.bindings + 1);

        atomic_counters_subtest(&status, GL_VERTEX_SHADER,
                                "Vertex shader test above maximum "
                                "number of atomic counter bindings",
                                !run_test_vertex_max_bindings,
                                ls.bindings + 1);

        piglit_report_result(status);
}

enum piglit_result
piglit_display(void)
{
        return PIGLIT_PASS;
}
