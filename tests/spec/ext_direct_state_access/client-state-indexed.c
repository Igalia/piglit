/*
 * Copyright © 2019 Advanced Micro Devices, Inc.
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

static int
parse_gl_version(int argc, char** argv)
{
	int version;
	if (argc < 2) {
		piglit_loge("Usage: %s 12|30\n", argv[0]);
		exit(1);
	}
	version = atoi(argv[1]);
	if (version != 12 && version != 30) {
		piglit_loge("Usage: %s 12|30\n", argv[0]);
		exit(1);
	}
	return version;
}

static int gl_compat_version;

PIGLIT_GL_TEST_CONFIG_BEGIN

	gl_compat_version = parse_gl_version(argc, argv);
	config.supports_gl_compat_version = gl_compat_version;
	config.khr_no_error_support = PIGLIT_HAS_ERRORS;

PIGLIT_GL_TEST_CONFIG_END

void
piglit_init(int argc, char **argv)
{
	static const GLenum invalid_caps[] = {
		GL_COLOR_ARRAY, GL_EDGE_FLAG_ARRAY, GL_FOG_COORD_ARRAY,
		GL_INDEX_ARRAY, GL_NORMAL_ARRAY, GL_SECONDARY_COLOR_ARRAY,
		GL_VERTEX_ARRAY
	};
	GLint max_textures;
	int i;
	bool pass = true;

	piglit_require_extension("GL_EXT_direct_state_access");

	glGetIntegerv(GL_MAX_TEXTURE_COORDS, &max_textures);

	for (i = 0; i < max_textures && pass; i++) {
		bool value = rand() % 2;
		if (value) {
			/* The GL_EXT_direct_state_access spec says:
			 *
			 *    Add OpenGL 3.0-style aliases for the version 1.0 commands
			 *    and queries that have "Indexed" in the name.  OpenGL 3.0 has a
			 *    convention where an "i" indexed indexed commands and queries. [...]
			 *    Likewise glEnableClientStateIndexedEXT and glEnableClientStateiEXT
			 *    are identical commands.
			 *
			 */
			if (gl_compat_version == 12) {
				glEnableClientStateIndexedEXT(GL_TEXTURE_COORD_ARRAY, i);
			} else {
				glEnableClientStateiEXT(GL_TEXTURE_COORD_ARRAY, i);
			}
		} else {
			if (gl_compat_version == 12) {
				glDisableClientStateIndexedEXT(GL_TEXTURE_COORD_ARRAY, i);
			} else {
				glDisableClientStateiEXT(GL_TEXTURE_COORD_ARRAY, i);
			}
		}

		/* Verify the state */
		glClientActiveTexture(GL_TEXTURE0 + i);
		if (!piglit_check_gl_error(GL_NO_ERROR) || glIsEnabled(GL_TEXTURE_COORD_ARRAY) != value) {
			piglit_loge("%s(GL_TEXTURE_COORD_ARRAY, GL_TEXTURE%d) failed\n",
				value ? "glEnableClientStateiEXT" : "glDisableClientStateiEXT",
				i);
			pass = false;
		}
	}

	/* The GL_EXT_direct_state_access spec says:
	 *
	 *    The error INVALID_ENUM is generated if array is not TEXTURE_COORD_ARRAY.
	 *
	 */
	for (i = 0; i < ARRAY_SIZE(invalid_caps); i++) {
		if (gl_compat_version == 12) {
			glEnableClientStateIndexedEXT(invalid_caps[i], GL_TEXTURE0);
		} else {
			glEnableClientStateiEXT(invalid_caps[i], GL_TEXTURE0);
		}
		if (!piglit_check_gl_error(GL_INVALID_ENUM)) {
			piglit_loge("glEnableClientStateiEXT(%s) should emit GL_INVALID_ENUM\n",
				piglit_get_gl_enum_name(invalid_caps[i]));
			pass = false;
		}
	}
	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}


enum piglit_result
piglit_display(void)
{
	/* UNREACHABLE */
	return PIGLIT_FAIL;
}

