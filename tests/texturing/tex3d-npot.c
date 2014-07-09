/*
 * Copyright (c) The Piglit project 2008
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * on the rights to use, copy, modify, merge, publish, distribute, sub
 * license, and/or sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.  IN NO EVENT SHALL
 * VA LINUX SYSTEM, IBM AND/OR THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/**
 * @file
 * Tests 3D textures.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

static GLuint Texture;

static int nrcomponents(GLenum format)
{
    switch(format) {
    case GL_RGBA: return 4;
    case GL_RGB: return 3;
    case GL_ALPHA: return 1;
    default: abort();
    }
}

static void
expected_rgba(GLenum format, const GLubyte *texdata, GLubyte *expected)
{
    switch(format) {
    case GL_RGBA:
        expected[0] = texdata[0];
        expected[1] = texdata[1];
        expected[2] = texdata[2];
        expected[3] = texdata[3];
        return;
    case GL_RGB:
        expected[0] = texdata[0];
        expected[1] = texdata[1];
        expected[2] = texdata[2];
        expected[3] = 255;
        return;
    case GL_ALPHA:
        expected[0] = 255;
        expected[1] = 255;
        expected[2] = 255;
        expected[3] = texdata[0];
        return;
    }
}

static bool
render_and_check(int w, int h, int d, GLenum format, float q,
                 const GLubyte *data, const char* test)
{
    int x, y, z;
    int layer;
    GLubyte *readback;
    const GLubyte *texp;
    GLubyte *readp;
    int ncomp = 0;

    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT);
    glEnable(GL_TEXTURE_3D);

    x = y = 0;
    for(layer = 0; layer < d; ++layer) {
        float r = (layer+0.5)/d;
        glBegin(GL_QUADS);
            glTexCoord4f(0, 0, r*q, q);
            glVertex2f(x, y);
            glTexCoord4f(q, 0, r*q, q);
            glVertex2f(x+w, y);
            glTexCoord4f(q, q, r*q, q);
            glVertex2f(x+w, y+h);
            glTexCoord4f(0, q, r*q, q);
            glVertex2f(x, y+h);
        glEnd();
        x += w;
        if (x+w >= piglit_width) {
            y += h;
            x = 0;
        }
    }

    readback = (unsigned char*)malloc(w*h*d*4);
    x = y = 0;
    for(layer = 0; layer < d; ++layer) {
        glReadPixels(x, y, w, h, GL_RGBA, GL_UNSIGNED_BYTE, readback+layer*w*h*4);
        x += w;
        if (x+w >= piglit_width) {
            y += h;
            x = 0;
        }
    }

    texp = data;
    readp = readback;
    ncomp = nrcomponents(format);
    for(z = 0; z < d; ++z) {
        for(y = 0; y < h; ++y) {
            for(x = 0; x < w; ++x, readp += 4, texp += ncomp) {
                unsigned char expected[4];
                int i;
                expected_rgba(format, texp, expected);
                for(i = 0; i < 4; ++i) {
                    if (expected[i] != readp[i]) {
                        fprintf(stderr, "%s: Mismatch at %ix%ix%i\n", test, x, y, z);
                        fprintf(stderr, " Expected: %i,%i,%i,%i\n",
                            expected[0], expected[1], expected[2], expected[3]);
                        fprintf(stderr, " Readback: %i,%i,%i,%i\n",
                            readp[0], readp[1], readp[2], readp[3]);
                        free(readback);
                        return false;
                    }
                }
            }
        }
    }
    free(readback);

    piglit_present_results();
    return true;
}


/**
 * Load non-mipmapped 3D texture of the given size
 * and check whether it is rendered correctly.
 */
static bool
test_simple(int w, int h, int d, GLenum format)
{
    int size;
    unsigned char *data;
    int i;
    bool success = true;

    assert(1 <= w && w <= 16);
    assert(1 <= h && h <= 16);
    assert(1 <= d && d <= 16);
    assert(format == GL_RGBA || format == GL_RGB || format == GL_ALPHA);

    size = w*h*d*nrcomponents(format);
    data = (unsigned char*)malloc(size);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    srand(size); /* Generate reproducible image data */
    for(i = 0; i < size; ++i)
        data[i] = rand() & 255;

    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage3D(GL_TEXTURE_3D, 0, format, w, h, d, 0, format, GL_UNSIGNED_BYTE, data);

    success = success && render_and_check(w, h, d, format, 1.0, data, "Render 3D texture");
    success = success && render_and_check(w, h, d, format, 1.4, data, "Render 3D texture (q != 1.0)");

    free(data);

    if (!success) {
        fprintf(stderr, "Failure with texture size %ix%ix%i, format = %s\n",
            w, h, d, piglit_get_gl_enum_name(format));
    }

    return success;
}

enum piglit_result
piglit_display(void)
{
    GLenum formats[] = { GL_RGBA, GL_RGB, GL_ALPHA };
    int w, h, d, fmt;
    bool pass = true;

    piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);

    for(fmt = 0; fmt < sizeof(formats)/sizeof(formats[0]); ++fmt) {
        for(w = 3; w <= 15; w++) {
            if (w != 4 && w != 8) {
                for(h = 3; h <= 15; h++) {
                    if (h != 4 && h != 8) {
                        for(d = 3; d <= 15; d++) {
                            if (d != 4 && d != 8) {
                               pass = test_simple(w, h, d, formats[fmt]);
                               if (!pass)
                                  goto end;
                            }
                        }
                    }
                }
            }
        }
    }

end:
    return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
    piglit_require_extension("GL_ARB_texture_non_power_of_two");

    glDisable(GL_DITHER);

    glGenTextures(1, &Texture);
    glBindTexture(GL_TEXTURE_3D, Texture);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
}
