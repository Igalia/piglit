/*
 * Copyright Â© 2015 Tobias Klausmann <tobias.johannes.klausmann@mni.thm.de>
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
 * \file max-distances.c
 *
 * From the ARB_cull_distance spec:
 *
 * Modify Section 7.3, Built-In Constants
 *
 * (add to the list of implementation-dependent constants after
 *  gl_MaxClipDistances on p. 132)
 *
 *  const int  gl_MaxCullDistances = 8;
 *  const int  gl_MaxCombinedClipAndCullDistances = 8;
 *
 *
 * New Implementation Dependent State
 *
 * (add to table 23.53, Implementation Dependent Values)
 *
 * Get Value                             Type  Get Command  Min. value  Description
 * ------------------------------------  ----  -----------  ----------  ------------------------------
 * MAX_CULL_DISTANCES                     Z+   GetIntegerv      8       Max no. of user culling planes
 * MAX_COMBINED_CLIP_AND_CULL_DISTANCES   Z+   GetIntegerv      8       Max combined no. of user
 *                                                                      clipping and culling planes
 *
 * This test verifies that glGetIntegerv() returns the appropriate values for
 * the tokens MAX_CULL_DISTANCES and MAX_COMBINED_CLIP_AND_CULL_DISTANCES, that
 * these values matches the values of gl_MaxCullDistances, respectively
 * gl_MaxCombinedClipAndCullDistances defined in the vertex and fragment shader
 * and that these values are at least 8.
 */
#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

   config.supports_gl_compat_version = 10;
   config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

static const char vert[] =
   "#version 130\n"
   "#extension GL_ARB_cull_distance: enable\n"
   "uniform int expected_value;\n"
   "uniform bool test_distances;\n"
   "uniform bool test_in_vs;\n"
   "void main()\n"
   "{\n"
   "  gl_Position = gl_Vertex;\n"
   "  if (test_in_vs) {\n"
   "    int value = test_distances ? gl_MaxCullDistances\n"
   "                               : gl_MaxCombinedClipAndCullDistances;\n"
   "    gl_FrontColor = (value == expected_value)\n"
   "                     ? vec4(0.0, 1.0, 0.0, 1.0)\n"
   "                     : vec4(1.0, 0.0, 0.0, 1.0);\n"
   "  }\n"
   "}\n";

static const char frag[] =
   "#version 130\n"
   "#extension GL_ARB_cull_distance: enable\n"
   "uniform int expected_value;\n"
   "uniform bool test_distances;\n"
   "uniform bool test_in_vs;\n"
   "void main()\n"
   "{\n"
   "  if (test_in_vs) {\n"
   "    gl_FragColor = gl_Color;\n"
   "  } else {\n"
   "    int value = test_distances ? gl_MaxCullDistances\n"
   "                               : gl_MaxCombinedClipAndCullDistances;\n"
   "    gl_FragColor = (value == expected_value)\n"
   "                    ? vec4(0.0, 1.0, 0.0, 1.0)\n"
   "                    : vec4(1.0, 0.0, 0.0, 1.0);\n"
   "  }\n"
   "}\n";

GLuint prog;

enum piglit_result
piglit_display(void)
{
   GLint max_cull_distances;
   GLint max_combined_clip_and_cull_distances;
   GLint expected_value;
   GLint test_distances, test_in_vs;
   float green[] = { 0.0, 1.0, 0.0, 1.0 };
   GLint loc;

   enum piglit_result result = PIGLIT_PASS;

   glGetIntegerv(GL_MAX_CULL_DISTANCES, &max_cull_distances);
   printf("GL_MAX_CULL_DISTANCES = %d\n", max_cull_distances);
   if (max_cull_distances < 8) {
      printf("GL_MAX_CULL_DISTANCES < 8\n");
      piglit_report_result(PIGLIT_FAIL);
   }

   glGetIntegerv(GL_MAX_COMBINED_CLIP_AND_CULL_DISTANCES,
                 &max_combined_clip_and_cull_distances);
   printf("GL_MAX_COMBINED_CLIP_AND_CULL_DISTANCES = %d\n",
          max_combined_clip_and_cull_distances);
   if (max_combined_clip_and_cull_distances < 8) {
      printf("GL_MAX_COMBINED_CLIP_AND_CULL_DISTANCES < 8\n");
      piglit_report_result(PIGLIT_FAIL);
   }

   expected_value = max_cull_distances;
   loc = glGetUniformLocation(prog, "expected_value");
   glUniform1i(loc, expected_value);

   for (test_distances = 0; test_distances <= 1; ++test_distances) {
      loc = glGetUniformLocation(prog, "test_distances");
      glUniform1i(loc, test_distances);
      for (test_in_vs = 0; test_in_vs <= 1; ++test_in_vs) {
         bool pass;
         loc = glGetUniformLocation(prog, "test_in_vs");
         glUniform1i(loc, test_in_vs);
         piglit_draw_rect(-1, -1, 2, 2);
         pass = piglit_probe_rect_rgba(0, 0, piglit_width,
                                       piglit_height, green);
         if (test_distances) {
            printf("Checking that gl_MaxCullDistances == %d in %s: %s\n",
                   expected_value,
                   test_in_vs ? "VS" : "FS",
                   pass ? "pass" : "fail");
         }
         else {
            printf("Checking that gl_MaxCombinedClipAndCullDistances "
                   "== %d in %s: %s\n",
                   expected_value,
                   test_in_vs ? "VS" : "FS",
                   pass ? "pass" : "fail");
         }
         if (!pass) {
            result = PIGLIT_FAIL;
         }
      }
   }
   return result;
}

void
piglit_init(int argc, char **argv)
{
	piglit_require_gl_version(30);
	piglit_require_GLSL();
	piglit_require_GLSL_version(130);
	piglit_require_extension("GL_ARB_cull_distance");
	prog = piglit_build_simple_program(vert, frag);
	glUseProgram(prog);
}
