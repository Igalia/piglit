/*
 * Copyright © 2011 Marek Olšák <maraeo@gmail.com>
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

/**
 * EXT_transform_feedback test.
 *
 * Test that writing a variable with a specific GLSL type into a TFB buffer
 * works as expected.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;

PIGLIT_GL_TEST_CONFIG_END

#define DEFAULT_VALUE 0.123456

#define MAX_VARYINGS 16
#define MAX_BUFFERS 4
#define MAX_ELEMENTS 256

struct test_desc {
	const char *name;
	const char *vs;
	unsigned num_varyings;
	const char *varyings[MAX_VARYINGS];
	bool is_floating_point;
	unsigned num_elements[MAX_BUFFERS];
	float expected_float[MAX_BUFFERS][MAX_ELEMENTS];
	GLint expected_int[MAX_BUFFERS][MAX_ELEMENTS];
	bool is_transform_feedback3;
} tests[] = {
	{
		"float", /* name */

		"#version 110\n"
		"varying float r;" /* vs */
		"void main() {"
		"  gl_Position = ftransform();"
		"  r = 666.0;"
		"}",

		1, /* num_varyings, varyings */
		{"r"},

		true, /* is_floating_point */
		{1}, /* num_elements, expected_float, expected_int */
		{{666}}
	},
	{
		"float[2]", /* name */

		"#version 120\n"
		"varying float r[2];" /* vs */
		"void main() {"
		"  gl_Position = ftransform();"
		"  r = float[2](666.0, 0.123);"
		"}",

		2, /* num_varyings, varyings */
		{"r[0]", "r[1]"},

		true, /* is_floating_point */
		{2}, /* num_elements, expected_float, expected_int */
		{{666, 0.123}}
	},
	{
		"float[2]-no-subscript", /* name */

		"#version 120\n"
		"varying float r[2];" /* vs */
		"void main() {"
		"  gl_Position = ftransform();"
		"  r = float[2](666.0, 0.123);"
		"}",

		1, /* num_varyings, varyings */
		{"r"},

		true, /* is_floating_point */
		{2}, /* num_elements, expected_float, expected_int */
		{{666, 0.123}}
	},
	{
		"vec2", /* name */

		"#version 110\n"
		"varying vec2 r;" /* vs */
		"void main() {"
		"  gl_Position = ftransform();"
		"  r = vec2(666.0, 999.0);"
		"}",

		1, /* num_varyings, varyings */
		{"r"},

		true, /* is_floating_point */
		{2}, /* num_elements, expected_float, expected_int */
		{{666, 999}}
	},
	{
		"vec2[2]", /* name */

		"#version 120\n"
		"varying vec2 r[2];" /* vs */
		"void main() {"
		"  gl_Position = ftransform();"
		"  r = vec2[2](vec2(666.0, 999.0), vec2(-1.5, -20.0));"
		"}",

		2, /* num_varyings, varyings */
		{"r[0]", "r[1]"},

		true, /* is_floating_point */
		{4}, /* num_elements, expected_float, expected_int */
		{{666, 999, -1.5, -20.0}}
	},
	{
		"vec2[2]-no-subscript", /* name */

		"#version 120\n"
		"varying vec2 r[2];" /* vs */
		"void main() {"
		"  gl_Position = ftransform();"
		"  r = vec2[2](vec2(666.0, 999.0), vec2(-1.5, -20.0));"
		"}",

		1, /* num_varyings, varyings */
		{"r"},

		true, /* is_floating_point */
		{4}, /* num_elements, expected_float, expected_int */
		{{666, 999, -1.5, -20.0}}
	},
	{
		"vec3", /* name */

		"#version 110\n"
		"varying vec3 r;" /* vs */
		"void main() {"
		"  gl_Position = ftransform();"
		"  r = vec3(666.0, 999.0, -2.0);"
		"}",

		1, /* num_varyings, varyings */
		{"r"},

		true, /* is_floating_point */
		{3}, /* num_elements, expected_float, expected_int */
		{{666, 999, -2}}
	},
	{
		"vec3[2]", /* name */

		"#version 120\n"
		"varying vec3 r[2];" /* vs */
		"void main() {"
		"  gl_Position = ftransform();"
		"  r = vec3[2](vec3(666.0, 999.0, -2.0), vec3(0.4, 1.4, 3.5));"
		"}",

		2, /* num_varyings, varyings */
		{"r[0]", "r[1]"},

		true, /* is_floating_point */
		{6}, /* num_elements, expected_float, expected_int */
		{{666, 999, -2, 0.4, 1.4, 3.5}}
	},
	{
		"vec3[2]-no-subscript", /* name */

		"#version 120\n"
		"varying vec3 r[2];" /* vs */
		"void main() {"
		"  gl_Position = ftransform();"
		"  r = vec3[2](vec3(666.0, 999.0, -2.0), vec3(0.4, 1.4, 3.5));"
		"}",

		1, /* num_varyings, varyings */
		{"r"},

		true, /* is_floating_point */
		{6}, /* num_elements, expected_float, expected_int */
		{{666, 999, -2, 0.4, 1.4, 3.5}}
	},
	{
		"vec4", /* name */

		"#version 110\n"
		"varying vec4 r;" /* vs */
		"void main() {"
		"  gl_Position = ftransform();"
		"  r = vec4(0.666, 666.0, 999.0, -2.0);"
		"}",

		1, /* num_varyings, varyings */
		{"r"},

		true, /* is_floating_point */
		{4}, /* num_elements, expected_float, expected_int */
		{{0.666, 666, 999, -2}}
	},
	{
		"vec4[2]", /* name */

		"#version 120\n"
		"varying vec4 r[2];" /* vs */
		"void main() {"
		"  gl_Position = ftransform();"
		"  r = vec4[2](vec4(0.666, 666.0, 999.0, -2.0), vec4(0.5, -0.4, 30.0, 40.0));"
		"}",

		2, /* num_varyings, varyings */
		{"r[0]", "r[1]"},

		true, /* is_floating_point */
		{8}, /* num_elements, expected_float, expected_int */
		{{0.666, 666, 999, -2, 0.5, -0.4, 30.0, 40.0}}
	},
	{
		"vec4[2]-no-subscript", /* name */

		"#version 120\n"
		"varying vec4 r[2];" /* vs */
		"void main() {"
		"  gl_Position = ftransform();"
		"  r = vec4[2](vec4(0.666, 666.0, 999.0, -2.0), vec4(0.5, -0.4, 30.0, 40.0));"
		"}",

		1, /* num_varyings, varyings */
		{"r"},

		true, /* is_floating_point */
		{8}, /* num_elements, expected_float, expected_int */
		{{0.666, 666, 999, -2, 0.5, -0.4, 30.0, 40.0}}
	},
	{
		"mat2", /* name */

		"#version 110\n"
		"varying mat2 r;" /* vs */
		"void main() {"
		"  gl_Position = ftransform();"
		"  r = mat2(0.666, 666.0, 999.0, -2.0);"
		"}",

		1, /* num_varyings, varyings */
		{"r"},

		true, /* is_floating_point */
		{4}, /* num_elements, expected_float, expected_int */
		{{0.666, 666, 999, -2}}
	},
	{
		"mat2[2]", /* name */

		"#version 120\n"
		"varying mat2 r[2];" /* vs */
		"void main() {"
		"  gl_Position = ftransform();"
		"  r = mat2[2](mat2(0.666, 666.0, 999.0, -2.0),"
		"              mat2(0.34, 0.65, 0.14, -0.97));"
		"}",

		2, /* num_varyings, varyings */
		{"r[0]", "r[1]"},

		true, /* is_floating_point */
		{8}, /* num_elements, expected_float, expected_int */
		{{0.666, 666, 999, -2, 0.34, 0.65, 0.14, -0.97}}
	},
	{
		"mat2[2]-no-subscript", /* name */

		"#version 120\n"
		"varying mat2 r[2];" /* vs */
		"void main() {"
		"  gl_Position = ftransform();"
		"  r = mat2[2](mat2(0.666, 666.0, 999.0, -2.0),"
		"              mat2(0.34, 0.65, 0.14, -0.97));"
		"}",

		1, /* num_varyings, varyings */
		{"r"},

		true, /* is_floating_point */
		{8}, /* num_elements, expected_float, expected_int */
		{{0.666, 666, 999, -2, 0.34, 0.65, 0.14, -0.97}}
	},
	{
		"mat2x3", /* name */

		"#version 120\n"
		"varying mat2x3 r;" /* vs */
		"void main() {"
		"  gl_Position = ftransform();"
		"  r = mat2x3(0.666, 666.0, 999.0, -2.0, 0.5, -0.4);"
		"}",

		1, /* num_varyings, varyings */
		{"r"},

		true, /* is_floating_point */
		{6}, /* num_elements, expected_float, expected_int */
		{{0.666, 666, 999, -2, 0.5, -0.4}}
	},
	{
		"mat2x3[2]", /* name */

		"#version 120\n"
		"varying mat2x3 r[2];" /* vs */
		"void main() {"
		"  gl_Position = ftransform();"
		"  r = mat2x3[2](mat2x3(0.666, 666.0, 999.0, -2.0, 0.5, -0.4),"
		"                mat2x3(0.34, 0.12, -10.0, 30.1, 5.3, 9.8));"
		"}",

		2, /* num_varyings, varyings */
		{"r[0]", "r[1]"},

		true, /* is_floating_point */
		{12}, /* num_elements, expected_float, expected_int */
		{{0.666, 666, 999, -2, 0.5, -0.4, 0.34, 0.12, -10.0, 30.1, 5.3, 9.8}}
	},
	{
		"mat2x3[2]-no-subscript", /* name */

		"#version 120\n"
		"varying mat2x3 r[2];" /* vs */
		"void main() {"
		"  gl_Position = ftransform();"
		"  r = mat2x3[2](mat2x3(0.666, 666.0, 999.0, -2.0, 0.5, -0.4),"
		"                mat2x3(0.34, 0.12, -10.0, 30.1, 5.3, 9.8));"
		"}",

		1, /* num_varyings, varyings */
		{"r"},

		true, /* is_floating_point */
		{12}, /* num_elements, expected_float, expected_int */
		{{0.666, 666, 999, -2, 0.5, -0.4, 0.34, 0.12, -10.0, 30.1, 5.3, 9.8}}
	},
	{
		"mat2x4", /* name */

		"#version 120\n"
		"varying mat2x4 r;" /* vs */
		"void main() {"
		"  gl_Position = ftransform();"
		"  r = mat2x4(0.666, 666.0, 999.0, -2.0, 0.5, -0.4, 30.0, 40.0);"
		"}",

		1, /* num_varyings, varyings */
		{"r"},

		true, /* is_floating_point */
		{8}, /* num_elements, expected_float, expected_int */
		{{0.666, 666, 999, -2, 0.5, -0.4, 30, 40}}
	},
	{
		"mat2x4[2]", /* name */

		"#version 120\n"
		"varying mat2x4 r[2];" /* vs */
		"void main() {"
		"  gl_Position = ftransform();"
		"  r = mat2x4[2](mat2x4(0.666, 666.0, 999.0, -2.0, 0.5, -0.4, 30.0, 40.0),"
		"		 mat2x4(0.12, 0.24, 0.34, 0.56, 0.67, 0.78, 0.89, 0.04));"
		"}",

		2, /* num_varyings, varyings */
		{"r[0]", "r[1]"},

		true, /* is_floating_point */
		{16}, /* num_elements, expected_float, expected_int */
		{{0.666, 666, 999, -2, 0.5, -0.4, 30, 40, 0.12, 0.24, 0.34, 0.56, 0.67, 0.78, 0.89, 0.04}}
	},
	{
		"mat2x4[2]-no-subscript", /* name */

		"#version 120\n"
		"varying mat2x4 r[2];" /* vs */
		"void main() {"
		"  gl_Position = ftransform();"
		"  r = mat2x4[2](mat2x4(0.666, 666.0, 999.0, -2.0, 0.5, -0.4, 30.0, 40.0),"
		"		 mat2x4(0.12, 0.24, 0.34, 0.56, 0.67, 0.78, 0.89, 0.04));"
		"}",

		1, /* num_varyings, varyings */
		{"r"},

		true, /* is_floating_point */
		{16}, /* num_elements, expected_float, expected_int */
		{{0.666, 666, 999, -2, 0.5, -0.4, 30, 40, 0.12, 0.24, 0.34, 0.56, 0.67, 0.78, 0.89, 0.04}}
	},
	{
		"mat3x2", /* name */

		"#version 120\n"
		"varying mat3x2 r;" /* vs */
		"void main() {"
		"  gl_Position = ftransform();"
		"  r = mat3x2(0.666, 666.0, 999.0,"
		"           -2.0, 0.2, 5.0);"
		"}",

		1, /* num_varyings, varyings */
		{"r"},

		true, /* is_floating_point */
		{6}, /* num_elements, expected_float, expected_int */
		{{0.666, 666.0, 999.0, -2.0, 0.2, 5.0}}
	},
	{
		"mat3x2[2]", /* name */

		"#version 120\n"
		"varying mat3x2 r[2];" /* vs */
		"void main() {"
		"  gl_Position = ftransform();"
		"  r = mat3x2[2](mat3x2(0.666, 666.0, 999.0, -2.0, 0.2, 5.0),"
		"		 mat3x2(0.98, 0.87, 0.76, 0.65, 0.54, 0.43));"
		"}",

		2, /* num_varyings, varyings */
		{"r[0]", "r[1]"},

		true, /* is_floating_point */
		{12}, /* num_elements, expected_float, expected_int */
		{{0.666, 666.0, 999.0, -2.0, 0.2, 5.0, 0.98, 0.87, 0.76, 0.65, 0.54, 0.43}}
	},
	{
		"mat3x2[2]-no-subscript", /* name */

		"#version 120\n"
		"varying mat3x2 r[2];" /* vs */
		"void main() {"
		"  gl_Position = ftransform();"
		"  r = mat3x2[2](mat3x2(0.666, 666.0, 999.0, -2.0, 0.2, 5.0),"
		"		 mat3x2(0.98, 0.87, 0.76, 0.65, 0.54, 0.43));"
		"}",

		1, /* num_varyings, varyings */
		{"r"},

		true, /* is_floating_point */
		{12}, /* num_elements, expected_float, expected_int */
		{{0.666, 666.0, 999.0, -2.0, 0.2, 5.0, 0.98, 0.87, 0.76, 0.65, 0.54, 0.43}}
	},
	{
		"mat3", /* name */

		"#version 110\n"
		"varying mat3 r;" /* vs */
		"void main() {"
		"  gl_Position = ftransform();"
		"  r = mat3(0.666, 666.0, 999.0,"
		"           -2.0, 0.2, 5.0,"
		"           3.0, 0.3, -10.0);"
		"}",

		1, /* num_varyings, varyings */
		{"r"},

		true, /* is_floating_point */
		{9}, /* num_elements, expected_float, expected_int */
		{{0.666, 666.0, 999.0, -2.0, 0.2, 5.0, 3.0, 0.3, -10.0}}
	},
	{
		"mat3[2]", /* name */

		"#version 120\n"
		"varying mat3 r[2];" /* vs */
		"void main() {"
		"  gl_Position = ftransform();"
		"  r = mat3[2](mat3(0.666, 666.0, 999.0,"
		"                   -2.0, 0.2, 5.0,"
		"                   3.0, 0.3, -10.0),"
		"	       mat3(20.0, 10.0, 5.0, 90.0, -4.0, 3.4, -2.3, -8.6, 8.0));"
		"}",

		2, /* num_varyings, varyings */
		{"r[0]", "r[1]"},

		true, /* is_floating_point */
		{18}, /* num_elements, expected_float, expected_int */
		{{0.666, 666.0, 999.0, -2.0, 0.2, 5.0, 3.0, 0.3, -10.0,
		 20.0, 10.0, 5.0, 90.0, -4.0, 3.4, -2.3, -8.6, 8.0}}
	},
	{
		"mat3[2]-no-subscript", /* name */

		"#version 120\n"
		"varying mat3 r[2];" /* vs */
		"void main() {"
		"  gl_Position = ftransform();"
		"  r = mat3[2](mat3(0.666, 666.0, 999.0,"
		"                   -2.0, 0.2, 5.0,"
		"                   3.0, 0.3, -10.0),"
		"	       mat3(20.0, 10.0, 5.0, 90.0, -4.0, 3.4, -2.3, -8.6, 8.0));"
		"}",

		1, /* num_varyings, varyings */
		{"r"},

		true, /* is_floating_point */
		{18}, /* num_elements, expected_float, expected_int */
		{{0.666, 666.0, 999.0, -2.0, 0.2, 5.0, 3.0, 0.3, -10.0,
		 20.0, 10.0, 5.0, 90.0, -4.0, 3.4, -2.3, -8.6, 8.0}}
	},
	{
		"mat3x4", /* name */

		"#version 120\n"
		"varying mat3x4 r;" /* vs */
		"void main() {"
		"  gl_Position = ftransform();"
		"  r = mat3x4(0.666, 666.0, 999.0,"
		"             -2.0, 0.2, 5.0,"
		"             3.0, 0.3, -10.0,"
		"             0.4, -4.1, -5.9);"
		"}",

		1, /* num_varyings, varyings */
		{"r"},

		true, /* is_floating_point */
		{12}, /* num_elements, expected_float, expected_int */
		{{0.666, 666.0, 999.0, -2.0, 0.2, 5.0, 3.0, 0.3, -10.0, 0.4, -4.1, -5.9}}
	},
	{
		"mat3x4[2]", /* name */

		"#version 120\n"
		"varying mat3x4 r[2];" /* vs */
		"void main() {"
		"  gl_Position = ftransform();"
		"  r = mat3x4[2](mat3x4(0.666, 666.0, 999.0, -2.0, 0.2, 5.0,"
		"                       3.0, 0.3, -10.0, 0.4, -4.1, -5.9),"
		"		 mat3x4(20.0, 10.0, 5.0, 90.0, -4.0, 3.4,"
		"                       -2.3, -8.6, 8.0, 0.4, -4.1, -5.9));"
		"}",

		2, /* num_varyings, varyings */
		{"r[0]", "r[1]"},

		true, /* is_floating_point */
		{24}, /* num_elements, expected_float, expected_int */
		{{0.666, 666.0, 999.0, -2.0, 0.2, 5.0, 3.0, 0.3, -10.0, 0.4, -4.1, -5.9,
		 20.0, 10.0, 5.0, 90.0, -4.0, 3.4, -2.3, -8.6, 8.0, 0.4, -4.1, -5.9}}
	},
	{
		"mat3x4[2]-no-subscript", /* name */

		"#version 120\n"
		"varying mat3x4 r[2];" /* vs */
		"void main() {"
		"  gl_Position = ftransform();"
		"  r = mat3x4[2](mat3x4(0.666, 666.0, 999.0, -2.0, 0.2, 5.0,"
		"                       3.0, 0.3, -10.0, 0.4, -4.1, -5.9),"
		"		 mat3x4(20.0, 10.0, 5.0, 90.0, -4.0, 3.4,"
		"                       -2.3, -8.6, 8.0, 0.4, -4.1, -5.9));"
		"}",

		1, /* num_varyings, varyings */
		{"r"},

		true, /* is_floating_point */
		{24}, /* num_elements, expected_float, expected_int */
		{{0.666, 666.0, 999.0, -2.0, 0.2, 5.0, 3.0, 0.3, -10.0, 0.4, -4.1, -5.9,
		 20.0, 10.0, 5.0, 90.0, -4.0, 3.4, -2.3, -8.6, 8.0, 0.4, -4.1, -5.9}}
	},
	{
		"mat4x2", /* name */

		"#version 120\n"
		"varying mat4x2 r;" /* vs */
		"void main() {"
		"  gl_Position = ftransform();"
		"  r = mat4x2(0.666, 666.0, 999.0, -2.0, 0.5, -0.4, 30.0, 40.0);"
		"}",

		1, /* num_varyings, varyings */
		{"r"},

		true, /* is_floating_point */
		{8}, /* num_elements, expected_float, expected_int */
		{{0.666, 666, 999, -2, 0.5, -0.4, 30, 40}}
	},
	{
		"mat4x2[2]", /* name */

		"#version 120\n"
		"varying mat4x2 r[2];" /* vs */
		"void main() {"
		"  gl_Position = ftransform();"
		"  r = mat4x2[2](mat4x2(0.666, 666.0, 999.0, -2.0, 0.5, -0.4, 30.0, 40.0),"
		"		 mat4x2(20.0, 10.0, 5.0, 90.0, -4.0, 3.4, -2.3, -8.6));"
		"}",

		2, /* num_varyings, varyings */
		{"r[0]", "r[1]"},

		true, /* is_floating_point */
		{16}, /* num_elements, expected_float, expected_int */
		{{0.666, 666, 999, -2, 0.5, -0.4, 30, 40,
		 20.0, 10.0, 5.0, 90.0, -4.0, 3.4, -2.3, -8.6}}
	},
	{
		"mat4x2[2]-no-subscript", /* name */

		"#version 120\n"
		"varying mat4x2 r[2];" /* vs */
		"void main() {"
		"  gl_Position = ftransform();"
		"  r = mat4x2[2](mat4x2(0.666, 666.0, 999.0, -2.0, 0.5, -0.4, 30.0, 40.0),"
		"		 mat4x2(20.0, 10.0, 5.0, 90.0, -4.0, 3.4, -2.3, -8.6));"
		"}",

		1, /* num_varyings, varyings */
		{"r"},

		true, /* is_floating_point */
		{16}, /* num_elements, expected_float, expected_int */
		{{0.666, 666, 999, -2, 0.5, -0.4, 30, 40,
		 20.0, 10.0, 5.0, 90.0, -4.0, 3.4, -2.3, -8.6}}
	},
	{
		"mat4x3", /* name */

		"#version 120\n"
		"varying mat4x3 r;" /* vs */
		"void main() {"
		"  gl_Position = ftransform();"
		"  r = mat4x3(0.666, 666.0, 999.0, -2.0,"
		"             0.5, -0.4, 30.0, 40.0,"
		"             0.3, 0.2, 0.1, 0.4);"
		"}",

		1, /* num_varyings, varyings */
		{"r"},

		true, /* is_floating_point */
		{12}, /* num_elements, expected_float, expected_int */
		{{0.666, 666, 999, -2, 0.5, -0.4, 30, 40, 0.3, 0.2, 0.1, 0.4}}
	},
	{
		"mat4x3[2]", /* name */

		"#version 120\n"
		"varying mat4x3 r[2];" /* vs */
		"void main() {"
		"  gl_Position = ftransform();"
		"  r = mat4x3[2](mat4x3(0.666, 666.0, 999.0, -2.0,"
		"                       0.5, -0.4, 30.0, 40.0,"
		"                       0.3, 0.2, 0.1, 0.4),"
		"		 mat4x3(20.0, 10.0, 5.0, 90.0, -4.0, 3.4,"
		"                       -2.3, -8.6, 8.0, 0.4, -4.1, -5.9));"
		"}",

		2, /* num_varyings, varyings */
		{"r[0]", "r[1]"},

		true, /* is_floating_point */
		{24}, /* num_elements, expected_float, expected_int */
		{{0.666, 666, 999, -2, 0.5, -0.4, 30, 40, 0.3, 0.2, 0.1, 0.4,
		 20.0, 10.0, 5.0, 90.0, -4.0, 3.4, -2.3, -8.6, 8.0, 0.4, -4.1, -5.9}}
	},
	{
		"mat4x3[2]-no-subscript", /* name */

		"#version 120\n"
		"varying mat4x3 r[2];" /* vs */
		"void main() {"
		"  gl_Position = ftransform();"
		"  r = mat4x3[2](mat4x3(0.666, 666.0, 999.0, -2.0,"
		"                       0.5, -0.4, 30.0, 40.0,"
		"                       0.3, 0.2, 0.1, 0.4),"
		"		 mat4x3(20.0, 10.0, 5.0, 90.0, -4.0, 3.4,"
		"                       -2.3, -8.6, 8.0, 0.4, -4.1, -5.9));"
		"}",

		1, /* num_varyings, varyings */
		{"r"},

		true, /* is_floating_point */
		{24}, /* num_elements, expected_float, expected_int */
		{{0.666, 666, 999, -2, 0.5, -0.4, 30, 40, 0.3, 0.2, 0.1, 0.4,
		 20.0, 10.0, 5.0, 90.0, -4.0, 3.4, -2.3, -8.6, 8.0, 0.4, -4.1, -5.9}}
	},
	{
		"mat4", /* name */

		"#version 110\n"
		"varying mat4 r;" /* vs */
		"void main() {"
		"  gl_Position = ftransform();"
		"  r = mat4(0.666, 666.0, 999.0, -2.0,"
		"           0.2, 5.0, 3.0, 0.3,"
		"           -10.0, 20.1, 52.4, -34.3,"
		"           45.0, 56.0, 67.0, 78.0);"
		"}",

		1, /* num_varyings, varyings */
		{"r"},

		true, /* is_floating_point */
		{16}, /* num_elements, expected_float, expected_int */
		{{0.666, 666.0, 999.0, -2.0,
		 0.2, 5.0, 3.0, 0.3,
		 -10.0, 20.1, 52.4, -34.3,
		 45.0, 56.0, 67.0, 78.0}}
	},
	{
		"mat4[2]", /* name */

		"#version 120\n"
		"varying mat4 r[2];" /* vs */
		"void main() {"
		"  gl_Position = ftransform();"
		"  r = mat4[2](mat4(0.666, 666.0, 999.0, -2.0, 0.2, 5.0, 3.0, 0.3,"
		"                   -10.0, 20.1, 52.4, -34.3, 45.0, 56.0, 67.0, 78.0),"
		"	       mat4(20.0, 10.0, 5.0, 90.0, -4.0, 3.4, -2.3, -8.6,"
		"                   8.0, 0.4, -4.1, -5.9, -10.0, 0.4, -4.1, -5.9));"
		"}",

		2, /* num_varyings, varyings */
		{"r[0]", "r[1]"},

		true, /* is_floating_point */
		{32}, /* num_elements, expected_float, expected_int */
		{{0.666, 666.0, 999.0, -2.0,
		 0.2, 5.0, 3.0, 0.3,
		 -10.0, 20.1, 52.4, -34.3,
		 45.0, 56.0, 67.0, 78.0,
		 20.0, 10.0, 5.0, 90.0, -4.0, 3.4, -2.3, -8.6,
		 8.0, 0.4, -4.1, -5.9, -10.0, 0.4, -4.1, -5.9}}
	},
	{
		"mat4[2]-no-subscript", /* name */

		"#version 120\n"
		"varying mat4 r[2];" /* vs */
		"void main() {"
		"  gl_Position = ftransform();"
		"  r = mat4[2](mat4(0.666, 666.0, 999.0, -2.0, 0.2, 5.0, 3.0, 0.3,"
		"                   -10.0, 20.1, 52.4, -34.3, 45.0, 56.0, 67.0, 78.0),"
		"	       mat4(20.0, 10.0, 5.0, 90.0, -4.0, 3.4, -2.3, -8.6,"
		"                   8.0, 0.4, -4.1, -5.9, -10.0, 0.4, -4.1, -5.9));"
		"}",

		1, /* num_varyings, varyings */
		{"r"},

		true, /* is_floating_point */
		{32}, /* num_elements, expected_float, expected_int */
		{{0.666, 666.0, 999.0, -2.0,
		 0.2, 5.0, 3.0, 0.3,
		 -10.0, 20.1, 52.4, -34.3,
		 45.0, 56.0, 67.0, 78.0,
		 20.0, 10.0, 5.0, 90.0, -4.0, 3.4, -2.3, -8.6,
		 8.0, 0.4, -4.1, -5.9, -10.0, 0.4, -4.1, -5.9}}
	},
	{
		"int", /* name */

		"#version 130\n"
		"flat out int r;" /* vs */
		"void main() {"
		"  gl_Position = ftransform();"
		"  r = 2145948354;"
		"}",

		1, /* num_varyings, varyings */
		{"r"},

		false, /* is_floating_point */
		{1}, /* num_elements, expected_float, expected_int */
		{{0}}, {{2145948354}}
	},
	{
		"int[2]", /* name */

		"#version 130\n"
		"flat out int[2] r;" /* vs */
		"void main() {"
		"  gl_Position = ftransform();"
		"  r = int[2](-362245257,"
		"             2074398469);"
		"}",

		2, /* num_varyings, varyings */
		{"r[0]", "r[1]"},

		false, /* is_floating_point */
		{2}, /* num_elements, expected_float, expected_int */
		{{0}}, {{-362245257, 2074398469}}
	},
	{
		"int[2]-no-subscript", /* name */

		"#version 130\n"
		"flat out int[2] r;" /* vs */
		"void main() {"
		"  gl_Position = ftransform();"
		"  r = int[2](-362245257,"
		"             2074398469);"
		"}",

		1, /* num_varyings, varyings */
		{"r"},

		false, /* is_floating_point */
		{2}, /* num_elements, expected_float, expected_int */
		{{0}}, {{-362245257, 2074398469}}
	},
	{
		"ivec2", /* name */

		"#version 130\n"
		"flat out ivec2 r;" /* vs */
		"void main() {"
		"  gl_Position = ftransform();"
		"  r = ivec2(408918569, -69869318);"
		"}",

		1, /* num_varyings, varyings */
		{"r"},

		false, /* is_floating_point */
		{2}, /* num_elements, expected_float, expected_int */
		{{0}}, {{408918569, -69869318}}
	},
	{
		"ivec2[2]", /* name */

		"#version 130\n"
		"flat out ivec2[2] r;" /* vs */
		"void main() {"
		"  gl_Position = ftransform();"
		"  r = ivec2[2](ivec2(5703639, 654049542),"
		"               ivec2(82927237, -1489678625));"
		"}",

		2, /* num_varyings, varyings */
		{"r[0]", "r[1]"},

		false, /* is_floating_point */
		{4}, /* num_elements, expected_float, expected_int */
		{{0}}, {{5703639, 654049542, 82927237, -1489678625}}
	},
	{
		"ivec2[2]-no-subscript", /* name */

		"#version 130\n"
		"flat out ivec2[2] r;" /* vs */
		"void main() {"
		"  gl_Position = ftransform();"
		"  r = ivec2[2](ivec2(5703639, 654049542),"
		"               ivec2(82927237, -1489678625));"
		"}",

		1, /* num_varyings, varyings */
		{"r"},

		false, /* is_floating_point */
		{4}, /* num_elements, expected_float, expected_int */
		{{0}}, {{5703639, 654049542, 82927237, -1489678625}}
	},
	{
		"ivec3", /* name */

		"#version 130\n"
		"flat out ivec3 r;" /* vs */
		"void main() {"
		"  gl_Position = ftransform();"
		"  r = ivec3(1402620337, -931103284, -1922128750);"
		"}",

		1, /* num_varyings, varyings */
		{"r"},

		false, /* is_floating_point */
		{3}, /* num_elements, expected_float, expected_int */
		{{0}}, {{1402620337, -931103284, -1922128750}}
	},
	{
		"ivec3[2]", /* name */

		"#version 130\n"
		"flat out ivec3[2] r;" /* vs */
		"void main() {"
		"  gl_Position = ftransform();"
		"  r = ivec3[2](ivec3(819762795, 292214138, 207695021),"
		"               ivec3(-541769145, -896550370, -322088831));"
		"}",

		2, /* num_varyings, varyings */
		{"r[0]", "r[1]"},

		false, /* is_floating_point */
		{6}, /* num_elements, expected_float, expected_int */
		{{0}}, {{819762795, 292214138, 207695021,
		     -541769145, -896550370, -322088831}}
	},
	{
		"ivec3[2]-no-subscript", /* name */

		"#version 130\n"
		"flat out ivec3[2] r;" /* vs */
		"void main() {"
		"  gl_Position = ftransform();"
		"  r = ivec3[2](ivec3(819762795, 292214138, 207695021),"
		"               ivec3(-541769145, -896550370, -322088831));"
		"}",

		1, /* num_varyings, varyings */
		{"r"},

		false, /* is_floating_point */
		{6}, /* num_elements, expected_float, expected_int */
		{{0}}, {{819762795, 292214138, 207695021,
		     -541769145, -896550370, -322088831}}
	},
	{
		"ivec4", /* name */

		"#version 130\n"
		"flat out ivec4 r;" /* vs */
		"void main() {"
		"  gl_Position = ftransform();"
		"  r = ivec4(1979209158, -791559088, -992849733, -59981678);"
		"}",

		1, /* num_varyings, varyings */
		{"r"},

		false, /* is_floating_point */
		{4}, /* num_elements, expected_float, expected_int */
		{{0}}, {{1979209158, -791559088, -992849733, -59981678}}
	},
	{
		"ivec4[2]", /* name */

		"#version 130\n"
		"flat out ivec4[2] r;" /* vs */
		"void main() {"
		"  gl_Position = ftransform();"
		"  r = ivec4[2](ivec4(-764612129, 395402837, -1260359913, 936205122),"
		"               ivec4(-1510453781, -707590649, -760434930, -1756396083));"
		"}",

		2, /* num_varyings, varyings */
		{"r[0]", "r[1]"},

		false, /* is_floating_point */
		{8}, /* num_elements, expected_float, expected_int */
		{{0}}, {{-764612129, 395402837, -1260359913, 936205122,
		     -1510453781, -707590649, -760434930, -1756396083}}
	},
	{
		"ivec4[2]-no-subscript", /* name */

		"#version 130\n"
		"flat out ivec4[2] r;" /* vs */
		"void main() {"
		"  gl_Position = ftransform();"
		"  r = ivec4[2](ivec4(-764612129, 395402837, -1260359913, 936205122),"
		"               ivec4(-1510453781, -707590649, -760434930, -1756396083));"
		"}",

		1, /* num_varyings, varyings */
		{"r"},

		false, /* is_floating_point */
		{8}, /* num_elements, expected_float, expected_int */
		{{0}}, {{-764612129, 395402837, -1260359913, 936205122,
		     -1510453781, -707590649, -760434930, -1756396083}}
	},
	{
		"uint", /* name */

		"#version 130\n"
		"flat out uint r;" /* vs */
		"void main() {"
		"  gl_Position = ftransform();"
		"  r = 2230472931u;"
		"}",

		1, /* num_varyings, varyings */
		{"r"},

		false, /* is_floating_point */
		{1}, /* num_elements, expected_float, expected_int */
		{{0}}, {{2230472931u}}
	},
	{
		"uint[2]", /* name */

		"#version 130\n"
		"flat out uint[2] r;" /* vs */
		"void main() {"
		"  gl_Position = ftransform();"
		"  r = uint[2](4073369952u,"
		"              1026348970u);"
		"}",

		2, /* num_varyings, varyings */
		{"r[0]", "r[1]"},

		false, /* is_floating_point */
		{2}, /* num_elements, expected_float, expected_int */
		{{0}}, {{4073369952u, 1026348970u}}
	},
	{
		"uint[2]-no-subscript", /* name */

		"#version 130\n"
		"flat out uint[2] r;" /* vs */
		"void main() {"
		"  gl_Position = ftransform();"
		"  r = uint[2](4073369952u,"
		"              1026348970u);"
		"}",

		1, /* num_varyings, varyings */
		{"r"},

		false, /* is_floating_point */
		{2}, /* num_elements, expected_float, expected_int */
		{{0}}, {{4073369952u, 1026348970u}}
	},
	{
		"uvec2", /* name */

		"#version 130\n"
		"flat out uvec2 r;" /* vs */
		"void main() {"
		"  gl_Position = ftransform();"
		"  r = uvec2(1214092884u, 3587337147u);"
		"}",

		1, /* num_varyings, varyings */
		{"r"},

		false, /* is_floating_point */
		{2}, /* num_elements, expected_float, expected_int */
		{{0}}, {{1214092884u, 3587337147u}}
	},
	{
		"uvec2[2]", /* name */

		"#version 130\n"
		"flat out uvec2[2] r;" /* vs */
		"void main() {"
		"  gl_Position = ftransform();"
		"  r = uvec2[2](uvec2(1011258288u, 684916166u),"
		"               uvec2(381807053u, 3306523233u));"
		"}",

		2, /* num_varyings, varyings */
		{"r[0]", "r[1]"},

		false, /* is_floating_point */
		{4}, /* num_elements, expected_float, expected_int */
		{{0}}, {{1011258288u, 684916166u, 381807053u, 3306523233u}}
	},
	{
		"uvec2[2]-no-subscript", /* name */

		"#version 130\n"
		"flat out uvec2[2] r;" /* vs */
		"void main() {"
		"  gl_Position = ftransform();"
		"  r = uvec2[2](uvec2(1011258288u, 684916166u),"
		"               uvec2(381807053u, 3306523233u));"
		"}",

		1, /* num_varyings, varyings */
		{"r"},

		false, /* is_floating_point */
		{4}, /* num_elements, expected_float, expected_int */
		{{0}}, {{1011258288u, 684916166u, 381807053u, 3306523233u}}
	},
	{
		"uvec3", /* name */

		"#version 130\n"
		"flat out uvec3 r;" /* vs */
		"void main() {"
		"  gl_Position = ftransform();"
		"  r = uvec3(1076370307u, 1186562996u, 3616039281u);"
		"}",

		1, /* num_varyings, varyings */
		{"r"},

		false, /* is_floating_point */
		{3}, /* num_elements, expected_float, expected_int */
		{{0}}, {{1076370307u, 1186562996u, 3616039281u}}
	},
	{
		"uvec3[2]", /* name */

		"#version 130\n"
		"flat out uvec3[2] r;" /* vs */
		"void main() {"
		"  gl_Position = ftransform();"
		"  r = uvec3[2](uvec3(2984731006u, 2324137892u, 876349448u),"
		"               uvec3(2493082028u, 1481747175u, 1530233730u));"
		"}",

		2, /* num_varyings, varyings */
		{"r[0]", "r[1]"},

		false, /* is_floating_point */
		{6}, /* num_elements, expected_float, expected_int */
		{{0}}, {{2984731006u, 2324137892u, 876349448u,
		     2493082028u, 1481747175u, 1530233730u}}
	},
	{
		"uvec3[2]-no-subscript", /* name */

		"#version 130\n"
		"flat out uvec3[2] r;" /* vs */
		"void main() {"
		"  gl_Position = ftransform();"
		"  r = uvec3[2](uvec3(2984731006u, 2324137892u, 876349448u),"
		"               uvec3(2493082028u, 1481747175u, 1530233730u));"
		"}",

		1, /* num_varyings, varyings */
		{"r"},

		false, /* is_floating_point */
		{6}, /* num_elements, expected_float, expected_int */
		{{0}}, {{2984731006u, 2324137892u, 876349448u,
		     2493082028u, 1481747175u, 1530233730u}}
	},
	{
		"uvec4", /* name */

		"#version 130\n"
		"flat out uvec4 r;" /* vs */
		"void main() {"
		"  gl_Position = ftransform();"
		"  r = uvec4(3046379279u, 3265138790u, 4109383147u, 2654056480u);"
		"}",

		1, /* num_varyings, varyings */
		{"r"},

		false, /* is_floating_point */
		{4}, /* num_elements, expected_float, expected_int */
		{{0}}, {{3046379279u, 3265138790u, 4109383147u, 2654056480u}}
	},
	{
		"uvec4[2]", /* name */

		"#version 130\n"
		"flat out uvec4[2] r;" /* vs */
		"void main() {"
		"  gl_Position = ftransform();"
		"  r = uvec4[2](uvec4(2563680931u, 754130007u, 230209823u, 707580188u),"
		"               uvec4(3015681429u, 3850948302u, 2224673498u, 2376088107u));"
		"}",

		2, /* num_varyings, varyings */
		{"r[0]", "r[1]"},

		false, /* is_floating_point */
		{8}, /* num_elements, expected_float, expected_int */
		{{0}}, {{2563680931u, 754130007u, 230209823u, 707580188u,
		     3015681429u, 3850948302u, 2224673498u, 2376088107u}}
	},
	{
		"uvec4[2]-no-subscript", /* name */

		"#version 130\n"
		"flat out uvec4[2] r;" /* vs */
		"void main() {"
		"  gl_Position = ftransform();"
		"  r = uvec4[2](uvec4(2563680931u, 754130007u, 230209823u, 707580188u),"
		"               uvec4(3015681429u, 3850948302u, 2224673498u, 2376088107u));"
		"}",

		1, /* num_varyings, varyings */
		{"r"},

		false, /* is_floating_point */
		{8}, /* num_elements, expected_float, expected_int */
		{{0}}, {{2563680931u, 754130007u, 230209823u, 707580188u,
		     3015681429u, 3850948302u, 2224673498u, 2376088107u}}
	},
	{
		"gl_NextBuffer-1", /* name */

		"#version 120\n" /* vs */
		"varying float r[2];"
		"void main() {"
		"  gl_Position = ftransform();"
		"  r = float[2](0.4, 0.5);"
		"}",

		3, /* num_varyings, varyings */
		{"r[0]", "gl_NextBuffer", "r[1]"},

		true, /* is_floating_point */
		{1, 1}, /* num_elements, expected_float, expected_int */
		{{0.4}, {0.5}}, {{0}},
		true /* is transform_feedback3 */
	},
	{
		"gl_NextBuffer-2", /* name */

		"#version 120\n" /* vs */
		"varying vec2 a;"
		"varying vec3 b;"
		"varying float c;"
		"varying vec3 d;"
		"varying vec4 e;"
		"varying vec3 f;"
		"varying vec4 g, h;"
		"void main() {"
		"  gl_Position = ftransform();"
		"  a = vec2(0.4, 0.5);"
		"  b = vec3(2.0, 3.0, 4.0);"
		"  c = 0.011;"
		"  d = vec3(0.35, 0.98, 0.59);"
		"  e = vec4(5.4, 34.4, 2.3, 9.6);"
		"  f = vec3(4.3, 6.2, 9.4);"
		"  g = vec4(3.4, 9.6, 3.7, 9.3);"
		"  h = vec4(8.1, 3.9, 3.6, 6.6);"
		"}",

		11, /* num_varyings, varyings */
		{"a", "b", "gl_NextBuffer", "c", "d", "gl_NextBuffer", "e", "gl_NextBuffer", "f", "g", "h"},

		true, /* is_floating_point */
		{5, 4, 4, 11}, /* num_elements, expected_float, expected_int */
		{{0.4, 0.5, 2.0, 3.0, 4.0}, {0.011, 0.35, 0.98, 0.59}, {5.4, 34.4, 2.3, 9.6},
		 {4.3, 6.2, 9.4, 3.4, 9.6, 3.7, 9.3, 8.1, 3.9, 3.6, 6.6}}, {{0}},
		true /* is transform_feedback3 */
	},
	{
		"gl_SkipComponents1-1", /* name */

		"#version 120\n" /* vs */
		"varying float r[2];"
		"void main() {"
		"  gl_Position = ftransform();"
		"  r = float[2](0.4, 0.5);"
		"}",

		2, /* num_varyings, varyings */
		{"r[0]", "gl_SkipComponents1"},

		true, /* is_floating_point */
		{2}, /* num_elements, expected_float, expected_int */
		{{0.4, DEFAULT_VALUE}}, {{0}},
		true /* is transform_feedback3 */
	},
	{
		"gl_SkipComponents1-2", /* name */

		"#version 120\n" /* vs */
		"varying float r[2];"
		"void main() {"
		"  gl_Position = ftransform();"
		"  r = float[2](0.4, 0.5);"
		"}",

		2, /* num_varyings, varyings */
		{"gl_SkipComponents1", "r[1]"},

		true, /* is_floating_point */
		{2}, /* num_elements, expected_float, expected_int */
		{{DEFAULT_VALUE, 0.5}}, {{0}},
		true /* is transform_feedback3 */
	},
	{
		"gl_SkipComponents1-3", /* name */

		"#version 120\n" /* vs */
		"varying float r[2];"
		"void main() {"
		"  gl_Position = ftransform();"
		"  r = float[2](0.4, 0.5);"
		"}",

		3, /* num_varyings, varyings */
		{"r[0]", "gl_SkipComponents1", "r[1]"},

		true, /* is_floating_point */
		{3}, /* num_elements, expected_float, expected_int */
		{{0.4, DEFAULT_VALUE, 0.5}}, {{0}},
		true /* is transform_feedback3 */
	},
	{
		"gl_SkipComponents2", /* name */

		"#version 120\n" /* vs */
		"varying float r[2];"
		"void main() {"
		"  gl_Position = ftransform();"
		"  r = float[2](0.4, 0.5);"
		"}",

		3, /* num_varyings, varyings */
		{"r[0]", "gl_SkipComponents2", "r[1]"},

		true, /* is_floating_point */
		{4}, /* num_elements, expected_float, expected_int */
		{{0.4, DEFAULT_VALUE, DEFAULT_VALUE, 0.5}}, {{0}},
		true /* is transform_feedback3 */
	},
	{
		"gl_SkipComponents3", /* name */

		"#version 120\n" /* vs */
		"varying float r[2];"
		"void main() {"
		"  gl_Position = ftransform();"
		"  r = float[2](0.4, 0.5);"
		"}",

		3, /* num_varyings, varyings */
		{"r[0]", "gl_SkipComponents3", "r[1]"},

		true, /* is_floating_point */
		{5}, /* num_elements, expected_float, expected_int */
		{{0.4, DEFAULT_VALUE, DEFAULT_VALUE, DEFAULT_VALUE, 0.5}}, {{0}},
		true /* is transform_feedback3 */
	},
	{
		"gl_SkipComponents4", /* name */

		"#version 120\n" /* vs */
		"varying float r[2];"
		"void main() {"
		"  gl_Position = ftransform();"
		"  r = float[2](0.4, 0.5);"
		"}",

		3, /* num_varyings, varyings */
		{"r[0]", "gl_SkipComponents4", "r[1]"},

		true, /* is_floating_point */
		{6}, /* num_elements, expected_float, expected_int */
		{{0.4, DEFAULT_VALUE, DEFAULT_VALUE, DEFAULT_VALUE, DEFAULT_VALUE, 0.5}}, {{0}},
		true /* is transform_feedback3 */
	},
	{
		"gl_SkipComponents1-gl_NextBuffer", /* name */

		"#version 120\n" /* vs */
		"varying float r[2];"
		"void main() {"
		"  gl_Position = ftransform();"
		"  r = float[2](0.4, 0.5);"
		"}",

		4, /* num_varyings, varyings */
		{"r[0]", "gl_SkipComponents1", "gl_NextBuffer", "r[1]"},

		true, /* is_floating_point */
		{2, 1}, /* num_elements, expected_float, expected_int */
		{{0.4, DEFAULT_VALUE}, {0.5}}, {{0}},
		true /* is transform_feedback3 */
	},
	{
		"gl_NextBuffer-gl_SkipComponents1-gl_NextBuffer", /* name */

		"#version 120\n" /* vs */
		"varying float r[2];"
		"void main() {"
		"  gl_Position = ftransform();"
		"  r = float[2](0.4, 0.5);"
		"}",

		5, /* num_varyings, varyings */
		{"r[0]", "gl_NextBuffer", "gl_SkipComponents1", "gl_NextBuffer", "r[1]"},

		true, /* is_floating_point */
		{1, 1, 1}, /* num_elements, expected_float, expected_int */
		{{0.4}, {DEFAULT_VALUE}, {0.5}}, {{0}},
		true /* is transform_feedback3 */
	},
	{
		"gl_NextBuffer-gl_NextBuffer", /* name */

		"#version 120\n" /* vs */
		"varying float r[2];"
		"void main() {"
		"  gl_Position = ftransform();"
		"  r = float[2](0.4, 0.5);"
		"}",

		4, /* num_varyings, varyings */
		{"r[0]", "gl_NextBuffer", "gl_NextBuffer", "r[1]"},

		true, /* is_floating_point */
		{1, 1, 1}, /* num_elements, expected_float, expected_int */
		{{0.4}, {DEFAULT_VALUE}, {0.5}}, {{0}},
		true /* is transform_feedback3 */
	},
	{
		"gl_SkipComponents1234", /* name */

		"#version 120\n" /* vs */
		"varying float r[2];"
		"void main() {"
		"  gl_Position = ftransform();"
		"  r = float[2](0.4, 0.5);"
		"}",

		6, /* num_varyings, varyings */
		{"r[0]", "gl_SkipComponents1", "gl_SkipComponents2", "gl_SkipComponents3", "gl_SkipComponents4", "r[1]"},

		true, /* is_floating_point */
		{12}, /* num_elements, expected_float, expected_int */
		{{0.4,
		  DEFAULT_VALUE,
		  DEFAULT_VALUE, DEFAULT_VALUE,
		  DEFAULT_VALUE, DEFAULT_VALUE, DEFAULT_VALUE,
		  DEFAULT_VALUE, DEFAULT_VALUE, DEFAULT_VALUE, DEFAULT_VALUE,
		  0.5}}, {{0}},
		true /* is transform_feedback3 */
	},

	{NULL}
};
struct test_desc *test;

GLuint buf[4];
GLuint prog;

#define NUM_VERTICES 3

void piglit_init(int argc, char **argv)
{
	GLuint vs;
	unsigned i,j;
	int maxcomps;
	float *data;

	/* Parse params. */
	for (i = 1; i < argc; i++) {
		struct test_desc *t;

		for (t = tests; t->name; t++) {
			if (!strcmp(argv[i], t->name)) {
				test = t;
				goto test_ready;
			}
		}
		fprintf(stderr, "Unknown test name.\n");
		exit(1);
	}
	test = &tests[0];
test_ready:

	printf("Testing type: %s\n", test->name);

	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);

	/* Check the driver. */
	piglit_require_gl_version(15);
	piglit_require_GLSL();
	piglit_require_transform_feedback();
	if (!test->is_floating_point)
		piglit_require_GLSL_version(130);
	if (test->is_transform_feedback3)
		piglit_require_extension("GL_ARB_transform_feedback3");

	glGetIntegerv(GL_MAX_TRANSFORM_FEEDBACK_INTERLEAVED_COMPONENTS, &maxcomps);
	for (i = 0; i < MAX_BUFFERS; i++) {
		if (maxcomps < test->num_elements[i]) {
			piglit_report_result(PIGLIT_SKIP);
		}
	}

	/* Create shaders. */
	vs = piglit_compile_shader_text(GL_VERTEX_SHADER, test->vs);
	prog = glCreateProgram();
	glAttachShader(prog, vs);
	glTransformFeedbackVaryings(prog, test->num_varyings,
				    test->varyings,
				    GL_INTERLEAVED_ATTRIBS_EXT);
	glLinkProgram(prog);
	if (!piglit_link_check_status(prog)) {
		glDeleteProgram(prog);
		piglit_report_result(PIGLIT_FAIL);
	}

	glGenBuffers(MAX_BUFFERS, buf);

	for (j = 0; j < MAX_BUFFERS; j++) {
		if (!test->num_elements[j]) {
			continue;
		}
		if (test->is_transform_feedback3) {
			GLint maxbufs;
			glGetIntegerv(GL_MAX_TRANSFORM_FEEDBACK_BUFFERS, &maxbufs);
			if (j >= maxbufs) {
				piglit_report_result(PIGLIT_SKIP);
			}
		}

		/* Set up the transform feedback buffer. */
		data = malloc(test->num_elements[j]*NUM_VERTICES*sizeof(float));
		for (i = 0; i < test->num_elements[j]*NUM_VERTICES; i++) {
			data[i] = DEFAULT_VALUE;
		}

		glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER_EXT, buf[j]);
		glBufferData(GL_TRANSFORM_FEEDBACK_BUFFER_EXT,
			     test->num_elements[j]*NUM_VERTICES*sizeof(float),
			     data, GL_STREAM_READ);

		if (!piglit_check_gl_error(GL_NO_ERROR))
		        piglit_report_result(PIGLIT_FAIL);

		glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER_EXT, j, buf[j]);

		if (!piglit_check_gl_error(GL_NO_ERROR))
		        piglit_report_result(PIGLIT_FAIL);

		free(data);
	}

	glClearColor(0.2, 0.2, 0.2, 1.0);
	glEnableClientState(GL_VERTEX_ARRAY);
}

enum piglit_result piglit_display(void)
{
	GLboolean pass = GL_TRUE;
	void *ptr;
	float *ptr_float;
	GLint *ptr_int;
	unsigned i,j;
	static const float verts[NUM_VERTICES*2] = {
		10, 10,
		10, 20,
		20, 20
	};

	glClear(GL_COLOR_BUFFER_BIT);

	/* Render into TFBO. */
	glLoadIdentity();
	glUseProgram(prog);
	glBeginTransformFeedback(GL_TRIANGLES);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glVertexPointer(2, GL_FLOAT, 0, verts);
	glDrawArrays(GL_TRIANGLES, 0, NUM_VERTICES);
	glEndTransformFeedback();

	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);

	for (j = 0; j < MAX_BUFFERS; j++) {
		if (!test->num_elements[j]) {
			continue;
		}

		glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER_EXT, buf[j]);
		ptr = glMapBuffer(GL_TRANSFORM_FEEDBACK_BUFFER_EXT, GL_READ_ONLY);
		ptr_float = ptr;
		ptr_int = ptr;

		for (i = 0; i < test->num_elements[j]*NUM_VERTICES; i++) {
			if (test->is_floating_point) {
				float value = test->expected_float[j][i % test->num_elements[j]];

				if (fabs(ptr_float[i] - value) > 0.01) {
					printf("Buffer[%i][%i]: %f,  Expected: %f\n", j, i,
					       ptr_float[i], value);
					pass = GL_FALSE;
				}
			} else {
				GLint value = test->expected_int[j][i % test->num_elements[j]];

				if (ptr_int[i] != value) {
					printf("Buffer[%i][%i]: %i,  Expected: %i\n", j, i,
					       ptr_int[i], value);
					pass = GL_FALSE;
				}
			}
		}
		glUnmapBuffer(GL_TRANSFORM_FEEDBACK_BUFFER_EXT);
		if (!piglit_check_gl_error(GL_NO_ERROR))
		        piglit_report_result(PIGLIT_FAIL);
	}

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}
