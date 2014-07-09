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

static bool try_attribute(int attribute)
{
	/* If the attribute is GLX_CONTEXT_CORE_PROFILE_BIT_ARB, use a valid
	 * value for that attribute.  This ensures that the attribute is
	 * rejected for the correct reason.
	 */
	const int attribs[] = {
		attribute,
		(attribute == GLX_CONTEXT_PROFILE_MASK_ARB)
		? GLX_CONTEXT_CORE_PROFILE_BIT_ARB : 0,
		None
	};
	GLXContext ctx;
	bool pass = true;

	ctx = glXCreateContextAttribsARB(dpy, fbconfig, NULL, True, attribs);
	XSync(dpy, 0);

	if (ctx != NULL) {
		fprintf(stderr,
			"Created OpenGL context with invalud attribute "
			"0x%08x, but this should have failed.\n",
			attribute);
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
		fprintf(stderr, "Attribute = 0x%08x\n", attribute);
		pass = false;
	}

	return pass;
}

int main(int argc, char **argv)
{
	static const int bad_attributes[] = {
		0xffff0000,
		GLX_SAMPLE_BUFFERS
	};
	bool pass = true;
	unsigned i;

	GLX_ARB_create_context_setup();

	for (i = 0; i < ARRAY_SIZE(bad_attributes); i++) {
		pass = try_attribute(bad_attributes[i]) && pass;
	}

	/* The GLX_ARB_create_context spec says:
	 *
	 *     "If GLX_ARB_create_context_profile is not supported, then the
	 *     GLX_CONTEXT_PROFILE_MASK_ARB attribute [is] not defined, and
	 *     specifying the attribute in <attribList> attribute will
	 *     generate BadValue."
	 */
	if (!piglit_is_glx_extension_supported(dpy, "GLX_ARB_create_context_profile")) {
		pass = try_attribute(GLX_CONTEXT_PROFILE_MASK_ARB)
			&& pass;
	}

	GLX_ARB_create_context_teardown();

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
	return 0;
}
