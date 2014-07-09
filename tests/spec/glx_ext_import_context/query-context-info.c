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

#include "piglit-util-gl.h"
#include "piglit-glx-util.h"
#include "common.h"

static bool
try_query(GLXContext ctx, int attribute, const char *attribute_string,
	  int expected_error, int expected_value)
{
	bool pass = true;
	int value = 0xDEADBEEF;
	int err;

	err = glXQueryContextInfoEXT(dpy, ctx, attribute, &value);
	XSync(dpy, 0);

	if (err != expected_error) {
		fprintf(stderr,
			"Query of %s had error %d, but %d was expected.\n",
			attribute_string,
			err,
			expected_error);
		pass = false;
	}

	/* There is no way in GLX_SGIX_fbconfig to get an XID from a
	 * GLXFBConfig.  As a result, there's no way to verify
	 * glXQueryContextInfoEXT has returned the correct value.  The
	 * required functionality was not added until GLX 1.3.
	 */
	if (attribute != GLX_FBCONFIG_ID_SGIX) {
		if (expected_error != Success)
			expected_value = 0xDEADBEEF;

		if (value != expected_value) {
			fprintf(stderr,
				"Query of %s had value %d, but %d was "
				"expected.\n",
				attribute_string,
				value,
				expected_value);
			pass = false;
		}
	} else {
		if (value == 0xDEADBEEF) {
			fprintf(stderr,
				"Query of %s did not set a value.\n",
				attribute_string);
			pass = false;
		}
	}

	/* No GLX protocol error should be generated.
	 */
	pass = validate_glx_error_code(Success, -1) && pass;

	return pass;
}

int main(int argc, char **argv)
{
	bool pass = true;
	GLXContext ctx;

	GLX_EXT_import_context_setup();

	/* Try the simple stuff.
	 */
	pass = try_query(indirectCtx,
			 GLX_SHARE_CONTEXT_EXT, "GLX_SHARE_CONTEXT_EXT",
			 Success, 0)
		&& pass;
	pass = try_query(indirectCtx,
			 GLX_SCREEN_EXT, "GLX_SCREEN_EXT",
			 Success, DefaultScreen(dpy))
		&& pass;
	pass = try_query(indirectCtx,
			 GLX_VISUAL_ID_EXT, "GLX_VISUAL_ID_EXT",
			 Success, visinfo->visualid)
		&& pass;
	pass = try_query(indirectCtx,
			 0xffff0000, "0xffff0000 (invalid)",
			 GLX_BAD_ATTRIBUTE, 0)
		&& pass;

	/* Create a second indirect-rendering context, and have it share the
	 * first indirect-rendering context.  The XID of the share conext for
	 * the original context should be unchanged.
	 */
	ctx = glXCreateContext(dpy, visinfo, indirectCtx, False);
	assert(ctx != NULL);

	pass = try_query(indirectCtx,
			 GLX_SHARE_CONTEXT_EXT, "GLX_SHARE_CONTEXT_EXT",
			 Success, 0)
		&& pass;
	pass = try_query(ctx,
			 GLX_SHARE_CONTEXT_EXT, "GLX_SHARE_CONTEXT_EXT",
			 Success, glXGetContextIDEXT(indirectCtx))
		&& pass;

	if (piglit_is_glx_extension_supported(dpy, "GLX_SGIX_fbconfig")) {
		pass = try_query(indirectCtx,
				 GLX_FBCONFIG_ID_SGIX, "GLX_FBCONFIG_ID_SGIX",
				 Success, 0)
			&& pass;
	}

	GLX_EXT_import_context_teardown();

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
	return 0;
}
