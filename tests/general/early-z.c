/*
 * Copyright (c) 2011 Christoph Bumiller
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * on the rights to use, copy, modify, merge, publish, distribute, sub
 * license, and/or sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NON-INFRINGEMENT.  IN NO EVENT SHALL VMWARE AND/OR THEIR SUPPLIERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

/**
 * @file
 * Test for bugs with early depth testing and early depth update.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_DEPTH;

PIGLIT_GL_TEST_CONFIG_END

GLint u_zval;

static const char *fpFragDepthText =
   "uniform float zval; \n"
   "void main() \n"
   "{ \n"
   "   gl_FragColor = vec4(gl_Color.rgb, 1.0); \n"
   "   gl_FragDepth = zval; \n"
   "} \n";

static const char *fpDiscardText =
   "void main() \n"
   "{ \n"
   "   if (gl_Color.r > 0.25) \n"
   "      discard; \n"
   "   gl_FragColor = vec4(gl_Color.rgb, 1.0); \n"
   "} \n";

static const char *fpAlphaText =
   "void main() \n"
   "{ \n"
   "   gl_FragColor = vec4(gl_Color.rgb, 0.0); \n"
   "} \n";


static GLuint shader[3];
static GLuint program[3];

static void quad(const GLfloat z, uint32_t colour)
{
   glColor3ub(colour & 0xff, (colour >> 8) & 0xff, (colour >> 16) & 0xff);
   glBegin(GL_QUADS);
   glVertex3f(-1.0f, -1.0f, z);
   glVertex3f(-1.0f,  1.0f, z);
   glVertex3f( 1.0f,  1.0f, z);
   glVertex3f( 1.0f, -1.0f, z);
   glEnd();
}

static GLboolean
test_early_depth(void)
{
   glClearColor(0.0, 0.0, 0.0, 0.0);
   glClearDepth(1.0);
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

   glEnable(GL_DEPTH_TEST);
   glDepthMask(GL_TRUE);
   glDepthFunc(GL_LESS);

   /* 1. (blue) depth should be adjusted to 0.8 by the FP */
   glUseProgram(program[0]);
   glUniform1f(u_zval, 0.8f);
   quad(0.3, 0xff0000);

   /* 2. (red) should be discarded, no depth value written */
   glUseProgram(program[1]);
   quad(0.2, 0x0000ff);

   /* 3. (white) should be discarded by the alpha test, no depth written */
   glUseProgram(program[2]);
   glEnable(GL_ALPHA_TEST);
   glAlphaFunc(GL_GREATER, 0.5f);
   quad(0.1, 0xffffff);
   glDisable(GL_ALPHA_TEST);

   /* 4. (green) should be drawn because depth is 0.8 and FragDepth is 0.5 */
   glUseProgram(program[0]);
   glUniform1f(u_zval, 0.5);
   quad(0.9, 0x00ff00);

   /* 5. (yellow) should be discarded because program sets depth to 0.9 */
   glUniform1f(u_zval, 0.9);
   quad(0.2, 0x00ffff);

   {
      const GLfloat color[4] = { 0.0f, 1.0f, 0.0f, 1.0f };
      GLint pos[4];

      /* use glRasterPos to determine where to read a sample pixel */
      glRasterPos2f(0.0f, 0.0f);
      glGetIntegerv(GL_CURRENT_RASTER_POSITION, pos);

      if (!piglit_probe_pixel_rgba(pos[0], pos[1], color)) {
         piglit_present_results();
         return GL_FALSE;
      }
   }

   piglit_present_results();

   return GL_TRUE;
}


enum piglit_result
piglit_display(void)
{
   if (!test_early_depth())
      return PIGLIT_FAIL;

   return PIGLIT_PASS;
}


void
piglit_init(int argc, char **argv)
{
   int i;

   piglit_require_gl_version(20);

   shader[0] = piglit_compile_shader_text(GL_FRAGMENT_SHADER, fpFragDepthText);
   assert(shader[0]);

   shader[1] = piglit_compile_shader_text(GL_FRAGMENT_SHADER, fpDiscardText);
   assert(shader[1]);

   shader[2] = piglit_compile_shader_text(GL_FRAGMENT_SHADER, fpAlphaText);
   assert(shader[2]);

   for (i = 0; i < 3; ++i)
      program[i] = piglit_link_simple_program(shader[i], 0);

   u_zval = glGetUniformLocation(program[0], "zval");
}
