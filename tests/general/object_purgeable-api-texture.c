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
 * \file object_purgeable-api-texture.c
 * Simple test of the API for GL_APPLE_object_purgeable with texture object.
 */

#include "piglit-util.h"

#define FAIL_ON_ERROR(string)					\
    do {							\
        const GLenum err = glGetError();			\
        if (err != GL_NO_ERROR) {				\
            fprintf(stderr, "%s generated error 0x%04x\n", 	\
                string, err);                                   \
                pass = GL_FALSE;				\
        }							\
    } while (0)

#ifndef APIENTRY
#define APIENTRY
#endif
#ifndef APIENTRYP
#define APIENTRYP APIENTRY *
#endif

#ifndef GL_APPLE_object_purgeable
#define GL_APPLE_object_purgeable 1

#define GL_RELEASED_APPLE                 0x8A19
#define GL_VOLATILE_APPLE                 0x8A1A
#define GL_RETAINED_APPLE                 0x8A1B
#define GL_UNDEFINED_APPLE                0x8A1C
#define GL_PURGEABLE_APPLE                0x8A1D
#define GL_BUFFER_OBJECT_APPLE            0x85B3


typedef GLenum (APIENTRYP PFNGLOBJECTPURGEABLEAPPLEPROC) (GLenum objectType, GLuint name, GLenum option);
typedef GLenum (APIENTRYP PFNGLOBJECTUNPURGEABLEAPPLEPROC) (GLenum objectType, GLuint name, GLenum option);
typedef void (APIENTRYP PFNGLGETOBJECTPARAMETERIVAPPLEPROC) (GLenum objectType, GLuint name, GLenum pname, GLint* params);

#endif


static PFNGLOBJECTPURGEABLEAPPLEPROC pglObjectPurgeableAPPLE = NULL;
static PFNGLOBJECTUNPURGEABLEAPPLEPROC pglObjectUnpurgeableAPPLE = NULL;
static PFNGLGETOBJECTPARAMETERIVAPPLEPROC pglGetObjectParameterivAPPLE = NULL;

static GLboolean Automatic = GL_FALSE;

static void
init(void)
{
    piglit_require_extension("GL_APPLE_object_purgeable");

    pglObjectPurgeableAPPLE = (PFNGLOBJECTPURGEABLEAPPLEPROC) piglit_get_proc_address("glObjectPurgeableAPPLE");
    pglObjectUnpurgeableAPPLE = (PFNGLOBJECTUNPURGEABLEAPPLEPROC) piglit_get_proc_address("glObjectUnpurgeableAPPLE");
    pglGetObjectParameterivAPPLE = (PFNGLGETOBJECTPARAMETERIVAPPLEPROC) piglit_get_proc_address("glGetObjectParameterivAPPLE");

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


GLboolean
test_ObjectpurgeableAPPLE(GLenum objectType, GLuint name, GLenum option)
{
    GLboolean pass = GL_TRUE;
    GLenum ret;
    ret = (*pglObjectPurgeableAPPLE)(objectType, name, option);
    FAIL_ON_ERROR("pglObjectpurgeableAPPLE");
    switch (option) {
    case GL_VOLATILE_APPLE:
        if (ret != GL_VOLATILE_APPLE) {
            fprintf(stderr, "GL_VOLATILE_APPLE should be returned when call purgeable with GL_VOLATILE_APPLE\n");
            pass = GL_FALSE;
        }
        break;
        case GL_RELEASED_APPLE:
        if (ret != GL_VOLATILE_APPLE && ret != GL_RELEASED_APPLE) {
            fprintf(stderr, "GL_VOLATILE_APPLE or GL_RELEASED_APPLE should be returned when call purgeable with GL_RELEASED_APPLE\n");
            pass = GL_FALSE;
        }
        break;
    }

    return pass;
}


GLboolean
test_ObjectunpurgeableAPPLE(GLenum objectType, GLuint name, GLenum option)
{
    GLboolean pass = GL_TRUE;
    GLenum ret;
    ret = (*pglObjectUnpurgeableAPPLE)(objectType, name, option);
    FAIL_ON_ERROR("pglObjectunpurgeableAPPLE");
    switch (option) {
    case GL_RETAINED_APPLE:
        if (ret != GL_RETAINED_APPLE && ret != GL_UNDEFINED_APPLE) {
            fprintf(stderr, "GL_RETAINED_APPLE or GL_UNDEFINED_APPLE should be returned when call purgeable with GL_RETAINED_APPLE\n");
            pass = GL_FALSE;
        }
        break;
        case GL_UNDEFINED_APPLE:
        if (ret != GL_UNDEFINED_APPLE) {
            fprintf(stderr, "GL_UNDEFINED_APPLE should be returned when call purgeable with GL_UNDEFINED_APPLE\n");
            pass = GL_FALSE;
        }
        break;
    }
    return pass;
}

GLboolean
test_GetObjectParameterivAPPLE(GLenum objectType, GLuint name, GLenum expect)
{
    GLboolean pass = GL_TRUE;
    GLint param;
    (*pglGetObjectParameterivAPPLE)(objectType, name, GL_PURGEABLE_APPLE, &param);
    FAIL_ON_ERROR("glGetObjectParameterivAPPLE");

    if (param != expect) {
        fprintf(stderr, "GL_PURGEABLE_APPLE state is not set to GL_TRUE for texture object\n");
        pass = GL_FALSE;
    }

    return pass;
}

static void
display(void)
{
    GLuint texture;
    GLboolean pass = GL_TRUE;

    glClear(GL_COLOR_BUFFER_BIT);
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 100, 100, 0, GL_RGB, GL_INT, NULL);

    glGetError();

    if (test_GetObjectParameterivAPPLE(GL_TEXTURE, texture, GL_FALSE) != GL_TRUE) {
        fprintf(stderr, "Default GL_PURGEABLE_APPLE state should GL_FALSE for texture object\n");
        pass = GL_FALSE;
    }

    if (test_ObjectpurgeableAPPLE(GL_TEXTURE, texture, GL_VOLATILE_APPLE) != GL_TRUE) {
        fprintf(stderr, "Error when mark object %x to purgeable(GL_VOLATILE_APPLE)\n", texture);
        pass = GL_FALSE;
    }

    if (test_GetObjectParameterivAPPLE(GL_TEXTURE, texture, GL_TRUE) != GL_TRUE) {
        fprintf(stderr, "Object %x is not set to purgeable\n", texture);
        pass = GL_FALSE;
    }

    if (test_ObjectunpurgeableAPPLE(GL_TEXTURE, texture, GL_RETAINED_APPLE) != GL_TRUE) {
        fprintf(stderr, "Error when mark object %x to unpurgeable(GL_RETAINED_APPLE)\n", texture);
        pass = GL_FALSE;
    }

    if (test_GetObjectParameterivAPPLE(GL_TEXTURE, texture, GL_FALSE) != GL_TRUE) {
        fprintf(stderr, "Object %x is not set to unpurgeable\n", texture);
        pass = GL_FALSE;
    }

    if (test_ObjectpurgeableAPPLE(GL_TEXTURE, texture, GL_RELEASED_APPLE) != GL_TRUE) {
        fprintf(stderr, "Error when mark object %x to purgeable(GL_RELEASED_APPLE)\n", texture);
        pass = GL_FALSE;
    }

    if (test_GetObjectParameterivAPPLE(GL_TEXTURE, texture, GL_TRUE) != GL_TRUE) {
        fprintf(stderr, "Object %x is not set to purgeable\n", texture);
        pass = GL_FALSE;
    }

    if (test_ObjectunpurgeableAPPLE(GL_TEXTURE, texture, GL_UNDEFINED_APPLE) != GL_TRUE) {
        fprintf(stderr, "Error when mark object %x to unpurgeable(GL_UNDEFINED_APPLE)\n", texture);
        pass = GL_FALSE;
    }

    if (test_GetObjectParameterivAPPLE(GL_TEXTURE, texture, GL_FALSE) != GL_TRUE) {
        fprintf(stderr, "Object %x is not set to unpurgeable\n", texture);
        pass = GL_FALSE;
    }


    glDeleteTextures(1, &texture);
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
    glutCreateWindow("GL_APPLE_object_purgeable API test with texture object");
    glutReshapeFunc(reshape);
    glutKeyboardFunc(piglit_escape_exit_key);
    glutDisplayFunc(display);

    init();

    glutMainLoop();
    return 0;
}

