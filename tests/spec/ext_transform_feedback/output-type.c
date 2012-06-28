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

#include "piglit-util-gl-common.h"

PIGLIT_GL_TEST_MAIN(
    64 /*window_width*/,
    32 /*window_height*/,
    GLUT_DOUBLE | GLUT_RGB | GLUT_ALPHA)

struct test_desc {
	const char *name;
	const char *vs;
	unsigned num_varyings;
	const char *varyings[16];
	bool is_floating_point;
	unsigned num_elements;
	float expected_float[256];
	GLint expected_int[256];
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
		1, /* num_elements, expected_float, expected_int */
		{666}, {0}
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
		2, /* num_elements, expected_float, expected_int */
		{666, 0.123}, {0}
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
		2, /* num_elements, expected_float, expected_int */
		{666, 0.123}, {0}
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
		2, /* num_elements, expected_float, expected_int */
		{666, 999}, {0}
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
		4, /* num_elements, expected_float, expected_int */
		{666, 999, -1.5, -20.0}, {0}
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
		4, /* num_elements, expected_float, expected_int */
		{666, 999, -1.5, -20.0}, {0}
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
		3, /* num_elements, expected_float, expected_int */
		{666, 999, -2}, {0}
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
		6, /* num_elements, expected_float, expected_int */
		{666, 999, -2, 0.4, 1.4, 3.5}, {0}
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
		6, /* num_elements, expected_float, expected_int */
		{666, 999, -2, 0.4, 1.4, 3.5}, {0}
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
		4, /* num_elements, expected_float, expected_int */
		{0.666, 666, 999, -2}, {0}
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
		8, /* num_elements, expected_float, expected_int */
		{0.666, 666, 999, -2, 0.5, -0.4, 30.0, 40.0}, {0}
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
		8, /* num_elements, expected_float, expected_int */
		{0.666, 666, 999, -2, 0.5, -0.4, 30.0, 40.0}, {0}
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
		4, /* num_elements, expected_float, expected_int */
		{0.666, 666, 999, -2}, {0}
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
		8, /* num_elements, expected_float, expected_int */
		{0.666, 666, 999, -2, 0.34, 0.65, 0.14, -0.97}, {0}
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
		8, /* num_elements, expected_float, expected_int */
		{0.666, 666, 999, -2, 0.34, 0.65, 0.14, -0.97}, {0}
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
		6, /* num_elements, expected_float, expected_int */
		{0.666, 666, 999, -2, 0.5, -0.4}, {0}
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
		12, /* num_elements, expected_float, expected_int */
		{0.666, 666, 999, -2, 0.5, -0.4, 0.34, 0.12, -10.0, 30.1, 5.3, 9.8}, {0}
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
		12, /* num_elements, expected_float, expected_int */
		{0.666, 666, 999, -2, 0.5, -0.4, 0.34, 0.12, -10.0, 30.1, 5.3, 9.8}, {0}
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
		8, /* num_elements, expected_float, expected_int */
		{0.666, 666, 999, -2, 0.5, -0.4, 30, 40}, {0}
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
		16, /* num_elements, expected_float, expected_int */
		{0.666, 666, 999, -2, 0.5, -0.4, 30, 40, 0.12, 0.24, 0.34, 0.56, 0.67, 0.78, 0.89, 0.04}, {0}
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
		16, /* num_elements, expected_float, expected_int */
		{0.666, 666, 999, -2, 0.5, -0.4, 30, 40, 0.12, 0.24, 0.34, 0.56, 0.67, 0.78, 0.89, 0.04}, {0}
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
		6, /* num_elements, expected_float, expected_int */
		{0.666, 666.0, 999.0, -2.0, 0.2, 5.0}, {0}
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
		12, /* num_elements, expected_float, expected_int */
		{0.666, 666.0, 999.0, -2.0, 0.2, 5.0, 0.98, 0.87, 0.76, 0.65, 0.54, 0.43}, {0}
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
		12, /* num_elements, expected_float, expected_int */
		{0.666, 666.0, 999.0, -2.0, 0.2, 5.0, 0.98, 0.87, 0.76, 0.65, 0.54, 0.43}, {0}
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
		9, /* num_elements, expected_float, expected_int */
		{0.666, 666.0, 999.0, -2.0, 0.2, 5.0, 3.0, 0.3, -10.0}, {0}
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
		18, /* num_elements, expected_float, expected_int */
		{0.666, 666.0, 999.0, -2.0, 0.2, 5.0, 3.0, 0.3, -10.0,
		 20.0, 10.0, 5.0, 90.0, -4.0, 3.4, -2.3, -8.6, 8.0}, {0}
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
		18, /* num_elements, expected_float, expected_int */
		{0.666, 666.0, 999.0, -2.0, 0.2, 5.0, 3.0, 0.3, -10.0,
		 20.0, 10.0, 5.0, 90.0, -4.0, 3.4, -2.3, -8.6, 8.0}, {0}
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
		12, /* num_elements, expected_float, expected_int */
		{0.666, 666.0, 999.0, -2.0, 0.2, 5.0, 3.0, 0.3, -10.0, 0.4, -4.1, -5.9}, {0}
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
		24, /* num_elements, expected_float, expected_int */
		{0.666, 666.0, 999.0, -2.0, 0.2, 5.0, 3.0, 0.3, -10.0, 0.4, -4.1, -5.9,
		 20.0, 10.0, 5.0, 90.0, -4.0, 3.4, -2.3, -8.6, 8.0, 0.4, -4.1, -5.9}, {0}
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
		24, /* num_elements, expected_float, expected_int */
		{0.666, 666.0, 999.0, -2.0, 0.2, 5.0, 3.0, 0.3, -10.0, 0.4, -4.1, -5.9,
		 20.0, 10.0, 5.0, 90.0, -4.0, 3.4, -2.3, -8.6, 8.0, 0.4, -4.1, -5.9}, {0}
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
		8, /* num_elements, expected_float, expected_int */
		{0.666, 666, 999, -2, 0.5, -0.4, 30, 40}, {0}
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
		16, /* num_elements, expected_float, expected_int */
		{0.666, 666, 999, -2, 0.5, -0.4, 30, 40,
		 20.0, 10.0, 5.0, 90.0, -4.0, 3.4, -2.3, -8.6}, {0}
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
		16, /* num_elements, expected_float, expected_int */
		{0.666, 666, 999, -2, 0.5, -0.4, 30, 40,
		 20.0, 10.0, 5.0, 90.0, -4.0, 3.4, -2.3, -8.6}, {0}
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
		12, /* num_elements, expected_float, expected_int */
		{0.666, 666, 999, -2, 0.5, -0.4, 30, 40, 0.3, 0.2, 0.1, 0.4}, {0}
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
		24, /* num_elements, expected_float, expected_int */
		{0.666, 666, 999, -2, 0.5, -0.4, 30, 40, 0.3, 0.2, 0.1, 0.4,
		 20.0, 10.0, 5.0, 90.0, -4.0, 3.4, -2.3, -8.6, 8.0, 0.4, -4.1, -5.9}, {0}
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
		24, /* num_elements, expected_float, expected_int */
		{0.666, 666, 999, -2, 0.5, -0.4, 30, 40, 0.3, 0.2, 0.1, 0.4,
		 20.0, 10.0, 5.0, 90.0, -4.0, 3.4, -2.3, -8.6, 8.0, 0.4, -4.1, -5.9}, {0}
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
		16, /* num_elements, expected_float, expected_int */
		{0.666, 666.0, 999.0, -2.0,
		 0.2, 5.0, 3.0, 0.3,
		 -10.0, 20.1, 52.4, -34.3,
		 45.0, 56.0, 67.0, 78.0}, {0}
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
		32, /* num_elements, expected_float, expected_int */
		{0.666, 666.0, 999.0, -2.0,
		 0.2, 5.0, 3.0, 0.3,
		 -10.0, 20.1, 52.4, -34.3,
		 45.0, 56.0, 67.0, 78.0,
		 20.0, 10.0, 5.0, 90.0, -4.0, 3.4, -2.3, -8.6,
		 8.0, 0.4, -4.1, -5.9, -10.0, 0.4, -4.1, -5.9}, {0}
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
		32, /* num_elements, expected_float, expected_int */
		{0.666, 666.0, 999.0, -2.0,
		 0.2, 5.0, 3.0, 0.3,
		 -10.0, 20.1, 52.4, -34.3,
		 45.0, 56.0, 67.0, 78.0,
		 20.0, 10.0, 5.0, 90.0, -4.0, 3.4, -2.3, -8.6,
		 8.0, 0.4, -4.1, -5.9, -10.0, 0.4, -4.1, -5.9}, {0}
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
		1, /* num_elements, expected_float, expected_int */
		{0}, {2145948354}
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
		2, /* num_elements, expected_float, expected_int */
		{0}, {-362245257, 2074398469}
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
		2, /* num_elements, expected_float, expected_int */
		{0}, {-362245257, 2074398469}
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
		2, /* num_elements, expected_float, expected_int */
		{0}, {408918569, -69869318}
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
		4, /* num_elements, expected_float, expected_int */
		{0}, {5703639, 654049542, 82927237, -1489678625}
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
		4, /* num_elements, expected_float, expected_int */
		{0}, {5703639, 654049542, 82927237, -1489678625}
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
		3, /* num_elements, expected_float, expected_int */
		{0}, {1402620337, -931103284, -1922128750}
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
		6, /* num_elements, expected_float, expected_int */
		{0}, {819762795, 292214138, 207695021,
		     -541769145, -896550370, -322088831}
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
		6, /* num_elements, expected_float, expected_int */
		{0}, {819762795, 292214138, 207695021,
		     -541769145, -896550370, -322088831}
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
		4, /* num_elements, expected_float, expected_int */
		{0}, {1979209158, -791559088, -992849733, -59981678}
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
		8, /* num_elements, expected_float, expected_int */
		{0}, {-764612129, 395402837, -1260359913, 936205122,
		     -1510453781, -707590649, -760434930, -1756396083}
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
		8, /* num_elements, expected_float, expected_int */
		{0}, {-764612129, 395402837, -1260359913, 936205122,
		     -1510453781, -707590649, -760434930, -1756396083}
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
		1, /* num_elements, expected_float, expected_int */
		{0}, {2230472931u}
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
		2, /* num_elements, expected_float, expected_int */
		{0}, {4073369952u, 1026348970u}
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
		2, /* num_elements, expected_float, expected_int */
		{0}, {4073369952u, 1026348970u}
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
		2, /* num_elements, expected_float, expected_int */
		{0}, {1214092884u, 3587337147u}
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
		4, /* num_elements, expected_float, expected_int */
		{0}, {1011258288u, 684916166u, 381807053u, 3306523233u}
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
		4, /* num_elements, expected_float, expected_int */
		{0}, {1011258288u, 684916166u, 381807053u, 3306523233u}
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
		3, /* num_elements, expected_float, expected_int */
		{0}, {1076370307u, 1186562996u, 3616039281u}
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
		6, /* num_elements, expected_float, expected_int */
		{0}, {2984731006u, 2324137892u, 876349448u,
		     2493082028u, 1481747175u, 1530233730u}
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
		6, /* num_elements, expected_float, expected_int */
		{0}, {2984731006u, 2324137892u, 876349448u,
		     2493082028u, 1481747175u, 1530233730u}
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
		4, /* num_elements, expected_float, expected_int */
		{0}, {3046379279u, 3265138790u, 4109383147u, 2654056480u}
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
		8, /* num_elements, expected_float, expected_int */
		{0}, {2563680931u, 754130007u, 230209823u, 707580188u,
		     3015681429u, 3850948302u, 2224673498u, 2376088107u}
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
		8, /* num_elements, expected_float, expected_int */
		{0}, {2563680931u, 754130007u, 230209823u, 707580188u,
		     3015681429u, 3850948302u, 2224673498u, 2376088107u}
	},

	{NULL}
};
struct test_desc *test;

GLuint buf;
GLuint prog;

#define NUM_VERTICES 3
#define DEFAULT_VALUE 0.123456

void piglit_init(int argc, char **argv)
{
	GLuint vs;
	unsigned i;
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
	if (piglit_get_gl_version() < 15) {
		fprintf(stderr, "OpenGL 1.5 required.\n");
		piglit_report_result(PIGLIT_SKIP);
	}
	piglit_require_GLSL();
	piglit_require_transform_feedback();
	if (!test->is_floating_point)
		piglit_require_GLSL_version(130);

	glGetIntegerv(GL_MAX_TRANSFORM_FEEDBACK_INTERLEAVED_COMPONENTS, &maxcomps);
	if (maxcomps < test->num_elements) {
		piglit_report_result(PIGLIT_SKIP);
	}

	/* Create shaders. */
	vs = piglit_compile_shader_text(GL_VERTEX_SHADER, test->vs);
	prog = piglit_CreateProgram();
	piglit_AttachShader(prog, vs);
	piglit_TransformFeedbackVaryings(prog, test->num_varyings,
					 test->varyings,
					 GL_INTERLEAVED_ATTRIBS_EXT);
	piglit_LinkProgram(prog);
	if (!piglit_link_check_status(prog)) {
		piglit_DeleteProgram(prog);
		piglit_report_result(PIGLIT_FAIL);
	}

	/* Set up the transform feedback buffer. */
	data = malloc(test->num_elements*NUM_VERTICES*sizeof(float));
	for (i = 0; i < test->num_elements*NUM_VERTICES; i++) {
		data[i] = DEFAULT_VALUE;
	}

	glGenBuffers(1, &buf);
	glBindBuffer(GL_TRANSFORM_FEEDBACK_BUFFER_EXT, buf);
	glBufferData(GL_TRANSFORM_FEEDBACK_BUFFER_EXT,
		     test->num_elements*NUM_VERTICES*sizeof(float),
		     data, GL_STREAM_READ);

	assert(glGetError() == 0);

	piglit_BindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER_EXT, 0, buf);

	assert(glGetError() == 0);

	glClearColor(0.2, 0.2, 0.2, 1.0);
	glEnableClientState(GL_VERTEX_ARRAY);

	free(data);
}

enum piglit_result piglit_display(void)
{
	GLboolean pass = GL_TRUE;
	void *ptr;
	float *ptr_float;
	GLint *ptr_int;
	unsigned i;
	static const float verts[NUM_VERTICES*2] = {
		10, 10,
		10, 20,
		20, 20
	};

	glClear(GL_COLOR_BUFFER_BIT);

	/* Render into TFBO. */
	glLoadIdentity();
	piglit_UseProgram(prog);
	piglit_BeginTransformFeedback(GL_TRIANGLES);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glVertexPointer(2, GL_FLOAT, 0, verts);
	glDrawArrays(GL_TRIANGLES, 0, NUM_VERTICES);
	piglit_EndTransformFeedback();

	assert(glGetError() == 0);

	ptr = glMapBuffer(GL_TRANSFORM_FEEDBACK_BUFFER_EXT, GL_READ_ONLY);
	ptr_float = ptr;
	ptr_int = ptr;
	for (i = 0; i < test->num_elements*NUM_VERTICES; i++) {
		if (test->is_floating_point) {
			float value = test->expected_float[i % test->num_elements];

			if (fabs(ptr_float[i] - value) > 0.01) {
				printf("Buffer[%i]: %f,  Expected: %f\n", i,
				       ptr_float[i], value);
				pass = GL_FALSE;
			}
		} else {
			GLint value = test->expected_int[i % test->num_elements];

			if (ptr_int[i] != value) {
				printf("Buffer[%i]: %i,  Expected: %i\n", i,
				       ptr_int[i], value);
				pass = GL_FALSE;
			}
		}
	}
	glUnmapBuffer(GL_TRANSFORM_FEEDBACK_BUFFER_EXT);

	assert(glGetError() == 0);

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}
