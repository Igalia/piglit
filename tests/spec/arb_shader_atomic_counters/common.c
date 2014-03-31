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

#include "common.h"

bool
atomic_counters_probe_buffer(unsigned base, unsigned count,
                             const uint32_t *expected)
{
        uint32_t *p = glMapBufferRange(
                GL_ATOMIC_COUNTER_BUFFER, base * sizeof(uint32_t),
                count * sizeof(uint32_t), GL_MAP_READ_BIT);
        unsigned i;

        if (!p) {
                printf("Couldn't map atomic counter buffer for read-back.\n");
                return false;
        }

        for (i = 0; i < count; ++i) {
                if (p[i] != expected[i]) {
                        printf("Probe value at (%i)\n", i);
                        printf("  Expected: 0x%08x\n", expected[i]);
                        printf("  Observed: 0x%08x\n", p[i]);
                        glUnmapBuffer(GL_ATOMIC_COUNTER_BUFFER);
                        return false;
                }
        }

        glUnmapBuffer(GL_ATOMIC_COUNTER_BUFFER);
        return true;
}

bool
atomic_counters_compile(GLuint prog, GLuint stage, const char *src)
{
        GLuint shader = glCreateShader(stage);
        int status, log_size;
        char *log;

        glShaderSource(shader, 1, (const GLchar **)&src, NULL);
        glCompileShader(shader);

        glGetShaderiv(shader, GL_COMPILE_STATUS, &status);

        if (status) {
                glAttachShader(prog, shader);
        } else {
                glGetShaderiv(prog, GL_INFO_LOG_LENGTH, &log_size);
                log = malloc(log_size);
                glGetShaderInfoLog(prog, log_size, NULL, log);

                printf("Failed to compile shader: %s\n", log);
                printf("source:\n%s", src);

                free(log);
        }

        glDeleteShader(shader);

        return status;
}

bool
atomic_counters_link(GLuint prog)
{
        int status;

        glGetProgramiv(prog, GL_LINK_STATUS, &status);
        if (!status) {
           glLinkProgram(prog);
           glGetProgramiv(prog, GL_LINK_STATUS, &status);
        }

        return status;
}

bool
atomic_counters_draw_point(GLuint prog, unsigned buf_size,
                           const uint32_t *buf)
{
        GLuint vao;

        /* Initialize the atomic counter buffer. */
        glBufferData(GL_ATOMIC_COUNTER_BUFFER,
                     buf_size * sizeof(uint32_t),
                     buf, GL_STATIC_DRAW);

        /* Link and set the current shader program. */
        atomic_counters_link(prog);
        glUseProgram(prog);

        /* Draw. */
        glClearColor(0.5, 0.5, 0.5, 0.5);
        glClear(GL_COLOR_BUFFER_BIT);

        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);
        glVertexAttrib4f(0, 0, 0, 0, 1);

        glDrawArrays(GL_POINTS, 0, 1);

        glDeleteVertexArrays(1, &vao);

        return piglit_check_gl_error(GL_NO_ERROR);
}

bool
atomic_counters_draw_rect(GLuint prog, unsigned buf_size, const uint32_t *buf)
{
        /* Initialize the atomic counter buffer. */
        glBufferData(GL_ATOMIC_COUNTER_BUFFER,
                     buf_size * sizeof(uint32_t),
                     buf, GL_STATIC_DRAW);

        /* Set current shader program. */
        glLinkProgram(prog);
        glUseProgram(prog);

        /* Draw. */
        glClearColor(0.5, 0.5, 0.5, 0.5);
        glClear(GL_COLOR_BUFFER_BIT);

        piglit_draw_rect(-1, -1, 2, 2);

        return piglit_check_gl_error(GL_NO_ERROR);
}

bool
atomic_counters_draw_patch(GLuint prog, unsigned buf_size,
                           const uint32_t *buf)
{

        const GLfloat verts[3][4] = { { 0.0, 0.0, 0.0, 1.0 },
                                      { 1.0, 0.0, 0.0, 1.0 },
                                      { 0.0, 1.0, 0.0, 1.0 } };
        GLuint vao, vbo;

        /* Initialize the atomic counter buffer. */
        glBufferData(GL_ATOMIC_COUNTER_BUFFER,
                     buf_size * sizeof(uint32_t),
                     buf, GL_STATIC_DRAW);

        /* Set current shader program. */
        glLinkProgram(prog);
        glUseProgram(prog);

        /* Initialize a vertex array object and a vertex buffer object. */
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);

        /* Set up the current vertex attributes. */
        glVertexAttribPointer(PIGLIT_ATTRIB_POS, 4, GL_FLOAT,
                              GL_FALSE, 0, 0);
        glEnableVertexAttribArray(PIGLIT_ATTRIB_POS);

        /* Draw. */
        glClearColor(0.5, 0.5, 0.5, 0.5);
        glClear(GL_COLOR_BUFFER_BIT);

        glDrawArrays(GL_PATCHES, 0, 3);

        /* Clean up. */
        glDisableVertexAttribArray(PIGLIT_ATTRIB_POS);
        glDeleteBuffers(1, &vbo);
        glDeleteVertexArrays(1, &vao);

        return piglit_check_gl_error(GL_NO_ERROR);
}

bool
atomic_counters_supported(GLenum shader_stage)
{
        int n = 0;

        switch (shader_stage) {
        case GL_NONE:
        case GL_FRAGMENT_SHADER:
                return true;

        case GL_VERTEX_SHADER:
                glGetIntegerv(GL_MAX_VERTEX_ATOMIC_COUNTERS, &n);
                return n;

        case GL_GEOMETRY_SHADER:
                if (piglit_is_extension_supported("GL_ARB_geometry_shader4"))
                        glGetIntegerv(GL_MAX_GEOMETRY_ATOMIC_COUNTERS, &n);
                return n;

        case GL_TESS_CONTROL_SHADER:
                if (piglit_is_extension_supported("GL_ARB_tesselation_shader"))
                        glGetIntegerv(GL_MAX_TESS_CONTROL_ATOMIC_COUNTERS, &n);
                return n;

        case GL_TESS_EVALUATION_SHADER:
                if (piglit_is_extension_supported("GL_ARB_tesselation_shader"))
                        glGetIntegerv(GL_MAX_TESS_EVALUATION_ATOMIC_COUNTERS,
                                      &n);
                return n;

        default:
                assert(!"Unreachable");
        }
}

struct atomic_counters_limits
atomic_counters_get_limits()
{
        struct atomic_counters_limits ls = { 0 };

        piglit_require_extension("GL_ARB_shader_atomic_counters");

        glGetIntegerv(GL_MAX_FRAGMENT_ATOMIC_COUNTERS,
                      &ls.fragment_counters);
        glGetIntegerv(GL_MAX_VERTEX_ATOMIC_COUNTERS,
                      &ls.vertex_counters);
        glGetIntegerv(GL_MAX_COMBINED_ATOMIC_COUNTERS,
                      &ls.combined_counters);
        glGetIntegerv(GL_MAX_FRAGMENT_ATOMIC_COUNTER_BUFFERS,
                      &ls.fragment_buffers);
        glGetIntegerv(GL_MAX_VERTEX_ATOMIC_COUNTER_BUFFERS,
                      &ls.vertex_buffers);
        glGetIntegerv(GL_MAX_COMBINED_ATOMIC_COUNTER_BUFFERS,
                      &ls.combined_buffers);
        glGetIntegerv(GL_MAX_ATOMIC_COUNTER_BUFFER_BINDINGS,
                      &ls.bindings);
        glGetIntegerv(GL_MAX_FRAGMENT_UNIFORM_COMPONENTS,
                      &ls.uniform_components);

        return ls;
}

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
char *
atomic_counters_generate_source(const char *src_template,
                                const char *decl_template,
                                const char *insn_template, unsigned n)
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

