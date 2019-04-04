/*
 * Copyright 2019 Mathias Fröhlich <Mathias.Froehlich@web.de>.
 *
 * Copyright 2018 Collabora, Ltd.
 *
 * Based on ext_mesa_platform_surfaceless.c which has
 * Copyright 2016 Google
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

/*
 * Test that enumerates all egl devices and tries to create an offscreen buffer
 * for each device, does a bunch of gl calls on it and tears down the objects
 * on the device again.
 */

#include <stdlib.h>

#include "piglit-util.h"
#include "piglit-util-egl.h"
#include "piglit-util-gl.h"


#define WIDTH 16


/* Extension function pointers.
 *
 * Use prefix 'pegl' (piglit egl) instead of 'egl' to avoid collisions with
 * prototypes in eglext.h.
 */

const char *(*peglQueryDeviceStringEXT)(EGLDeviceEXT device, EGLint name);
EGLBoolean (*peglQueryDevicesEXT)(EGLint max_devices, EGLDeviceEXT *devices,
	    EGLint *num_devices);
EGLDisplay (*peglGetPlatformDisplayEXT)(EGLenum platform, void *native_display,
	    const EGLint *attrib_list);

static void
init_egl_extension_funcs(void)
{
	peglQueryDeviceStringEXT = (void *)eglGetProcAddress("eglQueryDeviceStringEXT");
	peglQueryDevicesEXT = (void *)eglGetProcAddress("eglQueryDevicesEXT");
	peglGetPlatformDisplayEXT = (void *)eglGetProcAddress("eglGetPlatformDisplayEXT");
}


static enum piglit_result
commands()
{
	const GLfloat blue[3] = { 0, 0, 1 };
	const GLfloat red[3] = { 1, 0, 0 };

	printf("GL Vendor: %s\n", glGetString(GL_VENDOR));
	printf("GL Renderer: %s\n", glGetString(GL_RENDERER));
	printf("GL Version: %s\n", glGetString(GL_VERSION));

	glClearColor(blue[0], blue[1], blue[2], 1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (!piglit_probe_pixel_rgb(WIDTH/2, WIDTH/2, blue)) {
		printf("Pixel probe failed\n");
		return PIGLIT_FAIL;
	}

	glColor3fv(red);
	piglit_draw_rect(-1, -1, 2, 2);

	if (!piglit_probe_pixel_rgb(WIDTH/2, WIDTH/2, red)) {
		printf("Pixel probe failed\n");
		return PIGLIT_FAIL;
	}

	if (!piglit_check_gl_error(0)) {
		printf("Got OpenGL errors\n");
		return PIGLIT_FAIL;
	}

	return PIGLIT_PASS;
}


static enum piglit_result
commands_with_fbo()
{
	enum piglit_result result;
	GLuint fb, cb, db;
	GLenum status;

	if (!piglit_is_extension_supported("GL_ARB_framebuffer_object")) {
		printf("No GL_ARB_framebuffer_object available\n");
		return PIGLIT_SKIP;
	}

	glGenRenderbuffers(1, &cb);
	glBindRenderbuffer(GL_RENDERBUFFER, cb);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA, WIDTH, WIDTH);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	glGenRenderbuffers(1, &db);
	glBindRenderbuffer(GL_RENDERBUFFER, db);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, WIDTH, WIDTH);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	glGenFramebuffers(1, &fb);
	glBindFramebuffer(GL_FRAMEBUFFER, fb);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, cb);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, db);

	status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE) {
		printf("FBO incomplete status 0x%X\n", status);
		return PIGLIT_FAIL;
	}

	glViewport(0, 0, WIDTH, WIDTH);

	result = commands();

	glDeleteFramebuffers(1, &fb);
	glDeleteRenderbuffers(1, &cb);
	glDeleteRenderbuffers(1, &db);

	return result;
}


static enum piglit_result
pbuffer_test(EGLDisplay dpy)
{
	/* We are after desktop OpenGL - like requested in the config */
	if (!eglBindAPI(EGL_OPENGL_API)) {
		printf("Call to eglBindAPI() fails.\n");
		return PIGLIT_FAIL;
	}

	/* Select an appropriate configuration */
	const EGLint configAttribs[] = {
		EGL_SURFACE_TYPE, EGL_PBUFFER_BIT,
		EGL_BLUE_SIZE, 8,
		EGL_GREEN_SIZE, 8,
		EGL_RED_SIZE, 8,
		EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
		EGL_NONE
	};
	EGLint numConfigs;
	EGLConfig config;
	if (!eglChooseConfig(dpy, configAttribs, &config, 1, &numConfigs)) {
		printf("Call to eglChooseConfig() fails.\n");
		return PIGLIT_FAIL;
	}
	if (numConfigs <= 0) {
		printf("Call to eglChooseConfig() gave zero configs.\n");
		return PIGLIT_SKIP;
	}

	const EGLint pbufferAttribs[] = {
		EGL_WIDTH, WIDTH,
		EGL_HEIGHT, WIDTH,
		EGL_NONE
	};
	EGLSurface surf = eglCreatePbufferSurface(dpy, config, pbufferAttribs);
	if (surf == EGL_NO_SURFACE) {
		printf("Call to eglCreatePbufferSurface() fails.\n");
		return PIGLIT_FAIL;
	}

	/* Create a context */
	EGLContext ctx = eglCreateContext(dpy, config, EGL_NO_CONTEXT, NULL);
	if (EGL_NO_CONTEXT == ctx) {
		printf("Call to eglCreateContext() fails.\n");
		return PIGLIT_FAIL;
	}

	/* Make the context current */
	if (!eglMakeCurrent(dpy, surf, surf, ctx)) {
		printf("Call to eglMakeCurrent() fails.\n");
		return PIGLIT_FAIL;
	}

	piglit_dispatch_default_init(PIGLIT_DISPATCH_GL);

	/* Do the actual test */
	if (PIGLIT_FAIL == commands())
		return PIGLIT_FAIL;

	if (!eglMakeCurrent(dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT)) {
		printf("Call to eglMakeCurrent() fails.\n");
		return PIGLIT_FAIL;
	}

	if (!eglDestroySurface(dpy, surf)) {
		printf("Call to eglDestroySurface() fails.\n");
		return PIGLIT_FAIL;
	}

	return PIGLIT_PASS;
}


static enum piglit_result
surfaceless_test(EGLDisplay dpy)
{
	/* We are after desktop OpenGL - like requested in the config */
	if (!eglBindAPI(EGL_OPENGL_API)) {
		printf("Call to eglBindAPI() fails.\n");
		return PIGLIT_FAIL;
	}

	if (!piglit_is_egl_extension_supported(dpy, "EGL_KHR_surfaceless_context")) {
		printf("No EGL_KHR_surfaceless_context available\n");
		return PIGLIT_SKIP;
	}

	/* Select an appropriate configuration */
	const EGLint configAttribs[] = {
		EGL_SURFACE_TYPE, 0,
		EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
		EGL_NONE
	};
	EGLint numConfigs;
	EGLConfig config;
	if (!eglChooseConfig(dpy, configAttribs, &config, 1, &numConfigs)) {
		printf("Call to eglChooseConfig() fails.\n");
		return PIGLIT_FAIL;
	}
	if (numConfigs <= 0) {
		printf("Call to eglChooseConfig() gave zero configs.\n");
		return PIGLIT_SKIP;
	}

	/* Create a context */
	EGLContext ctx = eglCreateContext(dpy, config, EGL_NO_CONTEXT, NULL);
	if (EGL_NO_CONTEXT == ctx) {
		printf("Call to eglCreateContext() fails.\n");
		return PIGLIT_FAIL;
	}

	/* Make the context current */
	if (!eglMakeCurrent(dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, ctx)) {
		printf("Call to eglMakeCurrent() fails.\n");
		return PIGLIT_FAIL;
	}

	piglit_dispatch_default_init(PIGLIT_DISPATCH_GL);

	/* Do the actual test */
	if (PIGLIT_FAIL == commands_with_fbo())
		return PIGLIT_FAIL;

	if (!eglMakeCurrent(dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT)) {
		printf("Call to eglMakeCurrent() fails.\n");
		return PIGLIT_FAIL;
	}

	return PIGLIT_PASS;
}


static enum piglit_result
configless_test(EGLDisplay dpy)
{
	/* We are after desktop OpenGL - like requested in the config */
	if (!eglBindAPI(EGL_OPENGL_API)) {
		printf("Call to eglBindAPI() fails.\n");
		return PIGLIT_FAIL;
	}

	if (!piglit_is_egl_extension_supported(dpy, "EGL_KHR_no_config_context")) {
		printf("No EGL_KHR_no_config_context available\n");
		return PIGLIT_SKIP;
	}
	if (!piglit_is_egl_extension_supported(dpy, "EGL_KHR_surfaceless_context")) {
		printf("No EGL_KHR_surfaceless_context available\n");
		return PIGLIT_SKIP;
	}

	/* Create a context */
	EGLContext ctx = eglCreateContext(dpy, EGL_NO_CONFIG_KHR, EGL_NO_CONTEXT, NULL);
	if (EGL_NO_CONTEXT == ctx) {
		printf("Call to eglCreateContext() fails.\n");
		return PIGLIT_FAIL;
	}

	/* Make the context current */
	if (!eglMakeCurrent(dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, ctx)) {
		printf("Call to eglMakeCurrent() fails.\n");
		return PIGLIT_FAIL;
	}

	piglit_dispatch_default_init(PIGLIT_DISPATCH_GL);

	/* Do the actual test */
	if (PIGLIT_FAIL == commands_with_fbo())
		return PIGLIT_FAIL;

	if (!eglMakeCurrent(dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT)) {
		printf("Call to eglMakeCurrent() fails.\n");
		return PIGLIT_FAIL;
	}

	return PIGLIT_PASS;
}


static enum piglit_result
for_all_device_displays(enum piglit_result (*call)(EGLDisplay))
{
#define NDEVS 1024
	EGLDeviceEXT devices[NDEVS];
	EGLint i, num_devices;
	enum piglit_result result = PIGLIT_PASS;

	if (!peglQueryDevicesEXT(NDEVS, devices, &num_devices)) {
		printf("Failed to get egl device\n");
		result = PIGLIT_FAIL;
	}
	if (num_devices <= 0) {
		printf("Failed to get at least one egl device\n");
		result = PIGLIT_FAIL;
	}

	/* For all available devices */
	for (i = 0; i < num_devices; i++) {
		EGLDisplay dpy;

		printf("------------------------------------------------\n");
		printf("Device #%d\n", i);
		printf("------------------------------------------------\n");

		/* Get a display from the device */
		dpy = peglGetPlatformDisplayEXT(EGL_PLATFORM_DEVICE_EXT,
						devices[i], NULL);
		if (dpy == EGL_NO_DISPLAY) {
			printf("Platform display shall not be EGL_NO_DISPLAY\n");
			result = PIGLIT_FAIL;
			continue;
		}

		/* Initialize the display */
		EGLint major, minor;
		if (!eglInitialize(dpy, &major, &minor)) {
			printf("Call to eglInitialize() failed\n");
			result = PIGLIT_FAIL;
			continue;
		}

		/* Check the required EGL version */
		if (major < 1 || (major == 1 && minor < 4)) {
			printf("Call to eglInitialize() returned too low version\n");
			result = PIGLIT_FAIL;
		} else {
			/* Do the actual test */
			if (PIGLIT_FAIL == call(dpy))
				result = PIGLIT_FAIL;
		}

		/* Tear down the display */
		eglTerminate(dpy);
	}

	return result;
}


static enum piglit_result
pbuffer_tests(void *test_data)
{
	printf("================================================\n");
	printf("=============== PBUFFER ========================\n");
	printf("================================================\n");

	return for_all_device_displays(pbuffer_test);
}


static enum piglit_result
surfaceless_tests(void *test_data)
{
	printf("================================================\n");
	printf("=============== SURFACELESS ====================\n");
	printf("================================================\n");

	return for_all_device_displays(surfaceless_test);
}


static enum piglit_result
configless_tests(void *test_data)
{
	printf("================================================\n");
	printf("=============== CONFIGLESS =====================\n");
	printf("================================================\n");

	return for_all_device_displays(configless_test);
}


static const struct piglit_subtest subtests[] = {
	{ "pbuffer_tests", "pbuffer_tests", pbuffer_tests },
	{ "surfaceless_tests", "surfaceless_tests", surfaceless_tests },
	{ "configless_tests", "configless_tests", configless_tests },
	{ 0 },
};


int
main(int argc, char **argv)
{
	enum piglit_result result = PIGLIT_SKIP;
	const char **selected_names = NULL;
	size_t num_selected = 0;

#if _POSIX_C_SOURCE >= 200112L
	unsetenv("DISPLAY");
#endif

	piglit_require_egl_extension(EGL_NO_DISPLAY, "EGL_EXT_client_extensions");
	piglit_require_egl_extension(EGL_NO_DISPLAY, "EGL_EXT_device_base");
	piglit_require_egl_extension(EGL_NO_DISPLAY, "EGL_EXT_device_enumeration");
	piglit_require_egl_extension(EGL_NO_DISPLAY, "EGL_EXT_device_query");
	piglit_require_egl_extension(EGL_NO_DISPLAY, "EGL_EXT_platform_base");
	piglit_require_egl_extension(EGL_NO_DISPLAY, "EGL_EXT_platform_device");

	/* Strip common piglit args. */
	piglit_strip_arg(&argc, argv, "-fbo");
	piglit_strip_arg(&argc, argv, "-auto");

	piglit_parse_subtest_args(&argc, argv, subtests, &selected_names,
				  &num_selected);

	if (argc > 1) {
		fprintf(stderr, "usage error\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	init_egl_extension_funcs();
	result = piglit_run_selected_subtests(subtests, selected_names,
					      num_selected, result);

	piglit_report_result(result);
}
