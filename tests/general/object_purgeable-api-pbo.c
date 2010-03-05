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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Authors:
 *    Shuang He <shuang.he@intel.com>
 */

/**
 * \file object_purgeable-api-pbo.c
 * Simple test of the API for GL_APPLE_object_purgeable with GL_ARB_pixel_buffer_object.
 */

#include "piglit-util.h"
#include "object_purgeable.h"

// GL_ARB_vertex_buffer_object GL_ARB_pixel_buffer_object
static PFNGLBINDBUFFERARBPROC pglBindBufferARB = NULL;
static PFNGLDELETEBUFFERSARBPROC pglDeleteBuffersARB = NULL;
static PFNGLGENBUFFERSARBPROC pglGenBuffersARB = NULL;
static PFNGLBUFFERDATAARBPROC pglBufferDataARB = NULL;


static GLboolean Automatic = GL_FALSE;

static void
init(void)
{
	init_ObjectPurgeableAPI();

    piglit_require_extension("GL_ARB_pixel_buffer_object");

    pglGenBuffersARB = (PFNGLGENBUFFERSARBPROC) piglit_get_proc_address("glGenBuffersARB");
    pglBindBufferARB = (PFNGLBINDBUFFERARBPROC) piglit_get_proc_address("glBindBufferARB");
    pglDeleteBuffersARB = (PFNGLDELETEBUFFERSARBPROC) piglit_get_proc_address("glDeleteBuffersARB");
    pglBufferDataARB = (PFNGLBUFFERDATAARBPROC) piglit_get_proc_address("glBufferDataARB");

    glClearColor(0.1, 0.1, 0.3, 0.0);
}


static void
reshape(int width, int height)
{
    glViewport(0, 0, (GLint) width, (GLint) height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-1.0, 1.0, -1.0, 1.0, -0.5, 1000.0);
    glMatrixMode(GL_MODELVIEW);
}


static void
display(void)
{
    GLuint pbo;
    GLboolean pass = GL_TRUE;

    glClear(GL_COLOR_BUFFER_BIT);

    glGetError();

    (*pglGenBuffersARB)(1, &pbo);
    (*pglBindBufferARB)(GL_PIXEL_PACK_BUFFER_ARB, pbo);
    (*pglBufferDataARB)(GL_PIXEL_PACK_BUFFER_ARB, 100*100, NULL, GL_STATIC_DRAW_ARB);
    (*pglBindBufferARB)(GL_PIXEL_PACK_BUFFER_ARB, 0);


    if (test_GetObjectParameterivAPPLE(GL_BUFFER_OBJECT_APPLE, pbo, GL_FALSE) != GL_TRUE) {
        fprintf(stderr, "Default GL_PURGEABLE_APPLE state should GL_FALSE for texture object\n");
        pass = GL_FALSE;
    }

    if (test_ObjectpurgeableAPPLE(GL_BUFFER_OBJECT_APPLE, pbo, GL_VOLATILE_APPLE) != GL_TRUE) {
        fprintf(stderr, "Error when mark object %x to purgeable(GL_VOLATILE_APPLE)\n", pbo);
        pass = GL_FALSE;
    }

    if (test_GetObjectParameterivAPPLE(GL_BUFFER_OBJECT_APPLE, pbo, GL_TRUE) != GL_TRUE) {
        fprintf(stderr, "Object %x is not set to purgeable\n", pbo);
        pass = GL_FALSE;
    }

    if (test_ObjectunpurgeableAPPLE(GL_BUFFER_OBJECT_APPLE, pbo, GL_RETAINED_APPLE) != GL_TRUE) {
        fprintf(stderr, "Error when mark object %x to unpurgeable(GL_RETAINED_APPLE)\n", pbo);
        pass = GL_FALSE;
    }

    if (test_GetObjectParameterivAPPLE(GL_BUFFER_OBJECT_APPLE, pbo, GL_FALSE) != GL_TRUE) {
        fprintf(stderr, "Object %x is not set to unpurgeable\n", pbo);
        pass = GL_FALSE;
    }

    if (test_ObjectpurgeableAPPLE(GL_BUFFER_OBJECT_APPLE, pbo, GL_RELEASED_APPLE) != GL_TRUE) {
        fprintf(stderr, "Error when mark object %x to purgeable(GL_RELEASED_APPLE)\n", pbo);
        pass = GL_FALSE;
    }

    if (test_GetObjectParameterivAPPLE(GL_BUFFER_OBJECT_APPLE, pbo, GL_TRUE) != GL_TRUE) {
        fprintf(stderr, "Object %x is not set to purgeable\n", pbo);
        pass = GL_FALSE;
    }

    if (test_ObjectunpurgeableAPPLE(GL_BUFFER_OBJECT_APPLE, pbo, GL_UNDEFINED_APPLE) != GL_TRUE) {
        fprintf(stderr, "Error when mark object %x to unpurgeable(GL_UNDEFINED_APPLE)\n", pbo);
        pass = GL_FALSE;
    }

    if (test_GetObjectParameterivAPPLE(GL_BUFFER_OBJECT_APPLE, pbo, GL_FALSE) != GL_TRUE) {
        fprintf(stderr, "Object %x is not set to unpurgeable\n", pbo);
        pass = GL_FALSE;
    }


    (*pglDeleteBuffersARB)(1, &pbo);


    if (Automatic)
        piglit_report_result(pass ? PIGLIT_SUCCESS : PIGLIT_FAILURE);
}


int main(int argc, char **argv)
{
    glutInit(&argc, argv);

    if (argc == 2 && !strcmp(argv[1], "-auto"))
        Automatic = GL_TRUE;

    glutInitWindowSize(400, 300);
    glutInitDisplayMode(GLUT_RGB);
    glutCreateWindow("GL_APPLE_object_purgeable API test with GL_ARB_pixel_buffer_object");
    glutReshapeFunc(reshape);
    glutKeyboardFunc(piglit_escape_exit_key);
    glutDisplayFunc(display);

    init();

    glutMainLoop();
    return 0;
}
