/*
 * Copyright © 2009 Intel Corporation
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

#pragma once

#include <assert.h>
#include <stdbool.h>

/**
 * @brief Info needed to run an OpenGL test.
 *
 * To run a test, pass this to piglit_gl_test_run().
 *
 * This is named piglit_gl_test_info, rather than piglit_test_info, in
 * order to distinguish it from other test types, such as EGL and GLX tests.
 *
 * TODO: Add fields here that declare test requirements on GL context
 * TODO: flavor, extensions, and window system.
 */
struct piglit_gl_test_info {
	int window_width;
	int window_height;

	/** A bitmask such as `GLUT_RGBA | GLUT_DOUBLE`. */
	int window_visual;

	/**
	 * This is called once per test, after the GL context has been created
	 * and made current but before display() is called.
	 */
	void
	(*init)(int argc, char *argv[]);

	/**
	 * If the test is run in auto mode, then this is called once after
	 * init(). Otherwise, it is called repeatedly from some ill-defined
	 * event loop.
	 */
	enum piglit_result
	(*display)(void);
};

/**
 * Initialize @a info with default values.
 */
void
piglit_gl_test_info_init(struct piglit_gl_test_info *info);

/**
 * Run the OpenGL test described by @a info. Does not return.
 */
void
piglit_gl_test_run(int argc, char *argv[],
		   const struct piglit_gl_test_info *info);

#ifdef __cplusplus
#  define PIGLIT_EXTERN_C_BEGIN extern "C" {
#  define PIGLIT_EXTERN_C_END   }
#else
#  define PIGLIT_EXTERN_C_BEGIN
#  define PIGLIT_EXTERN_C_END
#endif

/**
 * Define a boilerplate main() that should be suitable for most OpenGL test
 * executables.
 */
#define PIGLIT_GL_TEST_MAIN(_window_width,                                   \
                            _window_height,                                  \
                            _window_visual)                                  \
                                                                             \
        PIGLIT_EXTERN_C_BEGIN                                                \
                                                                             \
        void                                                                 \
        piglit_init(int argc, char *argv[]);                                 \
                                                                             \
        enum piglit_result                                                   \
        piglit_display(void);                                                \
                                                                             \
        PIGLIT_EXTERN_C_END                                                  \
                                                                             \
        int                                                                  \
        main(int argc, char *argv[])                                         \
        {                                                                    \
                struct piglit_gl_test_info info;                             \
                                                                             \
                piglit_gl_test_info_init(&info);                             \
                                                                             \
                info.window_width = _window_width;                           \
                info.window_height = _window_height;                         \
                info.window_visual = _window_visual;                         \
                                                                             \
                info.init = piglit_init;                                     \
                info.display = piglit_display;                               \
                                                                             \
                piglit_gl_test_run(argc, argv, &info);                       \
                                                                             \
                assert(false);                                               \
                return 0;                                                    \
        }

extern int piglit_automatic;

extern int piglit_window_mode;
extern int piglit_width;
extern int piglit_height;
extern bool piglit_use_fbo;
extern unsigned int piglit_winsys_fbo;

extern enum piglit_result piglit_display(void);
extern void piglit_init(int argc, char **argv);
extern void piglit_present_results();
