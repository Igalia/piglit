/* Copyright © 2011 Intel Corporation
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

#include <assert.h>
#include "piglit-util-gl.h"
#include "piglit-glx-util.h"
#include "common.h"

PFNGLXGETCURRENTDISPLAYEXTPROC __piglit_glXGetCurrentDisplayEXT = NULL;
PFNGLXQUERYCONTEXTINFOEXTPROC __piglit_glXQueryContextInfoEXT = NULL;
PFNGLXGETCONTEXTIDEXTPROC __piglit_glXGetContextIDEXT = NULL;
PFNGLXIMPORTCONTEXTEXTPROC __piglit_glXImportContextEXT = NULL;
PFNGLXFREECONTEXTEXTPROC __piglit_glXFreeContextEXT = NULL;

#define ADD_FUNC(name) PIGLIT_GLX_PROC(__piglit_##name, name)
static const struct piglit_glx_proc_reference procedures[] = {
	ADD_FUNC(glXGetCurrentDisplayEXT),
	ADD_FUNC(glXQueryContextInfoEXT),
	ADD_FUNC(glXGetContextIDEXT),
	ADD_FUNC(glXImportContextEXT),
	ADD_FUNC(glXFreeContextEXT),
};

Display *dpy = NULL;
XVisualInfo *visinfo = NULL;
GLXContext directCtx = NULL;
GLXContextID directID = None;
GLXContext indirectCtx = NULL;
GLXContextID indirectID = None;

int glx_error_code = -1;
int x_error_code = Success;

int piglit_height = 50;
int piglit_width = 50;

static int (*old_handler)(Display *, XErrorEvent *);

static int x_error_handler(Display *dpy, XErrorEvent *e)
{

	x_error_code = e->error_code;
	glx_error_code = piglit_glx_get_error(dpy, e);

	return 0;
}

void GLX_EXT_import_context_setup_for_child(void)
{
	dpy = piglit_get_glx_display();
	old_handler = XSetErrorHandler(x_error_handler);
}

void GLX_EXT_import_context_setup(void)
{
	const char *vendor;

	dpy = piglit_get_glx_display();

	/* NVIDIA incorrectly only list the extension in the client
	 * extenstions list.  If the extension is available for applications
	 * to use, it is supposed to be included in the list returned by
	 * glXQueryExtensionsString.
	 *
	 * The glXImportContextEXT manual page is somewhat clear on this
	 * topic:
	 *
	 *     "If _glxextstring(EXT_import_context) is included in the string
	 *     returned by glXQueryExtensionsString, when called with argument
	 *     GLX_EXTENSIONS, extension EXT_import_context is supported."
	 *
	 * The text is a little weird because the only parameters to
	 * glXQueryExtensionsString are the display and the screen.
	 */
	vendor = glXGetClientString(dpy, GLX_VENDOR);
	if (strcmp("NVIDIA Corporation", vendor) == 0) {
		const char *const client_extensions =
			glXGetClientString(dpy, GLX_EXTENSIONS);

		if (!piglit_is_extension_in_string(client_extensions,
						   "GLX_EXT_import_context")) {
			fprintf(stderr,
				"Test requires GLX_EXT_import_context.\n");
			piglit_report_result(PIGLIT_SKIP);
		}
	} else {
		piglit_require_glx_extension(dpy, "GLX_EXT_import_context");
	}

	piglit_glx_get_all_proc_addresses(procedures, ARRAY_SIZE(procedures));

	visinfo = piglit_get_glx_visual(dpy);

	directCtx = glXCreateContext(dpy, visinfo, NULL, True);
	if (directCtx == NULL) {
		fprintf(stderr,
			"Could not create initial direct-rendering context.\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	if (!glXIsDirect(dpy, directCtx)) {
		glXDestroyContext(dpy, directCtx);
		directCtx = NULL;
	}

	indirectCtx = glXCreateContext(dpy, visinfo, NULL, False);
	if (indirectCtx == NULL) {
		fprintf(stderr,
			"Could not create initial indirect-rendering "
			"context.\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	piglit_glx_get_error(dpy, NULL);
	old_handler = XSetErrorHandler(x_error_handler);
}

void GLX_EXT_import_context_teardown(void)
{
	if (directCtx != NULL) {
		glXDestroyContext(dpy, directCtx);
		directCtx = NULL;
	}

	if (indirectCtx != NULL) {
		glXDestroyContext(dpy, indirectCtx);
		indirectCtx = NULL;
	}

	XFree(visinfo);
	visinfo = NULL;

	XSetErrorHandler(old_handler);
}

void get_context_IDs(void)
{
	directID = None;
	if (directCtx != NULL) {
		directID =  glXGetContextIDEXT(directCtx);
	}

	indirectID = None;
	if (indirectCtx != NULL) {
		indirectID =  glXGetContextIDEXT(indirectCtx);
	}
}

const char *context_mode_name(enum context_mode mode)
{
	switch (mode) {
	case direct_rendering:
		return "direct-rendering";
	case indirect_rendering:
		return "indirect-rendering";
	case invalid:
		return "invalid";
	default:
		assert(!"Should not get here.");
		piglit_report_result(PIGLIT_FAIL);
	}

	return "";
}

bool try_import_context(GLXContextID id, enum context_mode mode)
{
	const int expected_glx_error = (mode == invalid) ? GLXBadContext : -1;
	bool pass = true;
	GLXContext ctx = glXImportContextEXT(dpy, id);

	XSync(dpy, 0);

	switch (mode) {
	case direct_rendering:
		if (ctx != NULL) {
			fprintf(stderr,
				"Could import direct-rendering context, "
				"but should have failed.\n");
			pass = false;
		}
		break;

	case indirect_rendering:
		if (ctx == NULL) {
			fprintf(stderr,
				"Could not import indirect-rendering context, "
				"but should have succeeded.\n");
			pass = false;
		}
		break;

	case invalid:
		if (ctx != NULL) {
			fprintf(stderr,
				"Could import invalid context (0x%08x), "
				"but should have failed.\n",
				(int) id);
			pass = false;
		}
		break;
	}

	assert(mode != invalid || expected_glx_error != -1);
	pass = validate_glx_error_code(Success, expected_glx_error) && pass;

	if (!pass)
		fprintf(stderr, "Context ID = 0x%08x.\n", (int) id);

	if (ctx != NULL)
		glXFreeContextEXT(dpy, ctx);

	return pass;
}

bool validate_glx_error_code(int expected_x_error, int expected_glx_error)
{
	bool pass = true;

	if (expected_glx_error == -1
	    && expected_x_error == Success
	    && (glx_error_code != -1 || x_error_code != Success)) {
		fprintf(stderr,
			"X error %d (%s (%d)) was generated, but "
			"no error was expected.\n",
			x_error_code,
			piglit_glx_error_string(glx_error_code),
			glx_error_code);
		pass = false;
	}

	if (expected_glx_error != -1
	    && glx_error_code != expected_glx_error) {
		fprintf(stderr,
			"X error %d (%s (%d)) was generated, but "
			"%s (%d) was expected.\n",
			x_error_code,
			piglit_glx_error_string(glx_error_code),
			glx_error_code,
			piglit_glx_error_string(expected_glx_error),
			expected_glx_error);
		pass = false;
	} else if (expected_x_error != Success
		   && x_error_code != expected_x_error) {
		fprintf(stderr,
			"X error %d (%s (%d)) was generated, but "
			"X error %d was expected.\n",
			x_error_code,
			piglit_glx_error_string(glx_error_code),
			glx_error_code,
			expected_x_error);
		pass = false;
	}

	x_error_code = Success;
	glx_error_code = -1;
	return pass;
}
