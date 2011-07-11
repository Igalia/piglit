/*
 * Copyright (c) 2011 VMware, Inc.
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

/**
 * Test rendering to a depth texture, sampling from it, and comparing the
 * texture depth values against the fragment Z values when drawing the
 * same object a second time.
 *
 * The left side of the window should be mostly black.  Red pixels indicate
 * errors.
 * The center and right parts of the window should show gray-scale spheres
 * on a white background (they're just Z buffer images as gray-scale).
 *
 * Brian Paul
 * 17 Feb 2011
 */


#include <assert.h>
#include "piglit-util.h"


#define SIZE 256


int piglit_width = 3 * SIZE;
int piglit_height = SIZE;
int piglit_window_mode = GLUT_DOUBLE | GLUT_DEPTH;

static GLfloat ErrorScale = 0.0;
static GLuint ColorTex, DepthTex, FBO;
static GLuint ShaderProg;
static GLint Zbits;


static void
create_fbo(void)
{
   GLenum depthIntFormat = GL_DEPTH_COMPONENT24;
   GLenum status;

   /* depth texture */
   glGenTextures(1, &DepthTex);
   glBindTexture(GL_TEXTURE_2D, DepthTex);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
   glTexImage2D(GL_TEXTURE_2D, 0, depthIntFormat,
                SIZE, SIZE, 0,
                GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, NULL);
   assert(glGetError() == 0);
   glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_DEPTH_SIZE, &Zbits);

   /* color texture */
   glGenTextures(1, &ColorTex);
   glBindTexture(GL_TEXTURE_2D, ColorTex);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
   glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                SIZE, SIZE, 0,
                GL_RGBA, GL_UNSIGNED_BYTE, NULL);
   assert(glGetError() == 0);

   /* Create FBO */
   glGenFramebuffersEXT(1, &FBO);
   glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, FBO);

   glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
                             GL_COLOR_ATTACHMENT0_EXT,
                             GL_TEXTURE_2D,
                             ColorTex,
                             0);

   glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
                             GL_DEPTH_ATTACHMENT_EXT,
                             GL_TEXTURE_2D,
                             DepthTex,
                             0);

   assert(glGetError() == 0);

   status = glCheckFramebufferStatusEXT (GL_FRAMEBUFFER_EXT);
   if (status != GL_FRAMEBUFFER_COMPLETE_EXT) {
      piglit_report_result(PIGLIT_SKIP);
   }
}


static void
create_frag_shader(void)
{
   /* This shader samples the currently bound depth texture, then compares
    * that value to the current fragment Z value to produce a shade of red
    * indicating error/difference.
    *
    * E.g:  gl_FragColor = scale * abs(texture.Z - fragment.Z);
    *
    * Note that we have to be pretty careful with converting gl_FragCoord
    * into a texture coordinate.  There's a -0.5 bias and scale factor.
    */
   static const char *text =
      "uniform sampler2D zTex; \n"
      "uniform float sizeScale; \n"
      "uniform float errorScale; \n"
      "void main() \n"
      "{ \n"
      "   vec2 coord = (gl_FragCoord.xy - vec2(0.5)) / sizeScale; \n"
      "   vec4 z = texture2D(zTex, coord); \n"
      "   float diff = errorScale * abs(z.r - gl_FragCoord.z); \n"
      "   //gl_FragColor = vec4(gl_FragCoord.z, 0, 0, 0); \n"
      "   //gl_FragColor = z; \n"
      "   gl_FragColor = vec4(diff, 0, 0, 0); \n"
      "} \n";
   GLuint fs;
   GLint zTex, errorScale, sizeScale;

   fs = piglit_compile_shader_text(GL_FRAGMENT_SHADER, text);
   assert(fs);

   ShaderProg = piglit_link_simple_program(0, fs);
   assert(ShaderProg);

   piglit_UseProgram(ShaderProg);

   zTex = piglit_GetUniformLocation(ShaderProg, "zTex");
   piglit_Uniform1i(zTex, 0);  /* unit 0 */

   errorScale = piglit_GetUniformLocation(ShaderProg, "errorScale");
   piglit_Uniform1f(errorScale, ErrorScale);

   sizeScale = piglit_GetUniformLocation(ShaderProg, "sizeScale");
   piglit_Uniform1f(sizeScale, (float) (SIZE - 1));

   piglit_UseProgram(0);
}



static void
render_to_fbo(void)
{
   glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, FBO);

   glViewport(0, 0, SIZE, SIZE);

   glEnable(GL_DEPTH_TEST);

   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();
   glOrtho(-1.0, 1.0, -1.0, 1.0, -1, 1.0);

   glColor4f(1.0, 0.0, 0.0, 0.0);
   glutSolidSphere(0.95, 40, 20);

   glDisable(GL_DEPTH_TEST);

   glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
}


/** Show contents of depth buffer in middle of window */
static void
show_depth_fbo(void)
{
   GLfloat *z = (GLfloat *) malloc(SIZE * SIZE * sizeof(GLfloat));

   glViewport(1 * SIZE, 0, SIZE, SIZE); /* not really needed */

   glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, FBO);
   glReadPixels(0, 0, SIZE, SIZE, GL_DEPTH_COMPONENT, GL_FLOAT, z);

   glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

   glWindowPos2i(SIZE, 0);
   glDrawPixels(SIZE, SIZE, GL_LUMINANCE, GL_FLOAT, z);
   assert(glGetError() == 0);
   free(z);
}


/** Draw quad textured with depth image on right side of window */
static void
draw_quad_with_depth_texture(void)
{
   glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

   glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

   glViewport(2 * SIZE, 0, SIZE, SIZE);

   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();
   glOrtho(-1.0, 1.0, -1.0, 1.0, -1, 1.0);

   glBindTexture(GL_TEXTURE_2D, DepthTex);
   glEnable(GL_TEXTURE_2D);

   glBegin(GL_POLYGON);
   glTexCoord2f(0, 0);
   glVertex2f(-1, -1);
   glTexCoord2f(1, 0);
   glVertex2f( 1, -1);
   glTexCoord2f(1, 1);
   glVertex2f( 1,  1);
   glTexCoord2f(0, 1);
   glVertex2f(-1,  1);
   glEnd();

   glDisable(GL_TEXTURE_2D);
}


/**
 * Draw quad with fragment shader that compares fragment.z against the
 * depth texture value (draw on left side of window).
 * We draw on the left side of the window to easily convert gl_FragCoord
 * into a texture coordinate.
 */
static void
draw_sphere_with_fragment_shader_compare(void)
{
   glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

   glViewport(0 * SIZE, 0, SIZE, SIZE);

   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();

   glBindTexture(GL_TEXTURE_2D, DepthTex);

   piglit_UseProgram(ShaderProg);

   glOrtho(-1.0, 1.0, -1.0, 1.0, -1, 1.0);

   glEnable(GL_DEPTH_TEST);

   if (1) {
      glutSolidSphere(0.95, 40, 20);
   }
   else {
      /* To test using gl_TexCoord[0].xy instead of gl_FragCoord.xy in the shader
       */
      static const GLfloat sPlane[4] = {0.5, 0, 0, 0.5};
      static const GLfloat tPlane[4] = {0, 0.5, 0, 0.5};

      glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
      glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
      glTexGenfv(GL_S, GL_EYE_PLANE, sPlane);
      glTexGenfv(GL_T, GL_EYE_PLANE, tPlane);
      glEnable(GL_TEXTURE_GEN_S);
      glEnable(GL_TEXTURE_GEN_T);
      
      glutSolidSphere(0.95, 40, 20);

      glDisable(GL_TEXTURE_GEN_S);
      glDisable(GL_TEXTURE_GEN_T);
   }

   glDisable(GL_DEPTH_TEST);

   piglit_UseProgram(0);
}


static enum piglit_result
count_and_report_bad_pixels(void)
{
   GLubyte *z;
   GLuint error = 0;
   int i;

   z = (GLubyte *) malloc(SIZE * SIZE * 4 * sizeof(GLubyte));
   glReadPixels(0, 0, SIZE, SIZE, GL_RGBA, GL_UNSIGNED_BYTE, z);

   for (i = 0; i < SIZE * SIZE * 4; i += 4) {
      error += z[i];
   }
   free(z);

   if (!piglit_automatic)
      printf("total error = %u\n", error);

   /* XXX this error test is a total hack for now */
   if (error > ErrorScale)
      return PIGLIT_FAIL;
   else
      return PIGLIT_PASS;
}


enum piglit_result
piglit_display(void)
{
   enum piglit_result result;

   render_to_fbo();

   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

   show_depth_fbo();

   draw_quad_with_depth_texture();

   draw_sphere_with_fragment_shader_compare();

   result = count_and_report_bad_pixels();

   glutSwapBuffers();

   return result;
}


void
piglit_init(int argc, char **argv)
{
   if (argc > 1) {
      ErrorScale = atof(argv[1]);
   }

   piglit_require_extension("GL_EXT_framebuffer_object");
   piglit_require_fragment_shader();

   create_fbo();

   if (ErrorScale == 0.0) {
      /* A 1-bit error/difference in Z values results in a delta of 64 in
       * pixel intensity (where pixels are in [0,255]).
       */
      ErrorScale = ((double) (1ull << Zbits)) * 64.0 / 255.0;
   }

   create_frag_shader();

   if (!piglit_automatic) {
      printf("GL_RENDERER = %s\n", (char *) glGetString(GL_RENDERER));
      printf("Left: Shader showing difference pixels (black=good, red=error)\n");
      printf("Middle: Depth buffer of FBO\n");
      printf("Right: Quad textured with depth values\n");
      printf("Z bits = %d\n", Zbits);
      printf("ErrorScale = %f\n", ErrorScale);
   }
}
