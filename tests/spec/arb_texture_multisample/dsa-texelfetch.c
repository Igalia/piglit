/*
 * Copyright 2019 Intel Corporation
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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */


/**
 * File: dsa-texelfetch.c
 * Tests that texelFetch() gets texels from texture with different
 * internal types.
 *
 *
 * We populate an MS texture then we draw into
 * a rectangle texture with values from the MS texture.
 * We check that the data from rectangle texture is equal
 * with the data in the MS texture.
 *
 * Bugzilla: https://bugs.freedesktop.org/show_bug.cgi?id=109057
 */

#include "piglit-util-gl.h"


PIGLIT_GL_TEST_CONFIG_BEGIN

    config.supports_gl_core_version = 31;
    config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;
    config.khr_no_error_support = PIGLIT_NO_ERRORS;

PIGLIT_GL_TEST_CONFIG_END

   static const char *vs_src =
   "#version 130\n"
   "#extension GL_ARB_explicit_attrib_location : require\n"
   "layout (location = 0) in vec2 position;\n"
   "void main() {\n" " gl_Position = vec4(position, 0.0, 1.0);\n" "} \n";


static const char *fs_float_draw =
   "#version 130\n"
   "#extension GL_ARB_explicit_attrib_location : require\n"
   "layout (location = 0) out float my_output;\n"
   "void main() {\n" "  my_output = 1;\n" "} \n";

static const char *fs_float_read =
   "#version 130\n"
   "#extension GL_ARB_texture_multisample : require\n"
   "uniform sampler2DMS sampler;\n"
   "out float my_output;\n"
   "void main() {\n"
   "  my_output = texelFetch(sampler, ivec2(0,0), 0).r;\n" "} \n";

static const char *fs_int_draw =
   "#version 130\n"
   "#extension GL_ARB_explicit_attrib_location : require\n"
   "layout (location = 0) out uint my_output;\n"
   "void main() {\n" "  my_output = 1u;\n" "} \n";

static const char *fs_int_read =
   "#version 130\n"
   "#extension GL_ARB_texture_multisample : require\n"
   "uniform usampler2DMS sampler;\n"
   "out uint my_output;\n"
   "void main() {\n"
   "  my_output = uint(texelFetch(sampler, ivec2(0,0), 0).r);\n" "} \n";

struct texture_type
{
   const char *name;
   GLenum internal_type;
} types_float[] = {
   {"GL_R8", GL_R8 },
   {"GL_R8_SNORM", GL_R8_SNORM },
   {"GL_R16", GL_R16 },
   {"GL_R16_SNORM", GL_R16_SNORM },
   {"GL_RG8", GL_RG8 },
   {"GL_RG8_SNORM", GL_RG8_SNORM },
   {"GL_RG16", GL_RG16 },
   {"GL_RG16_SNORM", GL_RG16_SNORM },
   {"GL_R3_G3_B2", GL_R3_G3_B2 },
   {"GL_RGB4", GL_RGB4 },
   {"GL_RGB5", GL_RGB5 },
   {"GL_RGB8", GL_RGB8 },
   {"GL_RGB8_SNORM", GL_RGB8_SNORM },
   {"GL_RGB10", GL_RGB10 },
   {"GL_RGB12", GL_RGB12 },
   {"GL_RGB16_SNORM", GL_RGB16_SNORM },
   {"GL_RGBA2", GL_RGBA2 },
   {"GL_RGBA4", GL_RGBA4 },
   {"GL_RGB5_A1", GL_RGB5_A1 },
   {"GL_RGBA8", GL_RGBA8 },
   {"GL_RGBA8_SNORM", GL_RGBA8_SNORM },
   {"GL_RGB10_A2", GL_RGB10_A2 },
   {"GL_RGBA12", GL_RGBA12 },
   {"GL_RGBA16", GL_RGBA16 },
   {"GL_SRGB8", GL_SRGB8 },
   {"GL_SRGB8_ALPHA8", GL_SRGB8_ALPHA8 },
   {"GL_R16F", GL_R16F },
   {"GL_RG16F", GL_RG16F },
   {"GL_RGB16F", GL_RGB16F },
   {"GL_RGBA16F", GL_RGBA16F },
   {"GL_R32F", GL_R32F },
   {"GL_RG32F", GL_RG32F },
   {"GL_RGB32F", GL_RGB32F },
   {"GL_RGBA32F", GL_RGBA32F },
   {"GL_R11F_G11F_B10F", GL_R11F_G11F_B10F },
   {"GL_RGB9_E5", GL_RGB9_E5 }
},
types_int[]={
   {"GL_R8I", GL_R8I },
   {"GL_R8UI", GL_R8UI },
   {"GL_R16I", GL_R16I },
   {"GL_R16UI", GL_R16UI },
   {"GL_R32I", GL_R32I },
   {"GL_R32UI", GL_R32UI },
   {"GL_RG8I", GL_RG8I },
   {"GL_RG8UI", GL_RG8UI },
   {"GL_RG16I", GL_RG16I },
   {"GL_RG16UI", GL_RG16UI },
   {"GL_RG32I", GL_RG32I },
   {"GL_RG32UI", GL_RG32UI },
   {"GL_RGB8I", GL_RGB8I },
   {"GL_RGB8UI", GL_RGB8UI },
   {"GL_RGB16I", GL_RGB16I },
   {"GL_RGB16UI", GL_RGB16UI },
   {"GL_RGB32I", GL_RGB32I },
   {"GL_RGB32UI", GL_RGB32UI },
   {"GL_RGBA8I", GL_RGBA8I },
   {"GL_RGBA8UI", GL_RGBA8UI },
   {"GL_RGBA16I", GL_RGBA16I },
   {"GL_RGBA16UI", GL_RGBA16UI },
   {"GL_RGBA32I", GL_RGBA32I },
   {"GL_RGBA32UI", GL_RGBA32UI },
   {"GL_RGB10_A2UI", GL_RGB10_A2UI }
   };

static const float quad_points[] = {
   1.0f, -1.0f,
   1.0f, 1.0f,
   -1.0f, -1.0f,
   -1.0f, 1.0f
};

static GLuint prog_float_draw, prog_float_read, prog_int_draw, prog_int_read;

enum piglit_result
read_from_texture(GLenum internal_type, bool is_int)
{
   GLuint init_fbo, result_fbo;
   GLuint init_texture, result_texture;
   enum piglit_result subtest_result = PIGLIT_PASS;
   float *data_f;
   unsigned char *data_i;
   GLenum format = is_int ? GL_R8UI : GL_R16;

   glCreateFramebuffers(1, &init_fbo);
   glBindFramebuffer(GL_FRAMEBUFFER, init_fbo);

   glCreateTextures(GL_TEXTURE_2D_MULTISAMPLE, 1, &init_texture);
   glTextureStorage2DMultisample(init_texture, 4, internal_type, piglit_width,
                                 piglit_height, GL_FALSE);

   glNamedFramebufferTexture(init_fbo, GL_COLOR_ATTACHMENT0, init_texture, 0);
   glNamedFramebufferDrawBuffer(init_fbo, GL_COLOR_ATTACHMENT0);

   GLenum status = glCheckNamedFramebufferStatus(init_fbo, GL_DRAW_FRAMEBUFFER);
   if (status == GL_FRAMEBUFFER_UNSUPPORTED)
      return PIGLIT_SKIP;
   if (status != GL_FRAMEBUFFER_COMPLETE)
      return PIGLIT_FAIL;

   if (is_int)
      glUseProgram(prog_int_draw);
   else
      glUseProgram(prog_float_draw);

   glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

   glCreateFramebuffers(1, &result_fbo);
   glBindFramebuffer(GL_FRAMEBUFFER, result_fbo);

   glCreateTextures(GL_TEXTURE_RECTANGLE, 1, &result_texture);
   glTextureStorage2D(result_texture, 1, format, piglit_width, piglit_height);

   glNamedFramebufferTexture(result_fbo, GL_COLOR_ATTACHMENT0, result_texture, 0);
   glNamedFramebufferDrawBuffer(result_fbo, GL_COLOR_ATTACHMENT0);

   glBindTextureUnit(0, init_texture);

   if (is_int)
      glUseProgram(prog_int_read);
   else
      glUseProgram(prog_float_read);

   glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

   if (is_int) {
      data_i = (unsigned char *) malloc(piglit_width * piglit_height *
                                  sizeof(unsigned char));
      glGetTextureImage(result_texture, 0, GL_RED_INTEGER, GL_UNSIGNED_BYTE,
                        piglit_width * piglit_height * sizeof(unsigned char),
                        data_i);
   }
   else {
      data_f = (float *) malloc(piglit_width * piglit_height * sizeof(float));
      glGetTextureImage(result_texture, 0, GL_RED, GL_FLOAT,
                        piglit_width * piglit_height * sizeof(float), data_f);
   }

   for (int i = 0; i < piglit_width * piglit_height; ++i) {
      if (is_int) {
         if (data_i[i] != 1) {
            subtest_result = PIGLIT_FAIL;
            break;
         }
      }
      else {
         if (data_f[i] != 1.0) {
            subtest_result = PIGLIT_FAIL;
            break;
         }
      }
   }

   if (is_int)
      free(data_i);
   else
      free(data_f);

   return subtest_result;
}

void
run_subtest(struct texture_type type, bool is_int,
            enum piglit_result *piglit_test_state)
{
   enum piglit_result piglit_subtest_state = PIGLIT_PASS;

   piglit_subtest_state = read_from_texture(type.internal_type, is_int);
   piglit_report_subtest_result(piglit_subtest_state, "Texture type: %s",
                                type.name);
   piglit_merge_result(piglit_test_state, piglit_subtest_state);
}

enum piglit_result
piglit_display(void)
{
   enum piglit_result piglit_test_state = PIGLIT_PASS;

   for (int i = 0; i < ARRAY_SIZE(types_int); ++i)
         run_subtest(types_int[i],true, &piglit_test_state);

   for (int i = 0; i < ARRAY_SIZE(types_float); ++i)
         run_subtest(types_float[i],false, &piglit_test_state);

   return piglit_test_state;
}



void
piglit_init(int argc, char *argv[])
{

   piglit_require_extension("GL_ARB_direct_state_access");
   piglit_require_extension("GL_ARB_texture_multisample");

   GLuint vao;
   GLuint vbo;

   glGenVertexArrays(1, &vao);
   glBindVertexArray(vao);
   prog_float_draw = piglit_build_simple_program(vs_src, fs_float_draw);
   prog_float_read = piglit_build_simple_program(vs_src, fs_float_read);
   prog_int_draw = piglit_build_simple_program(vs_src, fs_int_draw);
   prog_int_read = piglit_build_simple_program(vs_src, fs_int_read);

   glGenBuffers(1, &vbo);
   glBindBuffer(GL_ARRAY_BUFFER, vbo);
   glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(float), quad_points,
                GL_STATIC_DRAW);
   glEnableVertexAttribArray(0);
   glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
}
