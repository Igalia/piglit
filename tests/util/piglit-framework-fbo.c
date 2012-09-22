/*
 * Copyright © 2009-2012 Intel Corporation
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

#if defined(PIGLIT_USE_OPENGL_ES1)
#	define PIGLIT_FRAMEWORK_FBO_DISABLED
#elif defined(PIGLIT_USE_WAFFLE)
#	define PIGLIT_FRAMEWORK_FBO_USE_WAFFLE
#elif defined(PIGLIT_USE_GLX)
#	define PIGLIT_FRAMEWORK_FBO_USE_GLX
#else
#	define PIGLIT_FRAMEWORK_FBO_DISABLED
#endif

#ifdef PIGLIT_USE_OPENGL_ES2
#	define GL_DEPTH_STENCIL GL_DEPTH_STENCIL_OES
#	define GL_UNSIGNED_INT_24_8 GL_UNSIGNED_INT_24_8_OES
#endif

#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "piglit-util-gl-common.h"
#include "piglit-framework-gl.h"
#include "piglit-framework-fbo.h"

#ifdef PIGLIT_FRAMEWORK_FBO_USE_GLX
#include "piglit-glx-util.h"
#endif

#ifdef PIGLIT_FRAMEWORK_FBO_USE_WAFFLE
#include <waffle.h>
#endif

#ifdef PIGLIT_FRAMEWORK_FBO_USE_GLX
Display *piglit_glx_dpy;
Window piglit_glx_window;
XVisualInfo *piglit_glx_visinfo;
GLXContext piglit_glx_context;
#endif

#ifdef PIGLIT_FRAMEWORK_FBO_USE_WAFFLE
static struct waffle_display *piglit_waffle_display;
static struct waffle_window *piglit_waffle_window;
static struct waffle_context *piglit_waffle_context;
#endif

static void
piglit_framework_fbo_destroy(void);

#ifdef PIGLIT_FRAMEWORK_FBO_USE_GLX
static void
piglit_framework_fbo_glx_init()
{
	piglit_glx_dpy = piglit_get_glx_display();

	/* Unfortunately in GLX we need a drawable to bind our context
	 * to.  Make an unmapped window.
	 */
	piglit_glx_visinfo = piglit_get_glx_visual(piglit_glx_dpy);

	piglit_glx_context = piglit_get_glx_context(piglit_glx_dpy,
						    piglit_glx_visinfo);

	piglit_glx_window = piglit_get_glx_window_unmapped(piglit_glx_dpy,
							   piglit_glx_visinfo);

	glXMakeCurrent(piglit_glx_dpy, piglit_glx_window, piglit_glx_context);
}

static void
piglit_framework_fbo_glx_destroy()
{
	glXMakeCurrent(piglit_glx_dpy, None, None);
	glXDestroyContext(piglit_glx_dpy, piglit_glx_context);
	XFree(piglit_glx_visinfo);
	XCloseDisplay(piglit_glx_dpy);
}
#endif

#ifdef PIGLIT_FRAMEWORK_FBO_USE_WAFFLE
/**
 * \brief Print a description of the Waffle error and report test failure.
 *
 * The \a func_name is the name of the Waffle function that failed.
 */
static void
fatal_waffle_error(const char *func_name)
{
	const struct waffle_error_info *info = waffle_error_get_info();
	const char *error_name = waffle_error_to_string(info->code);

	fflush(stdout);
	fprintf(stderr, "%s failed with error: %s", func_name, error_name);
	if (info->message_length > 0)
		fprintf(stderr, ": %s", info->message);
	fprintf(stderr, "\n");

	piglit_report_result(PIGLIT_FAIL);
}

static void
piglit_framework_fbo_waffle_init(void)
{
	int i;
	bool ok = true;
	const char *env_platform;
	int32_t waffle_platform;
	int32_t waffle_context_api;
	int32_t init_attrib_list[64];
	int32_t config_attrib_list[64];
	struct waffle_config *config;

	env_platform = getenv("WAFFLE_PLATFORM");

	if (env_platform == NULL) {
		waffle_platform = WAFFLE_PLATFORM_GLX;
	} else if (!strcmp(env_platform, "glx")) {
		waffle_platform = WAFFLE_PLATFORM_GLX;
	} else if (!strcmp(env_platform, "x11_egl")) {
		waffle_platform = WAFFLE_PLATFORM_X11_EGL;
	} else if (!strcmp(env_platform, "wayland")) {
		waffle_platform = WAFFLE_PLATFORM_WAYLAND;
	} else {
		fprintf(stderr, "environment var WAFFLE_PLATFORM has bad "
			"value \"%s\"", env_platform);
	}

#if defined(PIGLIT_USE_OPENGL)
	waffle_context_api = WAFFLE_CONTEXT_OPENGL;
#elif defined(PIGLIT_USE_OPENGL_ES1)
	waffle_context_api = WAFFLE_CONTEXT_OPENGL_ES1;
#elif defined(PIGLIT_USE_OPENGL_ES2)
	waffle_context_api = WAFFLE_CONTEXT_OPENGL_ES2;
#else
#	error
#endif
	i = 0;
	init_attrib_list[i++] = WAFFLE_PLATFORM;
	init_attrib_list[i++] = waffle_platform;
	init_attrib_list[i++] = WAFFLE_NONE;

	i = 0;
	config_attrib_list[i++] = WAFFLE_CONTEXT_API;
	config_attrib_list[i++] = waffle_context_api;
	config_attrib_list[i++] = WAFFLE_RED_SIZE;
	config_attrib_list[i++] = 1;
	config_attrib_list[i++] = WAFFLE_GREEN_SIZE;
	config_attrib_list[i++] = 1;
	config_attrib_list[i++] = WAFFLE_BLUE_SIZE;
	config_attrib_list[i++] = 1;
	config_attrib_list[i++] = WAFFLE_DOUBLE_BUFFERED;
	config_attrib_list[i++] = 1;
	config_attrib_list[i++] = WAFFLE_NONE;

	ok = waffle_init(init_attrib_list);
	if (!ok)
		fatal_waffle_error("waffle_init");

	piglit_waffle_display = waffle_display_connect(NULL);
	if (!piglit_waffle_display)
		fatal_waffle_error("waffle_display_connect");

	config = waffle_config_choose(piglit_waffle_display,
	                              config_attrib_list);
	if (!config)
		fatal_waffle_error("waffle_config_choose");

	piglit_waffle_context = waffle_context_create(config, NULL);
	if (!piglit_waffle_context)
		fatal_waffle_error("waffle_context_create");

	piglit_waffle_window = waffle_window_create(config,
	                                            piglit_width,
	                                            piglit_height);
	if (!piglit_waffle_window)
		fatal_waffle_error("waffle_window_create");

	ok = waffle_make_current(piglit_waffle_display,
			         piglit_waffle_window,
			         piglit_waffle_context);
	if (!ok)
		fatal_waffle_error("waffle_make_current");

	// Cleanup.
	ok = waffle_config_destroy(config);
	if (!ok)
		fatal_waffle_error("waffle_config_destroy");
}

static void
piglit_framework_fbo_waffle_destroy(void)
{
	bool ok = true;

	ok = waffle_make_current(piglit_waffle_display, NULL, NULL);
	if (!ok)
		fatal_waffle_error("waffle_make_current");

	ok = waffle_context_destroy(piglit_waffle_context);
	if (!ok)
		fatal_waffle_error("waffle_context_destroy");

	ok = waffle_display_disconnect(piglit_waffle_display);
	if (!ok)
		fatal_waffle_error("waffle_display_connect");

	piglit_waffle_display = NULL;
	piglit_waffle_context = NULL;
}
#endif

static bool
piglit_framework_fbo_gl_init(const struct piglit_gl_test_info *info)
{
#ifdef PIGLIT_FRAMEWORK_FBO_DISABLED
	return false;
#else
	GLuint tex, depth = 0;
	GLenum status;

#ifdef PIGLIT_USE_OPENGL
	glewInit();

	if (piglit_get_gl_version() < 20)
		return false;
#endif

	glGenFramebuffers(1, &piglit_winsys_fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, piglit_winsys_fbo);

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
		     piglit_width, piglit_height, 0,
		     GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glFramebufferTexture2D(GL_FRAMEBUFFER,
			       GL_COLOR_ATTACHMENT0,
			       GL_TEXTURE_2D,
			       tex,
			       0);

	if (info->window_visual & (PIGLIT_GL_VISUAL_DEPTH | PIGLIT_GL_VISUAL_STENCIL)) {
		/* Create a combined depth+stencil texture and attach it
		 * to the depth and stencil attachment points.
		 */
		glGenTextures(1, &depth);
		glBindTexture(GL_TEXTURE_2D, depth);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_STENCIL,
			     piglit_width, piglit_height, 0,
			     GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);
		glFramebufferTexture2D(GL_FRAMEBUFFER,
				       GL_DEPTH_ATTACHMENT,
				       GL_TEXTURE_2D,
				       depth,
				       0);
		glFramebufferTexture2D(GL_FRAMEBUFFER,
				       GL_STENCIL_ATTACHMENT,
				       GL_TEXTURE_2D,
				       depth,
				       0);
	}

	glBindTexture(GL_TEXTURE_2D, 0);

	status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE) {
		fprintf(stderr,
			"-fbo resulted in incomplete FBO, falling back\n");
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		glDeleteTextures(1, &depth);
		glDeleteTextures(1, &tex);

		piglit_framework_fbo_destroy();

		return false;
	}

	return true;
#endif /* PIGLIT_FRAMEWORK_FBO_DISABLED */
}

bool
piglit_framework_fbo_init(const struct piglit_gl_test_info *info)
{
#if defined(PIGLIT_FRAMEWORK_FBO_USE_GLX)
	piglit_framework_fbo_glx_init();
#elif defined(PIGLIT_FRAMEWORK_FBO_USE_WAFFLE)
	piglit_framework_fbo_waffle_init();
#endif

	return piglit_framework_fbo_gl_init(info);
}

static void
piglit_framework_fbo_destroy(void)
{
#ifdef PIGLIT_USE_OPENGL
	glDeleteFramebuffers(1, &piglit_winsys_fbo);
#endif

	piglit_winsys_fbo = 0;

#if defined(PIGLIT_FRAMEWORK_FBO_USE_GLX)
	piglit_framework_fbo_glx_destroy();
#elif defined(PIGLIT_FRAMEWORK_FBO_USE_WAFFLE)
	piglit_framework_fbo_waffle_destroy();
#endif
}

void
piglit_framework_fbo_run(const struct piglit_gl_test_info *info)
{
	enum piglit_result result = info->display();
	piglit_framework_fbo_destroy();
	piglit_report_result(result);
}

void
piglit_framework_fbo_swap_buffers(void)
{
#if defined(PIGLIT_FRAMEWORK_FBO_USE_GLX)
	glXSwapBuffers(piglit_glx_dpy, piglit_glx_window);
#elif defined(PIGLIT_FRAMEWORK_FBO_USE_WAFFLE)
	waffle_window_swap_buffers(piglit_waffle_window);
#endif
}
