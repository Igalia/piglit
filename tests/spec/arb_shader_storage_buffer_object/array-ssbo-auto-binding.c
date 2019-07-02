/*
 * Copyright © 2019 Intel Corporation
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
/*
 * \file array-ssbo-auto-binding.c
 *
 * This test verifies automatically assigned binding points for ssbo array and ssbo arrays of arrays
 *
 * \author Andrii Simiklit <asimiklit.work@gmail.com>
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

   config.supports_gl_compat_version = 32;
   config.supports_gl_core_version = 32;
   config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;
   config.khr_no_error_support = PIGLIT_NO_ERRORS;

PIGLIT_GL_TEST_CONFIG_END

static GLuint prog;

struct test_context
{
   bool packed;
   int max_fs_blocks;
};

static void
trace_binding_error(int buffer_binding, int expected_binding, const char *shader)
{
   fprintf(stderr,
           "error: binding point is %d but expected %d for shader:\n%s\n",
           buffer_binding,
           expected_binding,
           shader);
}

static const char*
get_layout(bool packed, int binding)
{
   static char buffer[256];
   const char *format = packed ?
          "layout(packed, binding=%d)" :
          "layout(binding=%d)";

   snprintf(buffer, sizeof(buffer), format, binding);
   return buffer;
}


static const char*
build_1d_shader(bool packed, int binding,
                int array_size, int used_element,
                const char **block_name)
{
   static char name[32];
   static char buffer[1024];
   const char *format =
      "#version 150\n"
      "#extension GL_ARB_arrays_of_arrays : enable\n"
      "#extension GL_ARB_shading_language_420pack : enable\n"
      "#extension GL_ARB_shader_storage_buffer_object : require\n"
      "\n"
      "%s buffer SSBO { vec4 a; } ssbo[%d];\n"
      "out vec4 color;\n"
      "\n"
      "void main()\n"
      "{\n"
      "   color = ssbo[%d].a;\n"
      "}\n";
   snprintf(buffer, sizeof(buffer), format,
            get_layout(packed, binding), array_size, used_element);
   snprintf(name, sizeof(name), "SSBO[%d]", used_element);
   *block_name = name;
   return buffer;
}

static const char*
build_2d_shader(bool packed, int binding,
                int *array_dims, int *used_elements,
                const char **block_name)
{
   static char name[32];
   static char buffer[1024];
   const char *format =
      "#version 150\n"
      "#extension GL_ARB_arrays_of_arrays : enable\n"
      "#extension GL_ARB_shading_language_420pack : enable\n"
      "#extension GL_ARB_shader_storage_buffer_object : require\n"
      "\n"
      "%s buffer SSBO { vec4 a; } ssbo[%d][%d];\n"
      "out vec4 color;\n"
      "\n"
      "void main()\n"
      "{\n"
      "   color = ssbo[%d][%d].a;\n"
      "}\n";
   snprintf(buffer, sizeof(buffer), format, get_layout(packed, binding),
            array_dims[0], array_dims[1],
            used_elements[0], used_elements[1]);
   snprintf(name, sizeof(name), "SSBO[%d][%d]",
            used_elements[0], used_elements[1]);
   *block_name = name;
   return buffer;
}

static const char*
build_3d_shader(bool packed, int binding,
                int *array_dims, int *used_elements,
                const char **block_name)
{
   static char name[32];
   static char buffer[1024];
   const char *format =
      "#version 150\n"
      "#extension GL_ARB_arrays_of_arrays : enable\n"
      "#extension GL_ARB_shading_language_420pack : enable\n"
      "#extension GL_ARB_shader_storage_buffer_object : require\n"
      "\n"
      "%s buffer SSBO { vec4 a; } ssbo[%d][%d][%d];\n"
      "out vec4 color;\n"
      "\n"
      "void main()\n"
      "{\n"
      "   color = ssbo[%d][%d][%d].a;\n"
      "}\n";
   snprintf(buffer, sizeof(buffer), format,
            get_layout(packed, binding),
            array_dims[0], array_dims[1],array_dims[2],
            used_elements[0], used_elements[1], used_elements[2]);
   snprintf(name, sizeof(name), "SSBO[%d][%d][%d]",
            used_elements[0], used_elements[1], used_elements[2]);
   *block_name = name;
   return buffer;
}

static bool
ssbo_array1d_test(struct test_context *ctx)
{
   bool pass = true;
   for (int array_size = 2;
        array_size < ctx->max_fs_blocks; array_size++) {
      for (int used_element = 0;
           used_element < array_size; used_element++) {
         int index;
         const char *blockname;
         int buffer_binding = 0;
         GLenum props = GL_BUFFER_BINDING;
         int expected_binding = 1 + used_element;

         const char *fs = build_1d_shader(ctx->packed, 1, array_size,
                                          used_element, &blockname);
         prog = piglit_build_simple_program(NULL, fs);
         index = glGetProgramResourceIndex(prog,
                 GL_SHADER_STORAGE_BLOCK, blockname);
         glGetProgramResourceiv(prog, GL_SHADER_STORAGE_BLOCK, index, 1,
                                &props, 1, NULL, &buffer_binding);
         if (expected_binding != buffer_binding) {
            trace_binding_error(buffer_binding, expected_binding, fs);
            pass = false;
         }
      }
   }
   return pass;
}

static bool
ssbo_array2d_test(struct test_context *ctx)
{
   bool pass = true;
   int array_dims[2];

   for (array_dims[0] = 2;
        array_dims[0] < (ctx->max_fs_blocks / 2); array_dims[0]++) {

      int used_elements[2];
      array_dims[1] = ctx->max_fs_blocks / array_dims[0];

      for (used_elements[0] = 0;
           used_elements[0] < array_dims[0]; used_elements[0]++) {
         for (used_elements[1] = 0;
              used_elements[1] < array_dims[1]; used_elements[1]++) {
            int index;
            const char *blockname;
            int buffer_binding = 0;
            GLenum props = GL_BUFFER_BINDING;
            int expected_binding = 1 +
               ((used_elements[0] * array_dims[1]) + used_elements[1]);

            const char *fs = build_2d_shader(ctx->packed, 1, array_dims,
                                             used_elements, &blockname);
            prog = piglit_build_simple_program(NULL, fs);
            index = glGetProgramResourceIndex(prog,
                    GL_SHADER_STORAGE_BLOCK, blockname);
            glGetProgramResourceiv(prog, GL_SHADER_STORAGE_BLOCK, index, 1,
                                   &props, 1, NULL, &buffer_binding);
            if (expected_binding != buffer_binding) {
               trace_binding_error(buffer_binding, expected_binding, fs);
               pass = false;
            }
         }
      }
   }
   return pass;
}

static bool
ssbo_array3d_test(struct test_context *ctx)
{
   bool pass = true;
   int array_dims[3];

   for(array_dims[0] = 2;
       array_dims[0] < (ctx->max_fs_blocks / 2); array_dims[0]++) {

      int used_elements[3];
      array_dims[1] = (ctx->max_fs_blocks / array_dims[0]) / 2;
      array_dims[2] = (ctx->max_fs_blocks / (array_dims[1] * array_dims[0]));

      for(used_elements[0] = 0;
          used_elements[0] < array_dims[0]; used_elements[0]++) {
         for(used_elements[1] = 0;
             used_elements[1] < array_dims[1]; used_elements[1]++) {
            for(used_elements[2] = 0;
                used_elements[2] < array_dims[2]; used_elements[2]++) {
               int index;
               const char *blockname;
               int buffer_binding = 0;
               GLenum props = GL_BUFFER_BINDING;
               int expected_binding = 1 +
                  (used_elements[0] * array_dims[1] * array_dims[2]) +
                  (used_elements[1] * array_dims[2]) +
                  used_elements[2];

               const char *fs = build_3d_shader(ctx->packed, 1, array_dims,
                                                used_elements, &blockname);
               prog = piglit_build_simple_program(NULL, fs);
               index = glGetProgramResourceIndex(prog,
                       GL_SHADER_STORAGE_BLOCK, blockname);
               glGetProgramResourceiv(prog, GL_SHADER_STORAGE_BLOCK, index, 1,
                                      &props, 1, NULL, &buffer_binding);
               if (expected_binding != buffer_binding) {
                  trace_binding_error(buffer_binding, expected_binding, fs);
                  pass = false;
               }
            }
         }
      }
   }
   return pass;
}


void
piglit_init(int argc, char **argv)
{
   bool pass = true;
   struct test_context ctx = {
      .packed = true
   };

   glGetIntegerv(GL_MAX_FRAGMENT_SHADER_STORAGE_BLOCKS, &ctx.max_fs_blocks);

   pass = ssbo_array1d_test(&ctx) && pass;
   pass = ssbo_array2d_test(&ctx) && pass;
   pass = ssbo_array3d_test(&ctx) && pass;

   ctx.packed = false;
   pass = ssbo_array1d_test(&ctx) && pass;
   pass = ssbo_array2d_test(&ctx) && pass;
   pass = ssbo_array3d_test(&ctx) && pass;

   piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}

enum piglit_result piglit_display(void)
{
   /* UNREACHED */
   return PIGLIT_FAIL;
}
