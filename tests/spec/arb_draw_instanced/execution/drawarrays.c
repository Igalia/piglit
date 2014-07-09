/*
 * Copyright (c) 2010 VMware, Inc.
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
 * Tests GL_ARB_draw_instanced
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_width = 500;
	config.window_height = 500;
	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

static const char *TestName = "draw-instanced";

static GLint PosUniform, ColorUniform;

#define PRIMS 8

static const char *VertShaderText =
   "#extension GL_ARB_draw_instanced: enable \n"
   "uniform vec2 Pos[8]; \n"
   "uniform vec4 Color[8]; \n"
   "void main() \n"
   "{ \n"
#if 0
   "   vec4 p = ftransform(); \n"
   "   vec2 pos = Pos[gl_InstanceIDARB]; \n"
   "   p.x += pos.x; \n"
   "   p.y += pos.y; \n"
   "   gl_Position = p; \n"
#else
   "   vec4 p = gl_Vertex; \n"
   "   vec2 pos = Pos[gl_InstanceIDARB]; \n"
   "   p.x += pos.x; \n"
   "   p.y += pos.y; \n"
   "   gl_Position = gl_ModelViewProjectionMatrix * p; \n"
#endif
   "   gl_FrontColor = Color[gl_InstanceIDARB]; \n"
   "} \n";

static const char *FragShaderText =
   "void main() \n"
   "{ \n"
   "   gl_FragColor = gl_Color; \n"
   "} \n";


static GLuint VertShader, FragShader, Program;

static const GLfloat Positions[PRIMS][2] = {
   { -6, 6 },
   { -4, 4 },
   { -2, 2 },
   {  0, 0 },
   {  2, -2 },
   {  4, -4 },
   {  6, -6 },
   {  8, -8 }
};

static const GLfloat Colors[PRIMS][4] = {
   {1, 0, 0, 1},
   {0, 1, 0, 1},
   {0, 0, 1, 1},
   {1, 1, 0, 1},
   {0, 1, 1, 1},
   {1, 0, 1, 1},
   {1, 1, 1, 1},
   {0.5, 0.5, 0.5, 1},
};


static GLboolean
test_instancing(void)
{
   static const GLfloat verts[4][2] = {
      {-1, -1}, {1, -1}, {1, 1}, {-1, 1}
   };
   
   glVertexPointer(2, GL_FLOAT, 0, verts);
   glEnableClientState(GL_VERTEX_ARRAY);
   
   glClear(GL_COLOR_BUFFER_BIT);

   glUseProgram(Program);

   glDrawArraysInstancedARB(GL_POLYGON, 0, 4, PRIMS);

   glUseProgram(0);

   {
      GLint i;
      GLint pos[4];

      for (i = 0; i < PRIMS; i++) {
         /* use glRasterPos to determine where to read a sample pixel */
         glRasterPos2fv(Positions[i]);
         glGetIntegerv(GL_CURRENT_RASTER_POSITION, pos);

         if (!piglit_probe_pixel_rgba(pos[0], pos[1], Colors[i])) {
            fprintf(stderr, "%s: instance %d failed to draw correctly\n",
                    TestName, i);
            piglit_present_results();
            return GL_FALSE;
         }
      }
   }

   piglit_present_results();

   return GL_TRUE;
}


enum piglit_result
piglit_display(void)
{
   if (!test_instancing())
      return PIGLIT_FAIL;

   return PIGLIT_PASS;
}


void
piglit_init(int argc, char **argv)
{
   piglit_require_extension("GL_ARB_draw_instanced");

   VertShader = piglit_compile_shader_text(GL_VERTEX_SHADER, VertShaderText);
   assert(VertShader);

   FragShader = piglit_compile_shader_text(GL_FRAGMENT_SHADER, FragShaderText);
   assert(FragShader);

   Program = piglit_link_simple_program(VertShader, FragShader);

   glUseProgram(Program);

   PosUniform = glGetUniformLocation(Program, "Pos");
   ColorUniform = glGetUniformLocation(Program, "Color");

   glUniform2fv(PosUniform, PRIMS, (GLfloat *) Positions);
   glUniform4fv(ColorUniform, PRIMS, (GLfloat *) Colors);

   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   glFrustum(-5, 5, -5, 5, 10, 20);

   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();
   glTranslatef(0, 0, -11.0);
   glScalef(0.5, 0.5, 1.0);
}
