/* Copyright 2012 Intel Corporation
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

#include "piglit-util-gl.h"

#if defined(_WIN32)

#elif defined(__APPLE__)

#include <stdlib.h>
#include <string.h>
#include <AvailabilityMacros.h>
#include <dlfcn.h>

#else /* Linux */

#include <dlfcn.h>

#if defined(PIGLIT_HAS_GLX)
#	include "glxew.h"
#elif defined(PIGLIT_HAS_EGL)
#	include <EGL/egl.h>
#endif

#endif

#if defined(PIGLIT_USE_WAFFLE)
#include <waffle.h>
#include "piglit-util-waffle.h"
#include "piglit-framework-gl.h"
#endif

/**
 * Generated code calls this function if the test tries to use a GL
 * function that is not supported on the current implementation.
 *
 * This function terminates the test with a SKIP; this saves the
 * piglit test from the burden of having to pre-check whether the
 * implementation supports the functionality being tested.
 */
static void
default_unsupported(const char *name)
{
	printf("Function \"%s\" not supported on this implementation\n", name);
	piglit_report_result(PIGLIT_SKIP);
}

/**
 * Generated code calls this function if a call to GetProcAddress()
 * returns NULL.
 *
 * We don't expect this to ever happen, since we only call
 * GetProcAddress() for functions that the implementation claims to
 * support.  So if it does happen we terminate the test with a FAIL.
 */
static void
default_get_proc_address_failure(const char *function_name)
{
	printf("GetProcAddress failed for \"%s\"\n", function_name);
	piglit_report_result(PIGLIT_FAIL);
}

#if defined(_WIN32)

/**
 * This function is used to retrieve the address of GL extension
 * functions, and core GL functions for GL versions above 1.3, on
 * Windows.
 */
static piglit_dispatch_function_ptr
get_ext_proc_address(const char *function_name)
{
	return (piglit_dispatch_function_ptr) wglGetProcAddress(function_name);
}

/**
 * This function is used to retrieve the address of core GL functions
 * on windows.
 */
static piglit_dispatch_function_ptr
get_core_proc_address(const char *function_name, int gl_10x_version)
{
	if (gl_10x_version > 11) {
		return get_ext_proc_address(function_name);
	} else {
		piglit_dispatch_function_ptr p;
		/* Try GetProcAddress() first.
		 * If that fails, try wglGetProcAddress().
		 */
		p = (piglit_dispatch_function_ptr)
			GetProcAddress(LoadLibraryA("OPENGL32"), function_name);
		if (!p)
			p = get_ext_proc_address(function_name);
		return p;

	}
}

#elif defined(__APPLE__)

/**
 * This function is used to retrieve the address of all GL functions
 * on Apple.
 */
static piglit_dispatch_function_ptr
get_ext_proc_address(const char *function_name)
{
	static const char *opengl_path =
		"/System/Library/Frameworks/OpenGL.framework/Versions/Current/OpenGL";
	static void *opengl_lib = NULL;

	/* Dynamically load lib */
	if (!opengl_lib)
	{
		opengl_lib = dlopen(opengl_path, RTLD_LAZY);
		if (!opengl_lib)
			return NULL;
	}

	return (piglit_dispatch_function_ptr) dlsym(opengl_lib, function_name);
}

/**
 * This function is used to retrieve the address of core GL functions
 * on Apple.
 */
static piglit_dispatch_function_ptr
get_core_proc_address(const char *function_name, int gl_10x_version)
{
	/* We don't need to worry about the GL version, since on Apple
	 * we retrieve all proc addresses in the same way.
	 */
	(void) gl_10x_version;

	return get_ext_proc_address(function_name);
}

#else /* Linux */

#if defined(PIGLIT_HAS_EGL)
#define GLES1_LIB "libGLESv1_CM.so.1"
#define GLES2_LIB "libGLESv2.so.2"

/** dlopen() return value for libGLESv1_CM.so.1 */
static void *gles1_handle;

/** dlopen() return value for libGLESv2.so.2 */
static void *gles2_handle;

static void *
do_dlsym(void **handle, const char *lib_name, const char *function_name)
{
    void *result;

    if (!*handle)
        *handle = dlopen(lib_name, RTLD_LAZY);

    if (!*handle) {
        fprintf(stderr, "Could not open %s: %s\n", lib_name, dlerror());
        return NULL;
    }

    result = dlsym(*handle, function_name);
    if (!result)
        fprintf(stderr, "%s() not found in %s: %s\n", function_name, lib_name,
                dlerror());

    return result;
}
#endif

/**
 * This function is used to retrieve the address of all GL functions
 * on Linux.
 */
static piglit_dispatch_function_ptr
get_ext_proc_address(const char *function_name)
{
#if defined(PIGLIT_HAS_GLX)
	return glXGetProcAddressARB((const GLubyte *) function_name);
#elif defined(PIGLIT_HAS_EGL)
	return eglGetProcAddress(function_name);
#else
	(void)function_name;
	return (piglit_dispatch_function_ptr)NULL;
#endif
}

/**
 * This function is used to retrieve the address of core GL functions
 * on Linux.
 *
 * eglGetProcAddress supports querying core functions only if EGL >= 1.5 or if
 * EGL_KHR_get_all_proc_addresses or EGL_KHR_client_get_all_proc_addresses is
 * supported. Rather than worry about such details, we consistently use dlysm()
 * to lookup core *OpenGL ES* functions on systems where EGL is available.
 *
 * Lookup for core *OpenGL* functions is more complicated because the EGL 1.4
 * specification, the antiquated OpenGL ABI for Linux [1] from year 2000, and
 * various libGL.so implementations all disagree on the set of symbols that
 * libGL.so should statically expose and which are queryable with
 * eglGetProcAddress.  The EGL 1.4 spec (as explained above) does not require
 * eglGetProcAddress to work for core functions.  The OpenGL ABI spec requires
 * that libGL.so expose *no* symbols statically except those contained in GL
 * 1.2 and select extensions. Actual driver vendors tend to expose most, if
 * not all, symbols statically from libGL.so.
 *
 * Considering how messy this situation is, the best way to query a core OpenGL
 * function on EGL is eglGetProcAddress (or even glXGetProcAddress!). Sometimes
 * Mesa's libGL doesn't statically expose all OpenGL functions supported by the
 * driver, but Mesa's eglGetProcAddress does work for all GL functions, core
 * and extension.  Some other vendors of desktop OpenGL drivers, such as
 * Nvidia, do the same. (By coincidence, Mesa's glXGetProcAddress also returns
 * the same addresses as eglGetProcAddress). We don't need to worry about
 * platforms on which eglGetProcAddress does not work for core functions, such
 * as Mali, because those platforms support only OpenGL ES.
 *
 * [1] https://www.opengl.org/registry/ABI/
 */
static piglit_dispatch_function_ptr
get_core_proc_address(const char *function_name, int gl_10x_version)
{
#if defined(PIGLIT_HAS_EGL)
	switch (gl_10x_version) {
	case 11:
		return do_dlsym(&gles1_handle, GLES1_LIB, function_name);
	case 20:
		return do_dlsym(&gles2_handle, GLES2_LIB, function_name);
	case 10:
	default:
		/* We query the address of core OpenGL functions as if they
		 * were extension functions. Read about the gory details
		 * above. */
		(void) gl_10x_version;
		return get_ext_proc_address(function_name);
	}
#else
	/* We don't need to worry about the GL version, since when using GLX
	 * we retrieve all proc addresses in the same way.
	 */
	(void) gl_10x_version;
	return get_ext_proc_address(function_name);
#endif
}

#endif

#ifdef PIGLIT_USE_WAFFLE
static enum waffle_enum piglit_waffle_dl = WAFFLE_DL_OPENGL;

/**
 * This function is used to retrieve the address of core GL functions
 * via the waffle library.
 */
static piglit_dispatch_function_ptr
get_wfl_core_proc(const char *name, int gl_10x_version)
{
	piglit_dispatch_function_ptr func;

	func = (piglit_dispatch_function_ptr)waffle_dl_sym(piglit_waffle_dl,
							   name);
	if (!func)
		wfl_log_error(__FUNCTION__);

	return func;
}

/**
 * This function is used to retrieve the address of functions not part of the
 * core GL specification via the waffle library.
 */
static piglit_dispatch_function_ptr
get_wfl_ext_proc(const char *name)
{
	piglit_dispatch_function_ptr func;

	func = (piglit_dispatch_function_ptr)waffle_get_proc_address(name);
	if (!func)
		wfl_log_error(__FUNCTION__);

	return func;
}
#endif

/**
 * Initialize the GL dispatch mechanism to a default configuration.
 *
 * Eventually we will want to replace this with code that initializes
 * the GL dispatch mechanism based on run-time parameters (e.g. to
 * select X vs Wayland, or desktop GL vs GLES).
 *
 * This function is safe to call multiple times--it only has an effect
 * on the first call.
 */
void
piglit_dispatch_default_init(piglit_dispatch_api api)
{
	static bool already_initialized = false;

	if (already_initialized)
		return;

#ifdef PIGLIT_USE_WAFFLE
	switch (api) {
	case PIGLIT_DISPATCH_GL:
		piglit_waffle_dl = WAFFLE_DL_OPENGL;
		break;
	case PIGLIT_DISPATCH_ES1:
		piglit_waffle_dl = WAFFLE_DL_OPENGL_ES1;
		break;
	case PIGLIT_DISPATCH_ES2:
		piglit_waffle_dl = WAFFLE_DL_OPENGL_ES2;
		break;
	}

	if (gl_fw) {
		piglit_dispatch_init(api,
				     get_wfl_core_proc,
				     get_wfl_ext_proc,
				     default_unsupported,
				     default_get_proc_address_failure);
	} else
#endif
	{

		piglit_dispatch_init(api,
				     get_core_proc_address,
				     get_ext_proc_address,
				     default_unsupported,
				     default_get_proc_address_failure);
	}

	already_initialized = true;
}
