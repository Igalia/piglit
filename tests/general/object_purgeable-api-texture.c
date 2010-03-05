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
#include "object_purgeable.h"

int piglit_width = 100, piglit_height = 100;
int piglit_window_mode = GLUT_RGB;

void
piglit_init(int argc, char **argv)
{
	(void) argc;
	(void) argv;

	init_ObjectPurgeableAPI();
	piglit_automatic = GL_TRUE;
}


enum piglit_result
piglit_display(void)
{
    GLuint texture;
    GLboolean pass = GL_TRUE;

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
    return pass ? PIGLIT_SUCCESS : PIGLIT_FAILURE;
}
