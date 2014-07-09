/*
 * Copyright (c) 2012 Marek Olšák <maraeo@gmail.com>
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
 * NON-INFRINGEMENT.  IN NO EVENT SHALL THE AUTHORS AND/OR THEIR SUPPLIERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

static const char *vstext =
   "void main() \n"
   "{ \n"
   "   gl_Position = ftransform(); \n"
   "   gl_FrontColor = vec4(1.0); \n"
   "   gl_PointSize = 0.001; \n"
   "} \n";


static GLuint vs, prog;

enum piglit_result
piglit_display(void)
{
   GLboolean pass;
   float black[] = {0, 0, 0};

   glClear(GL_COLOR_BUFFER_BIT);
   glUseProgram(prog);
   glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);

   glBegin(GL_POINTS);
   glVertex2i(50, 20);
   glEnd();

   glUseProgram(0);

   pass = !piglit_probe_rect_rgb_silent(49, 19, 3, 3, black);
   if (!pass)
      puts("Fail: nothing rendered.");

   piglit_present_results();

   return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
   piglit_require_gl_version(20);
   vs = piglit_compile_shader_text(GL_VERTEX_SHADER, vstext);
   assert(vs);
   prog = piglit_link_simple_program(vs, 0);

   piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);
}
