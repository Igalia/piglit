/* Copyright © 2013 Intel Corporation
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
#include "query-renderer-common.h"

struct test_vector {
	const char *name_string;
	int attribute;
	int value_count;
};

#define ENUM(name, count) { # name, name, count }

static const struct test_vector all_valid_integer_enums[] = {
        ENUM(GLX_RENDERER_VENDOR_ID_MESA, 1),
        ENUM(GLX_RENDERER_DEVICE_ID_MESA, 1),
        ENUM(GLX_RENDERER_VERSION_MESA, 3),
        ENUM(GLX_RENDERER_ACCELERATED_MESA, 1),
        ENUM(GLX_RENDERER_VIDEO_MEMORY_MESA, 1),
        ENUM(GLX_RENDERER_UNIFIED_MEMORY_ARCHITECTURE_MESA, 1),
        ENUM(GLX_RENDERER_PREFERRED_PROFILE_MESA, 1),
        ENUM(GLX_RENDERER_OPENGL_CORE_PROFILE_VERSION_MESA, 2),
        ENUM(GLX_RENDERER_OPENGL_COMPATIBILITY_PROFILE_VERSION_MESA, 2),
        ENUM(GLX_RENDERER_OPENGL_ES_PROFILE_VERSION_MESA, 2),
        ENUM(GLX_RENDERER_OPENGL_ES2_PROFILE_VERSION_MESA, 2),
};

static const struct test_vector all_valid_string_enums[] = {
        ENUM(GLX_RENDERER_VENDOR_ID_MESA, 0),
        ENUM(GLX_RENDERER_DEVICE_ID_MESA, 0),
};

static bool
verify_integer_values(const char *name, Bool success,
		      const struct test_vector *test,
		      const unsigned *buffer, unsigned buffer_size,
		      bool silent)
{
	char text[512];
	unsigned text_size;
	bool pass = true;
	unsigned j;

	if (!success) {
		fprintf(stderr, "%s(%s) failed.\n", name, test->name_string);

		/* If the call failed, don't bother checking that the correct
		 * number of values were written.
		 */
		return false;
	}

	if (!silent) {
		text_size = snprintf(text, sizeof(text), "%s(%s) values:\n    ",
				     name,
				     test->name_string);
		for (j = 0; j < test->value_count; j++) {
			text_size += snprintf(&text[text_size],
					      sizeof(text) - text_size,
					      "%d ",
					      buffer[j]);
		}

		printf("%s\n", text);
	}

	for (j = 0; j < test->value_count; j++) {
		if (buffer[j] == 0xDEADBEEF) {
			fprintf(stderr,
				"%s(%s) only wrote %d values, expected %d.\n",
				name,
				test->name_string,
				j,
				test->value_count);
			pass = false;
			break;
		}
	}

	for (j = test->value_count; j < buffer_size; j++) {
		if (buffer[j] != 0xDEADBEEF) {
			fprintf(stderr,
				"%s(%s) wrote at least %d values, expected "
				"only %d.\n",
				name,
				test->name_string,
				j + 1,
				test->value_count);
			pass = false;
			break;
		}
	}

	return pass;
}

static bool
subtest_QueryRendererInteger(Display *dpy)
{
	static const char subtest_name[] =
		"glXQueryRendererIntegerMESA and "
		"glXQueryCurrentRendererIntegerMESA";

	bool pass = true;
	unsigned i;

	for (i = 0; i < ARRAY_SIZE(all_valid_integer_enums); i++) {
		unsigned buffer_a[16];
		unsigned buffer_b[ARRAY_SIZE(buffer_a)];
		Bool success;
		unsigned j;

		for (j = 0; j < ARRAY_SIZE(buffer_a); j++) {
			buffer_a[j] = 0xDEADBEEF;
			buffer_b[j] = 0xDEADBEEF;
		}

		success =
			glXQueryRendererIntegerMESA(dpy, 0, 0,
						    all_valid_integer_enums[i].attribute,
						    buffer_a);
		pass = verify_integer_values("glXQueryRendererIntegerMESA",
					     success,
					     &all_valid_integer_enums[i],
					     buffer_a,
					     ARRAY_SIZE(buffer_a),
					     false)
			&& pass;

		success =
			glXQueryCurrentRendererIntegerMESA(
						    all_valid_integer_enums[i].attribute,
						    buffer_b);
		pass = verify_integer_values("glXQueryCurrentRendererIntegerMESA",
					     success,
					     &all_valid_integer_enums[i],
					     buffer_b,
					     ARRAY_SIZE(buffer_b),
					     true)
			&& pass;

		for (j = 0; j < all_valid_integer_enums[i].value_count; j++) {
			if (buffer_a[j] != buffer_b[j]) {
				fprintf(stderr,
					"glXQueryRendererIntegerMESA and "
					"glXQueryCurrentRendererIntegerMESA "
					"disagree about %s value %d: "
					"%d != %d\n",
					all_valid_integer_enums[i].name_string,
					j,
					buffer_a[j],
					buffer_b[j]);
				pass = false;
			}
		}
	}

	piglit_report_subtest_result(pass ? PIGLIT_PASS : PIGLIT_FAIL,
				     subtest_name);
	return pass;
}

static bool
subtest_QueryRendererString(Display *dpy)
{
	static const char subtest_name[] =
		"glXQueryRendererStringMESA and "
		"glXQueryCurrentRendererStringMESA";

	bool pass = true;
	unsigned i;

	for (i = 0; i < ARRAY_SIZE(all_valid_string_enums); i++) {
		const char *string_a;
		const char *string_b;

		string_a =
			glXQueryRendererStringMESA(dpy, 0, 0,
						   all_valid_string_enums[i].attribute);
		if (string_a == NULL) {
			fprintf(stderr,
				"glXQueryRendererStringMESA(%s) failed.\n",
				all_valid_string_enums[i].name_string);
			pass = false;
		} else {
			printf("glXQueryRendererStringMESA(%s) value:\n"
			       "    %s\n",
			       all_valid_string_enums[i].name_string,
			       string_a);
		}

		string_b = glXQueryCurrentRendererStringMESA(all_valid_string_enums[i].attribute);
		if (string_b == NULL) {
			fprintf(stderr,
				"glXQueryRendererStringMESA(%s) failed.\n",
				all_valid_string_enums[i].name_string);
			pass = false;
		}

		if (string_a != NULL && string_b != NULL
		    && strcmp(string_a, string_b) != 0) {
			fprintf(stderr,
				"glXQueryRendererIntegerMESA and "
				"glXQueryCurrentRendererIntegerMESA "
				"disagree about %s: %s != %s\n",
				all_valid_string_enums[i].name_string,
				string_a,
				string_b);
			pass = false;
		}
	}

	piglit_report_subtest_result(pass ? PIGLIT_PASS : PIGLIT_FAIL,
				     subtest_name);
	return pass;
}

int main(int argc, char **argv)
{
	bool pass = true;
	Display *dpy = NULL;
	GLXFBConfig fbconfig = None;
	XVisualInfo *visinfo = NULL;
	Window win = None;
	GLXWindow glxWin = None;
	GLXContext ctx;

	dpy = piglit_get_glx_display();

	piglit_require_glx_version(dpy, 1, 4);

	initialize_function_pointers(dpy);

	visinfo = piglit_get_glx_visual(dpy);
	fbconfig = piglit_glx_get_fbconfig_for_visinfo(dpy, visinfo);

	win = piglit_get_glx_window_unmapped(dpy, visinfo);
	glxWin = glXCreateWindow(dpy, fbconfig, win, NULL);

	ctx = glXCreateNewContext(dpy, fbconfig, GLX_RGBA, NULL, True);
	if (ctx == NULL) {
		fprintf(stderr,	"Unable to create OpenGL context!\n");
		piglit_report_result(PIGLIT_FAIL);
	}

        glXMakeContextCurrent(dpy, glxWin, glxWin, ctx);
        piglit_dispatch_default_init(PIGLIT_DISPATCH_GL);

	pass = subtest_QueryRendererInteger(dpy) && pass;
	pass = subtest_QueryRendererString(dpy) && pass;

        glXMakeContextCurrent(dpy, 0, 0, 0);

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
	return 0;
}
