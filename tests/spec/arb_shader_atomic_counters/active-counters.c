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

/** @file active-counters.c
 *
 * Compile a shader with a bunch of atomic counters and check that the
 * active atomic counter and buffer query functions return sane
 * results.
 */

#include "common.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

        config.supports_gl_core_version = 31;

        config.window_width = 1;
        config.window_height = 1;
        config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;
	config.khr_no_error_support = PIGLIT_NO_ERRORS;

PIGLIT_GL_TEST_CONFIG_END

const char *fs_source = "#version 140\n"
        "#extension GL_ARB_shader_atomic_counters : enable\n"
        "\n"
        "out ivec4 fcolor;\n"
        "\n"
        "layout(binding=0) uniform atomic_uint x0[2];\n"
        "layout(binding=0) uniform atomic_uint x1;\n"
        "layout(binding=3, offset=8) uniform atomic_uint x2;\n"
        "layout(binding=3, offset=12) uniform atomic_uint x3;\n"
        "\n"
        "layout(binding=7, binding=2, offset=4) uniform;\n"
        "\n"
        "layout(binding=2) uniform atomic_uint x4;\n"
        "layout(binding=7, offset=8, offset=0) uniform atomic_uint x5;\n"
        "layout(binding=3) uniform atomic_uint x6, x7;\n"
        "\n"
        "void main() {\n"
        "       fcolor.x = int(atomicCounter(x0[0]) + atomicCounter(x0[1])\n"
        "                      + atomicCounter(x1) + atomicCounter(x2)\n"
        "                      + atomicCounter(x3) + atomicCounter(x4)\n"
        "                      + atomicCounter(x5) + atomicCounter(x6)\n"
        "                      + atomicCounter(x7));\n"
        "}\n";

struct buffer_info {
        unsigned num_counters;
        unsigned min_reasonable_size;
};

static const struct buffer_info *
expected_buffer_info(unsigned binding)
{
        if (binding == 0) {
                static const struct buffer_info info = { 2, 12 };
                return &info;
        } else if (binding == 2) {
                static const struct buffer_info info = { 1, 4 };
                return &info;
        } else if (binding == 3) {
                static const struct buffer_info info = { 4, 24 };
                return &info;
        } else if (binding == 7) {
                static const struct buffer_info info = { 1, 4 };
                return &info;
        } else {
                return NULL;
        }
}

struct counter_info {
        unsigned binding;
        unsigned offset;
        unsigned size;
};

static const struct counter_info *
expected_counter_info(const char *name)
{
        static const struct {
                const char *name;
                const struct counter_info info;
        } counters[] = {
                { "x0[0]", { 0, 0, 2 } },
                { "x1", { 0, 8, 1 }},
                { "x2", { 3, 8, 1 }},
                { "x3", { 3, 12, 1 }},
                { "x4", { 2, 0, 1 }},
                { "x5", { 7, 0, 1 }},
                { "x6", { 3, 16, 1 }},
                { "x7", { 3, 20, 1 }},
        };
        int i;

        for (i = 0; i < ARRAY_SIZE(counters); i++) {
                if (!strcmp(counters[i].name, name))
                        return &counters[i].info;
        }

        return NULL;
}

void
piglit_init(int argc, char **argv)
{
        struct atomic_counters_limits ls = atomic_counters_get_limits();
        bool visited_buffers[8] = { false };
        bool visited_counters[8] = { false };
        GLuint prog = glCreateProgram();
        int i, j, n, ret;

        piglit_require_gl_version(31);
        piglit_require_extension("GL_ARB_shader_atomic_counters");

        if (ls.fragment_counters < 9) {
                fprintf(stderr, "Insufficient number of supported atomic "
                        "counters.\n");
                piglit_report_result(PIGLIT_SKIP);
        }

        if (ls.fragment_buffers < 4) {
                fprintf(stderr, "Insufficient number of supported atomic "
                        "counter buffers.\n");
                piglit_report_result(PIGLIT_SKIP);
        }

        if (!atomic_counters_compile(prog, GL_FRAGMENT_SHADER, fs_source)) {
                fprintf(stderr, "Program failed to compile.\n");
                piglit_report_result(PIGLIT_FAIL);
        }

        if (!atomic_counters_link(prog)) {
                fprintf(stderr, "Program failed to link.\n");
                piglit_report_result(PIGLIT_FAIL);
        }

        glGetProgramiv(prog, GL_ACTIVE_ATOMIC_COUNTER_BUFFERS, &n);
        if (n != 4) {
                fprintf(stderr, "Unexpected number of active counter "
                        "buffers.\n");
                piglit_report_result(PIGLIT_FAIL);
        }

        if (!piglit_khr_no_error) {
                ret = 0xdeadbeef;
                glGetActiveAtomicCounterBufferiv(
                        prog, n, GL_ATOMIC_COUNTER_BUFFER_BINDING, &ret);

                if (!piglit_check_gl_error(GL_INVALID_VALUE)) {
                        fprintf(stderr, "glGetActiveAtomicCounterBufferiv "
                                "should have failed when trying to query a "
                                "non-existent buffer.\n");
                        piglit_report_result(PIGLIT_FAIL);
                }

                if (ret != 0xdeadbeef) {
                        fprintf(stderr, "Failed call to "
                                "glGetActiveAtomicCounterBufferiv didn't "
                                "preserve the output parameter contents.\n");
                        piglit_report_result(PIGLIT_FAIL);
                }
        }

        for (i = 0; i < n; ++i) {
                const struct buffer_info *binfo;
                int binding, data_size, num_counters, ref;
                GLuint counters[4];

                glGetActiveAtomicCounterBufferiv(
                        prog, i, GL_ATOMIC_COUNTER_BUFFER_BINDING, &binding);
                if (!piglit_check_gl_error(GL_NO_ERROR)) {
                        fprintf(stderr, "Couldn't obtain counter buffer binding"
                                " point.\n");
                        piglit_report_result(PIGLIT_FAIL);
                }

                binfo = expected_buffer_info(binding);
                if (!binfo) {
                        fprintf(stderr, "Got unexpected buffer binding "
                                "point.\n");
                        piglit_report_result(PIGLIT_FAIL);
                }

                glGetActiveAtomicCounterBufferiv(
                        prog, i, GL_ATOMIC_COUNTER_BUFFER_DATA_SIZE,
                        &data_size);
                if (!piglit_check_gl_error(GL_NO_ERROR) ||
                    data_size < binfo->min_reasonable_size) {
                        fprintf(stderr, "Invalid buffer data size: %d,"
                               " expected at least: %d.\n", data_size,
                               binfo->min_reasonable_size);
                        piglit_report_result(PIGLIT_FAIL);
                }

                glGetActiveAtomicCounterBufferiv(
                        prog, i, GL_ATOMIC_COUNTER_BUFFER_ACTIVE_ATOMIC_COUNTERS,
                        &num_counters);
                if (!piglit_check_gl_error(GL_NO_ERROR) ||
                    num_counters != binfo->num_counters) {
                        fprintf(stderr, "Invalid number of atomic counters: %d,"
                               " expected: %d.\n", num_counters,
                               binfo->num_counters);
                        piglit_report_result(PIGLIT_FAIL);
                }

                if (visited_buffers[i]) {
                        fprintf(stderr, "Buffer at binding point %d seen twice."
                                "\n", binding);
                        piglit_report_result(PIGLIT_FAIL);
                }
                visited_buffers[i] = true;

                glGetActiveAtomicCounterBufferiv(prog, i,
                        GL_ATOMIC_COUNTER_BUFFER_REFERENCED_BY_VERTEX_SHADER,
                        &ref);
                if (!piglit_check_gl_error(GL_NO_ERROR) || ref) {
                        fprintf(stderr, "Buffer incorrectly reported to be "
                                "referenced by vertex shader.\n");
                        piglit_report_result(PIGLIT_FAIL);
                }

                glGetActiveAtomicCounterBufferiv(prog, i,
                        GL_ATOMIC_COUNTER_BUFFER_REFERENCED_BY_TESS_CONTROL_SHADER,
                        &ref);
                if (!piglit_check_gl_error(GL_NO_ERROR) || ref) {
                        fprintf(stderr, "Buffer incorrectly reported to be "
                                "referenced by tessellation control shader.\n");
                        piglit_report_result(PIGLIT_FAIL);
                }

                glGetActiveAtomicCounterBufferiv(prog, i,
                        GL_ATOMIC_COUNTER_BUFFER_REFERENCED_BY_TESS_EVALUATION_SHADER,
                        &ref);
                if (!piglit_check_gl_error(GL_NO_ERROR) || ref) {
                        fprintf(stderr, "Buffer incorrectly reported to be "
                                "referenced by tessellation evaluation shader."
                                "\n");
                        piglit_report_result(PIGLIT_FAIL);
                }

                glGetActiveAtomicCounterBufferiv(prog, i,
                        GL_ATOMIC_COUNTER_BUFFER_REFERENCED_BY_GEOMETRY_SHADER,
                        &ref);
                if (!piglit_check_gl_error(GL_NO_ERROR) || ref) {
                        fprintf(stderr, "Buffer incorrectly reported to be "
                                "referenced by geometry shader.\n");
                        piglit_report_result(PIGLIT_FAIL);
                }

                glGetActiveAtomicCounterBufferiv(prog, i,
                        GL_ATOMIC_COUNTER_BUFFER_REFERENCED_BY_FRAGMENT_SHADER,
                        &ref);
                if (!piglit_check_gl_error(GL_NO_ERROR) || !ref) {
                        fprintf(stderr, "Buffer incorrectly reported as "
                                "unreferenced from the fragment shader.\n");
                        piglit_report_result(PIGLIT_FAIL);
                }

                glGetActiveAtomicCounterBufferiv(prog, i,
                        GL_ATOMIC_COUNTER_BUFFER_ACTIVE_ATOMIC_COUNTER_INDICES,
                        (GLint *)counters);
                if (!piglit_check_gl_error(GL_NO_ERROR)) {
                        fprintf(stderr, "Couldn't obtain list of active atomic "
                                "counters for buffer at binding point %d.\n",
                                binding);
                        piglit_report_result(PIGLIT_FAIL);
                }

                for (j = 0; j < num_counters; ++j) {
                        const struct counter_info *cinfo;
                        int unif_type, unif_size, unif_name_len,
                                unif_block_idx, unif_offset, unif_stride,
                                unif_buffer_idx;
                        char unif_name[8];

                        glGetActiveUniformName(prog, counters[j],
                                               sizeof(unif_name), NULL,
                                               unif_name);

                        cinfo = expected_counter_info(unif_name);
                        if (!piglit_check_gl_error(GL_NO_ERROR) || !cinfo) {
                                fprintf(stderr, "Unknown atomic counter \"%s\"."
                                        "\n", unif_name);
                                piglit_report_result(PIGLIT_FAIL);
                        }

                        glGetActiveUniformsiv(prog, 1, &counters[j],
                                              GL_UNIFORM_TYPE, &unif_type);
                        if (!piglit_check_gl_error(GL_NO_ERROR) ||
                            unif_type != GL_UNSIGNED_INT_ATOMIC_COUNTER) {
                                fprintf(stderr, "Atomic counter \"%s\" has "
                                        "invalid type 0x%x, expected 0x%x.\n",
                                        unif_name, unif_type,
                                        GL_UNSIGNED_INT_ATOMIC_COUNTER);
                                piglit_report_result(PIGLIT_FAIL);
                        }

                        glGetActiveUniformsiv(prog, 1, &counters[j],
                                              GL_UNIFORM_SIZE, &unif_size);
                        if (!piglit_check_gl_error(GL_NO_ERROR) ||
                            unif_size != cinfo->size) {
                                fprintf(stderr, "Atomic counter \"%s\" has "
                                        "invalid size %d, expected: %d.\n",
                                        unif_name, unif_size, cinfo->size);
                                piglit_report_result(PIGLIT_FAIL);
                        }

                        glGetActiveUniformsiv(prog, 1, &counters[j],
                                              GL_UNIFORM_NAME_LENGTH, &unif_name_len);
                        if (!piglit_check_gl_error(GL_NO_ERROR) ||
                            unif_name_len != strlen(unif_name) + 1) {
                                fprintf(stderr, "Atomic counter \"%s\" has "
                                        "invalid name length %d, expected: %d."
                                        "\n", unif_name, unif_name_len,
                                        (int)strlen(unif_name) + 1);
                                piglit_report_result(PIGLIT_FAIL);
                        }

                        glGetActiveUniformsiv(prog, 1, &counters[j],
                                              GL_UNIFORM_BLOCK_INDEX, &unif_block_idx);
                        if (!piglit_check_gl_error(GL_NO_ERROR) ||
                            unif_block_idx != -1) {
                                fprintf(stderr, "Atomic counter \"%s\" has "
                                        "invalid block index %d, expected: -1."
                                        "\n", unif_name, unif_block_idx);
                                piglit_report_result(PIGLIT_FAIL);
                        }

                        glGetActiveUniformsiv(prog, 1, &counters[j],
                                              GL_UNIFORM_OFFSET, &unif_offset);
                        if (!piglit_check_gl_error(GL_NO_ERROR) ||
                            unif_offset != cinfo->offset) {
                                fprintf(stderr, "Atomic counter \"%s\" has "
                                        "invalid offset %d, expected: %d.\n",
                                        unif_name, unif_offset, cinfo->offset);
                                piglit_report_result(PIGLIT_FAIL);
                        }

                        glGetActiveUniformsiv(prog, 1, &counters[j],
                                              GL_UNIFORM_ARRAY_STRIDE, &unif_stride);
                        if (!piglit_check_gl_error(GL_NO_ERROR) ||
                            (cinfo->size > 1 && unif_stride < 4) ||
                            (cinfo->size == 1 && unif_stride != 0)) {
                                fprintf(stderr, "Atomic counter \"%s\" has "
                                        "invalid array stride %d.\n",
                                        unif_name, unif_stride);
                                piglit_report_result(PIGLIT_FAIL);
                        }

                        glGetActiveUniformsiv(prog, 1, &counters[j],
                                              GL_UNIFORM_ATOMIC_COUNTER_BUFFER_INDEX,
                                              &unif_buffer_idx);
                        if (!piglit_check_gl_error(GL_NO_ERROR) ||
                            unif_buffer_idx != i) {
                                fprintf(stderr, "Atomic counter \"%s\" has "
                                        "invalid buffer index %d, expected %d."
                                        "\n", unif_name, unif_buffer_idx, i);
                                piglit_report_result(PIGLIT_FAIL);
                        }

                        if (cinfo->binding != binding) {
                                fprintf(stderr, "Atomic counter \"%s\" belongs "
                                        "to the wrong binding point %d.\n",
                                        unif_name, binding);
                                piglit_report_result(PIGLIT_FAIL);
                        }

                        if (visited_counters[counters[j]]) {
                                fprintf(stderr, "Atomic counter \"%s\" seen "
                                        "twice.\n", unif_name);
                                piglit_report_result(PIGLIT_FAIL);
                        }
                        visited_counters[counters[j]] = true;
                }
        }

        glDeleteProgram(prog);

        piglit_report_result(PIGLIT_PASS);
}

enum piglit_result
piglit_display(void)
{
        return PIGLIT_PASS;
}
