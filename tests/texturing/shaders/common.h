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
 * \file common.h
 *
 * Common structures and functions used for GLSL 1.30+ texturing tests.
 */
#pragma once
#include "piglit-util-gl.h"

/**
 * Texture miplevel info:
 *  @{
 *
 * The total number of miplevels:
 */
int miplevels;

/** Size of the base level */
int base_size[3];

/**
 * Multidimensional array containing the dimensions of each miplevel, indexed
 * by miplevel and then x/y/z.  For example, miplevel[0][1] is the height of
 * miplevel 0, while miplevel[5][2] is the number of slices in miplevel 5.
 */
int **level_size;
/** @} */

struct sampler_info
{
	/** GLSL sampler name (such as "usampler2DArray"). */
	const char *name;

	/** GLSL sampler return type: vec4, ivec4, uvec4, or float. */
	const char *return_type;

	/** GL sampler type (such as GL_UNSIGNED_INT_SAMPLER_2D_ARRAY). */
	GLenum type;

	/** GL texture target (such as GL_TEXTURE_2D_ARRAY). */
	GLenum target;

	/** Texture format data type: GL_FLOAT, GL_INT, or GL_UNSIGNED_INT. */
	GLenum data_type;

	/** Texture format: GL_RGBA, GL_RGBA_INTEGER, or GL_DEPTH_COMPONENT */
	GLenum format;

	/**
	 * Texture internal format: GL_RGBA32F, GL_RGBA32I, GL_RGBA32UI, or
	 * GL_DEPTH_COMPONENT.
	 */
	GLenum internal_format;

	/** GL_EXT_texture_swizzle setting: GL_RED/GREEN/BLUE/ALPHA/ZERO/ONE */
	GLenum swizzle[4];
} sampler;

/** Whether or not we're using GL_EXT_texture_swizzle */
bool swizzling;
int minx, miny, minz, maxx, maxy, maxz;
int sample_count;
extern int shader_version;

/**
 * Which shader stage to test
 */
enum shader_target {
	UNKNOWN,
	VS,
	FS,
	GS
};

float max2(float x, float y);

bool has_height();
bool has_slices();
bool has_samples();

bool is_array_sampler();
bool is_shadow_sampler();

void swizzle(float vec4[]);

void upload_miplevel_data(GLenum target, int level, void *level_image);
void compute_miplevel_info();
void require_GL_features(enum shader_target test_stage);
bool select_sampler(const char *name);
bool parse_swizzle(const char *swiz);
