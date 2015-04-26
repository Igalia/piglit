/*
 * Copyright 2012 Blaž Tomažič <blaz.tomazic@gmail.com>
 * Copyright 2014 Intel Corporation
 * Copyright 2015 Advanced Micro Devices, Inc.
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

#include "piglit-framework-cl-api.h"
#include "piglit-util-egl.h"
#include "piglit-util-gl.h"


PIGLIT_CL_API_TEST_CONFIG_BEGIN

	config.name = "EGL_KHR_cl_event2";
	config.version_min = 10;

	config.run_per_device = true;
	config.create_context = true;

PIGLIT_CL_API_TEST_CONFIG_END

/* Extension function pointers.
 *
 * Use prefix 'pegl' (piglit egl) instead of 'egl' to avoid collisions with
 * prototypes in eglext.h. */
EGLSyncKHR (*peglCreateSyncKHR)(EGLDisplay dpy, EGLenum type, const EGLint *attrib_list);
EGLSyncKHR (*peglCreateSync64KHR)(EGLDisplay dpy, EGLenum type, const EGLAttribKHR *attrib_list);
EGLBoolean (*peglDestroySyncKHR)(EGLDisplay dpy, EGLSyncKHR sync);
EGLBoolean (*peglGetSyncAttribKHR)(EGLDisplay dpy, EGLSyncKHR sync, EGLint attribute, EGLint *value);

static const EGLint canary = 0x31415926;
static EGLDisplay g_dpy = 0;
static EGLContext g_ctx = 0;

static void
init_egl_extension_funcs(void)
{
	peglCreateSyncKHR = (void*) eglGetProcAddress("eglCreateSyncKHR");
	peglCreateSync64KHR = (void*) eglGetProcAddress("eglCreateSync64KHR");
	peglDestroySyncKHR = (void*) eglGetProcAddress("eglDestroySyncKHR");
	peglGetSyncAttribKHR = (void*) eglGetProcAddress("eglGetSyncAttribKHR");
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
 * Verify that eglCreateSync64KHR can create a sync object from a cl_event.
 *
 * From the EGL_KHR_cl_event2 spec:
 *
 *     Attributes of the fence sync object are
 *     set as follows:
 *
 * 	Attribute Name          Initial Attribute Value(s)
 *      -------------           --------------------------
 *      EGL_SYNC_TYPE_KHR       EGL_SYNC_CL_EVENT_KHR
 *      EGL_SYNC_STATUS_KHR     Depends on status of <event>
 *      EGL_SYNC_CONDITION_KHR  EGL_SYNC_CL_EVENT_COMPLETE_KHR
 */
static enum piglit_result
test_EGL_KHR_cl_event2(cl_event event)
{
	enum piglit_result result = PIGLIT_PASS;
	EGLSyncKHR sync = 0;
	EGLint sync_type = canary;
	EGLint sync_status = canary;
	EGLint sync_condition = canary;
	bool ok = false;
	EGLint attribs[] = {EGL_CL_EVENT_HANDLE_KHR, (EGLint)(intptr_t)event, EGL_NONE};
	EGLAttribKHR attribs64[] = {EGL_CL_EVENT_HANDLE_KHR, (EGLAttribKHR)(intptr_t)event, EGL_NONE};
	cl_int cl_err, cl_status;

	result = test_setup();
	if (result != PIGLIT_PASS) {
		return result;
	}

	piglit_require_egl_extension(g_dpy, "EGL_KHR_fence_sync");
	piglit_require_egl_extension(g_dpy, "EGL_KHR_cl_event2");
	init_egl_extension_funcs();

	sync = peglCreateSyncKHR(g_dpy, EGL_SYNC_CL_EVENT_KHR, NULL);
	if (sync != EGL_NO_SYNC_KHR) {
		piglit_loge("eglCreateSyncKHR(EGL_SYNC_CL_EVENT_KHR) should have failed (1)");
		result = PIGLIT_FAIL;
		goto cleanup;
	}

	if (piglit_is_egl_extension_supported(g_dpy, "EGL_KHR_cl_event")) {
		/* The older version of the extension allows using the non-64 version. */
		sync = peglCreateSyncKHR(g_dpy, EGL_SYNC_CL_EVENT_KHR, attribs);
		if (sync == EGL_NO_SYNC_KHR) {
			piglit_loge("eglCreateSyncKHR(EGL_SYNC_CL_EVENT_KHR) failed (2)");
			result = PIGLIT_FAIL;
			goto cleanup;
		}
		peglDestroySyncKHR(g_dpy, sync);
	}
	else {
		sync = peglCreateSyncKHR(g_dpy, EGL_SYNC_CL_EVENT_KHR, attribs);
		if (sync != EGL_NO_SYNC_KHR) {
			piglit_loge("eglCreateSyncKHR(EGL_SYNC_CL_EVENT_KHR) should have failed (2)");
			result = PIGLIT_FAIL;
			goto cleanup;
		}
	}

	sync = peglCreateSync64KHR(g_dpy, EGL_SYNC_CL_EVENT_KHR, NULL);
	if (sync != EGL_NO_SYNC_KHR) {
		piglit_loge("eglCreateSync64KHR(EGL_SYNC_CL_EVENT_KHR) should have failed (3)");
		result = PIGLIT_FAIL;
		goto cleanup;
	}

	sync = peglCreateSync64KHR(g_dpy, EGL_SYNC_CL_EVENT_KHR, attribs64);
	if (sync == EGL_NO_SYNC_KHR) {
		piglit_loge("eglCreateSync64KHR(EGL_SYNC_CL_EVENT_KHR) failed (4)");
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
	if (sync_type != EGL_SYNC_CL_EVENT_KHR) {
		piglit_loge("eglGetSyncAttribKHR(EGL_SYNC_TYPE_KHR) returned "
			  "0x%x but expected EGL_SYNC_CL_EVENT_KHR(0x%x)",
			  sync_type, EGL_SYNC_CL_EVENT_KHR);
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
	if (sync_condition != EGL_SYNC_CL_EVENT_COMPLETE_KHR) {
		piglit_loge("eglGetSyncAttribKHR(EGL_SYNC_CONDITION_KHR) "
			  "returned 0x%x but expected "
			  "EGL_SYNC_CL_EVENT_COMPLETE_KHR(0x%x)",
			  sync_condition, EGL_SYNC_CL_EVENT_COMPLETE_KHR);
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

	cl_err = clGetEventInfo(event, CL_EVENT_COMMAND_EXECUTION_STATUS,
				sizeof(cl_int), &cl_status, NULL);
	if (cl_err != CL_SUCCESS) {
		piglit_loge("clGetEventInfo(CL_EVENT_COMMAND_EXECUTION_STATUS) failed");
		result = PIGLIT_FAIL;
	}
	if ((sync_status == EGL_SIGNALED_KHR) != (cl_status == CL_COMPLETE)) {
		piglit_loge("CL_EVENT_COMMAND_EXECUTION_STATUS and "
			    "EGL_SYNC_STATUS_KHR don't match");
		result = PIGLIT_FAIL;
	}

cleanup:
	test_cleanup(sync, &result);
	return result;
}

enum piglit_result
piglit_cl_test(const int argc,
               const char** argv,
               const struct piglit_cl_api_test_config* config,
               const struct piglit_cl_api_test_env* env)
{
	cl_int errNo;
	cl_mem memobj;
	cl_event event;
	unsigned char buffer[1];
	enum piglit_result result;

	/*** Normal usage ***/

	memobj = clCreateBuffer(env->context->cl_ctx,
	                        CL_MEM_READ_WRITE,
	                        512,
	                        NULL,
	                        &errNo);
	if(!piglit_cl_check_error(errNo, CL_SUCCESS)) {
		fprintf(stderr,
		        "Failed (error code: %s): Create buffer.\n",
		        piglit_cl_get_error_name(errNo));
		return PIGLIT_FAIL;
	}

	errNo = clEnqueueReadBuffer(env->context->command_queues[0], memobj, true,
	                            0, 1, buffer, 0, NULL, &event);
	if(!piglit_cl_check_error(errNo, CL_SUCCESS)) {
		fprintf(stderr,
		        "Failed (error code: %s): Create event by enqueueing buffer read.\n",
		        piglit_cl_get_error_name(errNo));
		return PIGLIT_FAIL;
	}

	result = test_EGL_KHR_cl_event2(event);

	errNo = clReleaseEvent(event);
	if(!piglit_cl_check_error(errNo, CL_SUCCESS)){
		fprintf(stderr,
			"clReleaseEvent: Failed (error code: %s): Release event.\n",
			piglit_cl_get_error_name(errNo));
		return PIGLIT_FAIL;
	}

	return result;
}
