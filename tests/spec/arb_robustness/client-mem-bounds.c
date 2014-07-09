/*
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
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_width = 320;
	config.window_height = 320;
	config.window_visual = PIGLIT_GL_VISUAL_RGB;

PIGLIT_GL_TEST_CONFIG_END

#define width (10)
#define height (12)
#define depth (3)

void piglit_init(int argc, char **argv)
{
    piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);

   if (!piglit_is_extension_supported("GL_ARB_robustness"))
      piglit_report_result(PIGLIT_SKIP);

    glClearColor(0.2, 0.2, 0.2, 1.0);
}

static GLboolean
succeeded(int offby)
{
    GLboolean should_error = (offby < 0);
    GLenum err = glGetError();

    if (should_error) {
        if (err == GL_INVALID_OPERATION)
            return GL_TRUE;
        fprintf(stderr, "Did not give GL_INVALID_OPERATION "
                "with too small a buffer! (off by: %d)\n", offby);
        return GL_FALSE;
    } else {
        if (err == GL_NO_ERROR)
            return GL_TRUE;
        fprintf(stderr, "Unexpected error! (off by: %d)\n", offby);
        return GL_FALSE;
    }
}

static enum piglit_result
test_pixelmap(int offby)
{
#define MAPSIZE 32

#define TEST_PIXMAP(type, t)\
do {\
    GL##type v[MAPSIZE];\
    GLsizei bufSize = offby + (int)(sizeof v);\
    unsigned i;\
\
    memset(v, 0, sizeof v);\
    for (i = 0; i < MAPSIZE; i += 2)\
        v[i] = 1.0;\
\
    glClear(GL_COLOR_BUFFER_BIT);\
\
    glPixelTransferi(GL_MAP_COLOR, GL_FALSE);\
    glPixelMap##t##v(GL_PIXEL_MAP_R_TO_R, MAPSIZE, v);\
    if (!succeeded(0))\
        return PIGLIT_FAIL;\
    glPixelMap##t##v(GL_PIXEL_MAP_G_TO_G, MAPSIZE, v);\
    if (!succeeded(0))\
        return PIGLIT_FAIL;\
    glPixelMap##t##v(GL_PIXEL_MAP_B_TO_B, MAPSIZE, v);\
    if (!succeeded(0))\
        return PIGLIT_FAIL;\
    glPixelMap##t##v(GL_PIXEL_MAP_A_TO_A, MAPSIZE, v);\
    if (!succeeded(0))\
        return PIGLIT_FAIL;\
\
    glGetnPixelMap##t##vARB(GL_PIXEL_MAP_R_TO_R, bufSize, v);\
    if (!succeeded(offby))\
        return PIGLIT_FAIL;\
    glGetnPixelMap##t##vARB(GL_PIXEL_MAP_G_TO_G, bufSize, v);\
    if (!succeeded(offby))\
        return PIGLIT_FAIL;\
    glGetnPixelMap##t##vARB(GL_PIXEL_MAP_B_TO_B, bufSize, v);\
    if (!succeeded(offby))\
        return PIGLIT_FAIL;\
    glGetnPixelMap##t##vARB(GL_PIXEL_MAP_A_TO_A, bufSize, v);\
    if (!succeeded(offby))\
        return PIGLIT_FAIL;\
} while (0)

    TEST_PIXMAP(float, f);
    TEST_PIXMAP(uint, ui);
    TEST_PIXMAP(ushort, us);

#undef TEST_PIXMAP
#undef MAPSIZE
  return PIGLIT_PASS;
}

static enum piglit_result
test_readpix(int offby)
{
#define TEST_READPIX(gltype, enumtype) \
do {\
    GL##gltype v[4*width*height];\
    GLsizei bufSize = offby + (int)(sizeof v);\
\
    memset(v, 0, sizeof v);\
    glClear(GL_COLOR_BUFFER_BIT);\
\
    glReadnPixelsARB(0, 0, width, height, GL_RGBA, GL_##enumtype, bufSize, v);\
    if (!succeeded(offby))\
        return PIGLIT_FAIL;\
    glReadnPixelsARB(1, 1, width, height, GL_RGBA, GL_##enumtype, bufSize, v);\
    if (!succeeded(offby))\
        return PIGLIT_FAIL;\
} while (0)

  TEST_READPIX(float, FLOAT);
  TEST_READPIX(int, INT);
  TEST_READPIX(byte, BYTE);
#undef TEST_READPIX
  return PIGLIT_PASS;
}

static enum piglit_result
test_stipple(int offby)
{
    GLubyte pattern[4*32];
    GLsizei bufSize = offby + (int)(sizeof pattern);
    unsigned i;

    for (i = 0; i < sizeof pattern; i++)
        pattern[i] = 0x55;

    glPolygonStipple(pattern);
    if (!succeeded(0))
        return PIGLIT_FAIL;

    glGetnPolygonStippleARB(bufSize, pattern);
    if (!succeeded(offby))
        return PIGLIT_FAIL;

    return PIGLIT_PASS;
}

static enum piglit_result
test_teximage1d(int offby);
static enum piglit_result
test_teximage2d(int offby);
static enum piglit_result
test_teximage3d(int offby);

static enum piglit_result
test_teximage(int offby)
{
    if (test_teximage1d(offby) != PIGLIT_PASS)
        return PIGLIT_FAIL;
    if (test_teximage2d(offby) != PIGLIT_PASS)
        return PIGLIT_FAIL;
    if (test_teximage3d(offby) != PIGLIT_PASS)
        return PIGLIT_FAIL;

    return PIGLIT_PASS;
}

static enum piglit_result
test_teximage1d(int offby)
{
#define TEST_TEX1D(gltype, enumtype)\
do {\
    GL##gltype v[4*width];\
    GLsizei bufSize = offby + (int)(sizeof v);\
\
    memset(v, 0, sizeof v);\
    glClear(GL_COLOR_BUFFER_BIT);\
\
    glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA, width,\
                 0, GL_RGBA, GL_##enumtype, v);\
    if (!succeeded(0))\
        return PIGLIT_FAIL;\
\
    glGetnTexImageARB(GL_TEXTURE_1D, 0, GL_RGBA, GL_##enumtype, bufSize, v);\
    if (!succeeded(offby))\
        return PIGLIT_FAIL;\
} while(0)

    TEST_TEX1D(float, FLOAT);
    TEST_TEX1D(int, INT);
    TEST_TEX1D(byte, BYTE);
#undef TEST_TEX1D
    return PIGLIT_PASS;
}

static enum piglit_result
test_teximage2d(int offby)
{
#define TEST_TEX2D(gltype, enumtype)\
do {\
    GL##gltype v[4*width*height];\
    GLsizei bufSize = offby + (int)(sizeof v);\
\
    memset(v, 0, sizeof v);\
    glClear(GL_COLOR_BUFFER_BIT);\
\
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height,\
                 0, GL_RGBA, GL_##enumtype, v);\
    if (!succeeded(0))\
        return PIGLIT_FAIL;\
\
    glGetnTexImageARB(GL_TEXTURE_2D, 0, GL_RGBA, GL_##enumtype, bufSize, v);\
    if (!succeeded(offby))\
        return PIGLIT_FAIL;\
} while(0)

    TEST_TEX2D(float, FLOAT);
    TEST_TEX2D(int, INT);
    TEST_TEX2D(byte, BYTE);
#undef TEST_TEX2D
    return PIGLIT_PASS;
}

static enum piglit_result
test_teximage3d(int offby)
{
#define TEST_TEX3D(gltype, enumtype)\
do {\
    GL##gltype v[4*width*height*depth];\
    GLsizei bufSize = offby + (int)(sizeof v);\
\
    memset(v, 0, sizeof v);\
    glClear(GL_COLOR_BUFFER_BIT);\
\
    glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA, width, height, depth,\
                 0, GL_RGBA, GL_##enumtype, v);\
    if (!succeeded(0))\
        return PIGLIT_FAIL;\
\
    glGetnTexImageARB(GL_TEXTURE_3D, 0, GL_RGBA, GL_##enumtype, bufSize, v);\
    if (!succeeded(offby))\
        return PIGLIT_FAIL;\
} while(0)

    TEST_TEX3D(float, FLOAT);
    TEST_TEX3D(int, INT);
    TEST_TEX3D(byte, BYTE);
#undef TEST_TEX3D
    return PIGLIT_PASS;
}

static enum piglit_result
test(int offby)
{

   /* write to client memory, not a bound buffer */
   glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, 0);
   glBindBufferARB(GL_PIXEL_PACK_BUFFER_ARB, 0);

    if (test_pixelmap(offby) != PIGLIT_PASS)
         return PIGLIT_FAIL;
    if (test_stipple(offby) != PIGLIT_PASS)
         return PIGLIT_FAIL;
    if (test_readpix(offby) != PIGLIT_PASS)
         return PIGLIT_FAIL;
    if (test_teximage(offby) != PIGLIT_PASS)
         return PIGLIT_FAIL;

    return PIGLIT_PASS;
}

enum piglit_result
piglit_display(void)
{
    int i;
    enum piglit_result res;

    glClear(GL_COLOR_BUFFER_BIT);

    for (i = -9; i <= 1; ++i) {
        res = test(i);
        assert(glGetError() == GL_NO_ERROR);
        if (res != PIGLIT_PASS)
            break;
    }

    glFinish();

    return res;
}

