/*
 * Copyright (c) 2012 Marek Olšák <maraeo@gmail.com>
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
 * NON-INFRINGEMENT.  IN NO EVENT SHALL AUTHORS AND/OR THEIR SUPPLIERS
 * BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
 * ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;
	config.require_debug_context = true;

	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

#define USER_PARAM ((void*)(intptr_t)12345678)

static GLboolean callback_called;

enum piglit_result
piglit_display(void)
{
   return PIGLIT_PASS;
}

static void GLAPIENTRY debug_callback(GLenum source,
                                      GLenum type,
                                      GLuint id,
                                      GLenum severity,
                                      GLsizei length,
                                      const GLchar* message,
                                      GLvoid* userParam)
{
   if (source != GL_DEBUG_SOURCE_API_ARB) {
      puts("source isn't API");
      piglit_report_result(PIGLIT_FAIL);
   }

   if (type != GL_DEBUG_TYPE_ERROR_ARB) {
      puts("type isn't error");
      piglit_report_result(PIGLIT_FAIL);
   }

   if (userParam != USER_PARAM) {
      puts("wrong userParam in the callback");
      piglit_report_result(PIGLIT_FAIL);
   }

   callback_called = GL_TRUE;
   printf("Callback: ");
   fwrite(message, length, 1, stdout);
   printf("\n");
}

static GLboolean fetch_one_log_message()
{
   char log[4096];
   GLboolean ret =
      !!glGetDebugMessageLogARB(1, 4096, NULL, NULL, NULL, NULL, NULL, log);

   if (ret) {
      printf("Log: %s\n", log);
   }
   return ret;
}

enum {
   SKIP_SETUP = 1 << 0,
   DEBUG_ENABLE = 1 << 1,
   CALLBACK_ENABLE = 1 << 2
};

static void test_api_error(unsigned flags)
{
   GLboolean skip_setup = !!(flags & SKIP_SETUP);
   GLboolean debug_enable = !!(flags & DEBUG_ENABLE);
   GLboolean callback_enable = !!(flags & CALLBACK_ENABLE);

   if (!skip_setup) {
      printf("Testing Debug %s and Callback %s\n",
             debug_enable ? "enabled" : "disabled",
             callback_enable ? "enabled" : "disabled");

      glDebugMessageControlARB(GL_DEBUG_SOURCE_API_ARB, GL_DEBUG_TYPE_ERROR_ARB,
                               GL_DEBUG_SEVERITY_HIGH_ARB, 0, NULL, debug_enable);
      glDebugMessageCallbackARB(callback_enable ? debug_callback : NULL, USER_PARAM);
   } else {
      puts("Testing defaults.");
   }

   if (!piglit_check_gl_error(GL_NO_ERROR))
      piglit_report_result(PIGLIT_FAIL);

   /* empty the log */
   while (fetch_one_log_message());

   callback_called = GL_FALSE;
   glEnable(0xFFFFFFFF); /* GL error */

   if (!piglit_check_gl_error(GL_INVALID_ENUM))
      piglit_report_result(PIGLIT_FAIL);

   if (callback_called != (callback_enable && debug_enable)) {
      puts(callback_called ? "  The callback shouldn't have been called."
                           : "  The callback should have been called.");
      piglit_report_result(PIGLIT_FAIL);
   }

   if ((skip_setup || debug_enable) && !callback_enable) {
      /* the log must contain the error */
      if (!fetch_one_log_message()) {
         puts("  The log shouldn't be empty.");
         piglit_report_result(PIGLIT_FAIL);
      }
   } else {
      /* the log must be empty */
      if (fetch_one_log_message()) {
         puts("  The log should be empty.");
         piglit_report_result(PIGLIT_FAIL);
      }
   }

   if (!piglit_check_gl_error(GL_NO_ERROR))
      piglit_report_result(PIGLIT_FAIL);
}

void piglit_init(int argc, char **argv)
{
   piglit_automatic = GL_TRUE;
   piglit_require_extension("GL_ARB_debug_output");

   glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB);

   if (!piglit_check_gl_error(GL_NO_ERROR))
      piglit_report_result(PIGLIT_FAIL);

   test_api_error(SKIP_SETUP);
   test_api_error(CALLBACK_ENABLE);
   test_api_error(DEBUG_ENABLE);
   test_api_error(DEBUG_ENABLE | CALLBACK_ENABLE);
   test_api_error(0);

   piglit_report_result(PIGLIT_PASS);
}
