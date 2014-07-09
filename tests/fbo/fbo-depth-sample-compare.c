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
#include "piglit-util-gl.h"

#if defined(__APPLE__)
#  include <OpenGL/glu.h>
#else
#  include <GL/glu.h>
#endif


/** Set DEBUG to 1 to enable extra output when trying to debug failures */
#define DEBUG 0

#define SIZE 256

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_width = 3*SIZE;
	config.window_height = SIZE;
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_DEPTH;

PIGLIT_GL_TEST_CONFIG_END

static GLfloat ErrorScale = 0.0;
static GLuint ColorTex, DepthTex, FBO;
static GLuint ShaderProg;
static GLint Zbits;
static GLenum TexTarget = GL_TEXTURE_2D;
static GLUquadricObj *sphereObj = NULL;


static void
create_fbo(void)
{
   GLenum depthIntFormat = GL_DEPTH_COMPONENT24;
   GLenum status;

   /* depth texture */
   glGenTextures(1, &DepthTex);
   glBindTexture(TexTarget, DepthTex);
   glTexParameteri(TexTarget, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
   glTexParameteri(TexTarget, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
   glTexImage2D(TexTarget, 0, depthIntFormat,
                SIZE, SIZE, 0,
                GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, NULL);
   assert(glGetError() == 0);
   glGetTexLevelParameteriv(TexTarget, 0, GL_TEXTURE_DEPTH_SIZE, &Zbits);

   /* color texture */
   glGenTextures(1, &ColorTex);
   glBindTexture(TexTarget, ColorTex);
   glTexParameteri(TexTarget, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
   glTexParameteri(TexTarget, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
   glTexImage2D(TexTarget, 0, GL_RGBA,
                SIZE, SIZE, 0,
                GL_RGBA, GL_UNSIGNED_BYTE, NULL);
   assert(glGetError() == 0);

   /* Create FBO */
   glGenFramebuffersEXT(1, &FBO);
   glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, FBO);

   glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
                             GL_COLOR_ATTACHMENT0_EXT,
                             TexTarget,
                             ColorTex,
                             0);

   glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
                             GL_DEPTH_ATTACHMENT_EXT,
                             TexTarget,
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
    * into a 2D texture coordinate.  There's a -0.5 bias and scale factor.
    */
   static const char *text_2d =
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
      "   gl_FragDepth = gl_FragCoord.z; \n"
      "} \n";
   static const char *text_rect =
      "#extension GL_ARB_texture_rectangle: require \n"
      "uniform sampler2DRect zTex; \n"
      "uniform float sizeScale; \n"
      "uniform float errorScale; \n"
      "void main() \n"
      "{ \n"
      "   vec2 coord = gl_FragCoord.xy; \n"
      "   vec4 z = texture2DRect(zTex, coord); \n"
      "   float diff = errorScale * abs(z.r - gl_FragCoord.z); \n"
      "   //gl_FragColor = vec4(gl_FragCoord.z, 0, 0, 0); \n"
      "   //gl_FragColor = z; \n"
      "   gl_FragColor = vec4(diff, 0, 0, 0); \n"
      "   gl_FragDepth = gl_FragCoord.z; \n"
      "} \n";
   GLuint fs;
   GLint zTex, errorScale, sizeScale;

   if (TexTarget == GL_TEXTURE_2D)
      fs = piglit_compile_shader_text(GL_FRAGMENT_SHADER, text_2d);
   else
      fs = piglit_compile_shader_text(GL_FRAGMENT_SHADER, text_rect);

   assert(fs);

   ShaderProg = piglit_link_simple_program(0, fs);
   assert(ShaderProg);

   glUseProgram(ShaderProg);

   zTex = glGetUniformLocation(ShaderProg, "zTex");
   glUniform1i(zTex, 0);  /* unit 0 */

   errorScale = glGetUniformLocation(ShaderProg, "errorScale");
   glUniform1f(errorScale, ErrorScale);

   sizeScale = glGetUniformLocation(ShaderProg, "sizeScale");
   glUniform1f(sizeScale, (float) (SIZE - 1));

   glUseProgram(0);
}


#if DEBUG
static void
find_float_min_max_center(const GLfloat *buf, GLuint n,
			  GLfloat *min, GLfloat *max, GLfloat *center)
{
   GLint cx = SIZE/4, cy = SIZE/4;
   GLuint i;

   *min = 1.0e20;
   *max = -1.0e20;

   for (i = 0; i < n; i++) {
      if (buf[i] != 1.0) {
         if (buf[i] < *min)
            *min = buf[i];
         if (buf[i] > *max)
            *max = buf[i];
      }
   }

   *center = buf[cy * SIZE + cx];
}

static void
find_uint_min_max_center(const GLuint *buf, GLuint n,
			 GLuint *min, GLuint *max, GLuint *center)
{
   GLint cx = SIZE/4, cy = SIZE/4;
   GLuint i;

   *min = ~0U;
   *max = 0;

   for (i = 0; i < n; i++) {
      if (buf[i] != ~0U) {
         if (buf[i] < *min)
            *min = buf[i];
         if (buf[i] > *max)
            *max = buf[i];
      }
   }

   *center = buf[cy * SIZE + cx];
}
#endif /* DEBUG */


static void
draw_sphere(void)
{
   GLdouble radius = 0.95;
   GLint slices = 40;
   GLint stacks = 20;

   gluQuadricDrawStyle(sphereObj, GLU_FILL);
   gluQuadricNormals(sphereObj, GLU_SMOOTH);
   gluSphere(sphereObj, radius, slices, stacks);
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
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   glOrtho(-1.0, 1.0, -1.0, 1.0, -1, 1.0);

   glColor4f(1.0, 0.0, 0.0, 0.0);
   draw_sphere();

   glDisable(GL_DEPTH_TEST);

   glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, piglit_winsys_fbo);
}


static GLfloat *
read_float_z_image(GLint x, GLint y)
{
   GLfloat *z = (GLfloat *) malloc(SIZE * SIZE * sizeof(GLfloat));

   glReadPixels(x, y, SIZE, SIZE, GL_DEPTH_COMPONENT, GL_FLOAT, z);

   return z;
}


#if DEBUG
static GLuint *
read_uint_z_image(GLint x, GLint y)
{
   GLuint *z = (GLuint *) malloc(SIZE * SIZE * sizeof(GLuint));

   glReadPixels(x, y, SIZE, SIZE, GL_DEPTH_COMPONENT, GL_UNSIGNED_INT, z);

   return z;
}
#endif /* DEBUG */


/** Show contents of depth buffer in middle of window */
static void
show_depth_fbo(void)
{
   GLfloat *zf;

   glViewport(1 * SIZE, 0, SIZE, SIZE); /* not really needed */

   glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, FBO);
   zf = read_float_z_image(0, 0);

   glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, piglit_winsys_fbo);

   glWindowPos2i(SIZE, 0);
   glDrawPixels(SIZE, SIZE, GL_LUMINANCE, GL_FLOAT, zf);
   assert(glGetError() == 0);

#if DEBUG
   {
      GLfloat min, max, center;
      find_float_min_max_center(zf, SIZE * SIZE, &min, &max, &center);
      printf("depth fbo min %f  max %f  center %f\n", min, max, center);
   }

   {
      GLuint min, max, center;
      GLuint *zi;

      glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, FBO);
      zi = read_uint_z_image(0, 0);
      glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, piglit_winsys_fbo);

      find_uint_min_max_center(zi, SIZE * SIZE, &min, &max, &center);
      printf("depth fbo min 0x%x  max 0x%x  center 0x%x\n", min, max, center);
      free(zi);
   }
#endif /* DEBUG */

   free(zf);
}


/** Draw quad textured with depth image on right side of window */
static void
draw_quad_with_depth_texture(void)
{
   GLfloat s1, t1;

   if (TexTarget == GL_TEXTURE_2D) {
      s1 = t1 = 1.0;
   }
   else {
      s1 = SIZE;
      t1 = SIZE;
   }

   glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, piglit_winsys_fbo);

   glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

   glViewport(2 * SIZE, 0, SIZE, SIZE);

   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   glOrtho(-1.0, 1.0, -1.0, 1.0, -1, 1.0);

   glBindTexture(TexTarget, DepthTex);
   glEnable(TexTarget);

   glBegin(GL_POLYGON);
   glTexCoord2f(0, 0);
   glVertex2f(-1, -1);
   glTexCoord2f(s1, 0);
   glVertex2f( 1, -1);
   glTexCoord2f(s1, t1);
   glVertex2f( 1,  1);
   glTexCoord2f(0, t1);
   glVertex2f(-1,  1);
   glEnd();

   glDisable(TexTarget);
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
   glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, piglit_winsys_fbo);

   glViewport(0 * SIZE, 0, SIZE, SIZE);

   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   glOrtho(-1.0, 1.0, -1.0, 1.0, -1, 1.0);

   glBindTexture(TexTarget, DepthTex);

   glUseProgram(ShaderProg);

   glEnable(GL_DEPTH_TEST);

   if (1) {
      draw_sphere();
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
      
      draw_sphere();

      glDisable(GL_TEXTURE_GEN_S);
      glDisable(GL_TEXTURE_GEN_T);
   }

   glDisable(GL_DEPTH_TEST);

   glUseProgram(0);

#if DEBUG
   {
      GLfloat *z = read_float_z_image(0, 0);
      GLfloat min, max, center;

      find_float_min_max_center(z, SIZE * SIZE, &min, &max, &center);
      printf("rendered  min %f  max %f  center %f\n", min, max, center);

      free(z);
   }
   {
      GLuint *z = read_uint_z_image(0, 0);
      GLuint min, max, center;

      find_uint_min_max_center(z, SIZE * SIZE, &min, &max, &center);
      printf("rendered  min 0x%x  max 0x%x  center 0x%x\n", min, max, center);

      free(z);
   }
#endif /* DEBUG */
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

   piglit_present_results();

   return result;
}


void
piglit_init(int argc, char **argv)
{
   int i = 1;

   if (i < argc && strcmp(argv[i], "rect") == 0) {
      TexTarget = GL_TEXTURE_RECTANGLE;
      i++;
   }
   if (i < argc) {
      ErrorScale = atof(argv[i]);
   }

   piglit_require_extension("GL_EXT_framebuffer_object");
   piglit_require_fragment_shader();
   if (TexTarget == GL_TEXTURE_RECTANGLE) {
      piglit_require_extension("GL_ARB_texture_rectangle");
   }

   create_fbo();

   if (ErrorScale == 0.0) {
      /* A 1-bit error/difference in Z values results in a delta of 64 in
       * pixel intensity (where pixels are in [0,255]).
       */
      ErrorScale = ((double) (1ull << Zbits)) * 64.0 / 255.0;
   }

   create_frag_shader();

   sphereObj = gluNewQuadric();

   if (!piglit_automatic) {
      printf("GL_RENDERER = %s\n", (char *) glGetString(GL_RENDERER));
      printf("Left: Shader showing difference pixels (black=good, red=error)\n");
      printf("Middle: Depth buffer of FBO\n");
      printf("Right: Quad textured with depth values\n");
      printf("Z bits = %d\n", Zbits);
      printf("ErrorScale = %f\n", ErrorScale);
      printf("Texture target: %s\n",
	     TexTarget == GL_TEXTURE_RECTANGLE ? "RECTANGLE" : "2D" );
   }
}
