/*
 * Copyright (c) 2011 VMware, Inc.
 * Copyright © 2015 Intel Corporation
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

/** Set DEBUG to 1 to enable extra output when trying to debug failures */
#define DEBUG 0

#define SIZE 256

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_width = 3*SIZE;
	config.window_height = SIZE;
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_DEPTH;
	config.khr_no_error_support = PIGLIT_NO_ERRORS;

PIGLIT_GL_TEST_CONFIG_END

static GLfloat ErrorScale = 0.0;
static GLuint ColorTex, DepthTex, FBO;
static GLuint ShaderProg;
static GLint Zbits;
static GLenum TexTarget = GL_TEXTURE_2D;


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
   if (!piglit_check_gl_error(GL_NO_ERROR))
           piglit_report_result(PIGLIT_FAIL);
   glGetTexLevelParameteriv(TexTarget, 0, GL_TEXTURE_DEPTH_SIZE, &Zbits);

   /* color texture */
   glGenTextures(1, &ColorTex);
   glBindTexture(TexTarget, ColorTex);
   glTexParameteri(TexTarget, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
   glTexParameteri(TexTarget, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
   glTexImage2D(TexTarget, 0, GL_RGBA,
                SIZE, SIZE, 0,
                GL_RGBA, GL_UNSIGNED_BYTE, NULL);
   if (!piglit_check_gl_error(GL_NO_ERROR))
	    piglit_report_result(PIGLIT_FAIL);

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

   if (!piglit_check_gl_error(GL_NO_ERROR))
	    piglit_report_result(PIGLIT_FAIL);

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
   GLint zTex, errorScale, sizeScale;
   const char *const fs_source = (TexTarget == GL_TEXTURE_2D)
	   ? text_2d : text_rect;

   ShaderProg = piglit_build_simple_program(NULL, fs_source);
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
   /* Without this enum hack, GCC complains about variable length arrays
    * below... even if you make the variables const.
    */
   enum {
      slices = 40,
      stacks = 20,

      /* There are (stacks - 1) interior stacks (see the comment before y
       * below).  Each interior stack is (slices + 1) vertices.  There is on
       * additional vertex at the top, and there is one at the bottom.
       */
      num_vertices = (stacks - 1) * (slices + 1) + 2,

      /* Each slice is a single triangle strip.  There is a triangle at the
       * top (3 elements), and there is one at the bottom (1 element).
       * Between is (stacks - 2) quadrilaterals (2 elements each).
       */
      elements_per_slice = 3 + ((stacks - 2) * 2) + 1,
   };

   const GLdouble radius = 0.95;
   unsigned i;

   static float vertex_data[num_vertices * 4];
   static unsigned element_data[elements_per_slice * slices];
   static bool generated = false;

   if (!generated) {
      float *v = vertex_data;
      unsigned *e = element_data;
      unsigned j;

      assert(num_vertices < 65535);

      for (i = 1; i < stacks; i++) {
	 /* The y values of the sphere interpolate from -radius to radius.
	  * The two extrema have a single point (in terms of the "circular
	  * slice" mentioned below, r_c = 0).  Those points are generated at
	  * the very end.  If there are N slices of the sphere, there are N+1
	  * layers of data.  This loop generates data for layers 1 through
	  * N-1, inclusive.  Layers 0 and N are the extrema previously
	  * mentioned.
	  *
	  * NOTE: Then angle range from the north pole to the south pole is
	  * PI.  When going around the equator (inner loop below), the angle
	  * range is 0 to 2PI.
	  */
	 const double y = -cos(i * M_PI / stacks) * radius;

	 /* The radius of the sphere is, r_s, sqrt(x**2 + y**2 + z**2).  The
	  * radius of the circular slice of the sphere parallel to the X/Z
	  * plane, r_c, is sqrt(x**2 + z**2).  r_s and y are known.  Solve for
	  * r_c.
	  *
	  *     r_s**2 = x**2 + y**2 + z**2
	  *     r_s**2 = r_c**2 + y**2
	  *     r_c**2 = r_s**2 - y**2
	  *     r_c = sqrt(r_s**2 - y**2)
	  */
	 const double r_c = sqrt((radius * radius) - (y * y));

	 for (j = 0; j <= slices; j++) {
	    const double angle = j * 2.0 * M_PI / slices;

	    v[0] = r_c * sin(angle);
	    v[1] = y;
	    v[2] = r_c * cos(angle);
	    v[3] = 1.0;

	    assert(fabs(v[0] * v[0] + v[1] * v[1] + v[2] * v[2] -
			radius * radius) < 1e-6);
	    v += 4;
	    assert(v < &vertex_data[ARRAY_SIZE(vertex_data)]);
	 }
      }

      v[0] = 0.0;
      v[1] = -radius;
      v[2] = 0.0;
      v[3] = 1.0;

      v[4] = 0.0;
      v[5] = radius;
      v[6] = 0.0;
      v[7] = 1.0;
      assert(&v[8] == &vertex_data[ARRAY_SIZE(vertex_data)]);

      for (i = 0; i < slices; i++) {
	 /* The outer loop walks around the first circluar slice of vertex
	  * data.  This occupies vertices [0, slices].  Looking at the sphere,
	  * there is a vertex on the left side of the polygon being emitted,
	  * and the next vertex in the sequence is on the right.
	  */
	 unsigned left = i;

	 /* Emit the "base" triangle. */
	 e[0] = num_vertices - 2;
	 e++;
	 assert(e < &element_data[ARRAY_SIZE(element_data)]);

	 for (j = 0; j < (stacks - 1); j++) {
	    const unsigned right = left + 1;

	    e[0] = left;
	    e[1] = right;
	    e += 2;
	    assert(e < &element_data[ARRAY_SIZE(element_data)]);

	    left += (slices + 1);
	 }

	 /* Emit the bottom vertex for the final triangle. */
	 e[0] = num_vertices - 1;
	 e++;
	 assert(e <= &element_data[ARRAY_SIZE(element_data)]);
      }

      assert(e == &element_data[ARRAY_SIZE(element_data)]);
      generated = true;
   }

   glVertexPointer(4, GL_FLOAT, 4 * sizeof(float), vertex_data);
   glEnableClientState(GL_VERTEX_ARRAY);
   for (i = 0; i < slices; i++) {
      glDrawElements(GL_TRIANGLE_STRIP,
		     elements_per_slice,
		     GL_UNSIGNED_INT,
		     &element_data[i * elements_per_slice]);
   }
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
   if (!piglit_check_gl_error(GL_NO_ERROR))
	    piglit_report_result(PIGLIT_FAIL);

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

   piglit_draw_rect_tex(-1, -1, 2, 2, 0, 0, s1, t1);

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
