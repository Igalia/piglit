/*
 * Copyright © 2012 Intel Corporation
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

#include "piglit-util-gl.h"
#include "uniform-types.h"

const struct uniform_type uniform_types[] = {
	{ "float", GL_FLOAT, 4, 4 },
	{ "vec2", GL_FLOAT_VEC2, 8, 8 },
	{ "vec3", GL_FLOAT_VEC3, 12, 16 },
	{ "vec4", GL_FLOAT_VEC4, 16, 16 },

	{ "int", GL_INT, 4, 4 },
	{ "ivec2", GL_INT_VEC2, 8, 8 },
	{ "ivec3", GL_INT_VEC3, 12, 16 },
	{ "ivec4", GL_INT_VEC4, 16, 16 },

	{ "uint", GL_UNSIGNED_INT, 4, 4 },
	{ "uvec2", GL_UNSIGNED_INT_VEC2, 8, 8 },
	{ "uvec3", GL_UNSIGNED_INT_VEC3, 12, 16 },
	{ "uvec4", GL_UNSIGNED_INT_VEC4, 16, 16 },

	{ "bool", GL_BOOL, 4, 4 },
	{ "bvec2", GL_BOOL_VEC2, 8, 8 },
	{ "bvec3", GL_BOOL_VEC3, 12, 16 },
	{ "bvec4", GL_BOOL_VEC4, 16, 16 },

	{ "mat2", GL_FLOAT_MAT2, 32, 16 },
	{ "mat3", GL_FLOAT_MAT3, 48, 16 },
	{ "mat4", GL_FLOAT_MAT4, 64, 16 },

	{ "mat2x3", GL_FLOAT_MAT2x3, 32, 16 },
	{ "mat2x4", GL_FLOAT_MAT2x4, 32, 16 },

	{ "mat3x2", GL_FLOAT_MAT3x2, 48, 16 },
	{ "mat3x4", GL_FLOAT_MAT3x4, 48, 16 },

	{ "mat4x2", GL_FLOAT_MAT4x2, 64, 16 },
	{ "mat4x3", GL_FLOAT_MAT4x3, 64, 16 },

	/* No sampler types listed, because they don't work in
	 * UBOs.
	 */

	{ NULL }
};

const struct uniform_type *
get_transposed_type(const struct uniform_type *type)
{
	const char *name = NULL;
	int i;

	switch (type->gl_type) {
	case GL_FLOAT_MAT2x3:
		name = "mat3x2";
		break;
	case GL_FLOAT_MAT2x4:
		name = "mat4x2";
		break;
	case GL_FLOAT_MAT3x2:
		name = "mat2x3";
		break;
	case GL_FLOAT_MAT3x4:
		name = "mat4x3";
		break;
	case GL_FLOAT_MAT4x2:
		name = "mat2x4";
		break;
	case GL_FLOAT_MAT4x3:
		name = "mat3x4";
		break;
	default:
		return type;
	}

	for (i = 0; uniform_types[i].type; i++) {
		if (strcmp(uniform_types[i].type, name) == 0)
			return &uniform_types[i];
	}

	printf("failed lookup of %s\n", name);
	piglit_report_result(PIGLIT_FAIL);
	return type;
}
