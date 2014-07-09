/*
 * Copyright © 2011 Dave Airlie <airlied@redhat.com>
 * Based on draw-vertices © Marek Olšák <maraeo@gmail.com>
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
 *     Dave Airlie <airlied@redhat.com>
 */

/* this test does some basic tests of ARB_vertex_type_2_10_10_10 vbos */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_width = 320;
	config.window_height = 60;
	config.window_visual = PIGLIT_GL_VISUAL_RGBA |
			       PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

#define i32to10(x) ((x) >= 0 ? (x & 0x1ff) : 1024-(abs((x))& 0x1ff))
#define i32to2(x) ((x) >= 0 ? (x & 0x1) : 1-abs((x)))

static unsigned iconv(int x, int y, int z, int w)
{
	unsigned val;

	val = i32to10(x);
	val |= i32to10(y) << 10;
	val |= i32to10(z) << 20;
	val |= i32to2(w) << 30;
	return val;
}
#define conv(x,y,z,w) (((unsigned)(x) & 0x3ff) | ((unsigned)(y) & 0x3ff) << 10 | ((unsigned)(z) & 0x3ff)<< 20 | ((unsigned)(w) & 0x3) << 30)

void piglit_init(int argc, char **argv)
{
    piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);

    piglit_require_gl_version(15);

    piglit_require_extension("GL_ARB_vertex_type_2_10_10_10_rev");
    glShadeModel(GL_FLAT);
    glClearColor(0.2, 0.2, 0.2, 1.0);
}

static GLuint vboVertexPointer(GLint size, GLenum type, GLsizei stride,
                               const GLvoid *buf, GLsizei bufSize, intptr_t bufOffset)
{
    GLuint id;
    glGenBuffers(1, &id);
    glBindBuffer(GL_ARRAY_BUFFER, id);
    glBufferData(GL_ARRAY_BUFFER, bufSize, buf, GL_STATIC_DRAW);
    glVertexPointer(size, type, stride, (void*)bufOffset);
    return id;
}

static GLuint vboColorPointer(GLint size, GLenum type, GLsizei stride,
                              const GLvoid *buf, GLsizei bufSize, intptr_t bufOffset)
{
    GLuint id;
    glGenBuffers(1, &id);
    glBindBuffer(GL_ARRAY_BUFFER, id);
    glBufferData(GL_ARRAY_BUFFER, bufSize, buf, GL_STATIC_DRAW);
    glColorPointer(size, type, stride, (void*)bufOffset);
    return id;
}

static void test_packed_int_color_vertices(float x1, float y1, float x2, float y2, int index)
{
    unsigned int v[3];
    unsigned int c[3];
    GLuint vbo;

    v[0] = iconv(x1, y1, 0, 1);
    v[1] = iconv(x1, y2, 0, 1);
    v[2] = iconv(x2, y1, 0, 1);

    if (index == 0 || index == 2) {
	c[0] = iconv(511, 0, 0, 0);
	c[1] = iconv(511, 0, 0, 0);
	c[2] = iconv(511, 0, 0, 0);
    } else {
	c[0] = conv(1023, 0, 0, 0);
	c[1] = conv(1023, 0, 0, 0);
	c[2] = conv(1023, 0, 0, 0);
    }

    glVertexPointer(4, GL_INT_2_10_10_10_REV, 4, v);

    glEnableClientState(GL_COLOR_ARRAY);
    switch (index) {
        case 0: vbo = vboColorPointer(4, GL_INT_2_10_10_10_REV, 4, c, sizeof(c), 0); break;
        case 1: vbo = vboColorPointer(4, GL_UNSIGNED_INT_2_10_10_10_REV, 4, c, sizeof(c), 0); break;
        case 2: vbo = vboColorPointer(GL_BGRA, GL_INT_2_10_10_10_REV, 4, c, sizeof(c), 0); break;
        case 3: vbo = vboColorPointer(GL_BGRA, GL_UNSIGNED_INT_2_10_10_10_REV, 4, c, sizeof(c), 0); break;
     }

    glDrawArrays(GL_TRIANGLES, 0, 3);

    glDisableClientState(GL_COLOR_ARRAY);
    glDeleteBuffers(1, &vbo);
}

static void test_packed_int_vertices(float x1, float y1, float x2, float y2, int index)
{
    unsigned int v[3];
    GLuint vbo;

    if (index == 0) {
	v[0] = iconv(x1, y1, 0, 1);
	v[1] = iconv(x1, y2, 0, 1);
	v[2] = iconv(x2, y1, 0, 1);
    } else {
	v[0] = conv(x1, y1, 0, 1);
	v[1] = conv(x1, y2, 0, 1);
	v[2] = conv(x2, y1, 0, 1);
    }

    switch (index) {
        case 0: vbo = vboVertexPointer(4, GL_INT_2_10_10_10_REV, 4, v, sizeof(v), 0); break;
        case 1: vbo = vboVertexPointer(4, GL_UNSIGNED_INT_2_10_10_10_REV, 4, v, sizeof(v), 0); break;
    }

    glDrawArrays(GL_TRIANGLES, 0, 3);

    glDeleteBuffers(1, &vbo);
}

static void test_int_vertices_abi(float x1, float y1, float x2, float y2, int index)
{
    GLuint v[3];
    GLuint c[3];
    int i, type;

    glDisableClientState(GL_VERTEX_ARRAY);

    if (index == 0) {
	v[0] = iconv(x1, y1, 0, 1);
	v[1] = iconv(x1, y2, 0, 1);
	v[2] = iconv(x2, y1, 0, 1);
	type = GL_INT_2_10_10_10_REV;
    } else {
	v[0] = conv(x1, y1, 0, 1);
	v[1] = conv(x1, y2, 0, 1);
	v[2] = conv(x2, y1, 0, 1);
	type = GL_UNSIGNED_INT_2_10_10_10_REV;
    }

    c[0] = iconv(511, 0, 0, 0);
    c[1] = iconv(511, 0, 0, 0);
    c[2] = iconv(511, 0, 0, 0);

    glBegin(GL_TRIANGLES);
    for (i = 0; i < 3; i++) {
	glColorP3ui(GL_INT_2_10_10_10_REV, c[i]);
	glVertexP3ui(type, v[i]);
    }
    glEnd();

    glEnableClientState(GL_VERTEX_ARRAY);
}

struct test {
    void (*test)(float x1, float y1, float x2, float y2, int index);
    int index;
    float expected_color[4];
    const char *name;
};

struct test tests[] = {
    {test_packed_int_vertices, 0, {1, 1, 1, 1}, "Int vertices - 2/10/10/10"},
    {test_packed_int_vertices, 1, {1, 1, 1, 1}, "Unsigned Int vertices - 2/10/10/10"},
    {test_packed_int_color_vertices, 0, {1, 0, 0, 0.333}, "Int Color - 2/10/10/10"},
    {test_packed_int_color_vertices, 1, {1, 0, 0, 0}, "Unsigned Int Color - 2/10/10/10"},
    {test_packed_int_color_vertices, 2, {0, 0, 1, 0.333}, "Int BGRA Color - 2/10/10/10"},
    {test_packed_int_color_vertices, 3, {0, 0, 1, 0}, "Unsigned Int BGRA Color - 2/10/10/10"},

    {test_int_vertices_abi, 0, {1, 0, 0, 1}, "Int 2/10/10/10 - test ABI" },
    {test_int_vertices_abi, 1, {1, 0, 0, 1}, "Unsigned 2/10/10/10 - test ABI" },
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
    glColor4f(1,1,1,1);

    for (i = 0; tests[i].test; i++) {
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        printf("%s\n", tests[i].name);
        tests[i].test(x, y, x+20, y+20, tests[i].index);
        assert(glGetError() == 0);
        pass = piglit_probe_pixel_rgba(x+5, y+5, tests[i].expected_color) && pass;

        x += 20;
        if (x > 300) {
            x = 0;
            y += 20;
        }
    }

    glFinish();
    piglit_present_results();

    return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}
