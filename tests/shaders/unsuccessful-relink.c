/*
 * Copyright © 2017 Intel Corporation
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

/**
 * \file unsuccessful-relink.c
 *
 * Render using a program with a uniform. Modify the uniform and then
 * do a relink that will fail. This shouldn’t affect the original
 * program and it should render with the new uniform value.
 *
 * GLSL 4.6 spec section 7.3:
 *
 * “If a program object that is active for any shader stage is
 *  re-linked unsuccessfully, the link status will be set to FALSE,
 *  but any existing executables and associated state will remain part
 *  of the current rendering state until a subsequent call to
 *  UseProgram, UseProgramStages, or BindProgramPipeline removes them
 *  from use.”
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

        config.supports_gl_compat_version = 20;
        config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

static const char vs_source[] =
        "attribute vec4 piglit_vertex;\n"
        "void main()\n"
        "{\n"
        "       gl_Position = piglit_vertex;\n"
        "}\n";

static const char fs_source[] =
        "uniform vec4 color;\n"
        "void main()\n"
        "{\n"
        "       gl_FragColor = color;\n"
        "}\n";

static const float green[] = { 0.0f, 1.0f, 0.0f, 1.0f };
static const float purple[] = { 0.5f, 0.0f, 0.5f, 1.0f };

static bool
try_render(GLuint vertex_attrib,
           const float *color)
{
        struct vertex { float x, y; };
        static const struct vertex verts[] = {
                { -1, -1 }, { 1, -1 },
                { -1, 1 }, { 1, 1 }
        };
        bool pass;
        GLuint buf;

        /* This isn’t using piglit_draw_rect because that tries to
         * call glGetAttribLocation which won’t work on the unlinked
         * program
         */

        glGenBuffers(1, &buf);
        glBindBuffer(GL_ARRAY_BUFFER, buf);
        glBufferData(GL_ARRAY_BUFFER, sizeof verts, verts, GL_STATIC_DRAW);
        glVertexAttribPointer(vertex_attrib, 2, GL_FLOAT,
                              GL_FALSE, sizeof verts[0],
                              (void *) offsetof(struct vertex, x));
        glEnableVertexAttribArray(vertex_attrib);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glDeleteBuffers(1, &buf);

        if (!piglit_check_gl_error(GL_NO_ERROR)) {
                fprintf(stderr, "error while drawing\n");
                piglit_report_result(PIGLIT_FAIL);
        }

        pass = piglit_probe_rect_rgb(0, 0,
                                     piglit_width, piglit_height,
                                     color);
        if (!pass)
                fprintf(stderr, "render failed\n");

        return pass;
}

static void
unsuccessful_link(GLuint prog)
{
        GLuint shader;
        GLint status;
        int i;

        /* Add the fs shader again. This should cause a link error
         * because there would be two main functions in the fragment
         * stage. */
        for (i = 0; i < 2; i++) {
                shader = piglit_compile_shader_text(GL_FRAGMENT_SHADER,
                                                    fs_source);
                glAttachShader(prog, shader);
        }

        glLinkProgram(prog);

        glGetProgramiv(prog, GL_LINK_STATUS, &status);

        if (status) {
                fprintf(stderr, "Broken shader unexpectedly linked\n");
                piglit_report_result(PIGLIT_FAIL);
        }
}

enum piglit_result
piglit_display(void)
{
        bool pass = true;
        GLuint prog;
        GLint color_uniform;
        GLint vertex_attrib;
        void *dummy_data[2048];
        int i;

        glViewport(0, 0, piglit_width, piglit_height);
        glClear(GL_COLOR_BUFFER_BIT);

        prog = piglit_build_simple_program(vs_source, fs_source);

        color_uniform = glGetUniformLocation(prog, "color");

        if (color_uniform == -1) {
                fprintf(stderr, "color uniform missing\n");
                piglit_report_result(PIGLIT_FAIL);
        }

        vertex_attrib = glGetAttribLocation(prog, "piglit_vertex");

        if (vertex_attrib == -1) {
                fprintf(stderr, "piglit_vertex attrib missing\n");
                piglit_report_result(PIGLIT_FAIL);
        }

        glUseProgram(prog);

        glUniform4fv(color_uniform, 1, purple);

        if (!try_render(vertex_attrib, purple))
                pass = false;

        glUniform4fv(color_uniform, 1, green);

        unsuccessful_link(prog);

        /* This test currently causes a use-after-free error in Mesa
         * which causes sporadic failures. In order to increase the
         * chances of making the test fail, we will do lots of little
         * redundant allocations in the hope of overwriting the data
         * previously allocated and freed by Mesa.
         */
        for (i = 0; i < ARRAY_SIZE(dummy_data); i++) {
                dummy_data[i] = malloc(64);
                memset(dummy_data[i], 0, 64);
        }
        for (i = 0; i < ARRAY_SIZE(dummy_data); i++)
                free(dummy_data[i]);

        if (!try_render(vertex_attrib, green))
                pass = false;

        piglit_present_results();

        glDeleteProgram(prog);

        return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
        piglit_require_gl_version(20);
        piglit_require_GLSL_version(110);
}
