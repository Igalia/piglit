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
 * @file fbo-fragcoord2.c
 *
 * Test GLSL gl_FragCoord, gl_Frontfacing, polygon CCW vs CW and culling
 * with an FBO
 *
 * Author: Brian Paul
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_width = 256;
	config.window_height = 256;
	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

static GLuint prog;

static const char *TestName = "fbo-fragcoord2";

static GLuint FBO;


static char *VertShaderText =
   "void main() \n"
   "{ \n"
   "   gl_Position = ftransform(); \n"
   "} \n";

static char *FragShaderText =
   "void main() \n"
   "{ \n"
   "   vec4 scale = vec4(1.0/255.0, 1.0/255.0, 1.0, 1.0); \n"
   "   if (gl_FrontFacing) { \n"
   "      // front-facing \n"
   "      gl_FragColor = gl_FragCoord * scale; \n"
   "      gl_FragColor.z = 0.0; \n"
   "   } else { \n"
   "      // back-facing \n"
   "      gl_FragColor = vec4(0, 0, 1, 1); \n"
   "   } \n"
   "} \n";


static void
check_error(int line)
{
   GLenum err = glGetError();
   if (err) {
      printf("%s: Unexpected error 0x%x at line %d\n",
              TestName, err, line);
      piglit_report_result(PIGLIT_FAIL);
   }
}


static void
create_fbo(void)
{
   GLuint rb;

   glGenFramebuffersEXT(1, &FBO);
   glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, FBO);

   glGenRenderbuffersEXT(1, &rb);
   check_error(__LINE__);

   glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, rb);
   check_error(__LINE__);

   glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT,
                                GL_COLOR_ATTACHMENT0,
                                GL_RENDERBUFFER_EXT,
                                rb);
   check_error(__LINE__);

   glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_RGBA,
                            piglit_width, piglit_height);
   check_error(__LINE__);
}


static void
rect(float x0, float y0, float x1, float y1)
{
   glBegin(GL_POLYGON);
   glVertex2f(x0, y0);
   glVertex2f(x1, y0);
   glVertex2f(x1, y1);
   glVertex2f(x0, y1);
   glEnd();
}


static enum piglit_result
test(void)
{
   static const GLfloat black[4] = {0, 0, 0, 1};
   static const GLfloat blue[4] = {0, 0, 1, 1};
   static const GLfloat half_red[4] = {0.5, 0, 0, 1};
   static const GLfloat half_green[4] = {0, 0.5, 0, 1};
   static const GLfloat green_half_red[4] = {0.5, 1, 0, 1};
   GLenum buffer;
   enum piglit_result result = PIGLIT_PASS;

   create_fbo();

   /* draw to fbo */
   buffer = GL_COLOR_ATTACHMENT0_EXT;
   glDrawBuffersARB(1, &buffer);
   glReadBuffer(buffer);

   /* test drawing */
   glClearColor(0, 0, 0, 0);
   glClear(GL_COLOR_BUFFER_BIT);

   glUseProgram(prog);

   glEnable(GL_CULL_FACE);

   /* left half: front-facing, color varies with fragcoord */
   glFrontFace(GL_CCW);
   glCullFace(GL_BACK);
   rect(0, 0, piglit_width / 2, piglit_height);

   /* right half: back-facing, blue */
   glFrontFace(GL_CW);
   glCullFace(GL_FRONT);
   rect(piglit_width/2, 0, piglit_width, piglit_height);

   glUseProgram(0);

   glDisable(GL_CULL_FACE);

   /* copy image from FBO to window */
   {
      GLubyte *buf;
      buf = malloc(piglit_width * piglit_height * 4);
      glReadPixels(0, 0, piglit_width, piglit_height, GL_RGBA, GL_UNSIGNED_BYTE, buf);

      glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, piglit_winsys_fbo);
      glDrawBuffer(GL_BACK);
      glClear(GL_COLOR_BUFFER_BIT);
      glDrawPixels(piglit_width, piglit_height, GL_RGBA, GL_UNSIGNED_BYTE, buf);
      free(buf);
   }

   /* left half probes */
   if (!piglit_probe_pixel_rgb(0, 0, black)) {
      result = PIGLIT_FAIL;
   }

   if (!piglit_probe_pixel_rgb(piglit_width/2-1, 0, half_red)) {
      result = PIGLIT_FAIL;
   }

   if (!piglit_probe_pixel_rgb(0, piglit_height/2, half_green)) {
      result = PIGLIT_FAIL;
   }

   if (!piglit_probe_pixel_rgb(piglit_width/2-1, piglit_height-1, green_half_red)) {
      result = PIGLIT_FAIL;
   }

   /* right half probe */
   if (!piglit_probe_pixel_rgb(piglit_width*3/4, piglit_height/2, blue)) {
      result = PIGLIT_FAIL;
   }

   piglit_present_results();

   return result;
}


enum piglit_result
piglit_display(void)
{
   return test();
}


void
piglit_init(int argc, char**argv)
{
   GLuint vs, fs;

   piglit_require_gl_version(20);

   piglit_require_extension("GL_EXT_framebuffer_object");

   piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);

   vs = piglit_compile_shader_text(GL_VERTEX_SHADER, VertShaderText);
   assert(vs);

   fs = piglit_compile_shader_text(GL_FRAGMENT_SHADER, FragShaderText);
   assert(fs);

   prog = piglit_link_simple_program(vs, fs);
   assert(prog);
}
