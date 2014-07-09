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

static bool try_flag(uint32_t flag)
{
	const int attribs[] = {
		GLX_CONTEXT_FLAGS_ARB, flag,
		None
	};
	GLXContext ctx;
	bool pass = true;

	ctx = glXCreateContextAttribsARB(dpy, fbconfig, NULL, True, attribs);
	XSync(dpy, 0);

	if (ctx != NULL) {
		fprintf(stderr,
			"Created OpenGL context with invalid flag 0x%08x, "
			"but this should have failed.\n",
			flag);
		glXDestroyContext(dpy, ctx);
		pass = false;
	}

	/* The GLX_ARB_create_context spec says:
	 *
	 *     "* If an attribute or attribute value in <attrib_list> is not
	 *        recognized (including unrecognized bits in bitmask
	 *        attributes), BadValue is generated."
	 */
	if (!validate_glx_error_code(BadValue, -1)) {
		if (ctx == NULL)
			fprintf(stderr, "Flag = 0x%08x\n", flag);

		pass = false;
	}

	return pass;
}

int main(int argc, char **argv)
{
	bool pass = true;
	uint32_t flag = 0x80000000;
	uint32_t first_valid_flag = GLX_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB;

	GLX_ARB_create_context_setup();

	/* If GLX_ARB_create_context_robustness is supported, the first flag
	 * that can be valid is GLX_CONTEXT_ROBUST_ACCESS_BIT_ARB.  Otherwise
	 * the first valid flag is GLX_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB.
	 */
	if (piglit_is_glx_extension_supported(dpy,
					      "GLX_ARB_create_context_robustness")) {
		first_valid_flag = GLX_CONTEXT_ROBUST_ACCESS_BIT_ARB;
	}

	while (flag != first_valid_flag) {
		pass = try_flag(flag) && pass;
		flag >>= 1;
	}

	GLX_ARB_create_context_teardown();

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
	return 0;
}
