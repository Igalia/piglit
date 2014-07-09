/*
 * Copyright 2010 VMware, Inc.
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

/*
 * Test building a mipmap in various orders:
 *   From largest to smallest
 *   From smallest to largest
 *   Random order.
 *
 * Author: Brian Paul
 */


#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_width = 200;
	config.window_height = 200;
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGB;

PIGLIT_GL_TEST_CONFIG_END

#define MAX_LEVEL 7

enum order {
   NORMAL,
   REVERSE,
   RANDOM
};


static GLubyte
level_intensity(GLint level)
{
   return 100 + level * 20;
}


static void
setup_tex_image(GLint level)
{
   GLint size = 1 << (MAX_LEVEL - level);
   GLubyte *img = malloc(size * size * 4);
   GLubyte val = level_intensity(level);
   memset(img, val, size * size * 4);
   glTexImage2D(GL_TEXTURE_2D, level, GL_RGB, size, size, 0,
                GL_RGBA, GL_UNSIGNED_BYTE, img);
   free(img);
}


/*
 * Generate a mipmapped texture.
 * Define the mipmap levels/images in the order specified by 'ord'.
 */
static void
generate_mipmap(enum order ord)
{
   GLuint tex;
   GLint level;

   glGenTextures(1, &tex);
   glBindTexture(GL_TEXTURE_2D, tex);

   /* hint to the driver that there won't be a mipmap (but that's a lie) */
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

   /* Set each mipmap level to a different intensity, starting with the
    * smallest mipmap level and ending with the largest at level=0.
    */
   if (ord == NORMAL) {
      for (level = 0; level <= MAX_LEVEL; level++) {
         setup_tex_image(level);
      }
   }
   else if (ord == REVERSE) {
      for (level = MAX_LEVEL; level >= 0; level--) {
         setup_tex_image(level);
      }
   }
   else {
      static const GLuint rand[MAX_LEVEL + 1] = {3, 1, 2, 4, 7, 0, 6, 5};
      assert(ord == RANDOM);

      for (level = 0; level <= MAX_LEVEL; level++) {
         setup_tex_image(rand[level]);
      }
   }

   /* use mipmap filtering */
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}


static GLboolean
test(enum order ord, const char *order)
{
   const int px = piglit_width / 2, py = piglit_height / 2;
   GLboolean pass = GL_TRUE;
   GLuint level;

   generate_mipmap(ord);

   glEnable(GL_TEXTURE_2D);

   /* render a test polygon for each mipmap level */
   for (level = 0; level <= MAX_LEVEL; level++) {
      float expected[4];
      int p;

      expected[0] = expected[1] = expected[2] = level_intensity(level) / 255.0;

      /* force sampling from a specific mipmap level */
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_LOD, level);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LOD, level);

      glClear(GL_COLOR_BUFFER_BIT);

      glBegin(GL_POLYGON);
      glTexCoord2f(0, 0);
      glVertex2f(-1, -1);
      glTexCoord2f(1, 0);
      glVertex2f(1, -1);
      glTexCoord2f(1, 1);
      glVertex2f(1, 1);
      glTexCoord2f(0, 1);
      glVertex2f(-1, 1);
      glEnd();

      p = piglit_probe_pixel_rgb(px, py, expected);
      if (!p) {
         printf("  At mipmap level %d, order = %s\n", level, order);
      }

      pass &= p;
      
      piglit_present_results();
   }

   return pass;
}


void
piglit_init(int argc, char **argv)
{
   /* nothing */
}


enum piglit_result
piglit_display(void)
{
   if (!test(NORMAL, "Normal"))
      return PIGLIT_FAIL;

   if (!test(REVERSE, "Reverse"))
      return PIGLIT_FAIL;

   if (!test(RANDOM, "Random"))
      return PIGLIT_FAIL;

   return PIGLIT_PASS;
}
