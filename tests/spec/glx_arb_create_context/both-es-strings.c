/*
 * Copyright 2015 Red Hat, Inc.
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
#include "piglit-util.h"
#include "piglit-glx-util.h"
#include "common.h"

int main(int argc, char **argv)
{
	bool has_es, has_es2;

	GLX_ARB_create_context_setup();
	has_es = piglit_is_glx_extension_supported(dpy, "GLX_EXT_create_context_es_profile");
	has_es2 = piglit_is_glx_extension_supported(dpy, "GLX_EXT_create_context_es2_profile");
	GLX_ARB_create_context_teardown();

	/* The GLX_EXT_create_context_es2_profile spec (as of v4) says:
	 *
	 *  "NOTE: implementations of this extension must export BOTH extension
	 *   strings, for backwards compatibility with applications written
	 *   against version 1 of this extension."
	 */

	if (!has_es && !has_es2)
	    piglit_report_result(PIGLIT_SKIP);
	else if (has_es && has_es2)
	    piglit_report_result(PIGLIT_PASS);
	else
	    piglit_report_result(PIGLIT_FAIL);

	return 0;
}
