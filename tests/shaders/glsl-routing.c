/*
 * Copyright © 2009 Marek Olšák (maraeo@gmail.com)
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
 *
 * Authors:
 *    Marek Olšák <mareao@gmail.com>
 *
 */

/** @file glsl-routing.c
 *
 * Tests whether streams are routed in this chain correctly:
 * vertex attributes -> vertex shader -> fragment shader -> output
 * with emphasis on linking vertex and fragment shaders
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_width = 260;
	config.window_height = 365;
	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

#define COLORS 2
#define TEXCOORDS 6
#define ATTRIBS (COLORS+TEXCOORDS)
#define BOX_SIZE 25

static char vs_code[] =
    "void main()\n"
    "{\n"
    "    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;\n"
    "AA  gl_FrontColor = gl_Color;\n"
    "BB  gl_FrontSecondaryColor = gl_SecondaryColor;\n"
    "CC  gl_TexCoord[0] = gl_MultiTexCoord0;\n"
    "DD  gl_TexCoord[1] = gl_MultiTexCoord1;\n"
    "EE  gl_TexCoord[2] = gl_MultiTexCoord2;\n"
    "FF  gl_TexCoord[3] = gl_MultiTexCoord3;\n"
    "GG  gl_TexCoord[4] = gl_MultiTexCoord4;\n"
    "HH  gl_TexCoord[5] = gl_MultiTexCoord5;\n"
    "}\n";

static char fs_code[] =
    "uniform float index;\n"
    "float eq(float a, float b)\n"
    "{\n"
    "    return float(abs(a - b) < 0.01);\n"
    "}\n"
    "void main()\n"
    "{\n"
    "    vec4 r = vec4(0.0);\n"
    "    int i = 1;\n"
    "AA  r += eq(index, float(i)) * gl_Color;\n"
    "    ++i;\n"
    "BB  r += eq(index, float(i)) * gl_SecondaryColor;\n"
    "    ++i;\n"
    "CC  r += eq(index, float(i)) * gl_TexCoord[0];\n"
    "    ++i;\n"
    "DD  r += eq(index, float(i)) * gl_TexCoord[1];\n"
    "    ++i;\n"
    "EE  r += eq(index, float(i)) * gl_TexCoord[2];\n"
    "    ++i;\n"
    "FF  r += eq(index, float(i)) * gl_TexCoord[3];\n"
    "    ++i;\n"
    "GG  r += eq(index, float(i)) * gl_TexCoord[4];\n"
    "    ++i;\n"
    "HH  r += eq(index, float(i)) * gl_TexCoord[5];\n"
    "    gl_FragColor = r;\n"
    "}\n";

static GLint setup_shaders(unsigned vsbitmask, unsigned fsbitmask)
{
    GLuint vs, fs, prog;
    unsigned i;
    char fscode[1024], vscode[1024], pattern[3] = {'A', 'A', '\0'};
    char *ptr;

    memcpy(vscode, vs_code, strlen(vs_code)+1);
    memcpy(fscode, fs_code, strlen(fs_code)+1);

    for (i = 0; i < ATTRIBS; i++) {
        ptr = strstr(vscode, pattern);
        assert(ptr);

        if ((1 << i) & vsbitmask) {
            memcpy(ptr, "  ", 2);
        } else {
            memcpy(ptr, "//", 2);
        }

        ptr = strstr(fscode, pattern);
        assert(ptr);

        if ((1 << i) & fsbitmask) {
            memcpy(ptr, "  ", 2);
        } else {
            memcpy(ptr, "//", 2);
        }

        ++pattern[0];
        ++pattern[1];
    }

    vs = piglit_compile_shader_text(GL_VERTEX_SHADER, vscode);
    fs = piglit_compile_shader_text(GL_FRAGMENT_SHADER, fscode);
    prog = piglit_link_simple_program(vs, fs);

    glUseProgram(prog);
    return prog;
}

static void draw_rect(float posX, float posY, int attribIndex,
                      float attrib[ATTRIBS][16])
{
    float pos[8] = {
        posX,          posY,
        posX,          posY+BOX_SIZE,
        posX+BOX_SIZE, posY+BOX_SIZE,
        posX+BOX_SIZE, posY
    };
    int i;

    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(2, GL_FLOAT, 0, pos);

    if (COLORS > 0) {
        glEnableClientState(GL_COLOR_ARRAY);
        glColorPointer(4, GL_FLOAT, 0, &attrib[0][0]);
    }
    if (COLORS > 1) {
        glEnableClientState(GL_SECONDARY_COLOR_ARRAY);
        glSecondaryColorPointer(3, GL_FLOAT, 4*sizeof(float), &attrib[1][0]);
    }

    for (i = 0; i < TEXCOORDS; i++) {
        glClientActiveTexture(GL_TEXTURE0 + i);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        glTexCoordPointer(4, GL_FLOAT, 0, &attrib[COLORS+i][0]);
    }

    glDrawArrays(GL_QUADS, 0, 4);

    glDisableClientState(GL_VERTEX_ARRAY);
    if (COLORS > 0) {
        glDisableClientState(GL_COLOR_ARRAY);
    }
    if (COLORS > 1) {
        glDisableClientState(GL_SECONDARY_COLOR_ARRAY);
    }
    for (i = 0; i < TEXCOORDS; i++) {
        glClientActiveTexture(GL_TEXTURE0 + i);
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    }
    glClientActiveTexture(GL_TEXTURE0);
}

static GLboolean test(unsigned vsbitmask, unsigned fsbitmask, int line)
{
    GLint prog, location;
    GLboolean pass = GL_TRUE;
    int i, j;
    float input[4*ATTRIBS] = {
        0.0, 0.1, 0.2, 0.3,
        0.1, 0.2, 0.3, 0.4,
        0.2, 0.3, 0.4, 0.5,
        0.3, 0.4, 0.5, 0.6,
        0.4, 0.5, 0.6, 0.7,
        0.5, 0.6, 0.7, 0.8,
        0.6, 0.7, 0.8, 0.9,
        0.7, 0.8, 0.9, 1.0
    };
    float attrib[ATTRIBS][16];
    float height = line * 30;

    prog = setup_shaders(vsbitmask, fsbitmask);
    location = glGetUniformLocation(prog, "index");

    for (i = 0; i < ATTRIBS; i++) {
        for (j = 0; j < 16; j++) {
            attrib[i][j] = input[i*4+(j%4)];
        }
    }

    for (i = 0; i < ATTRIBS; i++) {
        if (((1 << i) & vsbitmask) && ((1 << i) & fsbitmask)) {
            j = i+1;
            glUniform1f(location, j);
            draw_rect(5 + (i*(BOX_SIZE+5)) % 240, 5 + height, i, attrib);
        }
    }
    assert(glGetError() == 0);

    for (i = 0; i < ATTRIBS; i++) {
        if (((1 << i) & vsbitmask) && ((1 << i) & fsbitmask)) {
            pass = piglit_probe_pixel_rgb(7 + (i*(BOX_SIZE+5)), 7 + height, &input[i*4]) && pass;
        }
    }

    return pass;
}

#define C0 1
#define C1 2
#define T0 4
#define T1 8
#define T2 16
#define T3 32
#define T4 64
#define T5 128

static void printconf(unsigned b)
{
    printf("%s %s %s %s %s %s %s %s",
           (1  & b) ? "C0" : "--",
           (2  & b) ? "C1" : "--",
           (4  & b) ? "T0" : "--",
           (8  & b) ? "T1" : "--",
           (16 & b) ? "T2" : "--",
           (32 & b) ? "T3" : "--",
           (64 & b) ? "T4" : "--",
           (128 & b) ? "T5" : "--");
}

enum piglit_result
piglit_display(void)
{
    GLboolean pass = GL_TRUE;
    unsigned i;
    static unsigned conf[][2] = {
        // All VS outputs, some FS inputs
        {
            C0 | C1 | T0 | T1 | T2 | T3 | T4 | T5,
            C0 |      T0 |      T2 |      T4 | T5
        },
        {
            C0 | C1 | T0 | T1 | T2 | T3 | T4 | T5,
                 C1 | T0 | T1 |           T4
        },
        {
            C0 | C1 | T0 | T1 | T2 | T3 | T4 | T5,
            C0 | C1 |           T2 |      T4 | T5
        },
        {
            C0 | C1 | T0 | T1 | T2 | T3 | T4 | T5,
                 C1 |           T2 | T3
        },
        // Some VS outputs, all FS inputs
        {
            C0 |      T0 |      T2 |      T4 | T5,
            C0 | C1 | T0 | T1 | T2 | T3 | T4 | T5
        },
        {
                 C1 | T0 | T1 |           T4,
            C0 | C1 | T0 | T1 | T2 | T3 | T4 | T5
        },
        {
            C0 | C1 |                T3 |      T5,
            C0 | C1 | T0 | T1 | T2 | T3 | T4 | T5
        },
        {
                 C1 |           T2 | T3 | T4,
            C0 | C1 | T0 | T1 | T2 | T3 | T4 | T5
        },
        // Some VS outputs, some FS inputs
        {
            C0 |      T0 |      T2 |      T4 | T5,
            C0 | C1 | T0 | T1 |           T4
        },
        {
            C0 | C1 |      T1 |           T4 | T5,
                 C1 |      T1 | T2 | T3 | T4
        },
        {
                 C1 | T0 |      T2 | T3 |      T5,
                 C1 |      T1 | T2 | T3 | T4 | T5
        },
        {
            C0 | C1 | T0 |      T2 | T3 | T4 | T5,
            C0 |      T0 |           T3 |      T5
        },
    };

    glClearColor(0.5, 0.5, 0.5, 0.5);
    glClear(GL_COLOR_BUFFER_BIT);

    for (i = 0; i < sizeof(conf) / sizeof(*conf); i++) {
        printf("\nTest: VS(");
        printconf(conf[i][0]);
        printf(")\n      FS(");
        printconf(conf[i][1]);
        printf(")\n");

        pass = test(conf[i][0], conf[i][1], i) && pass;
    }

    piglit_present_results();

    return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void piglit_init(int argc, char **argv)
{
    piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);

    piglit_require_gl_version(20);
}

