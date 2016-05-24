/*
 * Copyright © 2010 Intel Corporation
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
 * \file using-transform-feedback.c
 *
 * Basic example that using transform feedback to get the value passed
 * to the vertex shader.
 *
 * Transform feedback can't get the input attribute, but varying
 * output. So we copy the value to a out variable.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_core_version = 33;

	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

const char *vs_text_template =
        "#version 150\n"
        "in %s inValue;\n"
        "out %s outValue;\n"
        "void main()\n"
        "{\n"
        "outValue = inValue;\n"
        "}\n";

GLint vert;
GLint prog;
GLuint vao;
GLuint vbo;
GLint inputAttrib;
GLuint tbo;
const char *varyings[] = { "outValue" };

#define DEFAULT_NUM_SAMPLES 5
unsigned num_samples = DEFAULT_NUM_SAMPLES;
unsigned num_components = 1;
unsigned DATA_SIZE = 0;
GLint *data = NULL;
GLint *feedback = NULL;

static void
init_shader()
{
        int result;
        char *vs_text;
        char *type_string;

        switch (num_components) {
        case 1:
                type_string = "int";
                break;
        case 2:
                type_string = "ivec2";
                break;
        case 3:
                type_string = "ivec3";
                break;
        case 4:
                type_string = "ivec4";
                break;
        }

        result = asprintf(&vs_text, vs_text_template, type_string, type_string);
        if (result == -1)
                piglit_report_result(PIGLIT_FAIL);

        vert = piglit_compile_shader_text(GL_VERTEX_SHADER, vs_text);
        prog = glCreateProgram();
        glAttachShader(prog, vert);
        glTransformFeedbackVaryings(prog, 1, varyings, GL_INTERLEAVED_ATTRIBS);
        glLinkProgram(prog);
        glUseProgram(prog);
        inputAttrib= glGetAttribLocation(prog, "inValue");
}

static void
init_buffers()
{
        unsigned i;

        DATA_SIZE = sizeof(GLint) * num_samples * num_components;
        data = malloc(DATA_SIZE);
        feedback = malloc(DATA_SIZE);

        for (i = 0; i < num_samples * num_components; i++) {
                data[i] = i + 1;
        }

        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, DATA_SIZE, data, GL_STATIC_DRAW);

        glEnableVertexAttribArray(inputAttrib);
        glVertexAttribIPointer(inputAttrib, num_components, GL_INT, 0, 0);

        glGenBuffers(1, &tbo);
        glBindBuffer(GL_ARRAY_BUFFER, tbo);
        glBufferData(GL_ARRAY_BUFFER, DATA_SIZE, NULL, GL_STATIC_READ);
}

static void
render()
{
        glEnable(GL_RASTERIZER_DISCARD);

        glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, tbo);

        glBeginTransformFeedback(GL_POINTS);
        glDrawArrays(GL_POINTS, 0, num_samples);
        glEndTransformFeedback();

        glDisable(GL_RASTERIZER_DISCARD);

        glFlush();
}

static void
clean()
{
        glDeleteProgram(prog);
        glDeleteShader(vert);

        glDeleteBuffers(1, &tbo);
        glDeleteBuffers(1, &vbo);

        glDeleteVertexArrays(1, &vao);

        if (data != NULL)
                free(data);

        if (feedback != NULL)
                free(feedback);
}

static void
print_component(int c)
{
        if (num_components > 1) {
                switch (c) {
                case 0:
                        printf("x: ");
                        break;
                case 1:
                        printf("y: ");
                        break;
                case 2:
                        printf("z: ");
                        break;
                case 3:
                        printf("w: ");
                                        break;
                default:
                        piglit_report_result(PIGLIT_FAIL);
                        break;
                }
        }
}

static GLboolean
fetch_results()
{
        GLboolean result = GL_TRUE;
        int i, c, index;

        glGetBufferSubData(GL_TRANSFORM_FEEDBACK_BUFFER, 0, DATA_SIZE, feedback);

        index = 0;
        for (i = 0; i < num_samples; i++) {
                printf("Sample %i:  ", i+1);
                if (num_components > 1) printf("\n*********************************\n");

                for (c = 0; c < num_components; c++) {
                        print_component(c);
                        printf("Original = %i Fetched = %i",
                               data[index], feedback[index]);
                        if (data[index] == feedback[index])
                                printf("\tequal\n");
                        else
                                printf("\tWRONG\n");

                        result = result && (data[index] == feedback[index]);
                        index++;
                }
                if (num_components > 1) printf("\n");
        }

        return result;
}

enum piglit_result
piglit_display(void)
{
        return PIGLIT_FAIL;
}

static void
parse_args(int argc, char **argv)
{
        int num;

        if (argc != 3) {
                printf( "Usage: arb_vertex_attrib_64bit-using-transform-feedback <num_components> <num_samples>\n");
                piglit_report_result(PIGLIT_FAIL);
        }

        num = atoi(argv[1]);
        if (num >= 1 && num <= 4) {
                num_components = num;
        } else {
                fprintf(stderr, "Wrong value for components: %s\n", argv[2]);
                piglit_report_result(PIGLIT_FAIL);

        }

        num = atoi(argv[2]);
        if (num > 0) {
                num_samples = num;
        } else {
                fprintf(stderr, "Wrong value for samples: %s\n", argv[2]);
                piglit_report_result(PIGLIT_FAIL);
        }
}

void piglit_init(int argc, char **argv)
{
        GLboolean ok = GL_TRUE;

        piglit_require_GLSL_version(150);
        piglit_require_extension("GL_ARB_transform_feedback3");

        parse_args(argc, argv);

        init_shader();
        ok = ok && piglit_link_check_status(prog);

        if (ok) {
                init_buffers();
                render();

                ok = ok && fetch_results();
        }

        clean();
        piglit_report_result(ok ? PIGLIT_PASS : PIGLIT_FAIL);
}

