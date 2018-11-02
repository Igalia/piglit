/*
 * Copyright © 2018 Intel Corporation
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
 *
 */

/**
 * \file idtech-extension-strings.c
 * Verify extensions used by idTech2 and idTech3 games occur in the first 2k
 *
 * For a long time idTech2- and idTech3-based games contained a bug in
 * extension string handling.  The engine would copy the extension string
 * returned by the driver into a buffer on the stack.  The engine would not be
 * able to detect the existence of any extensions that occured after the size
 * of the buffer.
 *
 * A 2011 Wine bug
 * (https://www.winehq.org/pipermail/wine-bugs/2011-June/280463.html) suggests
 * that the limit for at least Return to Castle Wofenstein is 4k.  Some other
 * evidence suggests that other games may have limits as low as 2k.
 *
 * Based on this evidence, my guess is that the buffer is 2k, but extension
 * strings longer than 4k caused enough of a stack over run to lead to a
 * crash.
 *
 * There are separate subtests for each game that has a different set of
 * extension strings.  This acts as a catalog of sorts for which games use
 * which extensions.  It also makes it clear which games have been tested.
 */
#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_RGB;

PIGLIT_GL_TEST_CONFIG_END

enum piglit_result
piglit_display(void)
{
	/* UNREACHABLE */
	return PIGLIT_FAIL;
}

/* List of extensions scraped from the Quake3 demo found in
 * linuxq3ademo-1.11-6.x86.gz.sh.  The Return to Castle Wolfenstein demo found
 * in wolfspdemo-linux-1.1b.x86.run had the same list.
 */
const char *const q3demo_list[] = {
	"GL_S3_s3tc",
	"GL_EXT_texture_env_add",
	"GL_ARB_multitexture",
	"GL_EXT_compiled_vertex_array",
};

/* List of extensions used by the game "Star Trek Voyager" provided by
 * Federico Dossena.
 */
const char *const star_trek_voyager_list[] = {
	"GL_S3_s3tc",
	"GL_EXT_texture_compression_s3tc",
	"GL_EXT_texture_env_add",
	"GL_EXT_texture_filter_anisotropic",
	"GL_EXT_texture_edge_clamp",
	"GL_ARB_multitexture",
	"GL_EXT_compiled_vertex_array",

	/* GL_ARB_texture_compression wasn't listed in the output of the
	 * application, but since GL_EXT_texture_compression_s3tc is layered
	 * on top of it, it really should check for it too...
	 */
	"GL_ARB_texture_compression",
};

static bool
check_extension_list(const char *application_name,
		     const char *extension_string,
		     const char *const list[],
		     unsigned num_extensions)
{
	bool pass = true;

	for (unsigned i = 0; i < num_extensions; i++) {
		const unsigned len = strlen(list[i]);
		const char *ptr = strstr(extension_string, list[i]);

		while (ptr != NULL && ptr[len] != '\0' && ptr[len] != ' ')
			ptr = strstr(ptr + len, list[i]);

		if (ptr == NULL) {
			if (!piglit_automatic) {
				printf("Extension %s is not supported.\n",
				       list[i]);
			}

			continue;
		}

		const ptrdiff_t offset = (ptr - extension_string) + len;
		if (offset >= 2048) {
			printf("Extension %s is at offset %ld.  Too far!\n",
			       list[i], (long) offset);
			pass = false;
		} else if (!piglit_automatic) {
			printf("Extension %s is at offset %ld.\n",
			       list[i], (long) offset);
		}
	}

	piglit_report_subtest_result(pass ? PIGLIT_PASS : PIGLIT_FAIL,
				     "%s", application_name);
	return pass;
}

void
piglit_init(int argc, char **argv)
{
	bool pass = true;
	const char *const extension_string =
		(const char *) glGetString(GL_EXTENSIONS);

	pass = check_extension_list("linuxq3ademo-1.11-6.x86.gz.sh",
				    extension_string,
				    q3demo_list,
				    ARRAY_SIZE(q3demo_list))
		&& pass;

	pass = check_extension_list("Star Trek: Voyager - Elite Force",
				    extension_string,
				    star_trek_voyager_list,
				    ARRAY_SIZE(star_trek_voyager_list))
		&& pass;

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}
