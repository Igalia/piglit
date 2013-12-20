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
#ifndef PIGLIT_FRAMEWORK_H
#define PIGLIT_FRAMEWORK_H

#include <assert.h>
#include <stdbool.h>

/**
 * A bitmask of these enums specifies visual attributes for the test's window.
 *
 * \see piglit_gl_test_config::window_visual
 */
enum piglit_gl_visual {
	PIGLIT_GL_VISUAL_RGB 		= 1 << 0,
	PIGLIT_GL_VISUAL_RGBA 		= 1 << 1,
	PIGLIT_GL_VISUAL_DOUBLE 	= 1 << 2,
	PIGLIT_GL_VISUAL_ACCUM 		= 1 << 3,
	PIGLIT_GL_VISUAL_DEPTH 		= 1 << 4,
	PIGLIT_GL_VISUAL_STENCIL 	= 1 << 5
};

/**
 * An idividual subtest that makes up part of a test group.
 */
struct piglit_gl_subtest {
	/** Name of the subtest as it will appear in the log. */
	const char *name;

	/** Command line name used to select this test. */
	const char *option;

	/** Function that implements the test. */
	enum piglit_result (*subtest_func)(void *data);

	/** Passed as the data parameter to subtest_func.*/
	void *data;
};

/**
 * Detect the end of an array of piglit_gl_subtest structures
 *
 * The array of subtests is terminated by structure with a \c NULL \c
 * name pointer.
 */
#define PIGLIT_GL_SUBTEST_END(s) ((s)->name == NULL)

/**
 * @brief Configuration for running an OpenGL test.
 *
 * To run a test, pass this to piglit_gl_test_run().
 *
 * This is named piglit_gl_test_config, rather than piglit_test_config, in
 * order to distinguish it from other test types, such as EGL and GLX tests.
 *
 * At least one of the `supports` fields must be set.
 *
 * If `supports_gl_core_version` and `supports_gl_compat_version` are both
 * set, then Piglit will first attempt to run the test under a GL core context
 * of the requested version. If context creation fails, then Piglit will then
 * attempt to run the test under a GL compatibility context of the requested
 * version.
 */
struct piglit_gl_test_config {
	/**
	 * If this field is non-zero, then the test is able to run under any
	 * OpenGL ES context whose version is backwards-compatible with the
	 * given version.
	 *
	 * For example, if this field's value is '10', then Piglit will
	 * attempt to run the test under an OpenGL ES 1.0 context. Likewise
	 * for '20' and OpenGL ES 2.0.
	 *
	 * If Piglit fails to acquire the waffle_config or to create the
	 * waffle_context, then it skips its attempt to run the test under
	 * an OpenGL ES context.
	 *
	 * If this field is 0, then the test is not able to run under an
	 * OpenGL ES context of any version.
	 */
	int supports_gl_es_version;

	/**
	 * If this field is non-zero, then the test is able to run under a GL
	 * core context having at least the given version.
	 *
	 * When attempting run a test under a GL core context, Piglit chooses
	 * a waffle_config with the following attributes set.  (Note that
	 * Waffle ignores the profile attribute for versions less than 3.2).
	 *     - WAFFLE_CONTEXT_PROFILE       = WAFFLE_CONTEXT_CORE_PROFILE
	 *     - WAFFLE_CONTEXT_MAJOR_VERSION = supports_gl_core_version / 10
	 *     - WAFFLE_CONTEXT_MINOR_VERSION = supports_gl_core_version % 10
	 * If Piglit fails to acquire the waffle_config or to create the
	 * waffle_context, then it skips its attempt to run the test under
	 * a GL core context.
	 *
	 * It is an error if this field is less than 3.1 because the concept
	 * of "core context" does not apply before GL 3.1.
	 *
	 * Piglit handles a request for a GL 3.1 core context as a special
	 * case.  As noted above, Waffle ignores the profile attribute when
	 * choosing a 3.1 config. However, the concept of "core profile" is
	 * still applicable to 3.1 contexts and is indicated by the context's
	 * lack of support for the GL_ARB_compatibility extension. Therefore,
	 * Piglit attempts to run the test under a GL 3.1 core context by
	 * first creating the context and then skipping the attempt if the
	 * context supports the GL_ARB_compatibility extension.
	 *
	 * If this field is 0, then the test is not able to run under a GL
	 * core context of any version.
	 */
	int supports_gl_core_version;

	/**
	 * If this field is non-zero, then the test is able to run under a GL
	 * compatibility context having at least the given version.
	 *
	 * When attempting run a test under a GL compatibility context, Piglit
	 * chooses a waffle_config with the following attribute set.
	 *
	 *     WAFFLE_CONTEXT_PROFILE = WAFFLE_CONTEXT_COMPATIBILITY_PROFILE
	 *
	 * If context creation succeeds, then Piglit verifies with
	 * glGetString() that the context's actual version is no less than the
	 * requested version. Otherwise, If the version verification fails,
	 * then Piglit skips its attempt to run the test under a GL
	 * compatibility context.
	 *
	 * Piglit handles a request for a GL 3.1 compatibility context as
	 * a special case.  As noted above, Waffle ignores the profile
	 * attribute when choosing a 3.1 config. However, the concept of
	 * "compatibility profile" is still applicable to 3.1 contexts and is
	 * indicated by the context's support for the GL_ARB_compatibility
	 * extension. Therefore, Piglit attempts to run under a GL 3.1
	 * compatibility context by first creating the context and then
	 * skipping the attempt if the context lacks the GL_ARB_compatibility
	 * extension.
	 *
	 * If this field is 0, then the test is not able to run under a GL
	 * compatibility context of any version.
	 */
	int supports_gl_compat_version;

	/**
	 * If true, then this test requires a forward-compatible context.
	 *
	 * Piglit will choose a waffle_config with
	 * WAFFLE_CONTEXT_FORWARD_COMPATIBLE set to true. If context creation
	 * fails, then the test skips.
	 */
	bool require_forward_compatible_context;

	/**
	 * If true, then this test requires a debug context.
	 *
	 * Piglit will choose a waffle_config with WAFFLE_CONTEXT_DEBUG set to
	 * true. If context creation fails, then the test skips.
	 */
	bool require_debug_context;

	int window_width;
	int window_height;
	int window_samples;

	/**
	 * A bitmask of `enum piglit_gl_visual`.
	 */
	int window_visual;

	/**
	 * The test requires the window to be displayed in order to run
	 * correctly. Tests that read from the front buffer must enable
	 * this.
	 */
	bool requires_displayed_window;

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

	/**
	 * List of subtests supported by this test case
	 *
	 * This is only used during command line argument parsing to implement
	 * the -list-subtests option.
	 */
	const struct piglit_gl_subtest *subtests;

	/**
	 * Names of subtests supplied on the command line.
	 *
	 * The paramaters passed to each -subtest command line option is
	 * stored here in the order of appearance on the command line.
	 */
	const char **selected_subtests;
	size_t num_selected_subtests;
};

/**
 * Initialize @a config with default values, some of which may come
 * from command line arguments.
 */
void
piglit_gl_test_config_init(struct piglit_gl_test_config *config);

void
piglit_gl_process_args(int *argc, char *argv[],
		       struct piglit_gl_test_config *config);

/**
 * Get the list of command-line selected tests from the piglit_gl_test_config
 *
 * If the config structure does not contain a list of subtests or if no tests
 * were selected on the command line, this function will set \c
 * *selected_subtests to \c NULL and will return zero.
 *
 * \returns
 * The number of tests selected on the command line.
 */
size_t
piglit_get_selected_tests(const char ***selected_subtests);

/**
 * Run the OpenGL test described by @a config. Does not return.
 */
void
piglit_gl_test_run(int argc, char *argv[],
		   const struct piglit_gl_test_config *config);

#ifdef __cplusplus
#  define PIGLIT_EXTERN_C_BEGIN extern "C" {
#  define PIGLIT_EXTERN_C_END   }
#else
#  define PIGLIT_EXTERN_C_BEGIN
#  define PIGLIT_EXTERN_C_END
#endif

#define PIGLIT_GL_TEST_CONFIG_BEGIN                                          \
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
                struct piglit_gl_test_config config;                         \
                                                                             \
                piglit_gl_test_config_init(&config);                         \
                                                                             \
                config.init = piglit_init;                                   \
                config.display = piglit_display;                             \
                                                                             \
                /* Default window size.  Note: Win7's min window width */    \
                /* seems to be 116 pixels.  When the window size is */       \
                /* unexpectedly resized, tests are marked as "WARN". */      \
                /* Let's use a larger default to avoid that. */              \
                config.window_width = 150;                                   \
                config.window_height = 150;                                  \
                                                                             \
                /* Open a new scope so that tests can declare locals */      \
                /* between here and PIGLIT_GL_TEST_CONFIG_END. */            \
                {


#define PIGLIT_GL_TEST_CONFIG_END                                            \
                }                                                            \
                                                                             \
                piglit_gl_process_args(&argc, argv, &config);                \
                piglit_gl_test_run(argc, argv, &config);                     \
                                                                             \
                assert(false);                                               \
                return 0;                                                    \
        }

extern int piglit_automatic;

extern int piglit_width;
extern int piglit_height;
extern bool piglit_use_fbo;
extern unsigned int piglit_winsys_fbo;
extern struct piglit_gl_framework *gl_fw;

void piglit_swap_buffers(void);
void piglit_present_results();
void piglit_post_redisplay(void);
void piglit_set_keyboard_func(void (*func)(unsigned char key, int x, int y));
void piglit_set_reshape_func(void (*func)(int w, int h));

/**
 * Convenience macro for invoking piglit_strip_arg() from within
 * piglit_init() or between PIGLIT_GL_TEST_CONFIG_BEGIN and
 * PIGLIT_GL_TEST_CONFIG_END.
 */
#define PIGLIT_STRIP_ARG(arg) piglit_strip_arg(&argc, argv, arg)

struct piglit_dma_buf;

/**
 * Create buffer suitable for dma_buf importing and set its contents to the
 * given data (src_data). Different hardware may have different alignment
 * constraints and hence one can specify one stride for the source and get
 * another for the final buffer to be given further to EGL.
 * An opaque handle, file descriptor, stride and offset for the buffer are only
 * returned upon success indicated by the return value PIGLIT_PASS, otherwise
 * no buffer is created. In case the framework simply does not support dma
 * buffers, the return value is PIGLIT_SKIP instead of PIGLIT_FAIL.
 */
enum piglit_result
piglit_create_dma_buf(unsigned w, unsigned h, unsigned cpp,
		      const void *src_data, unsigned src_stride,
		      struct piglit_dma_buf **buf, int *fd,
		      unsigned *stride, unsigned *offset);

/**
 * Release all the resources allocated for the designated buffer. If the given
 * pointer (buf) is NULL no action is taken.
 */
void
piglit_destroy_dma_buf(struct piglit_dma_buf *buf);

const struct piglit_gl_subtest *
piglit_find_subtest(const struct piglit_gl_subtest *subtests, const char *name);

enum piglit_result
piglit_run_selected_subtests(const struct piglit_gl_subtest *all_subtests,
			     const char **selected_subtests,
			     size_t num_selected_subtests,
			     enum piglit_result previous_result);

#endif /* PIGLIT_FRAMEWORK_H */
