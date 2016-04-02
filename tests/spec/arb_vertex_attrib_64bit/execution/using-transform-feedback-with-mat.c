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
        "in mat%ix%i inValue;\n"
        "out mat%ix%i outValue;\n"
        "void main()\n"
        "{\n"
        "outValue = inValue;\n"
        "}\n";

char *vs_text;

GLint vert;
GLint prog;
GLuint vao;
GLuint vbo;
GLint inputAttrib;
GLuint tbo;
const char *varyings[] = { "outValue" };

unsigned NUM_COLUMNS = 0;
unsigned NUM_ROWS = 0;
unsigned NUM_ELEMENTS = 0;
unsigned NUM_SAMPLES = 5;

unsigned DATA_SIZE = 0;
unsigned ATTRIBUTE_SIZE = 0;

GLdouble LSB = 0.00000000010000111022302462516E0;

GLfloat *data;
GLfloat *feedback;
size_t stride = 0;

static void
init_shader()
{
        int result;
        result = asprintf(&vs_text, vs_text_template, NUM_COLUMNS, NUM_ROWS, NUM_COLUMNS, NUM_ROWS);
        if (result == -1)
                piglit_report_result(PIGLIT_FAIL);

        vert = piglit_compile_shader_text(GL_VERTEX_SHADER, vs_text);
        prog = glCreateProgram();
        glAttachShader(prog, vert);
        glTransformFeedbackVaryings(prog, 1, varyings, GL_INTERLEAVED_ATTRIBS);
        glLinkProgram(prog);
        glUseProgram(prog);
        inputAttrib= glGetAttribLocation(prog, "inValue");

        if (!piglit_link_check_status(prog))
                piglit_report_result(PIGLIT_FAIL);
}

static void
init_globals()
{
        ATTRIBUTE_SIZE = sizeof(GLfloat);
        NUM_ELEMENTS = NUM_ROWS*NUM_COLUMNS;
        stride = ATTRIBUTE_SIZE * NUM_ELEMENTS;
        DATA_SIZE = ATTRIBUTE_SIZE * NUM_ELEMENTS * NUM_SAMPLES;
}

static void
init_buffers(size_t *offset)
{
        unsigned row;
        unsigned column;
        unsigned sample;

        data = malloc(DATA_SIZE);
        feedback = malloc(DATA_SIZE);

        for (sample = 0; sample < NUM_SAMPLES; sample++)
                for (row = 0; row < NUM_ROWS; row++)
                        for (column = 0; column < NUM_COLUMNS; column++)
                                data[sample*NUM_ELEMENTS + column*NUM_ROWS + row] = column*NUM_ROWS + row + 1 + sample*10 + LSB;

        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, DATA_SIZE, data, GL_STATIC_DRAW);

        for (column = 0; column < NUM_COLUMNS; column++) {
                glEnableVertexAttribArray(inputAttrib + column);
                glVertexAttribPointer(inputAttrib + column, NUM_ROWS, GL_FLOAT, GL_FALSE, stride, (void*) *offset);
                *offset += NUM_ROWS * ATTRIBUTE_SIZE;
        }

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
        glDrawArrays(GL_POINTS, 0, NUM_SAMPLES);
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

        free(data);
        free(feedback);
}

static const char *double_to_hex(double d)
{
        union {
                double d;
                unsigned i[2];
        } b;

        b.d = d;
        char *s = (char *) malloc(100);
        sprintf(s, "0x%08X%08X", b.i[1], b.i[0]);
        return s;
}

static GLboolean
fetch_results()
{
        GLboolean result = GL_TRUE;
        unsigned index = 0;
        int i;
        int c;

        glGetBufferSubData(GL_TRANSFORM_FEEDBACK_BUFFER, 0, DATA_SIZE, feedback);

        for (c = 0; c < NUM_SAMPLES; c++) {
                printf("Sample %i\n", c);
                printf("*********************************\n");

                for (i = 0; i < NUM_ELEMENTS; i++) {
                        printf("%i - Original = %.14g[%s] Fetched = %.14g[%s]", i + 1,
                               data[index], double_to_hex(data[index]),
                               feedback[index], double_to_hex(feedback[index]));
                        if (data[index] == feedback[index])
                                printf("\tequal\n");
                        else
                                printf("\tWRONG\n");
                        result = result && (data[index] == feedback[index]);
                        index++;
                }
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
        if (argc != 4) {
                printf( "Usage: arb_vertex_attrib_64bit-using-transform-feedback-with-dmat num_columns num_rows num_samples\n");
                piglit_report_result(PIGLIT_FAIL);
        }
        NUM_COLUMNS = atoi(argv[1]);
        if ((NUM_COLUMNS < 2) || (NUM_COLUMNS > 4)) {
                printf("Wrong value for num_columns: %s\n", argv[1]);
                piglit_report_result(PIGLIT_FAIL);
        }

        NUM_ROWS = atoi(argv[2]);
        if ((NUM_ROWS < 2) || (NUM_ROWS > 4)) {
                printf("Wrong value for num_rows: %s\n", argv[2]);
                piglit_report_result(PIGLIT_FAIL);
        }

        NUM_SAMPLES = atoi(argv[3]);
        if (NUM_ROWS < 1) {
                printf("Wrong value for num_samples: %s\n", argv[2]);
                piglit_report_result(PIGLIT_FAIL);
        }

}

void piglit_init(int argc, char **argv)
{
        GLboolean ok = GL_TRUE;
        size_t offset = 0;

        piglit_require_GLSL_version(150);
        piglit_require_extension("GL_ARB_transform_feedback3");

        parse_args(argc, argv);

        init_globals();
        init_shader();
        init_buffers(&offset);
        render();
        ok = ok && fetch_results();
        clean();

        piglit_report_result(ok ? PIGLIT_PASS : PIGLIT_FAIL);
}

