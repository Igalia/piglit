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

static bool try_get_context_id(GLXContext ctx, enum context_mode mode)
{
	bool pass = true;
	GLXContextID id = glXGetContextIDEXT(ctx);

	XSync(dpy, 0);

	if (mode != invalid && id == None) {
		fprintf(stderr,
			"Could not get context ID for %s context.\n",
			context_mode_name(mode));
		pass = false;
	} else 	if (mode == invalid && id != None) {
		fprintf(stderr,
			"Got a context ID for %s context.\n",
			context_mode_name(mode));
		pass = false;
	}


	/* The glXGetContextIDEXT man page says:
	 *
	 *     "GLXBadContext is generated if ctx does not refer to a valid
	 *     context."
	 *
	 * However, glXGetContextIDEXT doesn't take a Display.  If the context
	 * is invalid and no context is current, it is impossible for
	 * glXGetContextIDEXT to get a Display.  Without a Display, it is
	 * impossible to generate a protocol error!
	 */
	pass = validate_glx_error_code(Success, -1) && pass;

	return pass;
}

int main(int argc, char **argv)
{
	bool pass = true;

	GLX_EXT_import_context_setup();

	pass = try_get_context_id(directCtx, direct_rendering) && pass;
	pass = try_get_context_id(indirectCtx, indirect_rendering) && pass;
	pass = try_get_context_id(NULL, invalid) && pass;

	GLX_EXT_import_context_teardown();

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
	return 0;
}
