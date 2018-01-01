/*
 * Copyright © 2011 Intel Corporation
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
 * \file common.c
 *
 * Helper functions for GLSL 1.30+ texturing tests.
 */
#include "common.h"

int shader_version = 130;

/**
 * Load a miplevel's texel data via glTexImage.
 *
 * \param level         The miplevel to be loaded (0 <= level <= miplevels)
 * \param level_image   The data to be loaded.
 *
 * This function assumes that select_sampler() and compute_miplevel_info()
 * have already been called.
 */
void
upload_miplevel_data(GLenum target, int level, void *level_image)
{
	const GLenum format          = sampler.format;
	const GLenum internal_format = sampler.internal_format;
	const GLenum data_type       = sampler.data_type;
	GLuint bo;

	switch (target) {
	case GL_TEXTURE_1D:
		glTexImage1D(GL_TEXTURE_1D, level, internal_format,
			     level_size[level][0],
			     0, format, data_type, level_image);
		break;
	case GL_TEXTURE_2D:
	case GL_TEXTURE_RECTANGLE:
	case GL_TEXTURE_CUBE_MAP_POSITIVE_X:
	case GL_TEXTURE_CUBE_MAP_NEGATIVE_X:
	case GL_TEXTURE_CUBE_MAP_POSITIVE_Y:
	case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y:
	case GL_TEXTURE_CUBE_MAP_POSITIVE_Z:
	case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z:
		glTexImage2D(target, level, internal_format,
			     level_size[level][0], level_size[level][1],
			     0, format, data_type, level_image);
		break;
	case GL_TEXTURE_3D:
	case GL_TEXTURE_2D_ARRAY:
	case GL_TEXTURE_CUBE_MAP_ARRAY:
		glTexImage3D(target, level, internal_format,
			     level_size[level][0],
			     level_size[level][1],
			     level_size[level][2],
			     0, format, data_type, level_image);
		break;
	case GL_TEXTURE_1D_ARRAY:
		glTexImage2D(GL_TEXTURE_1D_ARRAY, level, internal_format,
			     level_size[level][0], level_size[level][2],
			     0, format, data_type, level_image);
		break;

	case GL_TEXTURE_BUFFER:
		glGenBuffers(1, &bo);
		glBindBuffer(GL_TEXTURE_BUFFER, bo);
		glBufferData(GL_TEXTURE_BUFFER, 16 * level_size[level][0],
			     level_image, GL_STATIC_DRAW);
		glTexBuffer(GL_TEXTURE_BUFFER, internal_format, bo);
		break;

	case GL_TEXTURE_2D_MULTISAMPLE:
		glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4,
				internal_format,
				level_size[level][0],
				level_size[level][1],
				GL_TRUE);
		break;
	
	case GL_TEXTURE_2D_MULTISAMPLE_ARRAY:
		glTexImage3DMultisample(GL_TEXTURE_2D_MULTISAMPLE_ARRAY, 4,
				internal_format,
				level_size[level][0],
				level_size[level][1],
				level_size[level][2],
				GL_TRUE);
		break;

	default:
		assert(!"Not implemented yet.");
		break;
	}
}


float max2(float x, float y)
{
	return (x > y) ? x : y;
}

static float max3(float x, float y, float z)
{
	return max2(x, max2(y, z));
}

/**
 * Compute the number of miplevels, as well as the dimensions (width, height,
 * depth/number of array slices) of each level.
 *
 * This function assumes base_size is already set.
 */
void
compute_miplevel_info()
{
	int i, l;
	bool is_array = is_array_sampler();
	int max_dimension;

	/* Compute the number of miplevels */
	if (sampler.target == GL_TEXTURE_3D)
		max_dimension = max3(base_size[0], base_size[1], base_size[2]);
	else
		max_dimension = max2(base_size[0], base_size[1]);

	miplevels = (int) log2f(max_dimension) + 1;

	if (sampler.target == GL_TEXTURE_RECTANGLE ||
	    sampler.target == GL_TEXTURE_BUFFER)
		miplevels = 1;
	if (sampler.target == GL_TEXTURE_2D_MULTISAMPLE ||
		sampler.target == GL_TEXTURE_2D_MULTISAMPLE_ARRAY)
		miplevels = sample_count;

	/* Compute the size of each miplevel */
	level_size = malloc(miplevels * sizeof(int *));

	level_size[0] = malloc(3 * sizeof(int));
	memcpy(level_size[0], base_size, 3 * sizeof(int));
	for (l = 1; l < miplevels; l++) {
		level_size[l] = malloc(3 * sizeof(int));

		for (i = 0; i < 3 - is_array; i++)
			if (has_samples())
				/* same size for all sample planes */
				level_size[l][i] = level_size[l-1][i];
			else
				level_size[l][i] = max2(level_size[l-1][i] / 2, 1);

		if (is_array)
			level_size[l][2] = base_size[2];
	}
}

bool
has_height()
{
	return sampler.target == GL_TEXTURE_2D ||
	       sampler.target == GL_TEXTURE_3D ||
	       sampler.target == GL_TEXTURE_2D_ARRAY ||
	       sampler.target == GL_TEXTURE_RECTANGLE ||
	       sampler.target == GL_TEXTURE_2D_MULTISAMPLE ||
	       sampler.target == GL_TEXTURE_2D_MULTISAMPLE_ARRAY;
}

bool
has_slices()
{
	return is_array_sampler() || sampler.target == GL_TEXTURE_3D;
}

bool
has_samples()
{
    return sampler.target == GL_TEXTURE_2D_MULTISAMPLE ||
           sampler.target == GL_TEXTURE_2D_MULTISAMPLE_ARRAY;
}

bool
is_array_sampler()
{
	return sampler.target == GL_TEXTURE_1D_ARRAY ||
	       sampler.target == GL_TEXTURE_2D_ARRAY ||
	       sampler.target == GL_TEXTURE_CUBE_MAP_ARRAY ||
	       sampler.target == GL_TEXTURE_2D_MULTISAMPLE_ARRAY;
}

bool
is_shadow_sampler()
{
	return sampler.format == GL_DEPTH_COMPONENT;
}

/**
 * Check if a given command line argument is a valid GLSL sampler type.
 * If so, infer dimensionality and data format based on the name.
 *
 * Returns \c true if \p name was a valid sampler.
 */
bool
select_sampler(const char *name)
{
	int i;
	bool found = false;
	struct {
		const char *name;
		GLenum type;
		GLenum target;
	} samplers[] = {
		{ "sampler1D",              GL_SAMPLER_1D,                          GL_TEXTURE_1D,            },
		{ "sampler2D",              GL_SAMPLER_2D,                          GL_TEXTURE_2D,            },
		{ "sampler3D",              GL_SAMPLER_3D,                          GL_TEXTURE_3D,            },
		{ "samplerCube",            GL_SAMPLER_CUBE,                        GL_TEXTURE_CUBE_MAP       },
		{ "sampler2DRect",          GL_SAMPLER_2D_RECT,                     GL_TEXTURE_RECTANGLE      },
		{ "sampler1DArray",         GL_SAMPLER_1D_ARRAY,                    GL_TEXTURE_1D_ARRAY       },
		{ "sampler2DArray",         GL_SAMPLER_2D_ARRAY,                    GL_TEXTURE_2D_ARRAY       },
		{ "samplerCubeArray",       GL_SAMPLER_CUBE_MAP_ARRAY,              GL_TEXTURE_CUBE_MAP_ARRAY },
		{ "samplerBuffer",          GL_SAMPLER_BUFFER,                      GL_TEXTURE_BUFFER },
		{ "sampler2DMS",            GL_SAMPLER_2D_MULTISAMPLE,              GL_TEXTURE_2D_MULTISAMPLE },
		{ "sampler2DMSArray",       GL_SAMPLER_2D_MULTISAMPLE_ARRAY,        GL_TEXTURE_2D_MULTISAMPLE_ARRAY },

		{ "sampler1DShadow",        GL_SAMPLER_1D_SHADOW,                   GL_TEXTURE_1D             },
		{ "sampler2DShadow",        GL_SAMPLER_2D_SHADOW,                   GL_TEXTURE_2D             },
		{ "samplerCubeShadow",      GL_SAMPLER_CUBE_SHADOW,                 GL_TEXTURE_CUBE_MAP       },
		{ "sampler2DRectShadow",    GL_SAMPLER_2D_RECT_SHADOW,              GL_TEXTURE_RECTANGLE      },
		{ "sampler1DArrayShadow",   GL_SAMPLER_1D_ARRAY_SHADOW,             GL_TEXTURE_1D_ARRAY       },
		{ "sampler2DArrayShadow",   GL_SAMPLER_2D_ARRAY_SHADOW,             GL_TEXTURE_2D_ARRAY       },
		{ "samplerCubeArrayShadow", GL_SAMPLER_CUBE_MAP_ARRAY_SHADOW,       GL_TEXTURE_CUBE_MAP_ARRAY },

		{ "isampler1D",             GL_INT_SAMPLER_1D,                      GL_TEXTURE_1D             },
		{ "isampler2D",             GL_INT_SAMPLER_2D,                      GL_TEXTURE_2D             },
		{ "isampler3D",             GL_INT_SAMPLER_3D,                      GL_TEXTURE_3D             },
		{ "isamplerCube",           GL_INT_SAMPLER_CUBE,                    GL_TEXTURE_CUBE_MAP       },
		{ "isampler2DRect",         GL_INT_SAMPLER_2D_RECT,                 GL_TEXTURE_RECTANGLE      },
		{ "isampler1DArray",        GL_INT_SAMPLER_1D_ARRAY,                GL_TEXTURE_1D_ARRAY       },
		{ "isampler2DArray",        GL_INT_SAMPLER_2D_ARRAY,                GL_TEXTURE_2D_ARRAY       },
		{ "isamplerCubeArray",      GL_INT_SAMPLER_CUBE_MAP_ARRAY,          GL_TEXTURE_CUBE_MAP_ARRAY },
		{ "isamplerBuffer",         GL_INT_SAMPLER_BUFFER,                  GL_TEXTURE_BUFFER },
		{ "isampler2DMS",           GL_INT_SAMPLER_2D_MULTISAMPLE,          GL_TEXTURE_2D_MULTISAMPLE },
		{ "isampler2DMSArray",      GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY,    GL_TEXTURE_2D_MULTISAMPLE_ARRAY },

		{ "usampler1D",             GL_UNSIGNED_INT_SAMPLER_1D,             GL_TEXTURE_1D             },
		{ "usampler2D",             GL_UNSIGNED_INT_SAMPLER_2D,             GL_TEXTURE_2D             },
		{ "usampler3D",             GL_UNSIGNED_INT_SAMPLER_3D,             GL_TEXTURE_3D             },
		{ "usamplerCube",           GL_UNSIGNED_INT_SAMPLER_CUBE,           GL_TEXTURE_CUBE_MAP       },
		{ "usampler2DRect",         GL_UNSIGNED_INT_SAMPLER_2D_RECT,        GL_TEXTURE_RECTANGLE      },
		{ "usampler1DArray",        GL_UNSIGNED_INT_SAMPLER_1D_ARRAY,       GL_TEXTURE_1D_ARRAY       },
		{ "usampler2DArray",        GL_UNSIGNED_INT_SAMPLER_2D_ARRAY,       GL_TEXTURE_2D_ARRAY       },
		{ "usamplerCubeArray",      GL_UNSIGNED_INT_SAMPLER_CUBE_MAP_ARRAY, GL_TEXTURE_CUBE_MAP_ARRAY },
		{ "usamplerBuffer",         GL_UNSIGNED_INT_SAMPLER_BUFFER,         GL_TEXTURE_BUFFER },
		{ "usampler2DMS",           GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE, GL_TEXTURE_2D_MULTISAMPLE },
		{ "usampler2DMSArray",      GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY, GL_TEXTURE_2D_MULTISAMPLE_ARRAY },
	};

	for (i = 0; i < ARRAY_SIZE(samplers); i++) {
		if (strcmp(samplers[i].name, name) == 0) {
			found = true;
			break;
		}
	}

	if (!found)
		return false;

	sampler.name = name;
	sampler.type = samplers[i].type;
	sampler.target = samplers[i].target;

	/* Use 32bpc sized formats where possible; drop down to 16bpc for
	 * testing multisample targets to avoid hitting some hardware limits.
	 * on i965.
	 */

	if (name[0] == 'i') {
		sampler.data_type = GL_INT;
		sampler.format = GL_RGBA_INTEGER;
		sampler.internal_format = has_samples() ? GL_RGBA16I : GL_RGBA32I;
		sampler.return_type = "ivec4";
	} else if (name[0] == 'u') {
		sampler.data_type = GL_UNSIGNED_INT;
		sampler.format = GL_RGBA_INTEGER;
		sampler.internal_format = has_samples() ? GL_RGBA16UI : GL_RGBA32UI;
		sampler.return_type = "uvec4";
	} else if (strstr(name, "Shadow")) {
		/* Shadow Sampler */
		sampler.data_type = GL_FLOAT;
		sampler.format = GL_DEPTH_COMPONENT;
		sampler.internal_format = GL_DEPTH_COMPONENT;
		sampler.return_type = "float";
	} else {
		sampler.data_type = GL_FLOAT;
		sampler.format = GL_RGBA;
		sampler.internal_format = has_samples() ? GL_RGBA16F : GL_RGBA32F;
		sampler.return_type = "vec4";
	}

	return true;
}

/**
 * Ensures the driver supports the required extensions, GL, and GLSL versions.
 * If it doesn't, report PIGLIT_SKIP and exit the test.
 */
void
require_GL_features(enum shader_target test_stage)
{
	int tex_units;

	piglit_require_GLSL_version(shader_version);

	if (swizzling)
		piglit_require_extension("GL_EXT_texture_swizzle");

	switch (sampler.internal_format) {
	case GL_RGBA32I:
	case GL_RGBA16I:
		piglit_require_extension("GL_EXT_texture_integer");
		break;
	case GL_RGBA32UI:
	case GL_RGBA16UI:
		if (piglit_is_extension_supported("GL_EXT_gpu_shader4"))
			piglit_require_gl_version(21);
		else
			piglit_require_gl_version(30);
		break;
	case GL_RGBA32F:
	case GL_RGBA16F:
		piglit_require_extension("GL_ARB_texture_float");
		break;
	}

	switch (sampler.target) {
	case GL_TEXTURE_CUBE_MAP_ARRAY:
		piglit_require_extension("GL_ARB_texture_cube_map_array");
		break;
	case GL_TEXTURE_1D_ARRAY:
	case GL_TEXTURE_2D_ARRAY:
		piglit_require_extension("GL_EXT_texture_array");
		break;
	case GL_TEXTURE_CUBE_MAP:
		if (is_shadow_sampler()) {
			if (piglit_is_extension_supported("GL_EXT_gpu_shader4"))
				piglit_require_gl_version(21);
			else
				piglit_require_gl_version(30);
		}
		break;
	case GL_TEXTURE_RECTANGLE:
		piglit_require_extension("GL_ARB_texture_rectangle");
		break;
	case GL_TEXTURE_BUFFER:
		piglit_require_extension("GL_ARB_texture_buffer_object");
		break;
	case GL_TEXTURE_2D_MULTISAMPLE:
	case GL_TEXTURE_2D_MULTISAMPLE_ARRAY:
		piglit_require_extension("GL_ARB_texture_multisample");
	}

	/* If testing in the VS, check for VS texture units */
	glGetIntegerv(GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS, &tex_units);
	if (test_stage == VS && tex_units <= 0)
		piglit_report_result(PIGLIT_SKIP);

	if (test_stage == TES)
		piglit_require_extension("GL_ARB_tessellation_shader");
}

/**
 * Performs an in-place swizzle of a vec4 based on the EXT_texture_swizzle mode.
 */
void
swizzle(float vec[])
{
	int i;
	float temp[4];

	if (!swizzling)
		return;

	memcpy(temp, vec, 4*sizeof(float));

	for (i = 0; i < 4; i++) {
		switch (sampler.swizzle[i]) {
		case GL_RED:
			vec[i] = temp[0];
			break;
		case GL_GREEN:
			vec[i] = temp[1];
			break;
		case GL_BLUE:
			vec[i] = temp[2];
			break;
		case GL_ALPHA:
			vec[i] = temp[3];
			break;
		case GL_ZERO:
			vec[i] = 0.0;
			break;
		case GL_ONE:
			vec[i] = 1.0;
			break;
		default:
			assert(!"Should not get here.");
		}
	}
}

/**
 * Parse the command line argument for the EXT_texture_swizzle mode.
 * It should be a string of length 4 consisting of r, g, b, a, 0, or 1.
 * For example, "bgr1".
 */
bool
parse_swizzle(const char *swiz)
{
	int i;
	if (strlen(swiz) != 4 || strspn(swiz, "rgba01") != 4)
		return false;

	for (i = 0; i < 4; i++) {
		switch (swiz[i]) {
		case 'r':
			sampler.swizzle[i] = GL_RED;
			break;
		case 'g':
			sampler.swizzle[i] = GL_GREEN;
			break;
		case 'b':
			sampler.swizzle[i] = GL_BLUE;
			break;
		case 'a':
			sampler.swizzle[i] = GL_ALPHA;
			break;
		case '0':
			sampler.swizzle[i] = GL_ZERO;
			break;
		case '1':
			sampler.swizzle[i] = GL_ONE;
			break;
		default:
			assert(!"Should not get here.");
		}
	}

	return true;
}
