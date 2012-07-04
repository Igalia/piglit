/* Copyright © 2012 Intel Corporation
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

static PFNGLGETSTRINGIPROC GetStringi = NULL;

static bool try_context(int reset_strategy, int flags)
{
	const int attribs[] = {
		GLX_CONTEXT_MAJOR_VERSION_ARB, 3,
		GLX_CONTEXT_MINOR_VERSION_ARB, 0,
		GLX_CONTEXT_RESET_NOTIFICATION_STRATEGY_ARB, reset_strategy,
		GLX_CONTEXT_FLAGS_ARB, flags,
		None
	};
	GLXContext ctx;
	bool pass = true;
	int num_extensions;
	int i = 0;
	bool got_robustness = false;

	ctx = glXCreateContextAttribsARB(dpy, fbconfig, NULL, True, attribs);
	XSync(dpy, 0);

	if (ctx == NULL) {
		fprintf(stderr,
			"Could not create OpenGL 3.0 context.\n"
			"flags = 0x%08x, reset notification strategy = 0x%04x\n",
			flags, reset_strategy);
		return true;
	}

	if (!glXMakeContextCurrent(dpy, glxWin, glxWin, ctx)) {
		fprintf(stderr,
			"Created OpenGL context, but could not make it "
			"current.\n");
		pass = false;
		goto done;
	}

	GetStringi = (PFNGLGETSTRINGIPROC)
		glXGetProcAddress((const GLubyte *) "glGetStringi");
	if (GetStringi == NULL) {
		fprintf(stderr,
			"Created OpenGL 3.0+ context, but could not "
			"get glGetStringi function.\n");
		pass = false;
		goto done;
	}

	glGetIntegerv(GL_NUM_EXTENSIONS, &num_extensions);
	for (i = 0; i < num_extensions; i++) {
		const char *ext = (const char *) GetStringi(GL_EXTENSIONS, i);

		if (strcmp(ext, "GL_ARB_robustness") == 0) {
			got_robustness = true;
			break;
		}
	}

	if (!got_robustness) {
		fprintf(stderr,
			"GL context does not support GL_ARB_robustness "
			"extension.\n"
			"flags = 0x%08x, reset notification strategy = 0x%04x\n",
			flags, reset_strategy);
		pass = false;
	}

done:
	glXMakeContextCurrent(dpy, None, None, NULL);
	glXDestroyContext(dpy, ctx);
	return pass;
}

int main(int argc, char **argv)
{
	bool pass = true;

	GLX_ARB_create_context_setup();
	piglit_require_glx_extension(dpy, "GLX_ARB_create_context_robustness");

	/* The GLX_ARB_create_context_robustness spec says:
	 *
	 *     "If the GLX_CONTEXT_ROBUST_ACCESS_BIT_ARB bit is set in
	 *     GLX_CONTEXT_FLAGS_ARB, then a context supporting <robust buffer
	 *     access> will be created. Robust buffer access is defined in the
	 *     GL_ARB_robustness extension specification, and the resulting
	 *     context must also support either the GL_ARB_robustness
	 *     extension, or a version of OpenGL incorporating equivalent
	 *     functionality."
	 *
	 * It also says:
	 *
	 *     "The attribute name GLX_CONTEXT_RESET_NOTIFICATION_STRATEGY_ARB
	 *     specifies the <reset notification behavior> of the rendering
	 *     context. Reset notification behavior is defined in the
	 *     GL_ARB_robustness extension specification, and the resulting
	 *     context must also support either the GL_ARB_robustness
	 *     extension, or a version of OpenGL incorporating equivalent
	 *     functionality."
	 */
	pass = try_context(GLX_NO_RESET_NOTIFICATION_ARB,
			   GLX_CONTEXT_ROBUST_ACCESS_BIT_ARB)
		&& pass;
	pass = try_context(GLX_LOSE_CONTEXT_ON_RESET_ARB,
			   0)
		&& pass;
	pass = try_context(GLX_LOSE_CONTEXT_ON_RESET_ARB,
			   GLX_CONTEXT_ROBUST_ACCESS_BIT_ARB)
		&& pass;

	GLX_ARB_create_context_teardown();

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
	return 0;
}
