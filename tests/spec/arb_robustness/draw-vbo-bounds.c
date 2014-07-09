/*
 * Copyright (C) 2011 VMware, Inc.
 * Copyright (C) 2010 Marek Olšák <maraeo@gmail.com>
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
 *   Jose Fonseca <jfonseca@vmware.com>
 *   Based on code from Marek Olšák <maraeo@gmail.com>
 */

/* Test whether out-of-bounds vertex buffer object cause termination.
 *
 * Note that the original ARB_vertex_buffer_object extension explicitly states
 * program termination is allowed when out-of-bounds vertex buffer object
 * fetches occur.  The ARB_robustness extension does provides an enable to
 * guarantee that out-of-bounds buffer object accesses by the GPU will have
 * deterministic behavior and preclude application instability or termination
 * due to an incorrect buffer access.  But regardless of ARB_robustness
 * extension support it is a good idea not to crash.  For example,  viewperf
 * doesn't properly detect NV_primitive_restart and emits 0xffffffff indices
 * which can result in crashes.
 *
 * TODO:
 * - test more vertex/element formats
 * - add test for out-of-bound index buffer object access
 * - add test non-aligned offsets
 * - provide a command line option to actually enable ARB_robustness
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_width = 320;
	config.window_height = 320;
	config.window_visual = PIGLIT_GL_VISUAL_RGB;

PIGLIT_GL_TEST_CONFIG_END

void piglit_init(int argc, char **argv)
{
    piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);

    piglit_require_gl_version(15);

    glShadeModel(GL_FLAT);
    glClearColor(0.2, 0.2, 0.2, 1.0);
}

static void
random_vertices(GLsizei offset, GLsizei stride, GLsizei count)
{
    GLsizei element_size = 2 * sizeof(GLfloat);
    GLsizei size;
    GLubyte *vertices;
    GLsizei i;

    if (stride == 0) {
	stride = element_size;
    }

    size = offset + (count - 1)*stride + element_size;

    assert(offset % sizeof(GLfloat) == 0);
    assert(stride % sizeof(GLfloat) == 0);

    glBufferData(GL_ARRAY_BUFFER, size, NULL, GL_STATIC_DRAW);
    assert(glGetError() == GL_NO_ERROR);

    vertices = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
    assert(vertices);
    if (!vertices) {
        return;
    }

    for (i = 0; i < count; ++i) {
        GLfloat *vertex = (GLfloat *)(vertices + offset + i*stride);
        vertex[0] = (rand() % 1000) * .001;
        vertex[1] = (rand() % 1000) * .001;
    }

    glUnmapBuffer(GL_ARRAY_BUFFER);
}

static void
random_ushort_indices(GLsizei offset, GLsizei count, GLuint min_index, GLuint max_index)
{
    GLushort *indices;
    GLsizei size = offset + count*sizeof(*indices);
    GLsizei i;

    glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, NULL, GL_STATIC_DRAW);
    assert(glGetError() == GL_NO_ERROR);

    indices = glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY);
    assert(indices);
    if (!indices) {
        return;
    }

    assert(offset % sizeof(*indices) == 0);
    for (i = 0; i < count; ++i) {
        GLushort *index = indices + offset / sizeof *indices + i;
        *index = min_index + rand() % (max_index - min_index + 1);
    }

    glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
}

static void test(void)
{
    GLsizei vertex_offset;
    GLsizei vertex_stride;
    GLsizei vertex_count;
    GLuint vertex_buffer;

    GLsizei index_offset;
    GLsizei index_count;
    GLuint max_index;
    GLuint min_index;
    GLuint index_buffer;

    vertex_offset = (rand() % 0xff) * sizeof(GLfloat);
    vertex_stride = (rand() % 0xf) * sizeof(GLfloat);
    vertex_count = 1 + rand() % 0xffff;

    index_offset = (rand() % 0xff) * sizeof(GLushort);
    index_count = 1 + rand() % 0xffff;
    min_index = rand() % vertex_count;
    max_index = min_index + rand() % (vertex_count - min_index);

    if (!piglit_automatic) {
        fprintf(stdout, "vertex_offset = %i\n", vertex_offset);
        fprintf(stdout, "vertex_stride = %i\n", vertex_stride);
        fprintf(stdout, "vertex_count = %i\n", vertex_count);
        fprintf(stdout, "index_offset = %i\n", index_offset);
        fprintf(stdout, "index_count = %i\n", index_count);
        fprintf(stdout, "min_index = %u\n", min_index);
        fprintf(stdout, "max_index = %u\n", max_index);
        fprintf(stdout, "\n");
	fflush(stdout);
    }

    glGenBuffers(1, &vertex_buffer);
    glGenBuffers(1, &index_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);

    random_vertices(vertex_offset, vertex_stride, vertex_count);

    if (0) {
        /* Generate valid indices only */
        random_ushort_indices(index_offset, index_count, min_index, max_index);
    } else {
        /* Generate out-of-range indices */
        random_ushort_indices(index_offset, index_count, 0, 2*vertex_count - 1);
    }

    glVertexPointer(2, GL_FLOAT, vertex_stride, (const void*)(intptr_t)vertex_offset);
    glDrawRangeElements(GL_TRIANGLES,
                        min_index,
                        max_index,
                        index_count,
                        GL_UNSIGNED_SHORT,
                        (const void*)(intptr_t)index_offset);
    assert(glGetError() == GL_NO_ERROR);

    /* Call glFinish to prevent the draw from being batched, delaying the cpu crash / gpu crash
     * to much later. */
    glFinish();

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glDeleteBuffers(1, &index_buffer);
    glDeleteBuffers(1, &vertex_buffer);
}

enum piglit_result
piglit_display(void)
{
    unsigned i;

    glClear(GL_COLOR_BUFFER_BIT);
    glEnableClientState(GL_VERTEX_ARRAY);

    for (i = 0; i < 1000; ++i) {
        test();
        assert(glGetError() == GL_NO_ERROR);
    }

    glFinish();

    return PIGLIT_PASS;
}

