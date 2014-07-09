/*
 * Copyright © 2013 VMware, Inc.
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

/**
 * Simply query and print various glGetString() values.
 * This is helpful when running a complete piglit run since the
 * results file will have all the pertinant info for the GL driver
 * that was tested.
 *
 * Note that the piglit framework tries to run glxinfo/wglinfo and
 * put the output in the results file, but sometimes those programs
 * aren't installed.
 *
 * Brian Paul
 * April 2013
 */


#include "piglit-util-gl.h"


PIGLIT_GL_TEST_CONFIG_BEGIN
	config.supports_gl_compat_version = 10;
	config.window_visual = PIGLIT_GL_VISUAL_RGB;
PIGLIT_GL_TEST_CONFIG_END


enum piglit_result
piglit_display(void)
{
	return PIGLIT_PASS;
}


void
piglit_init(int argc, char **argv)
{
	const char *renderer = (const char *) glGetString(GL_RENDERER);
	const char *version = (const char *) glGetString(GL_VERSION);
	const char *vendor = (const char *) glGetString(GL_VENDOR);

	printf("GL_RENDERER = %s\n", renderer);
	printf("GL_VERSION = %s\n", version);
	printf("GL_VENDOR = %s\n", vendor);

	if (version[0] >= '2') {
		printf("GL_SHADING_LANGUAGE_VERSION = %s\n", (const char *)
		       glGetString(GL_SHADING_LANGUAGE_VERSION));
	}

	printf("Extensions:\n");
	if (version[0] >= '3') {
		GLint numExt, i;
		glGetIntegerv(GL_NUM_EXTENSIONS, &numExt);
		for (i = 0; i < numExt; i++) {
			printf("%s\n", (const char *)
			       glGetStringi(GL_EXTENSIONS, i));
		}
	}
	else {
		const char *ext = (const char *) glGetString(GL_EXTENSIONS);
		const char *c = ext;
		for (c = ext; *c; c++) {
			if (*c == ' ')
				putchar('\n');
			else
				putchar(*c);
		}
	}

	piglit_report_result(PIGLIT_PASS);
}
