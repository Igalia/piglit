/*
 * Copyright © 2009 VMware, Inc.
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
 * Author:
 *    Brian Paul
 *
 */

/**
 * @file glsl-arb-fragment-coord-conventions.c
 *
 * Test ARB_fragment_coord_conventions extension.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 20;

	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END


/* Test size / region */
#define WIDTH 100
#define HEIGHT 100


static const float black[4] = {0.0, 0.0, 0.0, 0.0};
static const float red[4] = {1.0, 0.0, 0.0, 0.0};
static const float green[4] = {0.0, 1.0, 0.0, 0.0};
static const float yellow[4] = {1.0, 1.0, 0.0, 0.0};
static const float gray25_l[4] = {0.25, 0.25, 0.0, 0.0};
static const float gray25_r[4] = {0.25, 0.25, 1.0, 0.0};
static const float gray75_l[4] = {0.75, 0.75, 0.5, 0.0};
static const float gray75_r[4] = {0.75, 0.75, 1.0, 0.0};

static int test = 0;

/*
 * For each of the various pixel center/origin layout qualifier modes
 * draw a full-window quad where the fragment color is a function of
 * the fragment coordinate.
 */
enum piglit_result
piglit_display(void)
{
   GLuint prog;
   GLuint vs, fs;
   GLboolean pass = GL_TRUE;

   if (piglit_width < WIDTH || piglit_height < HEIGHT) {
      printf("window is too small.\n");
      return PIGLIT_SKIP;
   }

   piglit_ortho_projection(WIDTH, HEIGHT, GL_FALSE);

   vs = piglit_compile_shader(GL_VERTEX_SHADER, "shaders/glsl-mvp.vert");

   /* No layout: test regular gl_FragCoord */
   if (piglit_automatic || test == 0)
   {
      const char *fragtext =
         "void main(void) \n"
         "{ \n"
         "   gl_FragColor = gl_FragCoord * 0.01; \n"
         "} \n";

      printf("Regular gl_FragCoord\n");
      fs = piglit_compile_shader_text(GL_FRAGMENT_SHADER, fragtext);
      prog = piglit_link_simple_program(vs, fs);
      glUseProgram(prog);

      glClear(GL_COLOR_BUFFER_BIT);
      glViewport(0, 0, WIDTH, HEIGHT);

      piglit_draw_rect(0, 0, WIDTH, HEIGHT);

      /* lower-left corner */
      pass = piglit_probe_pixel_rgb(0, 0, black) && pass;

      /* upper-right corner */
      pass = piglit_probe_pixel_rgb(WIDTH - 1, HEIGHT - 1, yellow) && pass;
   }

   /* No layout, test pixel center is half integer */
   if (piglit_automatic || test == 1)
   {
      const char *fragtext =
         "#extension GL_ARB_fragment_coord_conventions: enable \n"
         "void main(void) \n"
         "{ \n"
         "   gl_FragColor = fract(gl_FragCoord) + 0.25; \n"
         "   gl_FragColor.z = (gl_FragCoord.x + gl_FragCoord.y) * 0.5; \n"
         "} \n";

      printf("Pixel center half integer\n");
      fs = piglit_compile_shader_text(GL_FRAGMENT_SHADER, fragtext);
      prog = piglit_link_simple_program(vs, fs);
      glUseProgram(prog);

      glClear(GL_COLOR_BUFFER_BIT);
      glViewport(0, 0, WIDTH, HEIGHT);

      piglit_draw_rect(0, 0, WIDTH, HEIGHT);

      /* lower-left corner */
      pass = piglit_probe_pixel_rgb(0, 0, gray75_l) && pass;

      /* upper-right corner */
      pass = piglit_probe_pixel_rgb(WIDTH - 1, HEIGHT - 1, gray75_r) && pass;
   }

   /* Pixel center integer */
   if (piglit_automatic || test == 2)
   {
      const char *fragtext =
         "#extension GL_ARB_fragment_coord_conventions: enable \n"
         "layout(pixel_center_integer) varying vec4 gl_FragCoord; \n"
         "void main(void) \n"
         "{ \n"
         "   gl_FragColor = fract(gl_FragCoord) + 0.25; \n"
         "   gl_FragColor.z = (gl_FragCoord.x + gl_FragCoord.y) * 0.5; \n"
         "} \n";

      printf("Pixel center integer\n");
      fs = piglit_compile_shader_text(GL_FRAGMENT_SHADER, fragtext);
      prog = piglit_link_simple_program(vs, fs);
      glUseProgram(prog);

      glClear(GL_COLOR_BUFFER_BIT);
      glViewport(0, 0, WIDTH, HEIGHT);

      piglit_draw_rect(0, 0, WIDTH, HEIGHT);

      /* lower-left corner */
      pass = piglit_probe_pixel_rgb(0, 0, gray25_l) && pass;

      /* upper-right corner */
      pass = piglit_probe_pixel_rgb(WIDTH - 1, HEIGHT - 1, gray25_r) && pass;
   }

   /* Pixel origin upper left */
   if (piglit_automatic || test == 3)
   {
      const char *fragtext =
         "#extension GL_ARB_fragment_coord_conventions: enable \n"
         "layout(origin_upper_left) varying vec4 gl_FragCoord; \n"
         "void main(void) \n"
         "{ \n"
         "   gl_FragColor = gl_FragCoord * 0.01; \n"
         "   gl_FragColor.z = 0.0; \n"
         "} \n";
      int y0 = piglit_height - HEIGHT;

      printf("Pixel origin upper left\n");
      fs = piglit_compile_shader_text(GL_FRAGMENT_SHADER, fragtext);
      prog = piglit_link_simple_program(vs, fs);
      glUseProgram(prog);

      glClear(GL_COLOR_BUFFER_BIT);
      glViewport(0, y0, WIDTH, HEIGHT);

      piglit_draw_rect(0, 0, WIDTH, HEIGHT);

      /* lower-left corner */
      pass = piglit_probe_pixel_rgb(0, y0, green) && pass;

      /* upper-right corner */
      pass = piglit_probe_pixel_rgb(WIDTH - 1, y0 + HEIGHT - 1, red) && pass;
   }

   /* Pixel origin upper left and pixel center integer */
   if (piglit_automatic || test == 4)
   {
      static const float color1[4] = {0.125, 0.3725, 0.0, 0.0};
      static const float color2[4] = {0.3725, 0.125, 0.0, 0.0};
      const char *fragtext =
         "#extension GL_ARB_fragment_coord_conventions: enable \n"
         "layout(origin_upper_left, pixel_center_integer) varying vec4 gl_FragCoord; \n"
         "void main(void) \n"
         "{ \n"
         "   gl_FragColor = gl_FragCoord * 0.0025 + 0.125; \n"
         "   gl_FragColor.z = 0.0; \n"
         "} \n";
      int y0 = piglit_height - HEIGHT;

      printf("Pixel origin upper left and pixel center integer\n");
      fs = piglit_compile_shader_text(GL_FRAGMENT_SHADER, fragtext);
      prog = piglit_link_simple_program(vs, fs);
      glUseProgram(prog);

      glClear(GL_COLOR_BUFFER_BIT);
      glViewport(0, y0, WIDTH, HEIGHT);

      piglit_draw_rect(0, 0, WIDTH, HEIGHT);

      /* lower-left corner */
      pass = piglit_probe_pixel_rgb(0, y0, color1) && pass;

      /* upper-right corner */
      pass = piglit_probe_pixel_rgb(WIDTH - 1, y0 + HEIGHT - 1, color2) && pass;
   }

   piglit_present_results();

   return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

static void key_func(unsigned char key, int x, int y)
{
   switch (key) {
   case 't':
      test = (test + 1) % 5;
      break;
   }

   piglit_escape_exit_key(key, x, y);
}

void piglit_init(int argc, char **argv)
{
	piglit_require_extension("GL_ARB_fragment_coord_conventions");

	if (!piglit_automatic) {
		printf("Press t to switch between subtests.\n");
		piglit_set_keyboard_func(key_func);
	}
}
