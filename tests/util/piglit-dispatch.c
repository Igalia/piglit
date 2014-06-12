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

#include "piglit-dispatch.h"
#include "piglit-util-gl-common.h"

#if defined(PIGLIT_USE_WAFFLE)
#include <waffle.h>
#include "piglit-util-waffle.h"
#include "piglit-framework-gl.h"
#endif

/* Global state maintained by the Piglit dispatch mechanism: */

/**
 * Which function to call to get the address of a core function.
 */
static piglit_get_core_proc_address_function_ptr get_core_proc_address = NULL;

/**
 * Which function to call to get the address of a function defined in
 * an extension.
 */
static piglit_get_ext_proc_address_function_ptr get_ext_proc_address = NULL;

/**
 * Which function to call if the test attempts to call a function that
 * is not supported by the implementation.
 */
static piglit_error_function_ptr unsupported = NULL;

/**
 * Which function to call if get_core_proc_address or
 * get_ext_proc_address returns NULL.
 */
static piglit_error_function_ptr get_proc_address_failure = NULL;

/**
 * The GL version extracted from glGetString(GL_VERSION), times 10.
 * For example, if the GL version is 2.1, the value 21 is stored here.
 *
 * We cache this here because calling glGetString is prohibited
 * between glBegin and glEnd, and to avoid the inefficiency of
 * redundant glGetString queries.
 */
static int gl_version = 0;

/**
 * True if piglit_dispatch_init has been called.
 */
static bool is_initialized = false;

static piglit_dispatch_api dispatch_api;

/**
 * Generated code calls this function to verify that the dispatch
 * mechanism has been properly initialized.
 */
static void
check_initialized()
{
	if (is_initialized)
		return;

	printf("piglit_dispatch_init() must be called before GL functions\n");
	assert(false);
	exit(1);
}

#ifdef PIGLIT_USE_WAFFLE
static enum waffle_enum piglit_waffle_dl = WAFFLE_DL_OPENGL;

/**
 * Generated code calls this function to retrieve the address of a
 * core function.
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
 * Generated code calls this function to retrieve the address of a
 * core function.
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
 * Generated code calls this function to retrieve the address of a
 * core function.
 */
static piglit_dispatch_function_ptr
get_core_proc(const char *name, int gl_10x_version)
{
	piglit_dispatch_function_ptr function_pointer = get_core_proc_address(name, gl_10x_version);
	if (function_pointer == NULL)
		get_proc_address_failure(name);
	return function_pointer;
}

/**
 * Generated code calls this function to retrieve the address of a
 * function defined in an extension.
 */
static piglit_dispatch_function_ptr
get_ext_proc(const char *name)
{
	piglit_dispatch_function_ptr function_pointer = get_ext_proc_address(name);
	if (function_pointer == NULL)
		get_proc_address_failure(name);
	return function_pointer;
}

/**
 * Generated code calls this function to determine whether a given GL
 * version is supported.
 */
static inline bool
check_version(int required_version)
{
	return gl_version >= required_version;
}

/**
 * Generated code calls this function to determine whether a given
 * extension is supported.
 */
static inline bool
check_extension(const char *name)
{
	return piglit_is_extension_supported(name);
}

#include "piglit-dispatch-gen.c"

/**
 * Initialize the dispatch mechanism.
 *
 * \param api is the API under test.  This determines whether
 * deprecated functionality is supported (since deprecated functions
 * cannot be used in forward compatible contexts).  It also affects
 * which GL version is queried for (since, for example a function
 * might be supported in GLES as of version 2.0, but in OpenGL only as
 * of version 2.1).  Not yet implemented.
 *
 * \param get_core_proc and get_ext_proc are the functions to call to
 * retrieve the address of a core GL function or an extension
 * function.  Note that for OpenGL, these can both map to the same
 * function (e.g. glXGetProcAddressARB).  However, in GLES, core
 * functions are not allowed to be queried using the GetProcAddress
 * mechanism, so get_core_proc will need to be implemented by looking
 * up a symbol in a shared library (e.g. using dlsym()).  When Waffle
 * is in use, these are ignored and replaced with the Waffle functions
 * that do the appropriate lookup according to the platform.  One day
 * we'll drop non-waffle support and remove this part of the
 * interface.
 *
 * \param unsupported_proc is the function to call if a test attempts
 * to use unsupported GL functionality.  It is passed the name of the
 * function that the test attempted to use.
 *
 * \param failure_proc is the function to call if a call to
 * get_core_proc() or get_ext_proc() unexpectedly returned NULL.  It
 * is passed the name of the function that was passed to
 * get_core_proc() or get_ext_proc().
 */
void
piglit_dispatch_init(piglit_dispatch_api api,
		     piglit_get_core_proc_address_function_ptr get_core_proc,
		     piglit_get_ext_proc_address_function_ptr get_ext_proc,
		     piglit_error_function_ptr unsupported_proc,
		     piglit_error_function_ptr failure_proc)
{
	dispatch_api = api;

	get_core_proc_address = get_core_proc;
	get_ext_proc_address = get_ext_proc;
	unsupported = unsupported_proc;
	get_proc_address_failure = failure_proc;

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
		get_core_proc_address = get_wfl_core_proc;
		get_ext_proc_address = get_wfl_ext_proc;
	}
#endif

	/* No need to reset the dispatch pointers the first time */
	if (is_initialized) {
		reset_dispatch_pointers();
	}

	is_initialized = true;

	/* Store the GL version and extension string for use by
	 * check_version() and check_extension().  Note: the
	 * following two calls are safe because the only GL function
	 * they call is glGetString(), and the stub function for
	 * glGetString does not need to call check_version() or
	 * check_extension().
	 */
	gl_version = piglit_get_gl_version();
}

/**
 * Compare two strings in the function_names table.
 */
static int compare_function_names(const void *x, const void *y)
{
	const char *x_str = *(const char * const *) x;
	const char *y_str = *(const char * const *) y;
	return strcmp(x_str, y_str);
}

/**
 * Retrieve a GL function pointer given the function name.
 *
 * This function is similar to glXGetProcAddressARB(), except that:
 *
 * - It is platform-independent.
 *
 * - It may be called on any supported function, regardless of whether
 *   the function is defined in GL core or an extension, and
 *   regardless of whether desktop GL or GLES is in use.
 *
 * - Synonymous function names (e.g. glMapBuffer and glMapBufferARB)
 *   may be used interchangably; the correct function is automatically
 *   chosen based on the GL version and extension string.
 *
 * - If the requested function is not supported by the implementation,
 *   the unsupported_proc that was passed to piglit_dispatch_init() is
 *   called.
 */
piglit_dispatch_function_ptr
piglit_dispatch_resolve_function(const char *name)
{
	size_t item_size = sizeof(function_names[0]);
	size_t num_items = ARRAY_SIZE(function_names);
	const char * const *item = (const char * const *)
		bsearch(&name, function_names, num_items, item_size,
			compare_function_names);
	check_initialized();
	if (!item) {
		unsupported(name);
		return NULL;
	} else {
		size_t item_index = item - function_names;
		return function_resolvers[item_index]();
	}
}
