/*
 * Copyright © 2011 Marek Olšák <maraeo@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
 * AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_width = 250;
	config.window_height = 70;
	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

static const float colors[6][3] = {
   {1, 0, 0},
   {0, 1, 1},
   {0, 1, 0},
   {1, 0, 1},
   {0, 0, 1},
   {1, 1, 0}
};

static const GLenum targets[6] = {
   GL_TEXTURE_CUBE_MAP_POSITIVE_X_ARB,
   GL_TEXTURE_CUBE_MAP_NEGATIVE_X_ARB,
   GL_TEXTURE_CUBE_MAP_POSITIVE_Y_ARB,
   GL_TEXTURE_CUBE_MAP_NEGATIVE_Y_ARB,
   GL_TEXTURE_CUBE_MAP_POSITIVE_Z_ARB,
   GL_TEXTURE_CUBE_MAP_NEGATIVE_Z_ARB
};

static void draw_quad(int x, int y, float s, float t, float r, GLuint tex, GLboolean seamless)
{
   glBindTexture(GL_TEXTURE_CUBE_MAP_ARB, tex);
   glTexParameteri(GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_CUBE_MAP_SEAMLESS, seamless);
   glBegin(GL_QUADS);
      glTexCoord3f(s, t, r);
      glVertex2i(x,    y);
      glVertex2i(x,    y+20);
      glVertex2i(x+20, y+20);
      glVertex2i(x+20, y);
   glEnd();
}

enum piglit_result piglit_display(void)
{
   GLboolean pass = GL_TRUE;
   const float violet[3] = {0.5, 0, 0.5};

   glClear(GL_COLOR_BUFFER_BIT);

   draw_quad(10, 10, 0.99, 0, 1, 1, GL_FALSE);
   draw_quad(40, 10, 1, 0, 0.99, 2, GL_FALSE);

   draw_quad(70, 10, 0.99, 0, 1, 1, GL_TRUE);
   draw_quad(100, 10, 1, 0, 0.99, 2, GL_FALSE);

   draw_quad(130, 10, 0.99, 0, 1, 1, GL_FALSE);
   draw_quad(160, 10, 1, 0, 0.99, 2, GL_TRUE);

   draw_quad(190, 10, 0.99, 0, 1, 1, GL_TRUE);
   draw_quad(220, 10, 1, 0, 0.99, 2, GL_TRUE);

   glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

   draw_quad(10, 40, 0.99, 0, 1, 1, GL_FALSE);
   draw_quad(40, 40, 1, 0, 0.99, 2, GL_FALSE);

   draw_quad(70, 40, 0.99, 0, 1, 1, GL_TRUE);
   draw_quad(100, 40, 1, 0, 0.99, 2, GL_FALSE);

   draw_quad(130, 40, 0.99, 0, 1, 1, GL_FALSE);
   draw_quad(160, 40, 1, 0, 0.99, 2, GL_TRUE);

   draw_quad(190, 40, 0.99, 0, 1, 1, GL_TRUE);
   draw_quad(220, 40, 1, 0, 0.99, 2, GL_TRUE);

   glDisable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

   pass = piglit_probe_pixel_rgb(20, 20, colors[4]) && pass;
   pass = piglit_probe_pixel_rgb(50, 20, colors[0]) && pass;

   pass = piglit_probe_pixel_rgb(80, 20, violet) && pass;
   pass = piglit_probe_pixel_rgb(110, 20, colors[0]) && pass;

   pass = piglit_probe_pixel_rgb(140, 20, colors[4]) && pass;
   pass = piglit_probe_pixel_rgb(170, 20, violet) && pass;

   pass = piglit_probe_pixel_rgb(200, 20, violet) && pass;
   pass = piglit_probe_pixel_rgb(230, 20, violet) && pass;

   pass = piglit_probe_pixel_rgb(20, 50, violet) && pass;
   pass = piglit_probe_pixel_rgb(50, 50, violet) && pass;

   pass = piglit_probe_pixel_rgb(80, 50, violet) && pass;
   pass = piglit_probe_pixel_rgb(110, 50, violet) && pass;

   pass = piglit_probe_pixel_rgb(140, 50, violet) && pass;
   pass = piglit_probe_pixel_rgb(170, 50, violet) && pass;

   pass = piglit_probe_pixel_rgb(200, 50, violet) && pass;
   pass = piglit_probe_pixel_rgb(230, 50, violet) && pass;

   piglit_present_results();

   return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void piglit_init(int argc, char **argv)
{
   GLint i, t;

   piglit_require_extension("GL_ARB_texture_cube_map");
   piglit_require_extension("GL_ARB_seamless_cube_map");
   piglit_require_extension("GL_AMD_seamless_cubemap_per_texture");

   for (t = 0; t < 2; t++) {
      glBindTexture(GL_TEXTURE_CUBE_MAP_ARB, 1+t);
      glTexParameteri(GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_CUBE_MAP_ARB, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

      for (i = 0; i < 6; i++) {
         glTexImage2D(targets[i], 0, GL_RGBA8, 1, 1, 0, GL_RGB, GL_FLOAT, colors[i]);
      }
   }

   glEnable(GL_TEXTURE_CUBE_MAP_ARB);

   glClearColor(.3, .3, .3, 0);
   glColor3f( 1.0, 1.0, 1.0 );

   piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);
}
