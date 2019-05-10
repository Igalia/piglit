/*
 * Copyright (c) 2019 Baldur Karlsson <baldurk@baldurk.org>
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

/** @file glx-multithread-buffer.c
 *
 * Create a buffer shared between two contexts, invalidate it on one while it is
 * bound on another to exhibit broken descriptor handling.
 */

#include <unistd.h>

#include "piglit-util-gl.h"
#include "piglit-glx-util.h"

int piglit_width = 50, piglit_height = 50;

static Display *dpy;
static Window draw_win;
static GLXPixmap load_win;
static XVisualInfo *visinfo;

static const char *vs_text =
  "#version 140\n"
  "in vec4 vertex;\n"
  "void main() {\n"
  "const vec2 verts[4] = vec2[4](vec2(-0.7, -0.7), vec2( 0.7, -0.7),\n"
  "                              vec2(-0.7,  0.7), vec2( 0.7,  0.7));"
  "gl_Position = vec4(verts[gl_VertexID].xy, 0.0, 1.0);}\n"
  ;

static const char *fs_text =
  "#version 140\n"
  "out vec4 v;\n"
  "uniform buffoo0 { vec4 a; };\n"
  "void main() {\n"
  "    v = vec4(1.0, 0.0, 0.0, 1.0)+a;\n"
  "}\n"
  ;

static const float green[4] = { 0.0f, 1.0f, 0.0f, 1.0f };

enum piglit_result
draw(Display *dpy)
{
  int ret, i;
  GLuint prog, vs, fs, buf;
  GLXContext ctx1, ctx2;
  void *ptr;
  unsigned char pixel[4];

  ctx1 = piglit_get_glx_context_share(dpy, visinfo, NULL);
  ctx2 = piglit_get_glx_context_share(dpy, visinfo, ctx1);

  ret = glXMakeCurrent(dpy, draw_win, ctx1);
  assert(ret);

  piglit_dispatch_default_init(PIGLIT_DISPATCH_GL);

  glGenBuffers(1, &buf);
  glBindBuffer(GL_UNIFORM_BUFFER, buf);
  glBufferData(GL_UNIFORM_BUFFER, 2048, NULL, GL_DYNAMIC_DRAW);

  prog = glCreateProgram();
  vs = piglit_compile_shader_text(GL_VERTEX_SHADER, vs_text);
  fs = piglit_compile_shader_text(GL_FRAGMENT_SHADER, fs_text);
  if (!piglit_check_gl_error(GL_NO_ERROR))
    piglit_report_result(PIGLIT_FAIL);

  glAttachShader(prog, vs);
  glAttachShader(prog, fs);
  glLinkProgram(prog);
  if (!piglit_check_gl_error(GL_NO_ERROR))
    piglit_report_result(PIGLIT_FAIL);

  if (!piglit_link_check_status(prog)) {
    piglit_report_result(PIGLIT_FAIL);
  }

  glClearColor(0, 0, 1, 1);
  glUseProgram(prog);
  glBindBufferBase(GL_UNIFORM_BUFFER, 0, buf);
  
  ret = glXMakeCurrent(dpy, draw_win, ctx2);
  assert(ret);
  glClearColor(0, 0, 1, 1);
  glUseProgram(prog);
  glBindBufferBase(GL_UNIFORM_BUFFER, 0, buf);

  for (i = 0; i < 10; ++i) {
    ret = glXMakeCurrent(dpy, draw_win, ctx1);
    assert(ret);

    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(prog);

    ptr = glMapBufferRange(GL_UNIFORM_BUFFER, 0, 128,
                           GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
    memset(ptr, 0, 128);
    memcpy(ptr, green, sizeof(green));
    glUnmapBuffer(GL_UNIFORM_BUFFER);

    piglit_draw_rect(0, 0, 1, 1);

    memset(pixel, 0, sizeof(pixel));
    glReadPixels(piglit_width/2, piglit_height/2, 1, 1,
                 GL_RGBA, GL_UNSIGNED_BYTE, pixel);
    if (pixel[0] != 255 || pixel[1] != 255 || pixel[2] != 0) {
      printf("Incorrect pixel at iteration %d: %u,%u,%u\n",
             i, pixel[0], pixel[1], pixel[2]);
      piglit_report_result(PIGLIT_FAIL);
    }

    glXSwapBuffers(dpy, draw_win);

    ret = glXMakeCurrent(dpy, draw_win, ctx2);
    assert(ret);

    glClear(GL_COLOR_BUFFER_BIT);

    ptr = glMapBufferRange(GL_UNIFORM_BUFFER, 0, 128,
                           GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
    memset(ptr, 0, 128);
    memcpy(ptr, green, sizeof(green));
    glUnmapBuffer(GL_UNIFORM_BUFFER);

    piglit_draw_rect(0, 0, 1, 1);

    memset(pixel, 0, sizeof(pixel));
    glReadPixels(piglit_width/2, piglit_height/2, 1, 1,
                 GL_RGBA, GL_UNSIGNED_BYTE, pixel);
    if (pixel[0] != 255 || pixel[1] != 255 || pixel[2] != 0) {
      printf("Incorrect pixel at iteration %d: %u,%u,%u\n",
             i, pixel[0], pixel[1], pixel[2]);
      piglit_report_result(PIGLIT_FAIL);
    }

    glXSwapBuffers(dpy, draw_win);
  }
  
  glDeleteBuffers(1, &buf);

  glXDestroyContext(dpy, ctx1);
  glXDestroyContext(dpy, ctx2);

  return PIGLIT_PASS;
}

int
main(int argc, char **argv)
{
  int i;
  Pixmap pixmap;

  for(i = 1; i < argc; ++i) {
    if (!strcmp(argv[i], "-auto"))
      piglit_automatic = 1;
    else
      fprintf(stderr, "Unknown option: %s\n", argv[i]);
  }

  XInitThreads();
  dpy = XOpenDisplay(NULL);
  if (dpy == NULL) {
    fprintf(stderr, "couldn't open display\n");
    piglit_report_result(PIGLIT_FAIL);
  }
  visinfo = piglit_get_glx_visual(dpy);
  draw_win = piglit_get_glx_window(dpy, visinfo);
  pixmap = XCreatePixmap(dpy, DefaultRootWindow(dpy),
    piglit_width, piglit_height, visinfo->depth);
  load_win = glXCreateGLXPixmap(dpy, visinfo, pixmap);

  XMapWindow(dpy, draw_win);

  piglit_glx_event_loop(dpy, draw);

  return 0;
}
