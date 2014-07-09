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

static bool try_free_context(GLXContext ctx, enum context_mode mode,
			     const char *extra)
{
	bool pass;
	const GLXContextID id =
		(mode != invalid) ? glXGetContextIDEXT(ctx) : None;

	/* Free the context.
	 */
	glXFreeContextEXT(dpy, ctx);
	XSync(dpy, 0);

	pass = validate_glx_error_code(Success, -1);
	if (!pass)
		fprintf(stderr, "Context is %s in free\n\n", extra);

	/* The GLX_EXT_import_context spec says:
	 *
	 *     "glXFreeContext does not free the server-side context
	 *     information or the XID associated with the server-side
	 *     context."
	 *
	 * Attempt to verify that the context still exists on the server by
	 * trying to import it again.
	 */
	if (mode != invalid && pass) {
		assert(id != None);

		pass = try_import_context(id, mode);
		if (!pass)
			fprintf(stderr, "Context is %s in re-import\n\n",
				extra);
	}


	return pass;
}

int main(int argc, char **argv)
{
	bool pass = true;
	GLXContext ctx;

	GLX_EXT_import_context_setup();
	get_context_IDs();

	ctx = glXImportContextEXT(dpy, indirectID);
	if (ctx == NULL) {
		fprintf(stderr, "Could not import indirect context.\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	pass = try_free_context(ctx, indirect_rendering,
				"an imported context")
		&& pass;
	ctx = NULL;

	if (directCtx != NULL) {
		pass = try_free_context(directCtx, direct_rendering,
					"a non-imported direct-rendering "
					"context")
			&& pass;
		directCtx = NULL;
	}

	pass = try_free_context(indirectCtx, indirect_rendering,
				"a non-imported indirect-rendering context")
		&& pass;
	indirectCtx = NULL;

	pass = try_free_context(NULL, invalid,
				"a NULL pointer / invalid context")
		&& pass;

	GLX_EXT_import_context_teardown();

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
	return 0;
}
