/*
 * Copyright 2010 VMware, Inc.
 * Copyright © 2012 Intel Corporation
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
 * Test GL_NV_primitive_restart and/or GL 3.1 primitive restart.
 * Note that these two extensions/features use different enum values
 * and Enable/Disable functions!
 *
 * Authors:
 *    Brian Paul
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_width = 400;
	config.window_height = 300;
	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;
	config.khr_no_error_support = PIGLIT_NO_ERRORS;

PIGLIT_GL_TEST_CONFIG_END

static const char *TestName = "primitive-restart";

typedef enum {
   DISABLE_VBO,
   VBO_VERTEX_ONLY,
   VBO_INDEX_ONLY,
   VBO_SEPARATE_VERTEX_AND_INDEX,
   VBO_COMBINED_VERTEX_AND_INDEX,
   ALL_TESTS,
} VBO_CFG;

static char* vbo_cfg_names[] = {
   "DISABLE_VBO",
   "VBO_VERTEX_ONLY",
   "VBO_INDEX_ONLY",
   "VBO_SEPARATE_VERTEX_AND_INDEX",
   "VBO_COMBINED_VERTEX_AND_INDEX",
   "all",
};

static VBO_CFG vbo_init_cfg = DISABLE_VBO;

static const GLfloat red[4] = {1.0, 0.0, 0.0, 1.0};
static const GLfloat green[4] = {0.0, 1.0, 0.0, 0.0};
static const GLfloat black[4] = {0.0, 0.0, 0.0, 0.0};

static bool Have_NV;
static bool Have_31;
static bool TestGL31;


static bool
check_rendering(void)
{
   const GLfloat x0 = 0.0, x1 = piglit_width - 10.0, dx = 20.0;
   const GLint iy = piglit_height / 2;
   bool draw = true;
   GLfloat x;

   if (!piglit_probe_pixel_rgb(0, 0, black)) {
      return false;
   }

   for (x = x0 + 0.5 * dx; x < x1; x += dx) {
      bool pass;
      const int ix = (int) x;

      if (draw) {
         /* there should be triangle drawing here */
         pass = piglit_probe_pixel_rgb(ix, iy, green);
      }
      else {
         /* there should not be triangle drawing here */
         pass = piglit_probe_pixel_rgb(ix, iy, black);
      }

      /* debug */
      if (0) {
         glWindowPos2i(ix, iy);
         glDrawPixels(1, 1, GL_RGBA, GL_FLOAT, red);
      }

      if (!pass) {
         return false;
      }

      draw = !draw;
   }

   return true;
}


/**
 * Test glBegin(GL_TRIANGLE/LINE_STRIP), glPrimitiveRestartNV(), glEnd().
 */
static bool
test_begin_end(GLenum primMode)
{
   const GLfloat x0 = 0.0, x1 = piglit_width - 10.0, dx = 20.0;
   const GLfloat y0 = 0.5 * piglit_height - 10.0, y1 = y0 + 20.0, dy = 20.0;
   GLfloat x, y;
   GLint vert;
   bool pass;

   piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);

   glClear(GL_COLOR_BUFFER_BIT);

   glColor4fv(green);

   if (primMode == GL_TRIANGLE_STRIP) {
      /* Draw a tri-strip across the window, using restart to actually render
       * a series of quads/boxes.
       */
      glBegin(GL_TRIANGLE_STRIP);
      vert = 0;
      for (x = x0; x <= x1; x += dx) {
         for (y = y0; y <= y1; y += dy) {
            glVertex2f(x, y);
         }

         vert++;
         if (vert % 2 == 0)
            glPrimitiveRestartNV();
      }
      glEnd();
   }
   else {
      /* Draw a line strip across the window, using restart to actually render
       * a series of disconnected lines.
       */
      glLineWidth(5.0);
      glBegin(GL_LINE_STRIP);
      vert = 0;
      for (x = x0; x <= x1; x += dx) {
         y = 0.5 * piglit_height;

         glVertex2f(x, y);

         vert++;
         if (vert % 2 == 0)
            glPrimitiveRestartNV();
      }
      glEnd();
   }

   glFinish();

   pass = check_rendering();
   if (!pass) {
      fprintf(stderr, "%s: failure drawing with glBegin(%s) / glEnd()\n",
              TestName, piglit_get_prim_name(primMode));
   }

   piglit_present_results();

   return pass;
}


static void
enable_restart(GLuint restart_index)
{
   if (TestGL31) {
      glEnable(GL_PRIMITIVE_RESTART);
      glPrimitiveRestartIndex(restart_index);
   }
   else {
      glEnableClientState(GL_PRIMITIVE_RESTART_NV);
      glPrimitiveRestartIndexNV(restart_index);
   }
}


static void
disable_restart(void)
{
   if (TestGL31) {
      glDisable(GL_PRIMITIVE_RESTART);
   }
   else {
      glDisableClientState(GL_PRIMITIVE_RESTART_NV);
   }
}


static GLuint type_size(GLenum type)
{
   switch (type) {
   case GL_UNSIGNED_BYTE:
      return sizeof(GLubyte);
   case GL_UNSIGNED_SHORT:
      return sizeof(GLushort);
   case GL_UNSIGNED_INT:
      return sizeof(GLuint);
   default:
      assert(0);
      return 0;
   }
}


static GLuint type_array_size(GLenum type, GLuint length)
{
   return length * type_size(type);
}


static GLuint read_index_value(const GLvoid *indices, GLenum type, GLuint index)
{
   switch (type) {
   case GL_UNSIGNED_BYTE:
      return (GLuint)((GLubyte*)indices)[index];
   case GL_UNSIGNED_SHORT:
      return (GLuint)((GLushort*)indices)[index];
   case GL_UNSIGNED_INT:
      return ((GLuint*)indices)[index];
   default:
      assert(0);
      return 0;
   }
}


static void write_index_value(const GLvoid *indices, GLenum type, GLuint index, GLuint value)
{
   switch (type) {
   case GL_UNSIGNED_BYTE:
      ((GLubyte*)indices)[index] = (GLubyte) value;
      break;
   case GL_UNSIGNED_SHORT:
      ((GLushort*)indices)[index] = (GLushort) value;
      break;
   case GL_UNSIGNED_INT:
      ((GLuint*)indices)[index] = value;
      break;
   default:
      assert(0);
   }
}


static void do_ArrayElement(GLenum mode, GLsizei count,
                             GLenum type, const GLvoid *indices)
{
   GLuint index;

   glBegin(mode);
   for (index = 0; index < count; index++) {
      glArrayElement(read_index_value(indices, type, index));
   }
   glEnd();
}


/**
 * Test glDrawElements() with glPrimitiveRestartIndexNV().
 */
static bool
test_draw_by_index(VBO_CFG vbo_cfg, bool one_by_one, GLenum primMode, GLenum indexType)
{
#define NUM_VERTS 48
#define NUM_ELEMS (NUM_VERTS * 5 / 4)
   GLfloat verts[NUM_VERTS+2][2];
   GLubyte indices[sizeof(GLuint) * NUM_ELEMS];
   GLfloat x, dx;
   GLuint restart_index;
   GLuint num_elems;
   bool pass = true;
   GLuint vbo1, vbo2;
   bool create_vbo1 = false;
   bool create_vbo2 = false;
   uintptr_t index_offset = 0;
   uintptr_t vbo_data_size = sizeof(verts) + sizeof(indices);
   GLuint i, j;

   if ((vbo_cfg != DISABLE_VBO) && (vbo_cfg != VBO_INDEX_ONLY)) {
      create_vbo1 = true;
   }

   if ((vbo_cfg == VBO_INDEX_ONLY) || (vbo_cfg == VBO_SEPARATE_VERTEX_AND_INDEX)) {
      create_vbo2 = true;
   }

   if ((vbo_cfg == DISABLE_VBO) || (vbo_cfg == VBO_VERTEX_ONLY)) {
      index_offset = (uintptr_t) indices;
   } else if (vbo_cfg == VBO_COMBINED_VERTEX_AND_INDEX) {
      index_offset = sizeof(verts);
   } else {
      index_offset = 0;
   }

   switch (indexType) {
   case GL_UNSIGNED_BYTE:
      restart_index = 255;
      break;
   case GL_UNSIGNED_SHORT:
      restart_index = 1000;
      break;
   case GL_UNSIGNED_INT:
      restart_index = 1000 * 1000;
      break;
   default:
      assert(0);
      restart_index = 0;
   }

   x = 0.0;
   dx = 20.0;

   if (primMode == GL_TRIANGLE_STRIP) {
      const GLfloat y = 0.5 * piglit_height - 10.0, dy = 20.0;
      for (i = 0; i < NUM_VERTS / 2; i++) {
         verts[i*2+0][0] = x;
         verts[i*2+0][1] = y;
         verts[i*2+1][0] = x;
         verts[i*2+1][1] = y + dy;
         x += dx;
      }

      /* setup elements to draw series of squares w/ tri strip */
      for (i = j = 0; i < NUM_VERTS; i++) {
         write_index_value(indices, indexType, j, i);
         j++;
         if (i > 0 && i % 4 == 3) {
            write_index_value(indices, indexType, j, restart_index);
            j++;
         }
      }

      num_elems = j;
   }
   else {
      const GLfloat y = 0.5 * piglit_height;

      assert(primMode == GL_LINE_STRIP);

      glLineWidth(5.0);

      for (i = 0; i < NUM_VERTS; i++) {
         verts[i][0] = x;
         verts[i][1] = y;
         x += dx;
      }

      /* setup elements to draw series of disjoint lines w/ line strip */
      for (i = j = 0; i < NUM_VERTS / 2; i++) {
         write_index_value(indices, indexType, j, i);
         j++;
         if (i > 0 && i % 2 == 1) {
            write_index_value(indices, indexType, j, restart_index);
            j++;
         }
      }

      num_elems = j;
   }

   assert(num_elems <= NUM_ELEMS);

   /* debug */
   if (0) {
      for (i = 0; i < num_elems; i++)
         printf("%2d: %d\n", i, read_index_value(indices, indexType, i));
   }

   piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);

   glClear(GL_COLOR_BUFFER_BIT);

   glColor4fv(green);

   if (create_vbo1) {
      glGenBuffers(1, &vbo1);
      glBindBuffer(GL_ARRAY_BUFFER, vbo1);
      glBufferData(GL_ARRAY_BUFFER, vbo_data_size, NULL, GL_STATIC_DRAW);
   }

   if (create_vbo2) {
      glGenBuffers(1, &vbo2);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo2);
      glBufferData(GL_ELEMENT_ARRAY_BUFFER, vbo_data_size, NULL, GL_STATIC_DRAW);
   } else {
      vbo2 = vbo1;
   }

   if (create_vbo1) {
      /* Load vertex data into VBO */
      glBindBuffer(GL_ARRAY_BUFFER, vbo1);
      glBufferSubData(GL_ARRAY_BUFFER,
                      0, sizeof(verts),
                      verts);
      glVertexPointer(2, GL_FLOAT, 0, (void *)0);
   } else {
      glVertexPointer(2, GL_FLOAT, 0, (void *)verts);
   }

   if ((vbo_cfg != DISABLE_VBO) && (vbo_cfg != VBO_VERTEX_ONLY)) {
      /* Load index data into VBO */
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo2);
      glBufferSubData(GL_ELEMENT_ARRAY_BUFFER,
                      index_offset, type_array_size(indexType, num_elems),
                      indices);
   }

   glEnableClientState(GL_VERTEX_ARRAY);
   pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

   enable_restart(restart_index);

   /* Draw */
   if (one_by_one) {
      do_ArrayElement(primMode, num_elems, indexType, indices);
   } else {
      glDrawElements(primMode, num_elems, indexType, (void*) index_offset);
   }

   disable_restart();

   glDisableClientState(GL_VERTEX_ARRAY);

   if (vbo_cfg != DISABLE_VBO) {
      glBindBuffer(GL_ARRAY_BUFFER, 0);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
   }

   if (create_vbo1) {
      glDeleteBuffers(1, &vbo1);
   }

   if (create_vbo2) {
      glDeleteBuffers(1, &vbo2);
   }

   if (!check_rendering()) {
      fprintf(stderr, "%s: failure drawing with %s(%s, %s), %s\n",
              TestName,
              one_by_one ? "glArrayElement" : "glDrawElements",
              piglit_get_prim_name(primMode),
              piglit_get_gl_enum_name(indexType),
              vbo_cfg_names[vbo_cfg]);
      pass = false;
   }

   piglit_present_results();

   return pass;
#undef NUM_VERTS
}


/**
 * Test glDrawElements() with glPrimitiveRestartIndexNV().
 */
static bool
test_draw_elements(VBO_CFG vbo_cfg, GLenum primMode, GLenum indexType)
{
   return test_draw_by_index(vbo_cfg, false, primMode, indexType);
}


/**
 * Test glArrayElement() with glPrimitiveRestartIndexNV().
 */
static bool
test_array_element(VBO_CFG vbo_cfg, GLenum primMode, GLenum indexType)
{
   return test_draw_by_index(vbo_cfg, true, primMode, indexType);
}


bool
primitive_restart_test(VBO_CFG vbo_cfg)
{
   bool pass = true;

   if (Have_NV) {
      TestGL31 = false;
      pass = test_begin_end(GL_TRIANGLE_STRIP) && pass;
      pass = test_begin_end(GL_LINE_STRIP) && pass;
      pass = test_draw_elements(vbo_cfg, GL_TRIANGLE_STRIP, GL_UNSIGNED_BYTE) && pass;
      pass = test_draw_elements(vbo_cfg, GL_TRIANGLE_STRIP, GL_UNSIGNED_SHORT) && pass;
      pass = test_draw_elements(vbo_cfg, GL_TRIANGLE_STRIP, GL_UNSIGNED_INT) && pass;
      pass = test_draw_elements(vbo_cfg, GL_LINE_STRIP, GL_UNSIGNED_BYTE) && pass;
      pass = test_draw_elements(vbo_cfg, GL_LINE_STRIP, GL_UNSIGNED_SHORT) && pass;
      pass = test_draw_elements(vbo_cfg, GL_LINE_STRIP, GL_UNSIGNED_INT) && pass;
      pass = test_array_element(vbo_cfg, GL_TRIANGLE_STRIP, GL_UNSIGNED_BYTE) && pass;
      pass = test_array_element(vbo_cfg, GL_TRIANGLE_STRIP, GL_UNSIGNED_SHORT) && pass;
      pass = test_array_element(vbo_cfg, GL_TRIANGLE_STRIP, GL_UNSIGNED_INT) && pass;
      pass = test_array_element(vbo_cfg, GL_LINE_STRIP, GL_UNSIGNED_BYTE) && pass;
      pass = test_array_element(vbo_cfg, GL_LINE_STRIP, GL_UNSIGNED_SHORT) && pass;
      pass = test_array_element(vbo_cfg, GL_LINE_STRIP, GL_UNSIGNED_INT) && pass;
   }

   if (Have_31) {
      TestGL31 = true;
      pass = test_draw_elements(vbo_cfg, GL_TRIANGLE_STRIP, GL_UNSIGNED_BYTE) && pass;
      pass = test_draw_elements(vbo_cfg, GL_TRIANGLE_STRIP, GL_UNSIGNED_SHORT) && pass;
      pass = test_draw_elements(vbo_cfg, GL_TRIANGLE_STRIP, GL_UNSIGNED_INT) && pass;
      pass = test_draw_elements(vbo_cfg, GL_LINE_STRIP, GL_UNSIGNED_BYTE) && pass;
      pass = test_draw_elements(vbo_cfg, GL_LINE_STRIP, GL_UNSIGNED_SHORT) && pass;
      pass = test_draw_elements(vbo_cfg, GL_LINE_STRIP, GL_UNSIGNED_INT) && pass;
   }

   return pass;
}


enum piglit_result
piglit_display(void)
{
   if (vbo_init_cfg == ALL_TESTS) {
      VBO_CFG vbo_cfg;
      for (vbo_cfg = 0; vbo_cfg < ARRAY_SIZE(vbo_cfg_names); vbo_cfg++) {
         if ((vbo_cfg != ALL_TESTS) && !primitive_restart_test(vbo_cfg)) {
            return PIGLIT_FAIL;
         }
      }
      return PIGLIT_PASS;
   } else {
      return primitive_restart_test(vbo_init_cfg) ? PIGLIT_PASS : PIGLIT_FAIL;
   }
}


void
piglit_init(int argc, char **argv)
{
   Have_NV = piglit_is_extension_supported("GL_NV_primitive_restart");
   Have_31 = piglit_get_gl_version() >= 31;

   if (argc >= 2) {
      VBO_CFG vbo_cfg;
      for (vbo_cfg = 0; vbo_cfg < ARRAY_SIZE(vbo_cfg_names); vbo_cfg++) {
         if (strcmp(argv[1], vbo_cfg_names[vbo_cfg]) == 0) {
            vbo_init_cfg = vbo_cfg;
            break;
         }
      }
   }

   /* Debug */
   if (0) {
      printf("Have NV: %d\n", Have_NV);
      printf("Have 31: %d\n", Have_31);
   }

   if (!Have_NV && !Have_31) {
      piglit_report_result(PIGLIT_SKIP);
      exit(1);
   }

   glClearColor(0, 0, 0, 0);
}
