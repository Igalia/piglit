/*
 * Copyright (c) 2010 VMware, Inc.
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
 * @file glx-shader-sharing.c
 * @author Brian Paul
 *
 * Create two GLX contexts with shared shaders.  Destroy first context,
 * draw with second context.
 */

#include "piglit-util-gl.h"
#include "piglit-glx-util.h"

int piglit_width = 50, piglit_height = 50;

static const char *TestName = "glx-shader-sharing";

static Display *dpy;
static Window win;
static XVisualInfo *visinfo;


static const char *vert_shader_text =
   "void main() \n"
   "{ \n"
   "   gl_Position = ftransform(); \n"
   "   gl_FrontColor = gl_Color; \n"
   "} \n";

static const char *frag_shader_text =
   "void main() \n"
   "{ \n"
   "   gl_FragColor = vec4(1.0) - gl_Color; \n"
   "} \n";


static GLuint vert_shader, frag_shader;

static GLuint program;


static void
check_error(int line)
{
   GLenum e = glGetError();
   if (e)
      printf("GL Error 0x%x at line %d\n", e, line);
}


enum piglit_result
draw(Display *dpy)
{
   const GLfloat red[3] = {1.0F, 0.0F, 0.0F};
   const GLfloat green[3] = {0.0F, 1.0F, 0.0F};
   GLXContext ctx1 = piglit_get_glx_context(dpy, visinfo);
   GLXContext ctx2 = piglit_get_glx_context_share(dpy, visinfo, ctx1);
   int ok;

   if (!ctx1 || !ctx2) {
      fprintf(stderr, "%s: create contexts failed\n", TestName);
      piglit_report_result(PIGLIT_FAIL);
   }

   /*
    * Bind first context, make some shaders, draw something.
    */
   glXMakeCurrent(dpy, win, ctx1);

   piglit_dispatch_default_init(PIGLIT_DISPATCH_GL);

   if (piglit_get_gl_version() < 20) {
      printf("%s: Requires OpenGL 2.0\n", TestName);
      return PIGLIT_SKIP;
   }

   glClearColor(1.0, 0.0, 0.0, 1.0);
   glClear(GL_COLOR_BUFFER_BIT);

   vert_shader = piglit_compile_shader_text(GL_VERTEX_SHADER, vert_shader_text);
   frag_shader = piglit_compile_shader_text(GL_FRAGMENT_SHADER, frag_shader_text);
   program = piglit_link_simple_program(vert_shader, frag_shader);
   check_error(__LINE__);
   assert(program);

   glUseProgram(program);
   check_error(__LINE__);

   piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);
   glClearColor(0.1, 0.1, 0.1, 0.0);
   glClear(GL_COLOR_BUFFER_BIT);

   glColor3f(0, 1, 1);
   piglit_draw_rect(10, 10, piglit_width - 20, piglit_height - 20);
   check_error(__LINE__);

   ok = piglit_probe_pixel_rgb(piglit_width / 2, piglit_height / 2, red);

   glXSwapBuffers(dpy, win);

   if (!ok) {
      printf("%s: drawing with context 1 failed\n", TestName);
      return PIGLIT_FAIL;
   }

   /*
    * Destroy first context
    */
   glXDestroyContext(dpy, ctx1);


   /*
    * Draw something with second context (and shaders above)
    */
   glXMakeCurrent(dpy, win, ctx2);

   /*program = piglit_link_simple_program(vert_shader, frag_shader);*/
   check_error(__LINE__);
   assert(program);

   glUseProgram(program);
   check_error(__LINE__);

   piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);
   glClearColor(0.2, 0.2, 0.2, 0.0);
   glClear(GL_COLOR_BUFFER_BIT);

   glColor3f(1, 0, 1);
   piglit_draw_rect(10, 10, piglit_width - 20, piglit_height - 20);
   check_error(__LINE__);

   ok = piglit_probe_pixel_rgb(piglit_width / 2, piglit_height / 2, green);

   glXSwapBuffers(dpy, win);

   if (!ok) {
      printf("%s: drawing with context 2 failed\n", TestName);
      return PIGLIT_FAIL;
   }

   glXDestroyContext(dpy, ctx2);


   return PIGLIT_PASS;
}


int
main(int argc, char **argv)
{
   int i;

   for(i = 1; i < argc; ++i) {
      if (strcmp(argv[i], "-auto") == 0)
         piglit_automatic = 1;
      else
         fprintf(stderr, "%s bad option: %s\n", TestName, argv[i]);
   }

   dpy = XOpenDisplay(NULL);
   if (dpy == NULL) {
      fprintf(stderr, "%s: open display failed\n", TestName);
      piglit_report_result(PIGLIT_FAIL);
   }

   visinfo = piglit_get_glx_visual(dpy);
   win = piglit_get_glx_window(dpy, visinfo);

   XMapWindow(dpy, win);

   piglit_glx_event_loop(dpy, draw);

   return 0;
}
