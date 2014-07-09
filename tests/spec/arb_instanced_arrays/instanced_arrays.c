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
 * Tests GL_ARB_instanced_arrays
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_width = 500;
	config.window_height = 500;
	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

static const char *TestName = "draw-instanced-arrays";

static GLint PosAttrib, ColorAttrib;

#define PRIMS 8

static const char *VertShaderText =
   "attribute vec2 Pos; \n"
   "attribute vec4 Color; \n"
   "void main() \n"
   "{ \n"
   "   vec4 p = gl_Vertex; \n"
   "   vec2 pos = Pos; \n"
   "   p.x += pos.x; \n"
   "   p.y += pos.y; \n"
   "   gl_Position = gl_ModelViewProjectionMatrix * p; \n"
   "   gl_FrontColor = Color; \n"
   "} \n";

static const char *FragShaderText =
   "void main() \n"
   "{ \n"
   "   gl_FragColor = gl_Color; \n"
   "} \n";


static GLuint Program;

static GLboolean use_vbo = GL_FALSE;

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
test_instancing(GLuint divisor)
{
   static const GLfloat verts[4][2] = {
      {-1, -1}, {1, -1}, {1, 1}, {-1, 1}
   };
   GLuint vbo;
   uintptr_t offset = 0;

   if (use_vbo) {
      glGenBuffers(1, &vbo);
      glBindBuffer(GL_ARRAY_BUFFER, vbo);
      glBufferData(GL_ARRAY_BUFFER,
                   sizeof(verts) + sizeof(Positions) + sizeof(Colors),
                   NULL,
                   GL_STATIC_DRAW);
      glBufferSubData(GL_ARRAY_BUFFER, offset, sizeof(verts), verts);
      glVertexPointer(2, GL_FLOAT, 0, (void*) offset);
      offset += sizeof(verts);

      glBufferSubData(GL_ARRAY_BUFFER, offset, sizeof(Positions), Positions);
      glVertexAttribPointer(PosAttrib, 2, GL_FLOAT, GL_FALSE, 0, (void*) offset);
      offset += sizeof(Positions);

      glBufferSubData(GL_ARRAY_BUFFER, offset, sizeof(Colors), Colors);
      glVertexAttribPointer(ColorAttrib, 4, GL_FLOAT, GL_FALSE, 0, (void*) offset);
      offset += sizeof(Colors);

      glBindBuffer(GL_ARRAY_BUFFER, 0);
   } else {
      glVertexPointer(2, GL_FLOAT, 0, verts);
      glVertexAttribPointer(PosAttrib, 2, GL_FLOAT, GL_FALSE, 0, Positions);
      glVertexAttribPointer(ColorAttrib, 4, GL_FLOAT, GL_FALSE, 0, Colors);
   }

   glEnableClientState(GL_VERTEX_ARRAY);
   glEnableVertexAttribArray(PosAttrib);
   glEnableVertexAttribArray(ColorAttrib);

   glVertexAttribDivisorARB(PosAttrib, 1);
   /* advance color once every 'n' instances */
   glVertexAttribDivisorARB(ColorAttrib, divisor);

   glClear(GL_COLOR_BUFFER_BIT);

   glUseProgram(Program);

   glDrawArraysInstancedARB(GL_POLYGON, 0, 4, PRIMS);

   glUseProgram(0);

   if (use_vbo) {
      glDeleteBuffers(1, &vbo);
   }

   {
      GLint i;
      GLint pos[4];

      for (i = 0; i < PRIMS; i++) {
         GLuint elem = i / divisor;

         /* use glRasterPos to determine where to read a sample pixel */
         glRasterPos2fv(Positions[i]);
         glGetIntegerv(GL_CURRENT_RASTER_POSITION, pos);

         if (!piglit_probe_pixel_rgba(pos[0], pos[1], Colors[elem])) {
            fprintf(stderr, "%s: instance %d failed to draw correctly\n",
                    TestName, i);
            fprintf(stderr, "%s: color instance divisor = %u\n",
                    TestName, divisor);
            piglit_present_results();
            return GL_FALSE;
         }
      }
   }

   glDisableClientState(GL_VERTEX_ARRAY);
   glDisableVertexAttribArray(PosAttrib);
   glDisableVertexAttribArray(ColorAttrib);

   piglit_present_results();

   return GL_TRUE;
}


enum piglit_result
piglit_display(void)
{
   GLuint div;

   for (div = 1; div <= PRIMS; div++) {
      if (!test_instancing(div))
         return PIGLIT_FAIL;
   }

   return PIGLIT_PASS;
}


void
piglit_init(int argc, char **argv)
{
   piglit_require_GLSL();
   piglit_require_extension("GL_ARB_instanced_arrays");

   Program = piglit_build_simple_program(VertShaderText, FragShaderText);

   glUseProgram(Program);

   PosAttrib = glGetAttribLocation(Program, "Pos");
   ColorAttrib = glGetAttribLocation(Program, "Color");

   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   glFrustum(-5, 5, -5, 5, 10, 20);

   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();
   glTranslatef(0, 0, -11.0);
   glScalef(0.5, 0.5, 1.0);

   if ((argc >= 2) && (strcmp(argv[1], "vbo") == 0)) {
      use_vbo = GL_TRUE;
   }
}
