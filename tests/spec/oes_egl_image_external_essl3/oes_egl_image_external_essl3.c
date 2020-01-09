/*
 * Copyright (C) 2020 Intel Corporation
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
 *
 */

/**
  * @file oes_egl_image_external_essl3.c
  *
  * Test for OES_EGL_image_external_essl3 extension
  *
  * Test sampler functions:
  * - textureSize
  * - texture
  * - texelFetch
  * - textureProj
  *
  * Test image functions:
  * - imageLoad
  * - imageStore
  *
  * \author Yevhenii Kolesnikov <yevhenii.kolesnikov@globallogic.com>
  */

#include "piglit-util-egl.h"
#include "piglit-util-gl.h"

const struct piglit_subtest subtests[];
enum piglit_result test_case_sampler(void *data);
enum piglit_result test_case_image_load(void *data);
enum piglit_result test_case_image_store(void *data);
static struct piglit_gl_test_config *piglit_config;

PIGLIT_GL_TEST_CONFIG_BEGIN

        piglit_config = &config;
        config.supports_gl_es_version = 30;
        config.subtests = subtests;

        config.window_width = 128;
        config.window_height = 128;
        config.window_visual = PIGLIT_GL_VISUAL_RGBA;

PIGLIT_GL_TEST_CONFIG_END

PFNEGLCREATEIMAGEKHRPROC peglCreateImageKHR = NULL;

static const char vs_template[] =
   "#version %d es\n"
   "in vec4 piglit_vertex;\n"
   "in vec4 piglit_texcoords;\n"
   "out vec2 texcoords;\n"
   "\n"
   "void main()\n"
   "{\n"
   "   texcoords = piglit_texcoords.xy;\n"
   "   gl_Position = piglit_vertex;\n"
   "}\n";

static const char fs_template_sampler[] =
   "#version 300 es\n"
   "#extension GL_OES_EGL_image_external_essl3 : require\n"
   "precision mediump float;\n"
   "out vec4 colour;\n"
   "%s\n"
   "      colour = vec4(0.0, 1.0, 0.0, 1.0);\n"
   "   else\n"
   "      colour = vec4(1.0, 0.0, 0.0, 1.0);\n"
   "}\n";

static const char fs_template_image[] =
   "#version 310 es\n"
   "#extension GL_OES_EGL_image_external_essl3 : require\n"
   "precision mediump float;\n"
   "out vec4 colour;\n"
   "%s\n";

static const char fs_texturesize[] =
   "uniform samplerExternalOES sampler;\n"
   "void main()\n"
   "{\n"
   "   ivec2 texsize = textureSize(sampler, 0);\n"
   "   if (texsize == ivec2(128, 128))\n";

static const char fs_texture[] =
   "uniform samplerExternalOES sampler;\n"
   "void main()\n"
   "{\n"
   "   vec4 col = texture(sampler, vec2(0, 0));\n"
   "   if (col == vec4(1.0, 1.0, 1.0, 1.0))\n";

static const char fs_texelFetch[] =
   "uniform samplerExternalOES sampler;\n"
   "void main()\n"
   "{\n"
   "   vec4 col = texelFetch(sampler, ivec2(0, 0), 0);\n"
   "   if (col == vec4(1.0, 1.0, 1.0, 1.0))\n";

static const char fs_textureProj[] =
   "uniform samplerExternalOES sampler;\n"
   "void main()\n"
   "{\n"
   "   vec4 col = textureProj(sampler, vec4(0, 0, 0, 1));\n"
   "   if (col == vec4(1.0, 1.0, 1.0, 1.0))\n";

static const char fs_imageLoad[] =
   "precision mediump image2D;\n"
   "layout(binding = 1, rgba8) readonly uniform image2D img;\n"
   "void main()\n"
   "{\n"
   "   vec4 col = imageLoad(img, ivec2(0, 0));\n"
   "   if (col == vec4(1.0, 1.0, 1.0, 1.0))\n"
   "      colour = vec4(0.0, 1.0, 0.0, 1.0);\n"
   "   else\n"
   "      colour = vec4(1.0, 0.0, 0.0, 1.0);\n"
   "};\n";

static const char fs_imageStore_store[] =
   "precision mediump image2D;\n"
   "layout(binding = 1, rgba8) writeonly uniform image2D img;\n"
   "void main()\n"
   "{\n"
   "   vec4 data = vec4(1.0, 1.0, 0.0, 1.0);\n"
   "   imageStore(img, ivec2(0, 0), data);\n"
   "   colour = vec4(0.3, 0.3, 0.3, 1.0);\n"
   "}\n";

static const char fs_imageStore_load[] =
   "precision mediump image2D;\n"
   "layout(binding = 1, rgba8) readonly uniform image2D img;\n"
   "void main()\n"
   "{\n"
   "   vec4 col = imageLoad(img, ivec2(0, 0));\n"
   "   if (col == vec4(1.0, 1.0, 0.0, 1.0))\n"
   "      colour = vec4(0.0, 1.0, 0.0, 1.0);\n"
   "   else\n"
   "      colour = vec4(1.0, 0.0, 0.0, 1.0);\n"
   "}\n";

const struct piglit_subtest subtests[] = {
   {
      "oes_egl_image_external_essl3_textureSize",
      "texturesize",
      test_case_sampler,
      (void*)fs_texturesize
   },
   {
      "oes_egl_image_external_essl3_texture",
      "texture",
      test_case_sampler,
      (void*)fs_texture
   },
   {
      "oes_egl_image_external_essl3_texelFetch",
      "texelfetch",
      test_case_sampler,
      (void*)fs_texelFetch
   },
   {
      "oes_egl_image_external_essl3_textureProj",
      "textureproj",
      test_case_sampler,
      (void*)fs_textureProj
   },
   {
      "oes_egl_image_external_essl3_imageLoad",
      "imageload",
      test_case_image_load,
      (void*)fs_template_image
   },
   {
      "oes_egl_image_external_essl3_imageStore",
      "imagestore",
      test_case_image_store,
      (void*)fs_template_image
   },
   {0}
};

GLuint tex_external;

enum piglit_result
test_case_image_load(void *data)
{
   if (piglit_get_gl_version() < 31)
      piglit_report_result(PIGLIT_SKIP);

   enum piglit_result pass = PIGLIT_PASS;
   const float green[] = {0.0, 1.0, 0.0, 1.0};

   glBindImageTexture(1, tex_external, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA8);

   char *vs_src, *fs_src;
   asprintf(&vs_src, vs_template, 310);
   asprintf(&fs_src, (const char*) data, fs_imageLoad);

   GLuint prog;
   prog = piglit_build_simple_program(vs_src, fs_src);

   glUseProgram(prog);

   glViewport(0, 0, piglit_width, piglit_height);
   piglit_draw_rect_tex(-1, -1, 2, 2,
                        0, 0, piglit_width, piglit_height);

   glDeleteProgram(prog);
   glUseProgram(0);

   free(fs_src);

   if (!piglit_probe_rect_rgba(0, 0, piglit_width, piglit_height, green))
      pass = PIGLIT_FAIL;

   return pass;
}

enum piglit_result
test_case_image_store(void *data)
{
   if (piglit_get_gl_version() < 31)
      piglit_report_result(PIGLIT_SKIP);

   enum piglit_result pass = PIGLIT_PASS;
   const float green[] = {0.0, 1.0, 0.0, 1.0};

   glBindImageTexture(1, tex_external, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA8);

   char *vs_src, *fs_src;
   asprintf(&vs_src, vs_template, 310);
   asprintf(&fs_src, (const char*) data, fs_imageStore_store);

   GLuint prog;
   prog = piglit_build_simple_program(vs_src, fs_src);

   glUseProgram(prog);

   glViewport(0, 0, piglit_width, piglit_height);
   piglit_draw_rect_tex(-1, -1, 2, 2,
                        0, 0, piglit_width, piglit_height);

   glDeleteProgram(prog);
   glUseProgram(0);

   free(fs_src);

   asprintf(&fs_src, (const char*) data, fs_imageStore_load);

   prog = piglit_build_simple_program(vs_src, fs_src);

   glUseProgram(prog);

   glViewport(0, 0, piglit_width, piglit_height);
   piglit_draw_rect_tex(-1, -1, 2, 2,
                        0, 0, piglit_width, piglit_height);

   glDeleteProgram(prog);
   glUseProgram(0);

   free(fs_src);

   if (!piglit_probe_rect_rgba(0, 0, piglit_width, piglit_height, green))
      pass = PIGLIT_FAIL;

   return pass;
}

enum piglit_result
test_case_sampler(void *data)
{
   enum piglit_result pass = PIGLIT_PASS;
   const float green[] = {0.0, 1.0, 0.0, 1.0};

   char *vs_src, *fs_src;
   asprintf(&vs_src, vs_template, 300);
   asprintf(&fs_src, fs_template_sampler, (const char*) data);

   GLuint prog;
   prog = piglit_build_simple_program(vs_src, fs_src);

   glUseProgram(prog);
   glUniform1i(glGetUniformLocation(prog, "sampler"), 0);

   glViewport(0, 0, piglit_width, piglit_height);
   piglit_draw_rect_tex(-1, -1, 2, 2,
                        0, 0, piglit_width, piglit_height);

   glDeleteProgram(prog);
   glUseProgram(0);

   free(fs_src);

   if (!piglit_probe_rect_rgba(0, 0, piglit_width, piglit_height, green))
      pass = PIGLIT_FAIL;

   return pass;
}

enum piglit_result
piglit_display(void)
{
   enum piglit_result pass = PIGLIT_SKIP;
   GLuint tex_src, fb;
   EGLImage img;

   EGLint attrs[] = {
      EGL_WIDTH, piglit_width,
      EGL_HEIGHT, piglit_height,
      EGL_GL_TEXTURE_LEVEL_KHR, 0,
      EGL_NONE
   };

   glGenTextures(1, &tex_src);
   glBindTexture(GL_TEXTURE_2D, tex_src);

   glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, piglit_width, piglit_height);

   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

   glGenFramebuffers(1, &fb);
   glBindFramebuffer(GL_FRAMEBUFFER, fb);

   glFramebufferTexture2D(GL_FRAMEBUFFER,
                          GL_COLOR_ATTACHMENT0,
                          GL_TEXTURE_2D,
                          tex_src,
                          0);

   glClearColor(1.0, 1.0, 1.0, 1.0);
   glClear(GL_COLOR_BUFFER_BIT);

   glBindFramebuffer(GL_FRAMEBUFFER, 0);

   img = peglCreateImageKHR(eglGetCurrentDisplay(),
                            eglGetCurrentContext(),
                            EGL_GL_TEXTURE_2D,
                            (EGLClientBuffer) (intptr_t) tex_src,
                            attrs);

   if (!img)
      return PIGLIT_SKIP;

   glGenTextures(1, &tex_external);
   glBindTexture(GL_TEXTURE_EXTERNAL_OES, tex_external);

   glEGLImageTargetTexture2DOES(GL_TEXTURE_EXTERNAL_OES, img);

   pass = piglit_run_selected_subtests(piglit_config->subtests,
                                       piglit_config->selected_subtests,
                                       piglit_config->num_selected_subtests,
                                       pass);

   return pass;
}

void
piglit_init(int argc, char **argv)
{
   EGLDisplay egl_dpy;

   egl_dpy = eglGetCurrentDisplay();
   if (!egl_dpy)
      piglit_report_result(PIGLIT_SKIP);

   piglit_require_extension("GL_OES_EGL_image_external_essl3");
   piglit_require_egl_extension(egl_dpy, "EGL_KHR_image_base");

   peglCreateImageKHR =
           (PFNEGLCREATEIMAGEKHRPROC)eglGetProcAddress ("eglCreateImageKHR");

   if (!peglCreateImageKHR)
      piglit_report_result(PIGLIT_SKIP);
}
