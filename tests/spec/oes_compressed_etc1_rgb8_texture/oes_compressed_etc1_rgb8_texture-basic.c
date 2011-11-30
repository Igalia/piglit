/*
 * Copyright © 2011 LunarG, Inc.
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
 */

#include "piglit-util.h"

#ifndef GL_OES_compressed_ETC1_RGB8_texture
#define GL_ETC1_RGB8_OES                                        0x8D64
#endif

#define CLAMP(x, min, max) ((x) < (min) ? (min) : ((x) > (max) ? (max) : (x)))

int piglit_width = 100;
int piglit_height = 100;
int piglit_window_mode = GLUT_RGB | GLUT_DOUBLE;

struct etc1_rgb8_texture_8x8 {
    GLubyte data[32];
    GLubyte rgb[64][3];
};

static void
make_etc1_rgb8_texture_8x8(struct etc1_rgb8_texture_8x8 *tex)
{
    const GLubyte base_colors[4][3] = {
        { 0xff, 0x00, 0x00 }, /* red */
        { 0x00, 0xff, 0x00 }, /* green */
        { 0x10, 0x00, 0x00 }, /* R=16 */
        { 0x00, 0x00, 0x00 }  /* black */
    };
    const int modifiers[4] = {
          2, /* modifier table 0, pixel index 0 */
         17, /* modifier table 1, pixel index 1 */
         -9, /* modifier table 2, pixel index 2 */
        -42  /* modifier table 3, pixel index 3 */
    };
    GLubyte blocks[4][8];
    unsigned i;

    /* block 0: red and green, individual, modifier table 0, not flipped */
    blocks[0][0] = 0xf0;
    blocks[0][1] = 0x0f;
    blocks[0][2] = 0x0;
    blocks[0][3] = 0x0;
    /* set all pixel indices to 0 */
    blocks[0][4] = 0x0;
    blocks[0][5] = 0x0;
    blocks[0][6] = 0x0;
    blocks[0][7] = 0x0;

    /* block 1: red and green, individual, modifier table 1, flipped */
    blocks[1][0] = blocks[0][0];
    blocks[1][1] = blocks[0][1];
    blocks[1][2] = blocks[0][2];
    blocks[1][3] = 0x25;
    /* set all pixel indices to 1 */
    blocks[1][4] = 0x0;
    blocks[1][5] = 0x0;
    blocks[1][6] = 0xff;
    blocks[1][7] = 0xff;

    /* block 2: R=16 and black, differential, modifier table 2, not flipped */
    blocks[2][0] = 0x16;
    blocks[2][1] = 0x0;
    blocks[2][2] = 0x0;
    blocks[2][3] = 0x4a;
    /* set all pixel indices to 2 */
    blocks[2][4] = 0xff;
    blocks[2][5] = 0xff;
    blocks[2][6] = 0x0;
    blocks[2][7] = 0x0;

    /* block 3: R=16 and black, differential, modifier table 3, flipped */
    blocks[3][0] = blocks[2][0];
    blocks[3][1] = blocks[2][1];
    blocks[3][2] = blocks[2][2];
    blocks[3][3] = 0x6f;
    /* set all pixel indices to 3 */
    blocks[3][4] = 0xff;
    blocks[3][5] = 0xff;
    blocks[3][6] = 0xff;
    blocks[3][7] = 0xff;

    memcpy(tex->data, blocks, sizeof(blocks));

    /* decode */
    for (i = 0; i < 64; i++) {
        unsigned x, y, b, bx, by, sub;
        const GLubyte *base_color;
        int modifier;

        x = i % 8;
        y = i / 8;

        /* block number */
        b = (x < 4) ? 0 : 1;
        if (y >= 4)
            b += 2;

        /* block coordinates */
        bx = x % 4;
        by = y % 4;

        /* subblock number */
        sub = (blocks[b][3] & 0x1) ? (by >= 2) : (bx >= 2);

        if (b < 2)
            base_color = base_colors[sub];
        else
            base_color = base_colors[2 + sub];

        modifier = modifiers[b];

        tex->rgb[i][0] = CLAMP((int) base_color[0] + modifier, 0, 255);
        tex->rgb[i][1] = CLAMP((int) base_color[1] + modifier, 0, 255);
        tex->rgb[i][2] = CLAMP((int) base_color[2] + modifier, 0, 255);
    }
}

static int
test_etc1_rgb8_texture_8x8(const struct etc1_rgb8_texture_8x8 *tex)
{
    const GLenum format = GL_ETC1_RGB8_OES;
    const GLsizei width = 8, height = 8;
    unsigned x, y;
    int pass;

    glCompressedTexImage2D(GL_TEXTURE_2D, 0,
            format, width, height, 0, sizeof(tex->data), tex->data);
    piglit_check_gl_error(GL_NO_ERROR, PIGLIT_FAIL);

    glEnable(GL_TEXTURE_2D);

    piglit_draw_rect_tex(0, 0, width, height, 0.0f, 0.0f, 1.0f, 1.0f);

    glDisable(GL_TEXTURE_2D);

    for (y = 0; y < height; y++) {
        for (x = 0; x < width; x++) {
            const GLubyte *rgb = tex->rgb[y * width + x];
            float expected[3] = {
                rgb[0] / 255.0f,
                rgb[1] / 255.0f,
                rgb[2] / 255.0f
            };

            pass = piglit_probe_pixel_rgb(x, y, expected) && pass;
        }
    }

    return pass;
}

enum piglit_result
piglit_display(void)
{
    const GLenum format = GL_ETC1_RGB8_OES;
    const GLsizei width = 8, height = 8;
    struct etc1_rgb8_texture_8x8 tex;
    GLuint t;
    int pass;

    glGenTextures(1, &t);
    glBindTexture(GL_TEXTURE_2D, t);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    make_etc1_rgb8_texture_8x8(&tex);

    glClear(GL_COLOR_BUFFER_BIT);

    /* no compression support */
    glTexImage2D(GL_TEXTURE_2D, 0, format,
            width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, tex.rgb);
    piglit_check_gl_error(GL_INVALID_VALUE, PIGLIT_FAIL);

    glCopyTexImage2D(GL_TEXTURE_2D, 0, format, 0, 0, width, height, 0);
    piglit_check_gl_error(GL_INVALID_VALUE, PIGLIT_FAIL);

    /* test the texture */
    pass = test_etc1_rgb8_texture_8x8(&tex);

    /* no subimage support */
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0,
            width, height, GL_RGB, GL_UNSIGNED_BYTE, tex.rgb);
    piglit_check_gl_error(GL_INVALID_OPERATION, PIGLIT_FAIL);

    glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, width, height);
    piglit_check_gl_error(GL_INVALID_OPERATION, PIGLIT_FAIL);

    glCompressedTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, format, sizeof(tex.data), tex.data);
    piglit_check_gl_error(GL_INVALID_OPERATION, PIGLIT_FAIL);

    glDeleteTextures(1, &t);

    return (pass) ? PIGLIT_PASS : PIGLIT_FAIL;;
}

void
piglit_init(int argc, char **argv)
{
    piglit_require_extension("GL_OES_compressed_ETC1_RGB8_texture");
    piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);
}
