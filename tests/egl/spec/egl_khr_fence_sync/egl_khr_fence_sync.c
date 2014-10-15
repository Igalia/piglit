/*
 * Copyright 2014 Intel Corporation
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

/**
 * \file Tests for EGL_KHR_fence_sync.
 *
 * This file attempts to exhaustively test the EGL_KHR_fence_sync spec.
 * However, some portions of the spec proved too difficult to test and are
 * listed below.
 *
 * TODO: The following excerpts from the EGL_KHR_fence_sync spec remain
 * untested:
 *
 *    More than one eglClientWaitSyncKHR may be outstanding on the same
 *    <sync> at any given time. When there are multiple threads blocked on
 *    the same <sync> and the sync object is signaled, all such threads
 *    are released, but the order in which they are released is not
 *    defined.
 *
 *    [...]
 *
 *    If any eglClientWaitSyncKHR commands are blocking on <sync> when
 *    eglDestroySyncKHR is called, <sync> is flagged for deletion and will
 *    be deleted when it is no longer associated with any fence command
 *    and is no longer blocking any eglClientWaitSyncKHR command.
 */

#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>

#ifdef PIGLIT_HAS_PTHREADS
#include <pthread.h>
#endif

#include "piglit-util-egl.h"
#include "piglit-util-gl.h"

/* Extension function pointers.
 *
 * Use prefix 'pegl' (piglit egl) instead of 'egl' to avoid collisions with
 * prototypes in eglext.h. */
EGLSyncKHR (*peglCreateSyncKHR)(EGLDisplay dpy, EGLenum type, const EGLint *attrib_list);
EGLBoolean (*peglDestroySyncKHR)(EGLDisplay dpy, EGLSyncKHR sync);
EGLint (*peglClientWaitSyncKHR)(EGLDisplay dpy, EGLSyncKHR sync, EGLint flags, EGLTimeKHR timeout);
EGLBoolean (*peglGetSyncAttribKHR)(EGLDisplay dpy, EGLSyncKHR sync, EGLint attribute, EGLint *value);

static const char *prog_name;
static const struct piglit_subtest subtests[];

static const EGLint canary = 0x31415926;
static EGLDisplay g_dpy = 0;
static EGLContext g_ctx = 0;

static void
print_usage(void)
{
	const char *usage =
		"usage:\n"
		"  %1$s\n"
		"      Run all subtests.\n"
		"\n"
		"  %1$s -list-subtests\n"
		"      List all subtests.\n"
		"\n"
		"  %1$s -subtest SUBTEST [-subtest SUBTEST [...]]\n"
		"      Run only the given subtests.\n"
		"\n"
		"  %1$s -h|--help\n"
		"      Print this help message.\n"
		;

	printf(usage, prog_name);
}

static void
usage_error(void)
{
	printf("\n");
	print_usage();
	piglit_report_result(PIGLIT_FAIL);
}

static enum piglit_result
init_display(EGLenum platform, EGLDisplay *out_dpy)
{
	enum piglit_result result = PIGLIT_PASS;
	EGLDisplay dpy;
	EGLint egl_major, egl_minor;
	bool ok;

	dpy = piglit_egl_get_default_display(platform);
	if (!dpy) {
		result = PIGLIT_SKIP;
		goto error;
	}

	ok = eglInitialize(dpy, &egl_major, &egl_minor);
	if (!ok) {
		result = PIGLIT_SKIP;
		goto error;
	}

	if (!piglit_is_egl_extension_supported(dpy, "EGL_KHR_fence_sync")) {
		piglit_loge("display does not support EGL_KHR_fence_sync");
		result = PIGLIT_SKIP;
		goto error;

	}

	*out_dpy = dpy;
	return result;

error:
	if (dpy) {
		eglTerminate(dpy);
	}
	return result;
}

/**
 * Create OpenGL ES 2.0 context, make it current, and verify that it supports
 * GL_OES_EGL_sync.
 */
static enum piglit_result
init_context(EGLDisplay dpy, EGLContext *out_ctx)
{
	enum piglit_result result = PIGLIT_PASS;
	bool ok = false;
	EGLConfig config = 0;
	EGLint num_configs = 0;
	EGLContext ctx = 0;

	/* Create OpenGL ES 2.0 or backwards-compatible context. */
	static const EGLint config_attribs[] = {
		EGL_RED_SIZE,		EGL_DONT_CARE,
		EGL_GREEN_SIZE,		EGL_DONT_CARE,
		EGL_BLUE_SIZE,		EGL_DONT_CARE,
		EGL_ALPHA_SIZE,		EGL_DONT_CARE,
		EGL_DEPTH_SIZE, 	EGL_DONT_CARE,
		EGL_STENCIL_SIZE, 	EGL_DONT_CARE,
		EGL_RENDERABLE_TYPE, 	EGL_OPENGL_ES2_BIT
				        | EGL_OPENGL_ES3_BIT_KHR,
		EGL_NONE,
	};

	static const EGLint context_attribs[] = {
		EGL_CONTEXT_CLIENT_VERSION, 2,
		EGL_NONE,
	};

	ok = eglChooseConfig(dpy, config_attribs, &config, 1,
			     &num_configs);
	if (!ok || !config || num_configs == 0) {
		EGLint egl_error = eglGetError();
		piglit_loge("failed to get EGLConfig: %s(0x%x)",
			  piglit_get_egl_error_name(egl_error), egl_error);
		result = PIGLIT_SKIP;
		goto error;
	}

	ok = piglit_egl_bind_api(EGL_OPENGL_ES_API);
	if (!ok) {
		piglit_loge("failed to bind EGL_OPENGL_ES_API");
		result = PIGLIT_FAIL;
		goto error;

	}

	ctx = eglCreateContext(dpy, config, EGL_NO_CONTEXT, context_attribs);
	if (!ctx) {
		EGLint egl_error = eglGetError();
		piglit_loge("failed to create EGLContext: %s(0x%x)",
			  piglit_get_egl_error_name(egl_error), egl_error);
		result = PIGLIT_FAIL;
		goto error;
	}

	ok = eglMakeCurrent(dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, ctx);
	if (!ok) {
		/* Skip, don't fail. Assume the context doesn't support
		 * GL_OES_surfaceless_context or equivalent.
		 */
		piglit_loge("failed to make context current without surface");
		result = PIGLIT_SKIP;
		goto error;
	}

	piglit_dispatch_default_init(PIGLIT_DISPATCH_ES2);

	/* From the EGL_KHR_fence_sync spec:
	 *
	 *     Each client API which supports fence commands indicates this
	 *     support in the form of a client API extension. If the
	 *     GL_OES_EGL_sync extension is supported by OpenGL ES (either
	 *     version 1.x or 2.0), a fence sync object may be created when the
	 *     currently bound API is OpenGL ES.
	 */
	if (!piglit_is_extension_supported("GL_OES_EGL_sync")) {
		piglit_loge("context does not support GL_OES_EGL_sync; "
			  "skipping test");
		result = PIGLIT_SKIP;
		goto error;
	}

	*out_ctx = ctx;
	return result;

error:
	if (ctx) {
		eglMakeCurrent(dpy, EGL_NO_SURFACE, EGL_NO_SURFACE,
			       EGL_NO_CONTEXT);
		eglDestroyContext(dpy, ctx);
	}
	return result;
}

/**
 * Teardown state after each subtest completes.
 */
static void
test_cleanup(EGLSyncKHR sync, enum piglit_result *inout_result)
{
	bool ok = false;

	if (sync) {
		/* From the EGL_KHR_fence_sync spec:
		 *
		 *     If no errors are generated, EGL_TRUE is returned, and
		 *     <sync> will no longer be the handle of a valid sync
		 *     object.
		 */
		ok = peglDestroySyncKHR(g_dpy, sync);
		if (!ok) {
			piglit_loge("eglDestroySyncKHR failed");
			*inout_result = PIGLIT_FAIL;
		}
		if (!piglit_check_egl_error(EGL_SUCCESS)) {
			piglit_loge("eglDestroySyncKHR emitted an error");
			*inout_result = PIGLIT_FAIL;
		}
	}

	/* Ensure that no leftover GL commands impact the next test. */
	if (eglGetCurrentContext()) {
		glFinish();
	}

	if (g_dpy) {
		eglMakeCurrent(g_dpy, 0, 0, 0);
		ok = eglTerminate(g_dpy);
		if (!ok) {
			piglit_loge("failed to terminate EGLDisplay");
			*inout_result = PIGLIT_FAIL;
		}
	}

	g_dpy = EGL_NO_DISPLAY;
	g_ctx = EGL_NO_CONTEXT;
}

/**
 * Setup state before each subtest begins.
 */
static enum piglit_result
test_setup(void)
{
	enum piglit_result result = PIGLIT_PASS;

	/* Just in case the previous test forgot to unset these pointers... */
	g_dpy = EGL_NO_DISPLAY;
	g_ctx = EGL_NO_CONTEXT;

	result = init_display(EGL_NONE, &g_dpy);
	if (result != PIGLIT_PASS) {
		goto cleanup;
	}


	result = init_context(g_dpy, &g_ctx);
	if (result != PIGLIT_PASS) {
		goto cleanup;
	}
	/* Ensure that a context is bound so that the test can create syncs. */
	eglMakeCurrent(g_dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, g_ctx);

cleanup:
	if (result != PIGLIT_PASS) {
		test_cleanup(EGL_NO_SYNC_KHR, &result);
	}
	return result;
}

/**
 * Verify that eglCreateSyncKHR(), when given an empty attribute list,
 * intializes the sync object's attributes to the correct values.
 *
 * From the EGL_KHR_fence_sync spec:
 *
 *     Attributes not specified in the list will be assigned their default
 *     values.
 *
 *     Attributes of the fence sync object are
 *     set as follows:
 *
 *       Attribute Name         Initial Attribute Value(s)
 *       ---------------        --------------------------
 *       EGL_SYNC_TYPE_KHR      EGL_SYNC_FENCE_KHR
 *       EGL_SYNC_STATUS_KHR    EGL_UNSIGNALED_KHR
 *       EGL_SYNC_CONDITION_KHR EGL_SYNC_PRIOR_COMMANDS_COMPLETE_KHR
 */
static enum piglit_result
test_eglCreateSyncKHR_default_attributes(void *test_data)
{
	enum piglit_result result = PIGLIT_PASS;
	EGLSyncKHR sync = 0;
	EGLint sync_type = canary,
	       sync_status = canary,
	       sync_condition = canary;
	bool ok = false;

	result = test_setup();
	if (result != PIGLIT_PASS) {
		return result;
	}

	sync = peglCreateSyncKHR(g_dpy, EGL_SYNC_FENCE_KHR, NULL);
	if (sync == EGL_NO_SYNC_KHR) {
		piglit_loge("eglCreateSyncKHR(EGL_SYNC_FENCE_KHR) failed");
		result = PIGLIT_FAIL;
		goto cleanup;
	}

	ok = peglGetSyncAttribKHR(g_dpy, sync, EGL_SYNC_TYPE_KHR, &sync_type);
	if (!ok) {
		piglit_loge("eglGetSyncAttribKHR(EGL_SYNC_TYPE_KHR) failed");
		result = PIGLIT_FAIL;
	}
	if (!piglit_check_egl_error(EGL_SUCCESS)) {
		piglit_loge("eglGetSyncAttribKHR(EGL_SYNC_TYPE_KHR) emitted "
			  "an error");
		result = PIGLIT_FAIL;
	}
	if (sync_type != EGL_SYNC_FENCE_KHR) {
		piglit_loge("eglGetSyncAttribKHR(EGL_SYNC_TYPE_KHR) returned "
			  "0x%x but expected EGL_SYNC_FENCE_KHR(0x%x)",
			  sync_type, EGL_SYNC_FENCE_KHR);
		result = PIGLIT_FAIL;
	}

	ok = peglGetSyncAttribKHR(g_dpy, sync, EGL_SYNC_STATUS_KHR, &sync_status);
	if (!ok) {
		piglit_loge("eglGetSyncAttribKHR(EGL_SYNC_STATUS_KHR) failed");
		result = PIGLIT_FAIL;
	}
	if (!piglit_check_egl_error(EGL_SUCCESS)) {
		piglit_loge("eglGetSyncAttribKHR(EGL_SYNC_STATUS_KHR) emitted "
			  "an error");
		result = PIGLIT_FAIL;
	}

	ok = peglGetSyncAttribKHR(g_dpy, sync, EGL_SYNC_CONDITION_KHR, &sync_condition);
	if (!ok) {
		piglit_loge("eglGetSyncAttribKHR(EGL_SYNC_CONDITION_KHR) failed");
		result = PIGLIT_FAIL;
	}
	if (!piglit_check_egl_error(EGL_SUCCESS)) {
		piglit_loge("eglGetSyncAttribKHR(EGL_SYNC_CONDITION_KHR) "
			  "emitted an error");
		result = PIGLIT_FAIL;
	}
	if (sync_condition != EGL_SYNC_PRIOR_COMMANDS_COMPLETE_KHR) {
		piglit_loge("eglGetSyncAttribKHR(EGL_SYNC_CONDITION_KHR) "
			  "returned 0x%x but expected "
			  "EGL_SYNC_PRIOR_COMMANDS_COMPLETE_KHR(0x%x)",
			  sync_condition, EGL_SYNC_PRIOR_COMMANDS_COMPLETE_KHR);
		result = PIGLIT_FAIL;
	}

cleanup:
	test_cleanup(sync, &result);
	return result;
}

/**
 * Verify that eglCreateSyncKHR emits correct error when given an invalid
 * display.
 *
 * From the EGL_KHR_fence_sync spec:
 *
 *     If <dpy> is not the name of a valid, initialized EGLDisplay,
 *     EGL_NO_SYNC_KHR is returned and an EGL_BAD_DISPLAY error is
 *     generated.
 */
static enum piglit_result
test_eglCreateSyncKHR_invalid_display(void *test_data)
{
	enum piglit_result result = PIGLIT_PASS;
	EGLSyncKHR sync = 0;

	result = test_setup();
	if (result != PIGLIT_PASS) {
		return result;
	}

	sync = peglCreateSyncKHR(EGL_NO_DISPLAY, EGL_SYNC_FENCE_KHR, NULL);
	if (sync != EGL_NO_SYNC_KHR) {
		piglit_loge("eglCreateSyncKHR(EGL_NO_DISPLAY) succeeded");
		result = PIGLIT_FAIL;
	}
	if (!piglit_check_egl_error(EGL_BAD_DISPLAY)) {
		piglit_loge("eglCreateSyncKHR emitted wrong error");
		result = PIGLIT_FAIL;
	}

	test_cleanup(sync, &result);
	return result;
}

/**
 * Verify that eglCreateSyncKHR emits correct error when given an invalid
 * attribute list.
 *
 * From the EGL_KHR_fence_sync spec:
 *
 *     If <attrib_list> is neither NULL nor empty (containing only
 *     EGL_NONE), EGL_NO_SYNC_KHR is returned and an EGL_BAD_ATTRIBUTE
 *     error is generated.
 */
static enum piglit_result
test_eglCreateSyncKHR_invalid_attrib_list(void *test_data)
{
	enum piglit_result result = PIGLIT_PASS;
	EGLSyncKHR sync = 0;
	const EGLint attrib_list[] = {
		EGL_CONTEXT_CLIENT_VERSION, 2,
		EGL_NONE,
	};

	result = test_setup();
	if (result != PIGLIT_PASS) {
		return result;
	}

	sync = peglCreateSyncKHR(g_dpy, EGL_SYNC_FENCE_KHR, attrib_list);
	if (sync != EGL_NO_SYNC_KHR) {
		piglit_loge("eglCreateSyncKHR() succeeded with invalid "
			  "attrib list");
		result = PIGLIT_FAIL;
	}
	if (!piglit_check_egl_error(EGL_BAD_ATTRIBUTE)) {
		piglit_loge("eglCreateSyncKHR emitted wrong error");
		result = PIGLIT_FAIL;
	}

	test_cleanup(sync, &result);
	return result;
}

/**
 * Verify that eglCreateSyncKHR emits correct error when given an invalid
 * sync type.
 *
 * From the EGL_KHR_fence_sync spec:
 *
 *     If <type> is not a supported type of sync object,
 *     EGL_NO_SYNC_KHR is returned and an EGL_BAD_ATTRIBUTE error is
 *     generated.
 *
 * TODO(chadv,joshtriplett): eglCreateSyncKHR should generate EGL_BAD_PARAMETER
 * TODO: on bad sync types, not EGL_BAD_ATTRIBUTE. Bug filed in Khronos private
 * TODO: Bugzilla; update the test when resolved.
 */
static enum piglit_result
test_eglCreateSyncKHR_invalid_sync_type(void *test_data)
{
	enum piglit_result result = PIGLIT_PASS;
	EGLSyncKHR sync = 0;
	EGLenum bad_sync_type = EGL_SYNC_TYPE_KHR;

	result = test_setup();
	if (result != PIGLIT_PASS) {
		return result;
	}

	sync = peglCreateSyncKHR(g_dpy, bad_sync_type, NULL);
	if (sync != EGL_NO_SYNC_KHR) {
		piglit_loge("eglCreateSyncKHR() succeeded with invalid "
			  "sync type");
		result = PIGLIT_FAIL;
	}
	if (!piglit_check_egl_error(EGL_BAD_ATTRIBUTE)) {
		piglit_loge("eglCreateSyncKHR emitted wrong error");
		result = PIGLIT_FAIL;
	}

	test_cleanup(sync, &result);
	return result;
}

/**
 * Verify that eglCreateSyncKHR emits correct error when no context is current.
 *
 * From the EGL_KHR_fence_sync spec:
 *
 *     If <type> is EGL_SYNC_FENCE_KHR and no context is current for
 *     the bound API (i.e., eglGetCurrentContext returns
 *     EGL_NO_CONTEXT), EGL_NO_SYNC_KHR is returned and an
 *     EGL_BAD_MATCH error is generated.
 */
static enum piglit_result
test_eglCreateSyncKHR_no_current_context(void *test_data)
{
	enum piglit_result result = PIGLIT_PASS;
	EGLSyncKHR sync = 0;

	result = test_setup();
	if (result != PIGLIT_PASS) {
		return result;
	}
	eglMakeCurrent(g_dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);

	sync = peglCreateSyncKHR(g_dpy, EGL_SYNC_FENCE_KHR, NULL);
	if (sync != EGL_NO_SYNC_KHR) {
		piglit_loge("eglCreateSyncKHR() succeeded when no context was "
			  "current");
		peglDestroySyncKHR(g_dpy, sync);
		result = PIGLIT_FAIL;
	}
	if (!piglit_check_egl_error(EGL_BAD_MATCH)) {
		piglit_loge("eglCreateSyncKHR emitted wrong error");
		result = PIGLIT_FAIL;
	}

	test_cleanup(sync, &result);
	return result;
}

/**
 * Verify that eglGetSyncAttribKHR emits the correct error when given an object
 * that is not a sync object.
 *
 * From the EGL_KHR_fence_sync:
 *
 *    * If <sync> is not a valid sync object for <dpy>, EGL_FALSE is
 *      returned and an EGL_BAD_PARAMETER error is generated.
 *
 *    [...]
 *
 *    If any error occurs, <*value> is not modified.
 */
static enum piglit_result
test_eglGetSyncAttribKHR_invalid_sync(void *test_data)
{
	enum piglit_result result = PIGLIT_PASS;
	bool ok = false;
	EGLint sync_type = canary;
	EGLSyncKHR invalid_sync = (EGLSyncKHR) &canary;

	result = test_setup();
	if (result != PIGLIT_PASS) {
		return result;
	}

	ok = peglGetSyncAttribKHR(g_dpy, invalid_sync, EGL_SYNC_TYPE_KHR, &sync_type);
	if (ok) {
		piglit_loge("eglGetSyncAttribKHR incorrectly succeeded when "
		          "given an invalid sync object");
		result = PIGLIT_FAIL;
	}
	if (!piglit_check_egl_error(EGL_BAD_PARAMETER)) {
		piglit_loge("eglGetSyncAttribKHR emitted wrong error");
		result = PIGLIT_FAIL;
	}
	if (sync_type != canary) {
		piglit_loge("eglGetSynAttribKHR modified out parameter <value>");
		result = PIGLIT_FAIL;
	}

	test_cleanup(EGL_NO_SYNC_KHR, &result);
	return result;
}

/**
 * Verify that eglGetSyncAttribKHR emits the correct error when querying an
 * unrecognized attribute of a fence sync.
 *
 * From the EGL_KHR_fence_sync:
 *
 *    [eglGetSyncAttribKHR] is used to query attributes of the sync object
 *    <sync>. Legal values for <attribute> depend on the type of sync object,
 *    as shown in table
 *    3.cc. [...]
 *
 *    Attribute              Description                Supported Sync Objects
 *    -----------------      -----------------------    ----------------------
 *    EGL_SYNC_TYPE_KHR      Type of the sync object    All
 *    EGL_SYNC_STATUS_KHR    Status of the sync object  All
 *    EGL_SYNC_CONDITION_KHR Signaling condition        EGL_SYNC_FENCE_KHR only
 *
 *    Table 3.cc  Attributes Accepted by eglGetSyncAttribKHR Command
 *
 *    [...]
 *
 *    * If <attribute> is not one of the attributes in table 3.cc,
 *      EGL_FALSE is returned and an EGL_BAD_ATTRIBUTE error is
 *      generated.
 *
 *    [...]
 *
 *    If any error occurs, <*value> is not modified.
 */
static enum piglit_result
test_eglGetSyncAttribKHR_invalid_attrib(void *test_data)
{
	enum piglit_result result = PIGLIT_PASS;
	bool ok = false;
	EGLSyncKHR sync = 0;
	EGLint attrib_value = canary;

	result = test_setup();
	if (result != PIGLIT_PASS) {
		return result;
	}

	sync = peglCreateSyncKHR(g_dpy, EGL_SYNC_FENCE_KHR, NULL);
	if (sync == EGL_NO_SYNC_KHR) {
		piglit_loge("eglCreateSyncKHR(EGL_SYNC_FENCE_KHR) failed");
		result = PIGLIT_FAIL;
		goto cleanup;
	}

	ok = peglGetSyncAttribKHR(g_dpy, sync, EGL_BUFFER_PRESERVED,
				 &attrib_value);
	if (ok) {
		piglit_loge("eglGetSyncAttribKHR(attrib=EGL_BUFFER_PRESERVED) "
		          "incorrectly succeeded");
		result = PIGLIT_FAIL;
	}
	if (!piglit_check_egl_error(EGL_BAD_ATTRIBUTE)) {
		piglit_loge("eglGetSyncAttribKHR emitted wrong error");
		result = PIGLIT_FAIL;
	}
	if (attrib_value != canary) {
		piglit_loge("eglGetSynAttribKHR modified out parameter <value>");
		result = PIGLIT_FAIL;
	}

cleanup:
	test_cleanup(sync, &result);
	return result;
}

/**
 * Verify that glClientWaitSyncKHR emits correct error when given invalid flag.
 *
 * From the EGL_KHR_fence_sync spec:
 *
 *    Accepted in the <flags> parameter of eglClientWaitSyncKHR:
 *
 *    EGL_SYNC_FLUSH_COMMANDS_BIT_KHR         0x0001
 */
static enum piglit_result
test_eglClientWaitSyncKHR_invalid_flag(void *test_data)
{
	enum piglit_result result = PIGLIT_PASS;
	EGLSyncKHR sync = 0;
	EGLint wait_status = 0;
	EGLint invalid_flag = 0x8000;

	result = test_setup();
	if (result != PIGLIT_PASS) {
		return result;
	}

	sync = peglCreateSyncKHR(g_dpy, EGL_SYNC_FENCE_KHR, NULL);
	if (sync == EGL_NO_SYNC_KHR) {
		piglit_loge("eglCreateSyncKHR(EGL_SYNC_FENCE_KHR) failed");
		result = PIGLIT_FAIL;
		goto cleanup;
	}

	/* Use timeout=0 so that eglClientWaitSyncKHR immediately returns. */
	wait_status = peglClientWaitSyncKHR(g_dpy, sync, invalid_flag, 0);
	if (wait_status != EGL_FALSE) {
		piglit_loge("eglClientWaitSyncKHR succeeded when given invalid "
			  "flag 0x%x", invalid_flag);
		result = PIGLIT_FAIL;
	}
	if (!piglit_check_egl_error(EGL_BAD_PARAMETER)) {
		piglit_loge("eglClientWaitSyncKHR emitted wrong error");
		result = PIGLIT_FAIL;
	}

cleanup:
	test_cleanup(sync, &result);
	return result;
}

/**
 * Verify that eglClientWaitSyncKHR() correctly handles zero timeout before and
 * after glFinish().
 *
 * From the EGL_KHR_fence_sync:
 *
 *     If the value of <timeout> is zero, then eglClientWaitSyncKHR simply
 *     tests the current status of <sync>.
 *
 *     [...]
 *
 *     eglClientWaitSyncKHR returns one of three status values describing
 *     the reason for returning. A return value of EGL_TIMEOUT_EXPIRED_KHR
 *     indicates that the specified timeout period expired before <sync>
 *     was signaled. A return value of EGL_CONDITION_SATISFIED_KHR
 *     indicates that <sync> was signaled before the timeout expired, which
 *     includes the case when <sync> was already signaled when
 *     eglClientWaitSyncKHR was called. If an error occurs then an error is
 *     generated and EGL_FALSE is returned.
 */
static enum piglit_result
test_eglClientWaitSyncKHR_zero_timeout(void *test_data)
{
	enum piglit_result result = PIGLIT_PASS;
	EGLSyncKHR sync = 0;
	EGLint wait_status1 = 0, wait_status2 = 0;

	result = test_setup();
	if (result != PIGLIT_PASS) {
		return result;
	}
	glClear(GL_COLOR_BUFFER_BIT);

	sync = peglCreateSyncKHR(g_dpy, EGL_SYNC_FENCE_KHR, NULL);
	if (sync == EGL_NO_SYNC_KHR) {
		piglit_loge("eglCreateSyncKHR(EGL_SYNC_FENCE_KHR) failed");
		result = PIGLIT_FAIL;
		goto cleanup;
	}

	wait_status1 = peglClientWaitSyncKHR(g_dpy, sync, 0, 0);
	glFinish();
	wait_status2 = peglClientWaitSyncKHR(g_dpy, sync, 0, 0);

	if (wait_status1 != EGL_TIMEOUT_EXPIRED_KHR &&
	    wait_status1 != EGL_CONDITION_SATISFIED_KHR) {
		piglit_loge("eglClientWaitSyncKHR() before glFinish:\n"
			  "  Expected status: EGL_TIMEOUT_EXPIRED_KHR or "
			  "    EGL_CONDITION_SATISFIED_KHR\n"
			  "  Actual status:  0x%x\n", wait_status1);
		result = PIGLIT_FAIL;
	}
	if (wait_status2 != EGL_CONDITION_SATISFIED_KHR) {
		piglit_loge("eglClientWaitSyncKHR() after glFinish:\n"
			  "  Expected status: EGL_CONDITION_SATISFIED_KHR\n"
			  "  Actual status:  0x%x\n", wait_status1);
		result = PIGLIT_FAIL;
	}

cleanup:
	test_cleanup(sync, &result);
	return result;
}

/**
 * Verify that eglClientWaitSyncKHR() accepts the
 * EGL_SYNC_FLUSH_COMMANDS_BIT_KHR flag.
 *
 * From the EGL_KHR_fence_sync:
 *
 *   Accepted in the <flags> parameter of eglClientWaitSyncKHR:
 *     EGL_SYNC_FLUSH_COMMANDS_BIT_KHR         0x0001
 */
static enum piglit_result
test_eglClientWaitSyncKHR_flag_sync_flush(void *test_data)
{
	enum piglit_result result = PIGLIT_PASS;
	EGLSyncKHR sync = 0;
	EGLint wait_status = 0;

	result = test_setup();
	if (result != PIGLIT_PASS) {
		return result;
	}
	glClear(GL_COLOR_BUFFER_BIT);

	sync = peglCreateSyncKHR(g_dpy, EGL_SYNC_FENCE_KHR, NULL);
	if (sync == EGL_NO_SYNC_KHR) {
		piglit_loge("eglCreateSyncKHR(EGL_SYNC_FENCE_KHR) failed");
		result = PIGLIT_FAIL;
		goto cleanup;
	}

	wait_status = peglClientWaitSyncKHR(g_dpy, sync,
			EGL_SYNC_FLUSH_COMMANDS_BIT_KHR, 0);
	if (wait_status != EGL_TIMEOUT_EXPIRED_KHR &&
	    wait_status != EGL_CONDITION_SATISFIED_KHR) {
		piglit_loge("eglClientWaitSyncKHR() before glFinish:\n"
			  "  Expected status: EGL_TIMEOUT_EXPIRED_KHR or "
			  "    EGL_CONDITION_SATISFIED_KHR\n"
			  "  Actual status:  0x%x\n", wait_status);
		result = PIGLIT_FAIL;
	}

cleanup:
	test_cleanup(sync, &result);
	return result;
}

/**
 * Verify that eglGetSyncAttribKHR() reports correct sync status before and
 * after glFinish().
 */
static enum piglit_result
test_eglGetSyncAttribKHR_sync_status(void *test_data)
{
	enum piglit_result result = PIGLIT_PASS;
	EGLSyncKHR sync = 0;
	EGLint sync_status = 0;
	bool ok = false;

	result = test_setup();
	if (result != PIGLIT_PASS) {
		return result;
	}
	glClear(GL_COLOR_BUFFER_BIT);

	sync = peglCreateSyncKHR(g_dpy, EGL_SYNC_FENCE_KHR, NULL);
	if (sync == EGL_NO_SYNC_KHR) {
		piglit_loge("eglCreateSyncKHR(EGL_SYNC_FENCE_KHR) failed");
		result = PIGLIT_FAIL;
		goto cleanup;
	}

	ok = peglGetSyncAttribKHR(g_dpy, sync, EGL_SYNC_STATUS_KHR, &sync_status);
	if (!ok) {
		piglit_loge("before glFinish, eglGetSyncAttribKHR(EGL_SYNC_STATUS_KHR) "
			  "failed");
		result = PIGLIT_FAIL;
	}
	if (!piglit_check_egl_error(EGL_SUCCESS)) {
		piglit_loge("before glFinish, eglGetSyncAttribKHR(EGL_SYNC_STATUS_KHR) "
			  "emitted an error");
		result = PIGLIT_FAIL;
	}
	if (sync_status != EGL_SIGNALED_KHR &&
	    sync_status != EGL_UNSIGNALED_KHR) {
		piglit_loge("before glFinish, eglGetSyncAttribKHR(EGL_SYNC_STATUS_KHR):\n"
			  "  Expected status: EGL_SIGNALED_KHR or EGL_UNSIGNALED_KHR\n",
			  "  Actual status: 0x%x",
			  sync_status);
		result = PIGLIT_FAIL;
	}

	glFinish();

	ok = peglGetSyncAttribKHR(g_dpy, sync, EGL_SYNC_STATUS_KHR, &sync_status);
	if (!ok) {
		piglit_loge("after glFinish, eglGetSyncAttribKHR(EGL_SYNC_STATUS_KHR) "
			  "failed");
		result = PIGLIT_FAIL;
	}
	if (!piglit_check_egl_error(EGL_SUCCESS)) {
		piglit_loge("after glFinish, eglGetSyncAttribKHR(EGL_SYNC_STATUS_KHR) "
			  "emitted an error");
		result = PIGLIT_FAIL;
	}
	if (sync_status != EGL_SIGNALED_KHR) {
		piglit_loge("after glFinish, eglGetSyncAttribKHR(EGL_SYNC_STATUS_KHR):\n"
			  "  Expected status: EGL_SIGNALED_KHR\n",
			  "  Actual status: 0x%x",
			  sync_status);
		result = PIGLIT_FAIL;
	}

cleanup:
	test_cleanup(sync, &result);
	return result;
}

/**
 * Verify that eglClientWaitSyncKHR() emits correct error when given an invalid
 * sync object.
 *
 * From the EGL_KHR_fence_sync:
 *
 *    * If <sync> is not a valid sync object for <dpy>, EGL_FALSE is
 *      returned and an EGL_BAD_PARAMETER error is generated.
 */
static enum piglit_result
test_eglClientWaitSyncKHR_invalid_sync(void *test_data)
{
	enum piglit_result result = PIGLIT_PASS;
	EGLint wait_status = 0;
	EGLSyncKHR invalid_sync = (EGLSyncKHR) &canary;

	result = test_setup();
	if (result != PIGLIT_PASS) {
		return result;
	}

	wait_status = peglClientWaitSyncKHR(g_dpy, invalid_sync, 0, 0);
	if (wait_status != EGL_FALSE) {
		piglit_loge("Given an invalid sync object, eglClientWaitSyncKHR() "
			  "should return EGL_FALSE, but returned 0x%x",
			  wait_status);
		result = PIGLIT_FAIL;
	}
	if (!piglit_check_egl_error(EGL_BAD_PARAMETER)) {
		piglit_loge("Given an invalid sync object, eglClientWaitSyncKHR() "
			  "did not emit EGL_BAD_PARAMETER");
		result = PIGLIT_FAIL;
	}

	test_cleanup(EGL_NO_SYNC_KHR, &result);
	return result;
}

/**
 * Verify that eglClientWaitSyncKHR() accepts nonzero timeout values, including
 * EGL_FOREVER_KHR.
 */
static enum piglit_result
test_eglClientWaitSyncKHR_nonzero_timeout(void *test_data)
{
	enum piglit_result result = PIGLIT_PASS;
	EGLSyncKHR sync = 0;
	EGLint wait_status = 0;

	result = test_setup();
	if (result != PIGLIT_PASS) {
		return result;
	}

	sync = peglCreateSyncKHR(g_dpy, EGL_SYNC_FENCE_KHR, NULL);
	if (sync == EGL_NO_SYNC_KHR) {
		piglit_loge("eglCreateSyncKHR(EGL_SYNC_FENCE_KHR) failed");
		result = PIGLIT_FAIL;
		goto cleanup;
	}

	/* There exist no pending GL commands, so the sync status should be
	 * EGL_CONDITION_SATISFIED_KHR.
	 */
	wait_status = peglClientWaitSyncKHR(g_dpy, sync, 0, 0.5e9);
	if (wait_status != EGL_CONDITION_SATISFIED_KHR) {
		piglit_loge("eglClientWaitSyncKHR(timeout=0.5sec)\n"
			  "  Expected status EGL_CONDITION_SATISFIED_KHR(0x%x)\n"
			  "  Actual status 0x%x\n",
			  EGL_CONDITION_SATISFIED_KHR, wait_status);
		result = PIGLIT_FAIL;
	}

	wait_status = peglClientWaitSyncKHR(g_dpy, sync, 0, EGL_FOREVER_KHR);
	if (wait_status != EGL_CONDITION_SATISFIED_KHR) {
		piglit_loge("eglClientWaitSyncKHR(timeout=forever)\n"
			  "  Expected status EGL_CONDITION_SATISFIED_KHR(0x%x)\n"
			  "  Actual status 0x%x\n",
			  EGL_CONDITION_SATISFIED_KHR, wait_status);
		result = PIGLIT_FAIL;
	}

cleanup:
	test_cleanup(sync, &result);
	return result;
}

static enum piglit_result
init_other_display(EGLDisplay *out_other_dpy, EGLDisplay orig_dpy)
{
	enum piglit_result result = PIGLIT_PASS;
	EGLDisplay other_dpy = 0;
	int i;

	static const EGLint platforms[] = {
		EGL_PLATFORM_X11_EXT,
		EGL_PLATFORM_WAYLAND_EXT,
		EGL_PLATFORM_GBM_MESA,
		0,
	};

	for (i = 0; platforms[i] != 0; ++i) {
		result = init_display(platforms[i], &other_dpy);
		switch (result) {
		case PIGLIT_SKIP:
			break;
		case PIGLIT_PASS:
			if (other_dpy && other_dpy != orig_dpy) {
				*out_other_dpy = other_dpy;
				return PIGLIT_PASS;
			} else {
				result = PIGLIT_SKIP;
				break;
			}
		default:
			break;
		}
			
	}

	return result;
}

/**
 * Verify that eglCreateSyncKHR() emits correct error when given a display that
 * does not match the display of the bound context.
 *
 * From the EGL_KHR_fence_sync spec:
 *
 *      * If <type> is EGL_SYNC_FENCE_KHR and <dpy> does not match the
 *        EGLDisplay of the currently bound context for the currently
 *        bound client API (the EGLDisplay returned by
 *        eglGetCurrentDisplay()) then EGL_NO_SYNC_KHR is returned and an
 *        EGL_BAD_MATCH error is generated.
 *
 * This test verifies a simple case for the above error. It binds a context and
 * display to the main thread, creates a second display on the same threads but
 * does not bind it, then gives the second display to eglCreateSyncKHR().
 */
static enum piglit_result
test_eglCreateSyncKHR_wrong_display_same_thread(void *test_data)
{
	enum piglit_result result = PIGLIT_PASS;
	EGLDisplay wrong_dpy = 0;
	EGLSyncKHR sync = 0;

	result = test_setup();
	if (result != PIGLIT_PASS) {
		return result;
	}

	piglit_logi("create second EGLDisplay");
	result = init_other_display(&wrong_dpy, g_dpy);
	if (result != PIGLIT_PASS) {
		goto cleanup;
	}

	piglit_require_egl_extension(wrong_dpy, "EGL_KHR_fence_sync");

	piglit_logi("try to create sync with second display");
	sync = peglCreateSyncKHR(wrong_dpy, EGL_SYNC_FENCE_KHR, NULL);
	if (sync != EGL_NO_SYNC_KHR) {
		piglit_loge("eglCreateSyncKHR() incorrectly succeeded");
		result = PIGLIT_FAIL;
		goto cleanup;
	}
	if (!piglit_check_egl_error(EGL_BAD_MATCH)) {
		piglit_loge("eglCreateSyncKHR emitted wrong error");
		result = PIGLIT_FAIL;
		goto cleanup;
	}

cleanup:
	if (wrong_dpy) {
		eglTerminate(wrong_dpy);
	}
	test_cleanup(EGL_NO_SYNC_KHR, &result);
	return result;
}

#ifdef PIGLIT_HAS_PTHREADS
/**
 * Check that EGL can create and wait on sync fences in the current context.
 */
static enum piglit_result
check_sync_in_current_context(void)
{
	enum piglit_result result = PIGLIT_PASS;
	EGLDisplay dpy = eglGetCurrentDisplay();
	EGLSyncKHR sync = 0;
	EGLint wait_status = 0;
	
	if (!eglGetCurrentContext()) {
		piglit_loge("no context is bound");
		result = PIGLIT_FAIL;
		goto cleanup;
	}

	piglit_logi("verify that syncs can be created and waited on in "
		 "this thread");
	sync = peglCreateSyncKHR(dpy, EGL_SYNC_FENCE_KHR, NULL);
	if (sync == EGL_NO_SYNC_KHR) {
		piglit_loge("eglCreateSyncKHR failed");
		result = PIGLIT_FAIL;
		goto cleanup;
	}

	wait_status = peglClientWaitSyncKHR(dpy, sync, 0, 0);
	if (wait_status == EGL_FALSE) {
		piglit_loge("eglClientWaitSyncKHR failed");
		result = PIGLIT_FAIL;
		goto cleanup;
	}

cleanup:
	if (sync) {
		peglDestroySyncKHR(dpy, sync);
	}
	return result;
}

static void*
thread2_create_sync_with_display_bound_in_other_thread(void *arg)
{
	enum piglit_result *result;
	EGLDisplay t2_dpy = 0;
	EGLContext t2_ctx = 0;
	EGLSyncKHR t2_sync = 0;

	result = malloc(sizeof(*result));
	*result = PIGLIT_FAIL;

	piglit_logi("create second EGLDisplay");
	*result = init_other_display(&t2_dpy, g_dpy);
	if (*result != PIGLIT_PASS) {
		piglit_loge("failed to initialize a second EGLDisplay");
		goto cleanup;
	}

	if (!piglit_is_egl_extension_supported(t2_dpy, "EGL_KHR_fence_sync")) {
		piglit_loge("EGL_KHR_fence_sync unsupported on second display");
		*result = PIGLIT_SKIP;
		goto cleanup;
	}

	piglit_logi("create and make context current on second display");
	*result = init_context(t2_dpy, &t2_ctx);
	if (*result != PIGLIT_PASS) {
		goto cleanup;
	}

	*result = check_sync_in_current_context();
	if (*result != PIGLIT_PASS) {
		goto cleanup;
	}

	piglit_logi("try to create sync on first display, which is "
		 "bound on thread1");
	t2_sync = peglCreateSyncKHR(t2_dpy, EGL_SYNC_FENCE_KHR, NULL);
	if (t2_sync != EGL_NO_SYNC_KHR) {
		piglit_loge("eglCreateSyncKHR incorrectly succeeded");
		*result = PIGLIT_FAIL;
		goto cleanup;
	}
	if (!piglit_check_egl_error(EGL_BAD_MATCH)) {
		piglit_loge("eglCreateSyncKHR emitted wrong error");
		*result = PIGLIT_FAIL;
		goto cleanup;
	}
	piglit_logi("eglCreateSyncKHR correctly failed with "
		 "EGL_BAD_MATCH");

cleanup:
	if (t2_dpy) {
		eglMakeCurrent(t2_dpy, 0, 0, 0);
		eglTerminate(t2_dpy);
	}
	return result;
}

/**
 * Verify that eglCreateSyncKHR() emits correct error when given a display that
 * does not match the display of the bound context.
 *
 * From the EGL_KHR_fence_sync spec:
 *
 *      * If <type> is EGL_SYNC_FENCE_KHR and <dpy> does not match the
 *        EGLDisplay of the currently bound context for the currently
 *        bound client API (the EGLDisplay returned by
 *        eglGetCurrentDisplay()) then EGL_NO_SYNC_KHR is returned and an
 *        EGL_BAD_MATCH error is generated.
 *
 * This test strives to avoid false passes.  It initializes a second display in
 * a second thread and binds a context there, then verifies that EGL can
 * successfully create and wait on fence syncs in each thread. Then, one thread
 * calls eglCreateSyncKHR, supplying the display bound in the other thread.
 * The verification step reduces the possibility that eglCreateSyncKHR fails
 * for some reason not under test.
 */
static enum piglit_result
test_eglCreateSyncKHR_with_display_bound_in_other_thread(void *test_data)
{
	enum piglit_result result = PIGLIT_PASS;
	enum piglit_result *t2_result = NULL;
	bool orig_print_tid;
	pthread_t thread2;
	int err;

	orig_print_tid = piglit_log_get_opt(PIGLIT_LOG_PRINT_TID);
	piglit_log_set_opt(PIGLIT_LOG_PRINT_TID, true);

	result = test_setup();
	if (result != PIGLIT_PASS) {
		goto cleanup;
	}

	result = check_sync_in_current_context();
	if (result != PIGLIT_PASS) {
		goto cleanup;
	}

	err = pthread_create(
		&thread2, NULL,
		thread2_create_sync_with_display_bound_in_other_thread,
		NULL);
	if (err) {
		piglit_loge("failed to create second thread");
		result = PIGLIT_FAIL;
		goto cleanup;
	}

	err = pthread_join(thread2, (void**) &t2_result);
	if (err) {
		piglit_loge("failed to join thread %"PRIuMAX, (uintmax_t) thread2);
		result = PIGLIT_FAIL;
		goto cleanup;
	}

	if (t2_result) {
		piglit_merge_result(&result, *t2_result);
	} else {
		piglit_loge("thread %"PRIuMAX" returned no piglit_result");
		result = PIGLIT_FAIL;
	}

cleanup:
	free(t2_result);
	piglit_log_set_opt(PIGLIT_LOG_PRINT_TID, orig_print_tid);
	test_cleanup(EGL_NO_SYNC_KHR, &result);
	return result;
}
#endif /*PIGLIT_HAS_PTHREADS*/

/**
 * Verify that eglDestroySyncKHR() emits the correct error when given an
 * invalid sync object.
 *
 * From the EGL_KHR_fence_sync spec:
 *
 *      * If <sync> is not a valid sync object for <dpy>, EGL_FALSE is
 *        returned and an EGL_BAD_PARAMETER error is generated.
 */
static enum piglit_result
test_eglDestroySyncKHR_invalid_sync(void *test_data)
{
	enum piglit_result result = PIGLIT_PASS;
	bool ok = false;
	EGLSyncKHR invalid_sync = (EGLSyncKHR) &canary;

	result = test_setup();
	if (result != PIGLIT_PASS) {
		return result;
	}

	ok = peglDestroySyncKHR(g_dpy, invalid_sync);
	if (ok) {
		piglit_loge("eglDestroySyncKHR() succeeded when given invalid "
			  "sync object");
		result = PIGLIT_FAIL;
	}
	if (!piglit_check_egl_error(EGL_BAD_PARAMETER)) {
		piglit_loge("eglGetSyncAttribKHR(EGL_SYNC_TYPE_KHR) emitted "
			  "an incorrect error");
		result = PIGLIT_FAIL;
	}

	test_cleanup(EGL_NO_SYNC_KHR, &result);
	return result;
}

static const struct piglit_subtest subtests[] = {
	{
		"eglCreateSyncKHR_default_attributes",
		"eglCreateSyncKHR_default_attributes",
		test_eglCreateSyncKHR_default_attributes,
	},
	{
		"eglCreateSyncKHR_invalid_display",
		"eglCreateSyncKHR_invalid_display",
		test_eglCreateSyncKHR_invalid_display,
	},
	{
		"eglCreateSyncKHR_invalid_attrib_list",
		"eglCreateSyncKHR_invalid_attrib_list",
		test_eglCreateSyncKHR_invalid_attrib_list,
	},
	{
		"eglCreateSyncKHR_wrong_display_same_thread",
		"eglCreateSyncKHR_wrong_display_same_thread",
		test_eglCreateSyncKHR_wrong_display_same_thread,
	},
#ifdef PIGLIT_HAS_PTHREADS
	{
		"eglCreateSyncKHR_with_display_bound_in_other_thread",
		"eglCreateSyncKHR_with_display_bound_in_other_thread",
		test_eglCreateSyncKHR_with_display_bound_in_other_thread,
	},
#endif
	{
		"eglCreateSyncKHR_invalid_sync_type",
		"eglCreateSyncKHR_invalid_sync_type",
		test_eglCreateSyncKHR_invalid_sync_type,
	},
	{
		"eglCreateSyncKHR_no_current_context",
		"eglCreateSyncKHR_no_current_context",
		test_eglCreateSyncKHR_no_current_context,
	},
	{
		"eglGetSyncAttribKHR_invalid_sync",
		"eglGetSyncAttribKHR_invalid_sync",
		test_eglGetSyncAttribKHR_invalid_sync,
	},
	{
		"eglGetSyncAttribKHR_invalid_attrib",
		"eglGetSyncAttribKHR_invalid_attrib",
		test_eglGetSyncAttribKHR_invalid_attrib,
	},
	{
		"eglGetSyncAttribKHR_sync_status",
		"eglGetSyncAttribKHR_sync_status",
		test_eglGetSyncAttribKHR_sync_status,
	},
	{
		"eglClientWaitSyncKHR_invalid_flag",
		"eglClientWaitSyncKHR_invalid_flag",
		test_eglClientWaitSyncKHR_invalid_flag,
	},
	{
		"eglClientWaitSyncKHR_zero_timeout",
		"eglClientWaitSyncKHR_zero_timeout",
		test_eglClientWaitSyncKHR_zero_timeout,
	},
	{
		"eglClientWaitSyncKHR_flag_sync_flush",
		"eglClientWaitSyncKHR_flag_sync_flush",
		test_eglClientWaitSyncKHR_flag_sync_flush,
	},
	{
		"eglClientWaitSyncKHR_invalid_sync",
		"eglClientWaitSyncKHR_invalid_sync",
		test_eglClientWaitSyncKHR_invalid_sync,
	},
	{
		"eglClientWaitSyncKHR_nonzero_timeout",
		"eglClientWaitSyncKHR_nonzero_timeout",
		test_eglClientWaitSyncKHR_nonzero_timeout,
	},
	{
		"eglDestroySyncKHR_invalid_sync",
		"eglDestroySyncKHR_invalid_sync",
		test_eglDestroySyncKHR_invalid_sync,
	},
	{0},
};

static void
init_egl_extension_funcs(void)
{
	peglCreateSyncKHR = (void*) eglGetProcAddress("eglCreateSyncKHR");
	peglDestroySyncKHR = (void*) eglGetProcAddress("eglDestroySyncKHR");
	peglClientWaitSyncKHR = (void*) eglGetProcAddress("eglClientWaitSyncKHR");
	peglGetSyncAttribKHR = (void*) eglGetProcAddress("eglGetSyncAttribKHR");
}

static void
parse_args(int *argc, char **argv,
	   const char ***selected_subtests,
	   size_t *num_selected_subtests)
{
	*selected_subtests = NULL;
	*num_selected_subtests = 0;

	prog_name = basename(argv[0]);
	if (*argc == 1) {
		return;
	}

	if (streq(argv[1], "-h") || streq(argv[1], "--help")) {
		print_usage();
		exit(0);
	}

	/* Strip common piglit args. */
	piglit_strip_arg(argc, argv, "-fbo");
	piglit_strip_arg(argc, argv, "-auto");

	piglit_parse_subtest_args(argc, argv, subtests, selected_subtests,
			          num_selected_subtests);

	if (*argc > 1) {
		piglit_loge("unrecognized option: %s", argv[1]);
		usage_error();
	}
}

int
main(int argc, char **argv)
{
	enum piglit_result result = PIGLIT_SKIP;
	const char **selected_subtests = NULL;
	size_t num_selected_subtests = 0;

	parse_args(&argc, argv, &selected_subtests, &num_selected_subtests);
	init_egl_extension_funcs();
	result = piglit_run_selected_subtests(subtests, selected_subtests,
					      num_selected_subtests, result);
	piglit_report_result(result);
	assert(!"unreachable");
	return EXIT_FAILURE;
}
