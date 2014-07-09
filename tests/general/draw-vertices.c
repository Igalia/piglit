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
 * The emphasis is taken on non-dword-aligned strides and offsets,
 * but tests for various vertex formats are provided too.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_width = 320;
	config.window_height = 60;
	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

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

    piglit_require_gl_version(15);

    glShadeModel(GL_FLAT);
    glClearColor(0.2, 0.2, 0.2, 1.0);
}

static void test_large_vertex_count(float x1, float y1, float x2, float y2, int index)
{
    unsigned tris = 100000;
    float *v = (float*)malloc(sizeof(float) * 6 * tris);
    unsigned i;

    /* A large vertex count for DrawArrays */
    for (i = 0; i < tris*6; i += 6) {
        v[i+0] = x1;
        v[i+1] = y1;

        v[i+2] = x1;
        v[i+3] = y2;

        v[i+4] = x2;
        v[i+5] = y1;
    }

    glVertexPointer(2, GL_FLOAT, 0, v);
    glDrawArrays(GL_TRIANGLES, 0, tris*3);

    free(v);
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

static GLuint vboColorPointer(GLint size, GLenum type, GLsizei stride,
                              const GLvoid *buf, GLsizei bufSize, intptr_t bufOffset)
{
    GLuint id;
    if (user_va) {
        glColorPointer(size, type, stride, (char*)buf + bufOffset);
        return 0;
    }
    glGenBuffers(1, &id);
    glBindBuffer(GL_ARRAY_BUFFER, id);
    glBufferData(GL_ARRAY_BUFFER, bufSize, buf, GL_STATIC_DRAW);
    glColorPointer(size, type, stride, (void*)bufOffset);
    return id;
}

static GLuint vboVertexColorPointer(GLint vSize, GLenum vType, GLint vStride, intptr_t vOffset,
                                    GLint cSize, GLenum cType, GLint cStride, intptr_t cOffset,
                                    const GLvoid *buf, GLsizei bufSize)
{
    GLuint id;
    if (user_va) {
        glVertexPointer(vSize, vType, vStride, (char*)buf + vOffset);
        glColorPointer(cSize, cType, cStride, (char*)buf + cOffset);
        return 0;
    }
    glGenBuffers(1, &id);
    glBindBuffer(GL_ARRAY_BUFFER, id);
    glBufferData(GL_ARRAY_BUFFER, bufSize, buf, GL_STATIC_DRAW);
    glVertexPointer(vSize, vType, vStride, (void*)vOffset);
    glColorPointer(cSize, cType, cStride, (void*)cOffset);
    return id;
}

static void test_ubyte_colors(float x1, float y1, float x2, float y2, int index)
{
    float v[] = {
        x1, y1,
        x1, y2,
        x2, y1
    };
    unsigned char c3[] = {
        255, 0, 0,
        255, 0, 0,
        255, 0, 0,
    };
    unsigned char c4[] = {
        255, 0, 0, 0,
        255, 0, 0, 0,
        255, 0, 0, 0,
    };
    unsigned char c3o1[] = {
        0,
        255, 0, 0,
        255, 0, 0,
        255, 0, 0,
    };
    unsigned char c4o1[] = {
        0,
        255, 0, 0, 0,
        255, 0, 0, 0,
        255, 0, 0, 0,
    };
    unsigned char c3o2[] = {
        0, 0,
        255, 0, 0,
        255, 0, 0,
        255, 0, 0,
    };
    unsigned char c4o2[] = {
        0, 0,
        255, 0, 0, 0,
        255, 0, 0, 0,
        255, 0, 0, 0,
    };
    unsigned char c3o3[] = {
        0, 0, 0,
        255, 0, 0,
        255, 0, 0,
        255, 0, 0,
    };
    unsigned char c4o3[] = {
        0, 0, 0,
        255, 0, 0, 0,
        255, 0, 0, 0,
        255, 0, 0, 0,
    };
    GLuint vbo;

    glVertexPointer(2, GL_FLOAT, 0, v);

    glEnableClientState(GL_COLOR_ARRAY);
    switch (index) {
        case 0: vbo = vboColorPointer(3, GL_UNSIGNED_BYTE, 3, c3, sizeof(c3), 0); break;
        case 1: vbo = vboColorPointer(3, GL_UNSIGNED_BYTE, 4, c4, sizeof(c4), 0); break;
        case 2: vbo = vboColorPointer(4, GL_UNSIGNED_BYTE, 4, c4, sizeof(c4), 0); break;

        case 3: vbo = vboColorPointer(3, GL_UNSIGNED_BYTE, 3, c3o1, sizeof(c3o1), 1); break;
        case 4: vbo = vboColorPointer(3, GL_UNSIGNED_BYTE, 4, c4o1, sizeof(c4o1), 1); break;
        case 5: vbo = vboColorPointer(4, GL_UNSIGNED_BYTE, 4, c4o1, sizeof(c4o1), 1); break;

        case 6: vbo = vboColorPointer(3, GL_UNSIGNED_BYTE, 3, c3o2, sizeof(c3o2), 2); break;
        case 7: vbo = vboColorPointer(3, GL_UNSIGNED_BYTE, 4, c4o2, sizeof(c4o2), 2); break;
        case 8: vbo = vboColorPointer(4, GL_UNSIGNED_BYTE, 4, c4o2, sizeof(c4o2), 2); break;

        case 9: vbo = vboColorPointer(3, GL_UNSIGNED_BYTE, 3, c3o3, sizeof(c3o3), 3); break;
        case 10:vbo = vboColorPointer(3, GL_UNSIGNED_BYTE, 4, c4o3, sizeof(c4o3), 3); break;
        case 11:vbo = vboColorPointer(4, GL_UNSIGNED_BYTE, 4, c4o3, sizeof(c4o3), 3); break;

        default:vbo= 0; assert(0); break;
    }

    glDrawArrays(GL_TRIANGLES, 0, 3);

    glDisableClientState(GL_COLOR_ARRAY);
    if (vbo)
        glDeleteBuffers(1, &vbo);
}

static void test_short_vertices(float x1, float y1, float x2, float y2, int index)
{
    short v2[] = {
        x1, y1,
        x1, y2,
        x2, y1
    };
    short v3[] = {
        x1, y1, 0,
        x1, y2, 0,
        x2, y1, 0
    };
    short v4[] = {
        x1, y1, 0, 1,
        x1, y2, 0, 1,
        x2, y1, 0, 1
    };
    short v2o[] = {
        0,
        x1, y1,
        x1, y2,
        x2, y1
    };
    short v3o[] = {
        0,
        x1, y1, 0,
        x1, y2, 0,
        x2, y1, 0
    };
    short v4o[] = {
        0,
        x1, y1, 0, 1,
        x1, y2, 0, 1,
        x2, y1, 0, 1
    };
    GLuint vbo;

    switch (index) {
        case 0: vbo = vboVertexPointer(2, GL_SHORT, 4, v2, sizeof(v2), 0); break;

        case 1: vbo = vboVertexPointer(2, GL_SHORT, 6, v3, sizeof(v3), 0); break;
        case 2: vbo = vboVertexPointer(3, GL_SHORT, 6, v3, sizeof(v3), 0); break;

        case 3: vbo = vboVertexPointer(2, GL_SHORT, 8, v4, sizeof(v4), 0); break;
        case 4: vbo = vboVertexPointer(3, GL_SHORT, 8, v4, sizeof(v4), 0); break;
        case 5: vbo = vboVertexPointer(4, GL_SHORT, 8, v4, sizeof(v4), 0); break;

        case 6: vbo = vboVertexPointer(2, GL_SHORT, 4, v2o, sizeof(v2o), 2); break;

        case 7: vbo = vboVertexPointer(2, GL_SHORT, 6, v3o, sizeof(v3o), 2); break;
        case 8: vbo = vboVertexPointer(3, GL_SHORT, 6, v3o, sizeof(v3o), 2); break;

        case 9: vbo = vboVertexPointer(2, GL_SHORT, 8, v4o, sizeof(v4o), 2); break;
        case 10:vbo = vboVertexPointer(3, GL_SHORT, 8, v4o, sizeof(v4o), 2); break;
        case 11:vbo = vboVertexPointer(4, GL_SHORT, 8, v4o, sizeof(v4o), 2); break;

        default:vbo = 0; assert(0); break;
    }

    glDrawArrays(GL_TRIANGLES, 0, 3);

    if (vbo)
        glDeleteBuffers(1, &vbo);
}

static void test_int_vertices(float x1, float y1, float x2, float y2, int index)
{
    int v2[] = {
        x1, y1,
        x1, y2,
        x2, y1
    };
    int v3[] = {
        x1, y1, 0,
        x1, y2, 0,
        x2, y1, 0
    };
    int v4[] = {
        x1, y1, 0, 1,
        x1, y2, 0, 1,
        x2, y1, 0, 1
    };
    int v2o[] = {
	0,
        x1, y1,
        x1, y2,
        x2, y1
    };
    int v3o[] = {
	0,
        x1, y1, 0,
        x1, y2, 0,
        x2, y1, 0
    };
    int v4o[] = {
	0,
        x1, y1, 0, 1,
        x1, y2, 0, 1,
        x2, y1, 0, 1
    };
    GLuint vbo;

    switch (index) {
        case 0: vbo = vboVertexPointer(2, GL_INT, 8, v2, sizeof(v2), 0); break;
        case 1: vbo = vboVertexPointer(3, GL_INT, 12, v3, sizeof(v3), 0); break;
        case 2: vbo = vboVertexPointer(4, GL_INT, 16, v4, sizeof(v4), 0); break;
        case 3: vbo = vboVertexPointer(2, GL_INT, 8, v2o, sizeof(v2o), 4); break;
        case 4: vbo = vboVertexPointer(3, GL_INT, 12, v3o, sizeof(v3o), 4); break;
        case 5: vbo = vboVertexPointer(4, GL_INT, 16, v4o, sizeof(v4o), 4); break;
        default: vbo = 0; assert(0); break;
    }

    glDrawArrays(GL_TRIANGLES, 0, 3);

    if (vbo)
        glDeleteBuffers(1, &vbo);
}

static void test_double_vertices(float x1, float y1, float x2, float y2, int index)
{
	double v2[] = {
	    x1, y1,
	    x1, y2,
	    x2, y1
	};
	double v3[] = {
	    x1, y1, 0,
	    x1, y2, 0,
	    x2, y1, 0
	};
	double v4[] = {
	    x1, y1, 0, 1,
	    x1, y2, 0, 1,
	    x2, y1, 0, 1
	};
	double v2o[] = {
	    0,
	    x1, y1,
	    x1, y2,
	    x2, y1
	};
	double v3o[] = {
	    0,
	    x1, y1, 0,
	    x1, y2, 0,
	    x2, y1, 0
	};
	double v4o[] = {
	    0,
	    x1, y1, 0, 1,
	    x1, y2, 0, 1,
	    x2, y1, 0, 1
	};
	double v2s[] = {
	    0,  0,
	    x1, y1,
	    x1, y2,
	    x2, y1
	};
	double v3s[] = {
	    0,  0,  0,
	    x1, y1, 0,
	    x1, y2, 0,
	    x2, y1, 0
	};
	double v4s[] = {
	    0,  0,  0, 0,
	    x1, y1, 0, 1,
	    x1, y2, 0, 1,
	    x2, y1, 0, 1
	};
	GLuint vbo;

	switch (index) {
	case 0: vbo = vboVertexPointer(2, GL_DOUBLE, 16, v2, sizeof(v2), 0); break;
	case 1: vbo = vboVertexPointer(3, GL_DOUBLE, 24, v3, sizeof(v3), 0); break;
	case 2: vbo = vboVertexPointer(4, GL_DOUBLE, 32, v4, sizeof(v4), 0); break;
	case 3: vbo = vboVertexPointer(2, GL_DOUBLE, 16, v2o, sizeof(v2o), 8); break;
	case 4: vbo = vboVertexPointer(3, GL_DOUBLE, 24, v3o, sizeof(v3o), 8); break;
	case 5: vbo = vboVertexPointer(4, GL_DOUBLE, 32, v4o, sizeof(v4o), 8); break;
	case 6: vbo = vboVertexPointer(2, GL_DOUBLE, 16, v2s, sizeof(v2s), 0); break;
	case 7: vbo = vboVertexPointer(3, GL_DOUBLE, 24, v3s, sizeof(v3s), 0); break;
	case 8: vbo = vboVertexPointer(4, GL_DOUBLE, 32, v4s, sizeof(v4s), 0); break;
	default: vbo = 0; assert(0); break;
	}

	if (index <= 5)
		glDrawArrays(GL_TRIANGLES, 0, 3);
	else
		glDrawArrays(GL_TRIANGLES, 1, 3);

	if (vbo)
	    glDeleteBuffers(1, &vbo);
}

static void test_interleaved_vertices(float x1, float y1, float x2, float y2, int index)
{
    static const unsigned int offset[] = {4, 5, 6, 8};

    struct vertex {
        GLshort v[2];
        GLubyte c[8];
    } v[] = {
        {{x1, y1}, {0}},
        {{x1, y2}, {0}},
        {{x2, y1}, {0}}
    };

    unsigned int i;
    GLuint vbo;

    /* Set color green */
    for (i = 0; i < 3; ++i)
        v[i].c[offset[index] - 3] = 0xff;

    glEnableClientState(GL_COLOR_ARRAY);
    vbo = vboVertexColorPointer(2, GL_SHORT, sizeof(struct vertex), 0,
                                3, GL_UNSIGNED_BYTE, sizeof(struct vertex), offset[index],
                                v, sizeof(v));

    glDrawArrays(GL_TRIANGLES, 0, 3);

    glDisableClientState(GL_COLOR_ARRAY);
    if (vbo)
        glDeleteBuffers(1, &vbo);
}

static void test_mixed_user_and_vbo_buffers(float x1, float y1, float x2, float y2, int index)
{
    float v3[] = {
        x1, y1, 0,
        x1, y2, 0,
        x2, y1, 0
    };
    float c3[] = {
        0, 0, 1,
        0, 0, 1,
        0, 0, 1
    };
    GLuint vbo;

    glEnableClientState(GL_COLOR_ARRAY);

    if (!index) {
        glColorPointer(3, GL_FLOAT, 0, c3);
        vbo = vboVertexPointer(3, GL_FLOAT, 0, v3, sizeof(v3), 0);
    } else {
        glVertexPointer(3, GL_FLOAT, 0, v3);
        vbo = vboColorPointer(3, GL_FLOAT, 0, c3, sizeof(c3), 0);
    }

    glDrawArrays(GL_TRIANGLES, 0, 3);

    glDisableClientState(GL_COLOR_ARRAY);
    if (vbo)
        glDeleteBuffers(1, &vbo);
}

enum {
    BOTH,
    USER
};

struct test {
    void (*test)(float x1, float y1, float x2, float y2, int index);
    int index;
    float expected_color[3];
    int flag;
    const char *name;
};

struct test tests[] = {
    {test_ubyte_colors, 0, {1, 0, 0}, BOTH, "Ubyte colors - components: 3, stride: 3,"},
    {test_ubyte_colors, 1, {1, 0, 0}, BOTH, "Ubyte colors - components: 3, stride: 4,"},
    {test_ubyte_colors, 2, {1, 0, 0}, BOTH, "Ubyte colors - components: 4, stride: 4,"},
    {test_ubyte_colors, 3, {1, 0, 0}, BOTH, "Ubyte colors - components: 3, stride: 3, offset: 1"},
    {test_ubyte_colors, 4, {1, 0, 0}, BOTH, "Ubyte colors - components: 3, stride: 4, offset: 1"},
    {test_ubyte_colors, 5, {1, 0, 0}, BOTH, "Ubyte colors - components: 4, stride: 4, offset: 1"},
    {test_ubyte_colors, 6, {1, 0, 0}, BOTH, "Ubyte colors - components: 3, stride: 3, offset: 2"},
    {test_ubyte_colors, 7, {1, 0, 0}, BOTH, "Ubyte colors - components: 3, stride: 4, offset: 2"},
    {test_ubyte_colors, 8, {1, 0, 0}, BOTH, "Ubyte colors - components: 4, stride: 4, offset: 2"},
    {test_ubyte_colors, 9, {1, 0, 0}, BOTH, "Ubyte colors - components: 3, stride: 3, offset: 3"},
    {test_ubyte_colors, 10, {1, 0, 0}, BOTH, "Ubyte colors - components: 3, stride: 4, offset: 3"},
    {test_ubyte_colors, 11, {1, 0, 0}, BOTH, "Ubyte colors - components: 4, stride: 4, offset: 3"},

    {test_short_vertices, 0, {1, 1, 1}, BOTH, "Short vertices - components: 2, stride: 4,"},
    {test_short_vertices, 1, {1, 1, 1}, BOTH, "Short vertices - components: 2, stride: 6,"},
    {test_short_vertices, 2, {1, 1, 1}, BOTH, "Short vertices - components: 3, stride: 6,"},
    {test_short_vertices, 3, {1, 1, 1}, BOTH, "Short vertices - components: 2, stride: 8,"},
    {test_short_vertices, 4, {1, 1, 1}, BOTH, "Short vertices - components: 3, stride: 8,"},
    {test_short_vertices, 5, {1, 1, 1}, BOTH, "Short vertices - components: 4, stride: 8,"},
    {test_short_vertices, 6, {1, 1, 1}, BOTH, "Short vertices - components: 2, stride: 4, offset: 2"},
    {test_short_vertices, 7, {1, 1, 1}, BOTH, "Short vertices - components: 2, stride: 6, offset: 2"},
    {test_short_vertices, 8, {1, 1, 1}, BOTH, "Short vertices - components: 3, stride: 6, offset: 2"},
    {test_short_vertices, 9, {1, 1, 1}, BOTH, "Short vertices - components: 2, stride: 8, offset: 2"},
    {test_short_vertices, 10, {1, 1, 1}, BOTH, "Short vertices - components: 3, stride: 8, offset: 2"},
    {test_short_vertices, 11, {1, 1, 1}, BOTH, "Short vertices - components: 4, stride: 8, offset: 2"},

    {test_int_vertices, 0, {1, 1, 1}, BOTH, "Int vertices - components: 2, stride: 8"},
    {test_int_vertices, 1, {1, 1, 1}, BOTH, "Int vertices - components: 3, stride: 12"},
    {test_int_vertices, 2, {1, 1, 1}, BOTH, "Int vertices - components: 4, stride: 16"},
    {test_int_vertices, 3, {1, 1, 1}, BOTH, "Int vertices - components: 2, stride: 8,  offset: 4"},
    {test_int_vertices, 4, {1, 1, 1}, BOTH, "Int vertices - components: 3, stride: 12, offset: 4"},
    {test_int_vertices, 5, {1, 1, 1}, BOTH, "Int vertices - components: 4, stride: 16, offset: 4"},

    {test_double_vertices, 0, {1, 1, 1}, BOTH, "Double vertices - components: 2, stride: 16"},
    {test_double_vertices, 1, {1, 1, 1}, BOTH, "Double vertices - components: 3, stride: 24"},
    {test_double_vertices, 2, {1, 1, 1}, BOTH, "Double vertices - components: 4, stride: 32"},
    {test_double_vertices, 3, {1, 1, 1}, BOTH, "Double vertices - components: 2, stride: 16, offset: 8"},
    {test_double_vertices, 4, {1, 1, 1}, BOTH, "Double vertices - components: 3, stride: 24, offset: 8"},
    {test_double_vertices, 5, {1, 1, 1}, BOTH, "Double vertices - components: 4, stride: 32, offset: 8"},
    {test_double_vertices, 6, {1, 1, 1}, BOTH, "Double vertices - components: 2, stride: 16, start: 1"},
    {test_double_vertices, 7, {1, 1, 1}, BOTH, "Double vertices - components: 3, stride: 24, start: 1"},
    {test_double_vertices, 8, {1, 1, 1}, BOTH, "Double vertices - components: 4, stride: 32, start: 1"},

    {test_interleaved_vertices, 0, {0, 1, 0}, BOTH, "Interleaved VBO - gap: 0"},
    {test_interleaved_vertices, 1, {0, 1, 0}, BOTH, "Interleaved VBO - gap: 1"},
    {test_interleaved_vertices, 2, {0, 1, 0}, BOTH, "Interleaved VBO - gap: 2"},
    {test_interleaved_vertices, 3, {0, 1, 0}, BOTH, "Interleaved VBO - gap: 4"},

    {test_mixed_user_and_vbo_buffers, 0, {0, 0, 1}, BOTH, "Mixed buffers - 0: vbo,  1: user"},
    {test_mixed_user_and_vbo_buffers, 1, {0, 0, 1}, BOTH, "Mixed buffers - 0: user, 1: vbo"},

    {test_large_vertex_count, 0, {1, 1, 1}, USER, "Large vertex count"},

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
        if (!user_va && tests[i].flag == USER)
            continue;

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
    piglit_present_results();

    return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}
