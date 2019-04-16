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

static GLint max_units;
static GLint max_tex_coords;

struct get_float_indexed_t {
	void (*get_float_fn) (GLenum, GLuint, GLfloat *);
	const char* name;
	int min_gl_compat_version;
};
struct get_double_indexed_t {
	void (*get_double_fn) (GLenum, GLuint, GLdouble *);
	const char* name;
	int min_gl_compat_version;
};
struct get_pointer_indexed_t {
	void (*get_pointer_fn) (GLenum, GLuint, GLvoid **);
	const char* name;
	int min_gl_compat_version;
};


static enum piglit_result
test_GetBooleanIndexedvEXT(void* data)
{
	static const GLenum targets[] = {
		/*
		 * Targets for which:
		 *	glGetBooleanIndexedvEXT(target, index, params)
		 * is equivalent to:
		 *	glActiveTexture(GL_TEXTURE0+index);
	         *	glGetBooleanv(target, params);
		 */
		GL_TEXTURE_1D,
		GL_TEXTURE_2D,
		GL_TEXTURE_3D,
		GL_TEXTURE_CUBE_MAP,

		/* Separator */
		GL_NONE,

		/*
		 * Targets for which:
		 *	glGetBooleanIndexedvEXT(target, index, params)
		 * is equivalent to:
	         *	glClientActiveTexture(GL_TEXTURE0+index);
	         *	glGetBooleanv(target, params);
		 */
		GL_TEXTURE_COORD_ARRAY
	};
	int i, index;
	GLboolean value, expected_value;
	bool useActiveTexture = true;

	for (i = 0; i < ARRAY_SIZE(targets); i++) {
		if (targets[i] == GL_NONE) {
			useActiveTexture = false;
			continue;
		}

		if (useActiveTexture) {
			index = rand() % max_units;
		} else {
			index = rand() % max_tex_coords;
		}

		glGetBooleanIndexedvEXT(targets[i], index, &value);

		if (useActiveTexture) {
			glActiveTexture(GL_TEXTURE0 + index);
		} else {
			glClientActiveTexture(GL_TEXTURE0 + index);
		}
		glGetBooleanv(targets[i], &expected_value);

		if (value != expected_value || !piglit_check_gl_error(GL_NO_ERROR)) {
			piglit_loge("glGetBooleanIndexedvEXT(%s, %d, ...) failed. Expected: %d but got %d\n",
				piglit_get_gl_enum_name(targets[i]),
				index,
				expected_value,
				value);
			return PIGLIT_FAIL;
		}
	}

	return PIGLIT_PASS;
}

static enum piglit_result
test_GetIntegerIndexedvEXT(void* data)
{
	static const GLenum targets[] = {
		GL_TEXTURE_BINDING_1D, GL_TEXTURE_BINDING_1D_ARRAY,
		GL_TEXTURE_BINDING_2D, GL_TEXTURE_BINDING_2D_ARRAY,
		GL_TEXTURE_BINDING_3D, GL_TEXTURE_BINDING_CUBE_MAP
	};
	int i;
	int value, expected_value;

	for (i = 0; i < ARRAY_SIZE(targets); i++) {
		int index = rand() % max_units;
		glActiveTexture(GL_TEXTURE0 + (index + 1) % max_units);

		glGetIntegerIndexedvEXT(targets[i], index, &value);

		glActiveTexture(GL_TEXTURE0 + index);
		glGetIntegerv(targets[i], &expected_value);

		if (value != expected_value || !piglit_check_gl_error(GL_NO_ERROR)) {
			piglit_loge("glGetIntegerIndexedvEXT(%s, %d, ...) failed. Expected: %d but got %d\n",
				piglit_get_gl_enum_name(targets[i]),
				index,
				expected_value,
				value);
			return PIGLIT_FAIL;
		}
	}
	return PIGLIT_PASS;
}

static enum piglit_result
test_GetFloatIndexedvEXT(void* data)
{
	static const GLenum targets[] = {
		GL_TEXTURE_MATRIX, GL_TRANSPOSE_TEXTURE_MATRIX,
	};
	int i;
	float value[16], expected_value[16];

	struct get_float_indexed_t* test = (struct get_float_indexed_t*) data;

	if (gl_compat_version < test->min_gl_compat_version) {
		return PIGLIT_SKIP;
	}

	for (i = 0; i < ARRAY_SIZE(targets); i++) {
		int index = rand() % max_tex_coords;
		glActiveTexture(GL_TEXTURE0 + (index + 1) % max_tex_coords);

		test->get_float_fn(targets[i], index, value);

		glActiveTexture(GL_TEXTURE0 + index);
		glGetFloatv(targets[i], expected_value);

		if (memcmp(value, expected_value, sizeof(value)) != 0 ||
		    !piglit_check_gl_error(GL_NO_ERROR)) {
			piglit_loge("%s(%s, %d, ...) failed.\n",
				test->name,
				piglit_get_gl_enum_name(targets[i]),
				index);
			return PIGLIT_FAIL;
		}
	}
	return PIGLIT_PASS;
}

static enum piglit_result
test_GetDoubleIndexedvEXT(void* data)
{
	static const GLenum targets[] = {
		GL_TEXTURE_MATRIX, GL_TRANSPOSE_TEXTURE_MATRIX,
	};
	int i;
	double value[16], expected_value[16];

	struct get_double_indexed_t* test = (struct get_double_indexed_t*) data;
	if (gl_compat_version < test->min_gl_compat_version) {
		return PIGLIT_SKIP;
	}

	for (i = 0; i < ARRAY_SIZE(targets); i++) {
		int index = rand() % max_tex_coords;
		glActiveTexture(GL_TEXTURE0 + (index + 1) % max_tex_coords);

		test->get_double_fn(targets[i], index, value);

		glActiveTexture(GL_TEXTURE0 + index);
		glGetDoublev(targets[i], expected_value);

		if (memcmp(value, expected_value, sizeof(value)) != 0 ||
		    !piglit_check_gl_error(GL_NO_ERROR)) {
			piglit_loge("%s(%s, %d, ...) failed.\n",
				test->name,
				piglit_get_gl_enum_name(targets[i]),
				index);
			return PIGLIT_FAIL;
		}
	}
	return PIGLIT_PASS;
}

static enum piglit_result
test_GetPointerIndexedvEXT(void* data)
{
	/* The GL_EXT_direct_state_access spec says:
	 *
	 *   The following query
	 *
	 *       void GetPointerIndexedvEXT(enum pname, uint index, void **params);
	 *
	 *   is equivalent (assuming no errors) to the following:
	 *
	 *       int savedClientActiveTexture;
	 *
	 *       GetIntegerv(CLIENT_ACTIVE_TEXTURE, &savedClientActiveTexture);
	 *       ClientActiveTexture(TEXTURE0+index);
	 *       GetPointerv(pname, params);
	 *       ClientActiveTexture(savedClientActiveTexture);
	 *
	 *   [...] when the pname parameter is TEXTURE_COORD_ARRAY_POINTER.
	 *
	 */
	static const GLenum invalid_pnames[] = {
		GL_COLOR_ARRAY_POINTER, GL_EDGE_FLAG_ARRAY_POINTER,
		GL_FOG_COORD_ARRAY_POINTER, GL_FEEDBACK_BUFFER_POINTER,
		GL_INDEX_ARRAY_POINTER, GL_NORMAL_ARRAY_POINTER,
		GL_SECONDARY_COLOR_ARRAY_POINTER, GL_SELECTION_BUFFER_POINTER,
		GL_VERTEX_ARRAY_POINTER
	};
	int i;
	void* ptr, *expected_ptr;

	int index = rand() % max_tex_coords;

	struct get_pointer_indexed_t* test = (struct get_pointer_indexed_t*) data;
	if (gl_compat_version < test->min_gl_compat_version) {
		return PIGLIT_SKIP;
	}

	glActiveTexture(GL_TEXTURE0 + (index + 1) % max_tex_coords);

	test->get_pointer_fn(GL_TEXTURE_COORD_ARRAY_POINTER, index, &ptr);

	glActiveTexture(GL_TEXTURE0 + index);

	glGetPointerv(GL_TEXTURE_COORD_ARRAY_POINTER, &expected_ptr);

	if (expected_ptr != ptr || !piglit_check_gl_error(GL_NO_ERROR)) {
		return PIGLIT_FAIL;
	}

	for (i = 0; i < ARRAY_SIZE(invalid_pnames); i++) {
		glGetPointerIndexedvEXT(invalid_pnames[i], index, &ptr);

		if (!piglit_check_gl_error(GL_INVALID_ENUM)) {
			piglit_loge("glGetPointerIndexedvEXT(%s, ..., ...) should emit GL_INVALID_ENUM.\n",
				piglit_get_gl_enum_name(invalid_pnames[i]));
			return PIGLIT_FAIL;
		}
	}
	return PIGLIT_PASS;
}

void
piglit_init(int argc, char **argv)
{
	piglit_require_extension("GL_EXT_direct_state_access");

	glGetIntegerv(GL_MAX_TEXTURE_COORDS, &max_tex_coords);
	glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &max_units);

	/* The GL_EXT_direct_state_access spec says:
	 *
	 *     Add OpenGL 3.0-style aliases for the version 1.0 commands
	 *     and queries that have "Indexed" in the name.  OpenGL 3.0 has a
	 *     convention where an "i" indexed indexed commands and queries.
	 *     For example, glGetFloati_v and glGetFloatIndexedvEXT are
	 *     identical queries
	 *
	 * So glGetFloatIndexedvEXT/GetFloati_vEXT, GetDoubleIndexedvEXT/GetDoublei_vEXT
	 * and GetPointerIndexedvEXT/GetPointeri_vEXT use the same subtest
	 * where data describes which function is being tested.
	 */

	const struct get_float_indexed_t get_float_12 = {
		glGetFloatIndexedvEXT, "GetFloatIndexedvEXT", 12
	};
	const struct get_float_indexed_t get_float_30 = {
		glGetFloati_vEXT, "GetFloati_vEXT", 30
	};
	const struct get_double_indexed_t get_double_12 = {
		glGetDoubleIndexedvEXT, "GetDoubleIndexedvEXT", 12
	};
	const struct get_double_indexed_t get_double_30 = {
		glGetDoublei_vEXT, "GetDoublei_vEXT", 30
	};
	const struct get_pointer_indexed_t get_pointer_12 = {
		glGetPointerIndexedvEXT, "GetPointerIndexedvEXT", 12
	};
	const struct get_pointer_indexed_t get_pointer_30 = {
		glGetPointeri_vEXT, "GetPointeri_vEXT", 30
	};

	const struct piglit_subtest tests[] = {
		{
			"GetBooleanIndexedvEXT",
			NULL,
			test_GetBooleanIndexedvEXT
		},
		{
			"GetIntegerIndexedvEXT",
			NULL,
			test_GetIntegerIndexedvEXT
		},
		{
			"GetFloatIndexedvEXT",
			NULL,
			test_GetFloatIndexedvEXT,
			(void*) &get_float_12
		},
		{
			"GetFloati_vEXT",
			NULL,
			test_GetFloatIndexedvEXT,
			(void*) &get_float_30,
		},
		{
			"GetDoubleIndexedvEXT",
			NULL,
			test_GetDoubleIndexedvEXT,
			(void*) &get_double_12
		},
		{
			"GetDoublei_vEXT",
			NULL,
			test_GetDoubleIndexedvEXT,
			(void*) &get_double_30,
		},
		{
			"GetPointerIndexedvEXT",
			NULL,
			test_GetPointerIndexedvEXT,
			(void*) &get_pointer_12
		},
		{
			"GetPointeri_vEXT",
			NULL,
			test_GetPointerIndexedvEXT,
			(void*) &get_pointer_30,
		},
		{
			NULL
		}
	};
	piglit_report_result(piglit_run_selected_subtests(tests, NULL, 0, PIGLIT_PASS));
}

enum piglit_result
piglit_display(void)
{
	/* UNREACHABLE */
	return PIGLIT_FAIL;
}