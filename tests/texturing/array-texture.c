/*
 * Copyright (c) 2010 VMware, Inc.
 *
 * Permission is hereby , free of charge, to any person obtaining a
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
 *
 *
 * Test GL_EXT_texture_array and GL_MESA_texture_array.
 * Note that the Mesa extension works with fixed-function fragment
 * processing whereas the EXT version only works with shaders.
 *
 * Author: Brian Paul
 */


#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_width = 700;
	config.window_height = 400;
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGB;

PIGLIT_GL_TEST_CONFIG_END

int height = 100, ybase = 0;

static const char *prog = "array-texture";

static GLboolean have_MESA_texture_array;

static GLuint array_tex_1d;
static GLuint array_tex_2d;

/*
 * We'll set each texture slice to a different solid color.
 * XXX a better test would vary the color within each slice too.
 */
#define NUM_COLORS 7

static GLfloat colors[NUM_COLORS][3] = {
   {1.0, 0.0, 0.0},
   {0.0, 1.0, 0.0},
   {0.0, 0.0, 1.0},
   {0.0, 0.0, 1.0},
   {0.0, 1.0, 0.0},
   {1.0, 1.0, 0.0},
   {1.0, 1.0, 1.0}
};


static const char *frag_shader_2d_array_text =
   "#extension GL_EXT_texture_array : enable \n"
   "uniform sampler2DArray tex; \n"
   "void main() \n"
   "{ \n"
   "   gl_FragColor = texture2DArray(tex, gl_TexCoord[0].xyz); \n"
   "} \n";

static GLuint frag_shader_2d_array;
static GLuint program_2d_array;


static const char *frag_shader_1d_array_text =
   "#extension GL_EXT_texture_array : enable \n"
   "uniform sampler1DArray tex; \n"
   "void main() \n"
   "{ \n"
   "   gl_FragColor = texture1DArray(tex, gl_TexCoord[0].xy); \n"
   "} \n";

static GLuint frag_shader_1d_array;
static GLuint program_1d_array;



/* debug aid */
static void
check_error(int line)
{
   GLenum err = glGetError();
   if (err) {
      printf("%s: GL error 0x%x at line %d\n", prog, err, line);
   }
}


static GLuint
make_2d_array_texture(void)
{
   GLfloat img[NUM_COLORS][64][32][4];
   GLuint tex;
   int i, j, k;

   for (i = 0; i < NUM_COLORS; i++) {
      for (j = 0; j < 64; j++) {
         for (k = 0; k < 32; k++) {
            img[i][j][k][0] = colors[i][0];
            img[i][j][k][1] = colors[i][1];
            img[i][j][k][2] = colors[i][2];
            img[i][j][k][3] = 1.0;
         }
      }
   }

   glGenTextures(1, &tex);

   glBindTexture(GL_TEXTURE_2D_ARRAY_EXT, tex);
   check_error(__LINE__);

   glTexImage3D(GL_TEXTURE_2D_ARRAY_EXT, 0, GL_RGBA,
                32, 64, NUM_COLORS, /* w, h, d */
                0, /* border */
                GL_RGBA, GL_FLOAT, img);

   glTexParameteri(GL_TEXTURE_2D_ARRAY_EXT, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
   glTexParameteri(GL_TEXTURE_2D_ARRAY_EXT, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
   check_error(__LINE__);

   return tex;
}


static GLuint
make_1d_array_texture(void)
{
   GLfloat img[NUM_COLORS][16][4];
   GLuint tex;
   int i, j;

   for (i = 0; i < NUM_COLORS; i++) {
      for (j = 0; j < 16; j++) {
         img[i][j][0] = colors[i][0];
         img[i][j][1] = colors[i][1];
         img[i][j][2] = colors[i][2];
         img[i][j][3] = 1.0;
      }
   }

   glGenTextures(1, &tex);

   glBindTexture(GL_TEXTURE_1D_ARRAY_EXT, tex);
   check_error(__LINE__);

   glTexImage2D(GL_TEXTURE_1D_ARRAY_EXT, 0, GL_RGBA,
                16, NUM_COLORS, /* w, depth */
                0, /* border */
                GL_RGBA, GL_FLOAT, img);

   glTexParameteri(GL_TEXTURE_1D_ARRAY_EXT, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
   glTexParameteri(GL_TEXTURE_1D_ARRAY_EXT, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
   check_error(__LINE__);

   return tex;
}


static GLboolean
test_2d_array_texture(GLuint tex)
{
   GLboolean pass, ret = GL_TRUE;
   int i;
   float width = piglit_width / NUM_COLORS;
   float x = 0.0;

   glBindTexture(GL_TEXTURE_2D_ARRAY_EXT, tex);
   /* render each image in the array, check its color */
   for (i = 0; i < NUM_COLORS; i++) {
      GLfloat r = (GLfloat) i;

      glBegin(GL_POLYGON);
      glTexCoord3f(0, 0, r);  glVertex2f(x, ybase);
      glTexCoord3f(1, 0, r);  glVertex2f(x + width, ybase);
      glTexCoord3f(1, 1, r);  glVertex2f(x + width, ybase + height);
      glTexCoord3f(0, 1, r);  glVertex2f(x, ybase + height);
      glEnd();

      pass = piglit_probe_pixel_rgb(x + (width / 2), ybase + (height / 2),
				                       colors[i]);

      x += width;
      if (!pass) {
         printf("%s: failed for 2D image/slice %d\n", prog, i);
      }

      ret &= pass;
   }

   glBindTexture(GL_TEXTURE_2D_ARRAY_EXT, 0);

   return ret;
}


static GLboolean
test_1d_array_texture(GLuint tex)
{
   GLboolean pass, ret = GL_TRUE;
   int i;
   float width = piglit_width / NUM_COLORS;
   float x = 0.0;

   glBindTexture(GL_TEXTURE_1D_ARRAY_EXT, tex);

   /* render each image in the array, check its color */
   for (i = 0; i < NUM_COLORS; i++) {
      GLfloat r = (GLfloat) i;

      glBegin(GL_POLYGON);
      glTexCoord2f(0, r);  glVertex2f(x, ybase);
      glTexCoord2f(1, r);  glVertex2f(x + width, ybase);
      glTexCoord2f(1, r);  glVertex2f(x + width, ybase + height);
      glTexCoord2f(0, r);  glVertex2f(x, ybase + height);
      glEnd();

      glFinish();

      pass = piglit_probe_pixel_rgb(x + (width / 2), ybase + (height / 2),
				    colors[i]);

      x += width;

      if (!pass) {
         printf("%s: failed for 1D image/slice %d\n", prog, i);
      }

      ret &= pass;
   }

   glBindTexture(GL_TEXTURE_1D_ARRAY_EXT, 0);

   return ret;
}


enum piglit_result
piglit_display(void)
{
   GLboolean pass = GL_TRUE;
   GLint loc;

   if (!frag_shader_2d_array) {
      printf("%s: failed to compile 2D fragment shader.\n", prog);
      return PIGLIT_FAIL;
   }

   if (!program_2d_array) {
      printf("%s: failed to link 2D shader program.\n", prog);
      return PIGLIT_FAIL;
   }

   if (!frag_shader_1d_array) {
      printf("%s: failed to compile 1D fragment shader.\n", prog);
      return PIGLIT_FAIL;
   }

   if (!program_1d_array) {
      printf("%s: failed to link 1D shader program.\n", prog);
      return PIGLIT_FAIL;
   }


   piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);

   ybase = 0;
   /*
    * Test 2d array texture with fragment shader
    */
   {
      glClear(GL_COLOR_BUFFER_BIT);
      glUseProgram(program_2d_array);
      loc = glGetUniformLocation(program_2d_array, "tex");
      glUniform1i(loc, 0); /* texture unit p */
      pass &= test_2d_array_texture(array_tex_2d);
      glUseProgram(0);
   }

   ybase = 100;
   /*
    * Test 2d array texture with fixed function
    */
   if (have_MESA_texture_array) {
      glEnable(GL_TEXTURE_2D_ARRAY_EXT);
      check_error(__LINE__);
      pass &= test_2d_array_texture(array_tex_2d);
      glDisable(GL_TEXTURE_2D_ARRAY_EXT);
      check_error(__LINE__);
   }

   ybase = 200;
   /*
    * Test 1d array texture with fragment shader
    */
   {
      glUseProgram(program_1d_array);
      loc = glGetUniformLocation(program_1d_array, "tex");
      glUniform1i(loc, 0); /* texture unit p */
      pass &= test_1d_array_texture(array_tex_1d);
      glUseProgram(0);
   }

   ybase = 300;
   /*
    * Test 1d array texture with fixed function
    */
   if (have_MESA_texture_array) {
      glEnable(GL_TEXTURE_1D_ARRAY_EXT);
      check_error(__LINE__);
      pass &= test_1d_array_texture(array_tex_1d);
      glDisable(GL_TEXTURE_1D_ARRAY_EXT);
      check_error(__LINE__);
   }

   piglit_present_results();
   return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}


void
piglit_init(int argc, char **argv)
{
   piglit_require_extension("GL_EXT_texture_array");

   have_MESA_texture_array = piglit_is_extension_supported("GL_MESA_texture_array");

   /* Make shader programs */
   frag_shader_2d_array =
      piglit_compile_shader_text(GL_FRAGMENT_SHADER,
                                 frag_shader_2d_array_text);
   check_error(__LINE__);

   program_2d_array = piglit_link_simple_program(0, frag_shader_2d_array);
   check_error(__LINE__);

   frag_shader_1d_array =
      piglit_compile_shader_text(GL_FRAGMENT_SHADER,
                                 frag_shader_1d_array_text);
   check_error(__LINE__);

   program_1d_array = piglit_link_simple_program(0, frag_shader_1d_array);
   check_error(__LINE__);

   /* make array textures */
   array_tex_2d = make_2d_array_texture();
   array_tex_1d = make_1d_array_texture();
}
