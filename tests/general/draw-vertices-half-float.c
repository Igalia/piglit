/*
 * Copyright © 2010 Marek Olšák <maraeo@gmail.com>
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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Authors:
 *    Marek Olšák <maraeo@gmail.com>
 */

/* The test for some tricky bits of the OpenGL vertex submission.
 * The emphasis is taken on non-dword-aligned strides and offsets.
 * This one is for half float vertices.
 */

#include "piglit-util.h"

int piglit_width = 320, piglit_height = 60;
int piglit_window_mode = GLUT_RGB | GLUT_DOUBLE;

GLboolean user_va = GL_FALSE;

void piglit_init(int argc, char **argv)
{
    unsigned i;

    for (i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "user")) {
            user_va = GL_TRUE;
            puts("Testing user vertex arrays.");
        }
    }

    piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);

    if (!GLEW_VERSION_1_5) {
        printf("Requires OpenGL 1.5\n");
        piglit_report_result(PIGLIT_SKIP);
    }
    piglit_require_extension("GL_ARB_half_float_vertex");

    glShadeModel(GL_FLAT);
    glClearColor(0.2, 0.2, 0.2, 1.0);
}

static GLuint vboVertexPointer(GLint size, GLenum type, GLsizei stride,
                               const GLvoid *buf, GLsizei bufSize, intptr_t bufOffset)
{
    GLuint id;
    if (user_va) {
        glVertexPointer(size, type, stride, (char*)buf + bufOffset);
        return 0;
    }
    glGenBuffers(1, &id);
    glBindBuffer(GL_ARRAY_BUFFER, id);
    glBufferData(GL_ARRAY_BUFFER, bufSize, buf, GL_STATIC_DRAW);
    glVertexPointer(size, type, stride, (void*)bufOffset);
    return id;
}

typedef union { GLfloat f; GLint i; } fi_type;

/**
 * Convert a 4-byte float to a 2-byte half float.
 * Based on code from:
 * http://www.opengl.org/discussion_boards/ubb/Forum3/HTML/008786.html
 *
 * Taken over from Mesa.
 */
static unsigned short half(float val)
{
   const fi_type fi = {val};
   const int flt_m = fi.i & 0x7fffff;
   const int flt_e = (fi.i >> 23) & 0xff;
   const int flt_s = (fi.i >> 31) & 0x1;
   int s, e, m = 0;
   unsigned short result;

   /* sign bit */
   s = flt_s;

   /* handle special cases */
   if ((flt_e == 0) && (flt_m == 0)) {
      /* zero */
      /* m = 0; - already set */
      e = 0;
   }
   else if ((flt_e == 0) && (flt_m != 0)) {
      /* denorm -- denorm float maps to 0 half */
      /* m = 0; - already set */
      e = 0;
   }
   else if ((flt_e == 0xff) && (flt_m == 0)) {
      /* infinity */
      /* m = 0; - already set */
      e = 31;
   }
   else if ((flt_e == 0xff) && (flt_m != 0)) {
      /* NaN */
      m = 1;
      e = 31;
   }
   else {
      /* regular number */
      const int new_exp = flt_e - 127;
      if (new_exp < -24) {
         /* this maps to 0 */
         /* m = 0; - already set */
         e = 0;
      }
      else if (new_exp < -14) {
         /* this maps to a denorm */
         unsigned int exp_val = (unsigned int) (-14 - new_exp); /* 2^-exp_val*/
         e = 0;
         switch (exp_val) {
            case 0:
               /* m = 0; - already set */
               break;
            case 1: m = 512 + (flt_m >> 14); break;
            case 2: m = 256 + (flt_m >> 15); break;
            case 3: m = 128 + (flt_m >> 16); break;
            case 4: m = 64 + (flt_m >> 17); break;
            case 5: m = 32 + (flt_m >> 18); break;
            case 6: m = 16 + (flt_m >> 19); break;
            case 7: m = 8 + (flt_m >> 20); break;
            case 8: m = 4 + (flt_m >> 21); break;
            case 9: m = 2 + (flt_m >> 22); break;
            case 10: m = 1; break;
         }
      }
      else if (new_exp > 15) {
         /* map this value to infinity */
         /* m = 0; - already set */
         e = 31;
      }
      else {
         /* regular */
         e = new_exp + 15;
         m = flt_m >> 13;
      }
   }

   result = (s << 15) | (e << 10) | m;
   return result;
}

static void test_half_vertices_wrapped(unsigned short x1, unsigned short y1,
                                       unsigned short x2, unsigned short y2,
                                       unsigned short one, int index)
{
    unsigned short v2[] = {
        x1, y1,
        x1, y2,
        x2, y1
    };
    unsigned short v3[] = {
        x1, y1, 0,
        x1, y2, 0,
        x2, y1, 0
    };
    unsigned short v4[] = {
        x1, y1, 0, one,
        x1, y2, 0, one,
        x2, y1, 0, one
    };
    unsigned short v2o[] = {
        0,
        x1, y1,
        x1, y2,
        x2, y1
    };
    unsigned short v3o[] = {
        0,
        x1, y1, 0,
        x1, y2, 0,
        x2, y1, 0
    };
    unsigned short v4o[] = {
        0,
        x1, y1, 0, one,
        x1, y2, 0, one,
        x2, y1, 0, one
    };
    GLuint vbo;

    switch (index) {
        case 0: vbo = vboVertexPointer(2, GL_HALF_FLOAT_ARB, 4, v2, sizeof(v2), 0); break;

        case 1: vbo = vboVertexPointer(2, GL_HALF_FLOAT_ARB, 6, v3, sizeof(v3), 0); break;
        case 2: vbo = vboVertexPointer(3, GL_HALF_FLOAT_ARB, 6, v3, sizeof(v3), 0); break;

        case 3: vbo = vboVertexPointer(2, GL_HALF_FLOAT_ARB, 8, v4, sizeof(v4), 0); break;
        case 4: vbo = vboVertexPointer(3, GL_HALF_FLOAT_ARB, 8, v4, sizeof(v4), 0); break;
        case 5: vbo = vboVertexPointer(4, GL_HALF_FLOAT_ARB, 8, v4, sizeof(v4), 0); break;

        case 6: vbo = vboVertexPointer(2, GL_HALF_FLOAT_ARB, 4, v2o, sizeof(v2o), 2); break;

        case 7: vbo = vboVertexPointer(2, GL_HALF_FLOAT_ARB, 6, v3o, sizeof(v3o), 2); break;
        case 8: vbo = vboVertexPointer(3, GL_HALF_FLOAT_ARB, 6, v3o, sizeof(v3o), 2); break;

        case 9: vbo = vboVertexPointer(2, GL_HALF_FLOAT_ARB, 8, v4o, sizeof(v4o), 2); break;
        case 10:vbo = vboVertexPointer(3, GL_HALF_FLOAT_ARB, 8, v4o, sizeof(v4o), 2); break;
        case 11:vbo = vboVertexPointer(4, GL_HALF_FLOAT_ARB, 8, v4o, sizeof(v4o), 2); break;

        default:vbo = 0; assert(0); break;
    }

    glDrawArrays(GL_TRIANGLES, 0, 3);

    if (vbo)
        glDeleteBuffers(1, &vbo);
}

static void test_half_vertices(float fx1, float fy1, float fx2, float fy2, int index)
{
    unsigned short x1, y1, x2, y2, one;
    x1 = half(fx1);
    y1 = half(fy1);
    x2 = half(fx2);
    y2 = half(fy2);
    one = half(1);

    test_half_vertices_wrapped(x1, y1, x2, y2, one, index);
}

struct test {
    void (*test)(float x1, float y1, float x2, float y2, int index);
    int index;
    float expected_color[3];
    const char *name;
};

struct test tests[] = {
    {test_half_vertices, 0, {1, 1, 1}, "Half vertices - components: 2, stride: 4, offset: 0"},
    {test_half_vertices, 1, {1, 1, 1}, "Half vertices - components: 2, stride: 6, offset: 0"},
    {test_half_vertices, 2, {1, 1, 1}, "Half vertices - components: 3, stride: 6, offset: 0"},
    {test_half_vertices, 3, {1, 1, 1}, "Half vertices - components: 2, stride: 8, offset: 0"},
    {test_half_vertices, 4, {1, 1, 1}, "Half vertices - components: 3, stride: 8, offset: 0"},
    {test_half_vertices, 5, {1, 1, 1}, "Half vertices - components: 4, stride: 8, offset: 0"},
    {test_half_vertices, 6, {1, 1, 1}, "Half vertices - components: 2, stride: 4, offset: 2"},
    {test_half_vertices, 7, {1, 1, 1}, "Half vertices - components: 2, stride: 6, offset: 2"},
    {test_half_vertices, 8, {1, 1, 1}, "Half vertices - components: 3, stride: 6, offset: 2"},
    {test_half_vertices, 9, {1, 1, 1}, "Half vertices - components: 2, stride: 8, offset: 2"},
    {test_half_vertices, 10, {1, 1, 1}, "Half vertices - components: 3, stride: 8, offset: 2"},
    {test_half_vertices, 11, {1, 1, 1}, "Half vertices - components: 4, stride: 8, offset: 2"},

    {0}
};

enum piglit_result
piglit_display(void)
{
    GLboolean pass = GL_TRUE;
    unsigned i;
    float x = 0, y = 0;

    glClear(GL_COLOR_BUFFER_BIT);
    glEnableClientState(GL_VERTEX_ARRAY);

    for (i = 0; tests[i].test; i++) {
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        printf("%s\n", tests[i].name);
        tests[i].test(x, y, x+20, y+20, tests[i].index);
        assert(glGetError() == 0);
        pass = piglit_probe_pixel_rgb(x+5, y+5, tests[i].expected_color) && pass;

        x += 20;
        if (x > 300) {
            x = 0;
            y += 20;
        }
    }

    glFinish();
    glutSwapBuffers();

    return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

