/*
 * Copyright (c) 2015 Intel Corporation
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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

/*
 * Test texture upload with scale and bias pixel transfer options
 *
 * Iago Toral Quiroga <itoral@igalia.com>
 * Feb 13, 2015
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

        config.supports_gl_compat_version = 10;

        config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGB;

PIGLIT_GL_TEST_CONFIG_END

static const GLfloat colors[4] = { 0.5, 0.25, 0.1, 0.5 };
static const GLfloat scale[4] = { 2.0, 3.0, 1.0, 1.0 };
static const GLfloat bias[4] = { -0.25, 0.0, 0.4, 0.0 };

static GLuint
create_texture()
{
        GLuint tex, size, i;
        GLfloat *image;

        glGenTextures(1, &tex);

        glBindTexture(GL_TEXTURE_2D, tex);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        /* Upload texture color data and apply a bias and scale
         */
        glPixelTransferf(GL_RED_SCALE, scale[0]);
        glPixelTransferf(GL_RED_BIAS, bias[0]);
        glPixelTransferf(GL_GREEN_SCALE, scale[1]);
        glPixelTransferf(GL_GREEN_BIAS, bias[1]);
        glPixelTransferf(GL_BLUE_SCALE, scale[2]);
        glPixelTransferf(GL_BLUE_BIAS, bias[2]);
        glPixelTransferf(GL_ALPHA_SCALE, scale[3]);
        glPixelTransferf(GL_ALPHA_BIAS, bias[3]);

        size = 64;
        image = malloc(size * size * 4 * sizeof(GLfloat));
        for (i = 0; i < size * size; i++) {
                image[i*4+0] = colors[0];
                image[i*4+1] = colors[1];
                image[i*4+2] = colors[2];
                image[i*4+3] = colors[3];
        }

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                     size, size, 0, GL_RGBA, GL_FLOAT, image);

        /* Reset pixel transfer to defaults */
        glPixelTransferf(GL_RED_SCALE, 1.0);
        glPixelTransferf(GL_RED_BIAS, 0.0);
        glPixelTransferf(GL_GREEN_SCALE, 1.0);
        glPixelTransferf(GL_GREEN_BIAS, 0.0);
        glPixelTransferf(GL_BLUE_SCALE, 1.0);
        glPixelTransferf(GL_BLUE_BIAS, 0.0);
        glPixelTransferf(GL_ALPHA_SCALE, 1.0);
        glPixelTransferf(GL_ALPHA_BIAS, 0.0);

        free(image);

        return tex;
}


enum piglit_result
piglit_display(void)
{
        GLuint tex;
        GLboolean pass;
        GLfloat expected[4];
        int i;

        /* Create a texture and upload color data with scale and bias options */
        glEnable(GL_TEXTURE_2D);
        tex = create_texture();

        /* Render with our texture */
        glClear(GL_COLOR_BUFFER_BIT);
        glBindTexture(GL_TEXTURE_2D, tex);
        piglit_draw_rect_tex(0, 0, piglit_width, piglit_height,
                             0.0, 0.0, 1.0, 1.0);

        /* Compute expected color result after applying scale and bias.
         * Read back pixels from the framebuffer and check that they have scale
         * and bias applied.
         */
        for (i = 0; i < 4; i++)
                expected[i] = colors[i] * scale[i] + bias[i];

        pass = piglit_probe_pixel_rgba(piglit_width/2, piglit_height/2, expected);

        glDeleteTextures(1, &tex);
        glDisable(GL_TEXTURE_2D);

        return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}


void
piglit_init(int argc, char **argv)
{
        piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);
        glClearColor(0.0, 0.0, 0.0, 1.0);
}
