/*
 * Copyright © 2013 Intel Corporation
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
 * \file structs.c
 *
 * Test proper functioning of transform feedback with varying structs.
 *
 * The spec is ambiguous about how transform feedback is supposed to
 * interact with varying structs.  However, the Khronos board has
 * clarified that:
 *
 * - Whole structures (or array of structures) cannot be bound all at
 *   once using glTransformFeedbackVaryings().
 *
 * - Instead, the caller must apply transform feedback to individual
 *   elements of structs, by using the "." character in the string
 *   passed to glTransformFeedbackVaryings().
 *
 * - The intention is for the transform feedback API to behave
 *   similary to glGetUniformLocation() and
 *   glGetProgramResourceLocation().
 *
 * This test verifies proper operation of transform feedback varyings
 * according to the above clarifications.
 *
 * Because of the subtle interactions between structs and arrays, this
 * test contains several sub-tests, each concerned with verifying a
 * particular combination of arrays and structs:
 *
 * - basic-struct: Each varying is a struct containing simple types
 *   (e.g. vec4, float, mat3).
 *
 * - struct-whole-array: Each varying is a struct containing arrays of
 *   simple types, and transform feedback is applied to whole arrays.
 *
 * - struct-array-elem: Each varying is a struct containing arrays of
 *   simple types, and transform feedback is applied to individual
 *   array elements.
 *
 * - array-struct: Each varying is an array of structs containing
 *   simple types.
 *
 * - array-struct-whole-array: Each varying is an array of structs
 *   containing arrays of simple types, and transform feedback is
 *   applied to whole arrays within each struct.
 *
 * - array-struct-array-elem: Each varying is an array of structs
 *   containing arrays of simple types, and transform feedback is
 *   applied to individual array elements.
 *
 * - struct-struct: Each varying is a struct containing structs.
 *
 * - array-struct-array-struct: Each varying is an array of structs
 *   containing arrays of structs.
 *
 * Each of these variants may be run in one of four modes:
 *
 * - error: attempt to specify invalid values for
 *   glTransformFeedbackVaryings() and verify that the shaders fail to
 *   link.
 *
 * - get: link the shaders and verify that the values returned by
 *   glGetTransformFeedbackVarying() are correct.
 *
 * - run: draw using the shaders, and verify that (a) the values
 *   stored in the transform feedback buffer are correct, and (b) the
 *   values delivered to the fragment shader are correct.
 *
 * - run-no-fs: link with just a vertex shader and no fragment
 *   shader*, and draw with GL_RASTERIZER_DISCARD enabled.  Verify
 *   that the values stored in the transform feedback buffer are
 *   correct.
 *
 * (*In GLES3, a fragment shader is required, so "run-no-fs" mode
 * links to a generic do-nothing fragment shader).
 *
 * Furthermore, when testing on desktop GL, the optional command-line
 * parameter "interface" may be given, to cause the test to be run
 * using interface blocks.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

#ifdef PIGLIT_USE_OPENGL
	config.supports_gl_core_version = 32;
#else /* PIGLIT_USE_OPENGL_ES3 */
	config.supports_gl_es_version = 30;
#endif
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;

PIGLIT_GL_TEST_CONFIG_END


#define MAX_VARYINGS 21
#define NUM_VERTICES 6
#define MAX_COMPONENTS 20
#define VERTEX_ATTRIB_POS 0


/**
 * Header attached to the top of each shader when testing GLES 3.0.
 */
static const char gles3_header[] =
	"#version 300 es\n"
	"precision highp float;\n"
	"#define DECLARE_VARYING(DIR, TYPE, NAME) flat DIR TYPE NAME\n"
	"#define VARYING(NAME) NAME\n";


/**
 * Header attached to the top of each shader when testing desktop GL
 * and not using interface blocks.
 */
static const char desktop_header[] =
	"#version 150\n"
	"#define DECLARE_VARYING(DIR, TYPE, NAME) flat DIR TYPE NAME\n"
	"#define VARYING(NAME) NAME\n";


/**
 * Header attached to the top of each shader when testing desktop GL
 * and using interface blocks.
 */
static const char desktop_header_interface[] =
	"#version 150\n"
	"#define DECLARE_VARYING(DIR, TYPE, NAME) DIR Blk { flat TYPE NAME; } blk\n"
	"#define VARYING(NAME) blk.NAME\n";


/**
 * Description of each possible sub-test.
 */
static struct test_desc {
	/** Name of the test. */
	const char *name;

	/**
	 * Vertex shader source text.  This will be concatenated with
	 * one of the above "header" strings.  Accordingly, it should
	 * use the macros DECLARE_VARYING() to declare a varying, and
	 * VARYING() to access a varying.  (These macros expand
	 * differently depending whether the test is using interface
	 * blocks or not).
	 */
	const char *vs;

	/**
	 * Fragment shader source text.  As with the vertex shader,
	 * this will be concatenated with one of the above "header"
	 * strings.
	 */
	const char *fs;

	/**
	 * Names which, if passed to glTransformFeedbackVaryings(),
	 * should result in a link error.  Terminated by a NULL
	 * pointer.
	 */
	const char *bad_varyings[MAX_VARYINGS];

	/**
	 * Names which, if passed to glTransformFeedbackVaryings(),
	 * should result in proper operation.  When using interface
	 * blocks, each of these varying names will be prefixed with
	 * "Blk.".  Terminated by a NULL pointer.
	 */
	const char *good_varyings[MAX_VARYINGS];

	/**
	 * Expected types which should be returned by
	 * glGetTransformFeedbackVarying() for each of the varyings in
	 * \c good_varyings.
	 */
	GLenum expected_types[MAX_VARYINGS];

	/**
	 * Expected sizes which should be returned by
	 * glGetTransformFeedbackVarying() for each of the varyings in
	 * \c good_varyings.
	 */
	unsigned expected_sizes[MAX_VARYINGS];

	/**
	 * Expected varying values which transform feedback should
	 * capture when running this test (floating-point values
	 * only).
	 */
	float expected_floats[MAX_COMPONENTS];

	/**
	 * Expected varying values which transform feedback should
	 * capture when running this test (integral values only).
	 */
	int expected_ints[MAX_COMPONENTS];
} tests[] = {
	{
		/* name */
		"basic-struct",

		/* vs */
		"struct S { float a; vec4 b; mat3 c; ivec2 d; uvec3 e; };\n"
		"in vec4 pos;\n"
		"DECLARE_VARYING(out, S, v);\n"
		"void main()\n"
		"{\n"
		"  gl_Position = pos;\n"
		"  VARYING(v).a = 1.0;\n"
		"  VARYING(v).b = vec4(2.0, 3.0, 4.0, 5.0);\n"
		"  VARYING(v).c = mat3(6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 12.0, 13.0, 14.0);\n"
		"  VARYING(v).d = ivec2(15, 16);\n"
		"  VARYING(v).e = uvec3(17, 18, 19);\n"
		"}\n",

		/* fs */
		"struct S { float a; vec4 b; mat3 c; ivec2 d; uvec3 e; };\n"
		"DECLARE_VARYING(in, S, v);\n"
		"out vec4 color;\n"
		"void main()\n"
		"{\n"
		"  bool pass = true;\n"
		"  if (VARYING(v).a != 1.0) pass = false;\n"
		"  if (VARYING(v).b != vec4(2.0, 3.0, 4.0, 5.0)) pass = false;\n"
		"  if (VARYING(v).c != mat3(6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 12.0, 13.0, 14.0)) pass = false;\n"
		"  if (VARYING(v).d != ivec2(15, 16)) pass = false;\n"
		"  if (VARYING(v).e != uvec3(17, 18, 19)) pass = false;\n"
		"  if (pass)\n"
		"    color = vec4(0.0, 1.0, 0.0, 1.0);\n"
		"  else\n"
		"    color = vec4(1.0, 0.0, 0.0, 1.0);\n"
		"}\n",

		/* bad_varyings */
		{ "v", NULL },

		/* good_varyings */
		{ "v.a", "v.b", "v.c", "v.d", "v.e", NULL },

		/* expected_types */
		{ GL_FLOAT, GL_FLOAT_VEC4, GL_FLOAT_MAT3, GL_INT_VEC2,
		  GL_UNSIGNED_INT_VEC3 },

		/* expected_sizes */
		{ 1, 1, 1, 1, 1 },

		/* expected_floats */
		{ 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0,
		  12.0, 13.0, 14.0 },

		/* expected_ints */
		{ 15, 16, 17, 18, 19 }
	},
	{
		/* name */
		"struct-whole-array",

		/* vs */
		"struct S { uvec4[4] a; vec2[2] b; int[3] c; };\n"
		"in vec4 pos;\n"
		"DECLARE_VARYING(out, S, v);\n"
		"void main()\n"
		"{\n"
		"  gl_Position = pos;\n"
		"  for (int i = 0; i < 4; i++) {\n"
		"    if (i < 4) VARYING(v).a[i] = uvec4(100, 200, 300, 400) + uint(i);\n"
		"    if (i < 2) VARYING(v).b[i] = vec2(500.0, 600.0) + float(i);\n"
		"    if (i < 3) VARYING(v).c[i] = 700 + i;\n"
		"  }\n"
		"}\n",

		/* fs */
		"struct S { uvec4[4] a; vec2[2] b; int[3] c; };\n"
		"DECLARE_VARYING(in, S, v);\n"
		"out vec4 color;\n"
		"void main()\n"
		"{\n"
		"  bool pass = true;\n"
		"  for (int i = 0; i < 3; i++) {\n"
		"    if (i < 4 && VARYING(v).a[i] != uvec4(100, 200, 300, 400) + uint(i)) pass = false;\n"
		"    if (i < 2 && VARYING(v).b[i] != vec2(500.0, 600.0) + float(i)) pass = false;\n"
		"    if (i < 3 && VARYING(v).c[i] != 700 + i) pass = false;\n"
		"  }\n"
		"  if (pass)\n"
		"    color = vec4(0.0, 1.0, 0.0, 1.0);\n"
		"  else\n"
		"    color = vec4(1.0, 0.0, 0.0, 1.0);\n"
		"}\n",

		/* bad_varyings */
		{ "v", NULL },

		/* good_varyings */
		{ "v.a", "v.b", "v.c", NULL },

		/* expected_types */
		{ GL_UNSIGNED_INT_VEC4, GL_FLOAT_VEC2, GL_INT },

		/* expected_sizes */
		{ 4, 2, 3 },

		/* expected_floats */
		{ 500.0, 600.0, 501.0, 601.0 },

		/* expected_ints */
		{ 100, 200, 300, 400, 101, 201, 301, 401, 102, 202, 302, 402,
		  103, 203, 303, 403, 700, 701, 702 }
	},
	{
		/* name */
		"struct-array-elem",

		/* vs */
		"struct S { ivec4[2] a; uint[4] b; vec3[3] c; };\n"
		"in vec4 pos;\n"
		"DECLARE_VARYING(out, S, v);\n"
		"void main()\n"
		"{\n"
		"  gl_Position = pos;\n"
		"  for (int i = 0; i < 4; i++) {\n"
		"    if (i < 2) VARYING(v).a[i] = ivec4(100, 200, 300, 400) + i;\n"
		"    if (i < 4) VARYING(v).b[i] = 500u + uint(i);\n"
		"    if (i < 3) VARYING(v).c[i] = vec3(600.0, 700.0, 800.0) + float(i);\n"
		"  }\n"
		"}\n",

		/* fs */
		"struct S { ivec4[2] a; uint[4] b; vec3[3] c; };\n"
		"DECLARE_VARYING(in, S, v);\n"
		"out vec4 color;\n"
		"void main()\n"
		"{\n"
		"  bool pass = true;\n"
		"  for (int i = 0; i < 3; i++) {\n"
		"    if (i < 2 && VARYING(v).a[i] != ivec4(100, 200, 300, 400) + i) pass = false;\n"
		"    if (i < 4 && VARYING(v).b[i] != 500u + uint(i)) pass = false;\n"
		"    if (i < 3 && VARYING(v).c[i] != vec3(600.0, 700.0, 800.0) + float(i)) pass = false;\n"
		"  }\n"
		"  if (pass)\n"
		"    color = vec4(0.0, 1.0, 0.0, 1.0);\n"
		"  else\n"
		"    color = vec4(1.0, 0.0, 0.0, 1.0);\n"
		"}\n",

		/* bad_varyings */
		{ "v", NULL },

		/* good_varyings */
		{ "v.a[0]", "v.a[1]", "v.b[0]", "v.b[1]", "v.b[2]", "v.b[3]",
		  "v.c[0]", "v.c[1]", "v.c[2]", NULL },

		/* expected_types */
		{ GL_INT_VEC4, GL_INT_VEC4, GL_UNSIGNED_INT, GL_UNSIGNED_INT,
		  GL_UNSIGNED_INT, GL_UNSIGNED_INT, GL_FLOAT_VEC3,
		  GL_FLOAT_VEC3, GL_FLOAT_VEC3 },

		/* expected_sizes */
		{ 1, 1, 1, 1, 1, 1, 1, 1, 1 },

		/* expected_floats */
		{ 600.0, 700.0, 800.0, 601.0, 701.0, 801.0, 602.0, 702.0, 802.0 },

		/* expected_ints */
		{ 100, 200, 300, 400, 101, 201, 301, 401, 500, 501, 502, 503 }
	},
	{
		/* name */
		"array-struct",

		/* vs */
		"struct S { mat2 a; ivec3 b; uvec2 c; };\n"
		"in vec4 pos;\n"
		"DECLARE_VARYING(out, S[3], v);\n"
		"void main()\n"
		"{\n"
		"  gl_Position = pos;\n"
		"  for (int i = 0; i < 3; i++) {\n"
		"    VARYING(v)[i].a = mat2(100.0, 200.0, 300.0, 400.0) + float(i);\n"
		"    VARYING(v)[i].b = ivec3(500, 600, 700) + i;\n"
		"    VARYING(v)[i].c = uvec2(800, 900) + uint(i);\n"
		"  }\n"
		"}\n",

		/* fs */
		"struct S { mat2 a; ivec3 b; uvec2 c; };\n"
		"DECLARE_VARYING(in, S[3], v);\n"
		"out vec4 color;\n"
		"void main()\n"
		"{\n"
		"  bool pass = true;\n"
		"  for (int i = 0; i < 3; i++) {\n"
		"    if (VARYING(v)[i].a != mat2(100.0, 200.0, 300.0, 400.0) + float(i)) pass = false;\n"
		"    if (VARYING(v)[i].b != ivec3(500, 600, 700) + i) pass = false;\n"
		"    if (VARYING(v)[i].c != uvec2(800, 900) + uint(i)) pass = false;\n"
		"  }\n"
		"  if (pass)\n"
		"    color = vec4(0.0, 1.0, 0.0, 1.0);\n"
		"  else\n"
		"    color = vec4(1.0, 0.0, 0.0, 1.0);\n"
		"}\n",

		/* bad_varyings */
		{ "v", "v[0]", "v[1]", "v[2]", "v.a", "v.b", "v.c", NULL },

		/* good_varyings */
		{ "v[0].a", "v[0].b", "v[0].c", "v[1].a", "v[1].b", "v[1].c",
		  "v[2].a", "v[2].b", "v[2].c", NULL },

		/* expected_types */
		{ GL_FLOAT_MAT2, GL_INT_VEC3, GL_UNSIGNED_INT_VEC2,
		  GL_FLOAT_MAT2, GL_INT_VEC3, GL_UNSIGNED_INT_VEC2,
		  GL_FLOAT_MAT2, GL_INT_VEC3, GL_UNSIGNED_INT_VEC2 },

		/* expected_sizes */
		{ 1, 1, 1, 1, 1, 1, 1, 1, 1 },

		/* expected_floats */
		{ 100.0, 200.0, 300.0, 400.0, 101.0, 201.0, 301.0, 401.0,
		  102.0, 202.0, 302.0, 402.0 },

		/* expected_ints */
		{ 500, 600, 700, 800, 900, 501, 601, 701, 801, 901, 502, 602,
		  702, 802, 902 }
	},
	{
		/* name */
		"array-struct-whole-array",

		/* vs */
		"struct S { int[2] a; int[3] b; };\n"
		"in vec4 pos;\n"
		"DECLARE_VARYING(out, S[4], v);\n"
		"void main()\n"
		"{\n"
		"  gl_Position = pos;\n"
		"  for (int i = 0; i < 4; i++) {\n"
		"    for (int j = 0; j < 3; j++) {\n"
		"      if (j < 2) VARYING(v)[i].a[j] = 100 * i + 10 * j + 1;\n"
		"      if (j < 3) VARYING(v)[i].b[j] = 100 * i + 10 * j + 2;\n"
		"    }\n"
		"  }\n"
		"}\n",

		/* fs */
		"struct S { int[2] a; int[3] b; };\n"
		"DECLARE_VARYING(in, S[4], v);\n"
		"out vec4 color;\n"
		"void main()\n"
		"{\n"
		"  bool pass = true;\n"
		"  for (int i = 0; i < 4; i++) {\n"
		"    for (int j = 0; j < 3; j++) {\n"
		"      if (j < 2 && VARYING(v)[i].a[j] != 100 * i + 10 * j + 1) pass = false;\n"
		"      if (j < 3 && VARYING(v)[i].b[j] != 100 * i + 10 * j + 2) pass = false;\n"
		"    }\n"
		"  }\n"
		"  if (pass)\n"
		"    color = vec4(0.0, 1.0, 0.0, 1.0);\n"
		"  else\n"
		"    color = vec4(1.0, 0.0, 0.0, 1.0);\n"
		"}\n",

		/* bad_varyings */
		{ "v", "v[0]", "v[1]", "v[2]", "v[3]", "v.a", "v.b", NULL },

		/* good_varyings */
		{ "v[0].a", "v[0].b", "v[1].a", "v[1].b", "v[2].a", "v[2].b",
		  "v[3].a", "v[3].b", NULL },

		/* expected_types */
		{ GL_INT, GL_INT, GL_INT, GL_INT, GL_INT, GL_INT, GL_INT,
		  GL_INT },

		/* expected_sizes */
		{ 2, 3, 2, 3, 2, 3, 2, 3 },

		/* expected_floats */
		{ 0 },

		/* expected_ints */
		{ 1, 11, 2, 12, 22, 101, 111, 102, 112, 122, 201, 211, 202,
		  212, 222, 301, 311, 302, 312, 322 }
	},
	{
		/* name */
		"array-struct-array-elem",

		/* vs */
		"struct S { int[2] a; int[3] b; };\n"
		"in vec4 pos;\n"
		"DECLARE_VARYING(out, S[4], v);\n"
		"void main()\n"
		"{\n"
		"  gl_Position = pos;\n"
		"  for (int i = 0; i < 4; i++) {\n"
		"    for (int j = 0; j < 3; j++) {\n"
		"      if (j < 2) VARYING(v)[i].a[j] = 100 * i + 10 * j + 1;\n"
		"      if (j < 3) VARYING(v)[i].b[j] = 100 * i + 10 * j + 2;\n"
		"    }\n"
		"  }\n"
		"}\n",

		/* fs */
		"struct S { int[2] a; int[3] b; };\n"
		"DECLARE_VARYING(in, S[4], v);\n"
		"out vec4 color;\n"
		"void main()\n"
		"{\n"
		"  bool pass = true;\n"
		"  for (int i = 0; i < 4; i++) {\n"
		"    for (int j = 0; j < 3; j++) {\n"
		"      if (j < 2 && VARYING(v)[i].a[j] != 100 * i + 10 * j + 1) pass = false;\n"
		"      if (j < 3 && VARYING(v)[i].b[j] != 100 * i + 10 * j + 2) pass = false;\n"
		"    }\n"
		"  }\n"
		"  if (pass)\n"
		"    color = vec4(0.0, 1.0, 0.0, 1.0);\n"
		"  else\n"
		"    color = vec4(1.0, 0.0, 0.0, 1.0);\n"
		"}\n",

		/* bad_varyings */
		{ "v", "v[0]", "v[1]", "v[2]", "v[3]", "v.a", "v.b", NULL },

		/* good_varyings */
		{ "v[0].a[0]", "v[0].a[1]", "v[0].b[0]", "v[0].b[1]",
		  "v[0].b[2]", "v[1].a[0]", "v[1].a[1]", "v[1].b[0]",
		  "v[1].b[1]", "v[1].b[2]", "v[2].a[0]", "v[2].a[1]",
		  "v[2].b[0]", "v[2].b[1]", "v[2].b[2]", "v[3].a[0]",
		  "v[3].a[1]", "v[3].b[0]", "v[3].b[1]", "v[3].b[2]", NULL },

		/* expected_types */
		{ GL_INT, GL_INT, GL_INT, GL_INT, GL_INT, GL_INT, GL_INT,
		  GL_INT, GL_INT, GL_INT, GL_INT, GL_INT, GL_INT, GL_INT,
		  GL_INT, GL_INT, GL_INT, GL_INT, GL_INT, GL_INT },

		/* expected_sizes */
		{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },

		/* expected_floats */
		{ 0 },

		/* expected_ints */
		{ 1, 11, 2, 12, 22, 101, 111, 102, 112, 122, 201, 211, 202,
		  212, 222, 301, 311, 302, 312, 322 }
	},
	{
		/* name */
		"struct-struct",

		/* vs */
		"struct S { int a; float b; };\n"
		"struct T { float c; int d; };\n"
		"struct U { S e; T f; };\n"
		"in vec4 pos;\n"
		"DECLARE_VARYING(out, U, v);\n"
		"void main()\n"
		"{\n"
		"  gl_Position = pos;\n"
		"  VARYING(v).e.a = 1;\n"
		"  VARYING(v).e.b = 2.0;\n"
		"  VARYING(v).f.c = 3.0;\n"
		"  VARYING(v).f.d = 4;\n"
		"}\n",

		/* fs */
		"struct S { int a; float b; };\n"
		"struct T { float c; int d; };\n"
		"struct U { S e; T f; };\n"
		"DECLARE_VARYING(in, U, v);\n"
		"out vec4 color;\n"
		"void main()\n"
		"{\n"
		"  bool pass = true;\n"
		"  if (VARYING(v).e.a != 1) pass = false;\n"
		"  if (VARYING(v).e.b != 2.0) pass = false;\n"
		"  if (VARYING(v).f.c != 3.0) pass = false;\n"
		"  if (VARYING(v).f.d != 4) pass = false;\n"
		"  if (pass)\n"
		"    color = vec4(0.0, 1.0, 0.0, 1.0);\n"
		"  else\n"
		"    color = vec4(1.0, 0.0, 0.0, 1.0);\n"
		"}\n",

		/* bad_varyings */
		{ "v", "v.e", "v.f", NULL },

		/* good_varyings */
		{ "v.e.a", "v.e.b", "v.f.c", "v.f.d", NULL },

		/* expected_types */
		{ GL_INT, GL_FLOAT, GL_FLOAT, GL_INT },

		/* expected_sizes */
		{ 1, 1, 1, 1 },

		/* expected_floats */
		{ 2.0, 3.0 },

		/* expected_ints */
		{ 1, 4 }
	},
	{
		/* name */
		"array-struct-array-struct",

		/* vs */
		"struct S { int a; float b; };\n"
		"struct T { float c; int d; };\n"
		"struct U { S[2] e; T[2] f; };\n"
		"in vec4 pos;\n"
		"DECLARE_VARYING(out, U[2], v);\n"
		"void main()\n"
		"{\n"
		"  gl_Position = pos;\n"
		"  for (int i = 0; i < 2; i++) {\n"
		"    for (int j = 0; j < 2; j++) {\n"
		"      VARYING(v)[i].e[j].a = 100 * i + 10 * j + 1;\n"
		"      VARYING(v)[i].e[j].b = float(100 * i + 10 * j + 2);\n"
		"      VARYING(v)[i].f[j].c = float(100 * i + 10 * j + 3);\n"
		"      VARYING(v)[i].f[j].d = 100 * i + 10 * j + 4;\n"
		"    }\n"
		"  }\n"
		"}\n",

		/* fs */
		"struct S { int a; float b; };\n"
		"struct T { float c; int d; };\n"
		"struct U { S[2] e; T[2] f; };\n"
		"DECLARE_VARYING(in, U[2], v);\n"
		"out vec4 color;\n"
		"void main()\n"
		"{\n"
		"  bool pass = true;\n"
		"  for (int i = 0; i < 2; i++) {\n"
		"    for (int j = 0; j < 2; j++) {\n"
		"      if (VARYING(v)[i].e[j].a != 100 * i + 10 * j + 1) pass = false;\n"
		"      if (VARYING(v)[i].e[j].b != float(100 * i + 10 * j + 2)) pass = false;\n"
		"      if (VARYING(v)[i].f[j].c != float(100 * i + 10 * j + 3)) pass = false;\n"
		"      if (VARYING(v)[i].f[j].d != 100 * i + 10 * j + 4) pass = false;\n"
		"    }\n"
		"  }\n"
		"  if (pass)\n"
		"    color = vec4(0.0, 1.0, 0.0, 1.0);\n"
		"  else\n"
		"    color = vec4(1.0, 0.0, 0.0, 1.0);\n"
		"}\n",

		/* bad_varyings */
		{ "v", "v[0]", "v[1]", "v[0].e", "v[0].f", "v[1].e",
		  "v[1].f", "v[0].e[0]", "v[0].e[1]", "v[0].f[0]", "v[0].f[1]",
		  "v[1].e[0]", "v[1].e[1]", "v[1].f[0]", "v[1].f[1]", "v.e.a",
		  "v.e.b", "v.f.c", "v.f.d", NULL },

		/* good_varyings */
		{ "v[0].e[0].a", "v[0].e[1].a", "v[1].e[0].a", "v[1].e[1].a",
		  "v[0].e[0].b", "v[0].e[1].b", "v[1].e[0].b", "v[1].e[1].b",
		  "v[0].f[0].c", "v[0].f[1].c", "v[1].f[0].c", "v[1].f[1].c",
		  "v[0].f[0].d", "v[0].f[1].d", "v[1].f[0].d", "v[1].f[1].d",
		  NULL },

		/* expected_types */
		{ GL_INT, GL_INT, GL_INT, GL_INT, GL_FLOAT, GL_FLOAT, GL_FLOAT,
		  GL_FLOAT, GL_FLOAT, GL_FLOAT, GL_FLOAT, GL_FLOAT, GL_INT,
		  GL_INT, GL_INT, GL_INT },

		/* expected_sizes */
		{ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },

		/* expected_floats */
		{ 2.0, 12.0, 102.0, 112.0, 3.0, 13.0, 103.0, 113.0 },

		/* expected_ints */
		{ 1, 11, 101, 111, 4, 14, 104, 114 }
	},
};


#ifdef PIGLIT_USE_OPENGL_ES3
/**
 * Generic do-nothing fragment shader used when running tests in
 * "run-no-fs" mode on GLES3, since GLES3 always requires a fragment
 * shader to be present.
 */
static const char *generic_gles3_fs_text =
	"out vec4 color;\n"
	"void main()\n"
	"{\n"
	"  color = vec4(0.5);\n"
	"}\n";
#endif


static struct test_desc *test = NULL;
static GLuint prog;
static bool use_interface_blocks = false;


/**
 * Count the number of strings in the given NULL-terminated array of
 * strings.
 */
static unsigned
count_strings(const char * const *strings)
{
	unsigned i;
	for (i = 0; ; i++) {
		if (strings[i] == NULL)
			return i;
	}
}


/**
 * Choose which header should be prepended to each of the shaders
 * being tested, based on whether GL or GLSL is being used, and based
 * on whether the test uses interface blocks.
 */
static const char *
choose_header()
{
#ifdef PIGLIT_USE_OPENGL
	(void) gles3_header;
	if (use_interface_blocks)
		return desktop_header_interface;
	else
		return desktop_header;
#else /* PIGLIT_USE_OPENGL_ES3 */
	(void) desktop_header_interface;
	(void) desktop_header;
	return gles3_header;
#endif
}


/**
 * Report the result, and if it's a failure, describe the shaders used
 * in the test.
 */
static void
report_result(enum piglit_result result)
{
	if (result == PIGLIT_FAIL) {
		printf("Vertex shader:\n%s%s\n", choose_header(), test->vs);
		printf("Fragment shader:\n%s%s\n", choose_header(), test->fs);
	}
	piglit_report_result(result);
}


/**
 * Attach the appropriate header to the shader and compile it.
 */
static GLuint
compile_shader(GLenum target, const char *shader_text)
{
	const char *header = choose_header();
	char *concatenated_text = NULL;
	GLuint shader;
	asprintf(&concatenated_text, "%s%s", header, shader_text);
	shader = piglit_compile_shader_text(target, concatenated_text);
	free(concatenated_text);
	return shader;
}


/**
 * Given an array of strings representing the names of varyings,
 * return a newly allocated array with \c prefix prepended to each
 * varying name.
 */
static const char **
prepend_varyings(const char *prefix, const char * const *varyings)
{
	unsigned num_varyings = count_strings(varyings);
	char **result = calloc(num_varyings + 1, sizeof(char *));
	unsigned i;
	for (i = 0; i < num_varyings; i++)
		asprintf(&result[i], "%s%s", prefix, varyings[i]);
	result[num_varyings] = NULL;
	return (const char **) result;
}


/**
 * Free an array allocated by \c prepend_varyings.
 */
static void
free_varyings(const char **varyings)
{
	unsigned i;
	for (i = 0; varyings[i] != NULL; i++)
		free((char *) varyings[i]);
	free(varyings);
}


/**
 * Link the appropriate set of shaders for running a positive test,
 * calling glTransformFeedbackVaryings() to set up transform feedback.
 */
static void
link_shaders(bool use_fs)
{
	GLuint vs;
	const char **varyings;
	prog = glCreateProgram();
	vs = compile_shader(GL_VERTEX_SHADER, test->vs);
	glAttachShader(prog, vs);
	glDeleteShader(vs);
	if (use_fs) {
		GLuint fs = compile_shader(GL_FRAGMENT_SHADER, test->fs);
		glAttachShader(prog, fs);
		glDeleteShader(fs);
	} else {
#ifdef PIGLIT_USE_OPENGL_ES3
		GLuint fs = compile_shader(GL_FRAGMENT_SHADER, generic_gles3_fs_text);
		glAttachShader(prog, fs);
		glDeleteShader(fs);
#endif
	}
	if (use_interface_blocks)
		varyings = prepend_varyings("Blk.", test->good_varyings);
	else
		varyings = test->good_varyings;
	glTransformFeedbackVaryings(prog, count_strings(test->good_varyings),
				    (const GLchar **) varyings,
				    GL_INTERLEAVED_ATTRIBS);
	if (use_interface_blocks)
		free_varyings(varyings);
	if (!piglit_check_gl_error(GL_NO_ERROR)) {
		glDeleteProgram(prog);
		report_result(PIGLIT_FAIL);
	}
	glBindAttribLocation(prog, VERTEX_ATTRIB_POS, "pos");
	glLinkProgram(prog);
	if (!piglit_link_check_status(prog)) {
		glDeleteProgram(prog);
		report_result(PIGLIT_FAIL);
	}
}


/**
 * Verify that passing the name \c varying to
 * glTransformFeedbackVaryings() produces a link error.
 *
 * If it does not, a description of the problem is printed and false
 * is returned.
 */
static bool
test_bad_varying(GLuint vs, GLuint fs, const char *varying)
{
	GLint ok;

	prog = glCreateProgram();
	glAttachShader(prog, vs);
	glAttachShader(prog, fs);
	glTransformFeedbackVaryings(prog, 1, &varying, GL_INTERLEAVED_ATTRIBS);
	if (!piglit_check_gl_error(GL_NO_ERROR))
		goto fail;
	glLinkProgram(prog);
	glGetProgramiv(prog, GL_LINK_STATUS, &ok);
	if (ok) {
		printf("Varying %s linked successfully, should have failed.\n",
		       varying);
		goto fail;
	}
	/* Link failure shouldn't produce a GL error */
	if (!piglit_check_gl_error(GL_NO_ERROR))
		goto fail;
	glDeleteProgram(prog);
	prog = 0;
	if (!piglit_check_gl_error(GL_NO_ERROR))
		goto fail;
	return true;

 fail:
	if (prog != 0)
		glDeleteProgram(prog);
	return false;
}


/**
 * Verify that "bad" varying names produces the expected link error.
 *
 * When using interface blocks, this function also verifies that (a)
 * "bad" varying names produce a link error if they are prepended with
 * "Blk.", and (b) "good" varying names produce a link error if they
 * are *not* prepended with "Blk.".
 */
static enum piglit_result
test_errors()
{
	unsigned i;
	unsigned num_bad_varyings = count_strings(test->bad_varyings);
	unsigned num_good_varyings = count_strings(test->good_varyings);
	GLuint vs, fs;
	bool pass = true;

	prog = 0;
	vs = compile_shader(GL_VERTEX_SHADER, test->vs);
	fs = compile_shader(GL_FRAGMENT_SHADER, test->fs);

	/* Test one bad varying at a time to make sure they all
	 * produce the proper error.
	 */
	for (i = 0; i < num_bad_varyings; i++)
		pass = test_bad_varying(vs, fs, test->bad_varyings[i]) && pass;

	if (use_interface_blocks) {
		/* Test that the "bad" varyings fail if prepended with
		 * "Blk."
		 */
		const char **varyings
			= prepend_varyings("Blk.", test->bad_varyings);
		for (i = 0; i < num_bad_varyings; i++)
			pass = test_bad_varying(vs, fs, varyings[i]) && pass;
		free_varyings(varyings);

		/* Test that the "good" varyings fail if *not*
		 * prepended with "Blk."
		 */
		for (i = 0; i < num_good_varyings; i++) {
			pass = test_bad_varying(vs, fs, test->good_varyings[i])
				&& pass;
		}
	}

	glDeleteShader(vs);
	glDeleteShader(fs);

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}


/**
 * Verify that glGetTransformFeedbackVarying() returns the proper
 * information for all "good" varying names.
 *
 * The program should already be linked and stored in the global \c
 * prog.
 */
static enum piglit_result
test_gets()
{
	unsigned i;
	unsigned num_good_varyings = count_strings(test->good_varyings);
	const char **varyings;
	bool pass = true;

	if (use_interface_blocks)
		varyings = prepend_varyings("Blk.", test->good_varyings);
	else
		varyings = test->good_varyings;

	for (i = 0; i < num_good_varyings; i++) {
		const char *exp_name = varyings[i];
		GLsizei exp_length = strlen(exp_name);
		GLsizei exp_size = test->expected_sizes[i];
		GLenum exp_type = test->expected_types[i];
		GLsizei length;
		GLsizei size;
		GLenum type;
		char name[100];
		glGetTransformFeedbackVarying(prog, i, sizeof(name), &length,
					      &size, &type, name);
		if (length != exp_length || size != exp_size
		    || type != exp_type || strcmp(name, exp_name) != 0) {
			pass = false;
			printf("glGetTransformFeedbackVarying() returned "
			       "unexpected data for varying %u:\n", i);
			printf("  length: expected %u, got %u\n",
			       exp_length, length);
			printf("  size: expected %u, got %u\n",
			       exp_size, size);
			printf("  type: expected %u (%s), got %u (%s)\n",
			       exp_type, piglit_get_gl_enum_name(exp_type),
			       type, piglit_get_gl_enum_name(type));
			printf("  name: expected %s, got %s\n",
			       exp_name, name);
		}
	}

	if (use_interface_blocks)
		free_varyings(varyings);

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}


/**
 * Compute the number of varying slots occupied by a given type.
 */
static unsigned
size_of_type(GLenum type)
{
	switch (type) {
	case GL_FLOAT:             return  1;
	case GL_FLOAT_VEC2:        return  2;
	case GL_FLOAT_VEC3:        return  3;
	case GL_FLOAT_VEC4:        return  4;
	case GL_FLOAT_MAT2:        return  4;
	case GL_FLOAT_MAT2x3:      return  6;
	case GL_FLOAT_MAT2x4:      return  8;
	case GL_FLOAT_MAT3x2:      return  6;
	case GL_FLOAT_MAT3:        return  9;
	case GL_FLOAT_MAT3x4:      return 12;
	case GL_FLOAT_MAT4x2:      return  8;
	case GL_FLOAT_MAT4x3:      return 12;
	case GL_FLOAT_MAT4:        return 16;
	case GL_INT:               return  1;
	case GL_INT_VEC2:          return  2;
	case GL_INT_VEC3:          return  3;
	case GL_INT_VEC4:          return  4;
	case GL_UNSIGNED_INT:      return  1;
	case GL_UNSIGNED_INT_VEC2: return  2;
	case GL_UNSIGNED_INT_VEC3: return  3;
	case GL_UNSIGNED_INT_VEC4: return  4;
	default:
		printf("Unexpected type: %u (%s)\n", type,
		       piglit_get_gl_enum_name(type));
		piglit_report_result(PIGLIT_FAIL);
		return 0;
	}
}


/**
 * Determine whether the given type contains floating-point values.
 */
static bool
is_floating_type(GLenum type)
{
	switch (type) {
	case GL_FLOAT:
	case GL_FLOAT_VEC2:
	case GL_FLOAT_VEC3:
	case GL_FLOAT_VEC4:
	case GL_FLOAT_MAT2:
	case GL_FLOAT_MAT2x3:
	case GL_FLOAT_MAT2x4:
	case GL_FLOAT_MAT3x2:
	case GL_FLOAT_MAT3:
	case GL_FLOAT_MAT3x4:
	case GL_FLOAT_MAT4x2:
	case GL_FLOAT_MAT4x3:
	case GL_FLOAT_MAT4:
		return true;
	case GL_INT:
	case GL_INT_VEC2:
	case GL_INT_VEC3:
	case GL_INT_VEC4:
	case GL_UNSIGNED_INT:
	case GL_UNSIGNED_INT_VEC2:
	case GL_UNSIGNED_INT_VEC3:
	case GL_UNSIGNED_INT_VEC4:
		return false;
	default:
		printf("Unexpected type: %u (%s)\n", type,
		       piglit_get_gl_enum_name(type));
		piglit_report_result(PIGLIT_FAIL);
		return false;
	}
}


/**
 * Compute the expected number of transform feedback outputs for the
 * current test.  This is used to size the transform feedback buffer.
 */
static unsigned
count_outputs()
{
	unsigned num_good_varyings = count_strings(test->good_varyings);
	unsigned i;
	unsigned num_outputs = 0;

	for (i = 0; i < num_good_varyings; i++) {
		num_outputs += size_of_type(test->expected_types[i])
			* test->expected_sizes[i];
	}
	return num_outputs;
}


/**
 * Check that the buffer pointed to by \c readback contains the
 * expected transform feedback output data.
 */
static bool
check_outputs(const void *readback)
{
	const float *readback_f = readback;
	const int *readback_i = readback;
	unsigned num_good_varyings = count_strings(test->good_varyings);
	unsigned i;
	unsigned output_component = 0;
	unsigned float_index = 0;
	unsigned int_index = 0;
	bool pass = true;

	for (i = 0; i < num_good_varyings; i++) {
		const char *varying_name = test->good_varyings[i];
		unsigned varying_size = test->expected_sizes[i]
			* size_of_type(test->expected_types[i]);
		if (is_floating_type(test->expected_types[i])) {
			unsigned j;
			for (j = 0; j < varying_size; j++) {
				float actual = readback_f[output_component];
				float expected
					= test->expected_floats[float_index];
				if (actual != expected) {
					printf("Output %s element %u: "
					       "expected %f, got %f\n",
					       varying_name, j, expected,
					       actual);
					pass = false;
				}
				output_component++;
				float_index++;
			}
		} else {
			unsigned j;
			for (j = 0; j < varying_size; j++) {
				int actual = readback_i[output_component];
				int expected = test->expected_ints[int_index];
				if (actual != expected) {
					printf("Output %s element %u: "
					       "expected %i, got %i\n",
					       varying_name, j, expected,
					       actual);
					pass = false;
				}
				output_component++;
				int_index++;
			}
		}
	}

	return pass;
}


/**
 * Call glDrawArrays, passing the given vertex data using a VAO and a
 * VBO.
 */
static void
draw_arrays(const void *verts, unsigned verts_size)
{
	GLuint vao, vbo;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, verts_size, verts, GL_STREAM_DRAW);

	glVertexAttribPointer(VERTEX_ATTRIB_POS, 4, GL_FLOAT, GL_FALSE, 0,
			      NULL);
	glEnableVertexAttribArray(VERTEX_ATTRIB_POS);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	glDisableVertexAttribArray(VERTEX_ATTRIB_POS);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDeleteBuffers(1, &vbo);
	glBindVertexArray(0);
	glDeleteVertexArrays(1, &vao);
}


/**
 * Draw a rectangle using the given coordinates.
 *
 * In an ideal world, instead of using this function we would do the
 * drawing using piglit_draw_rect(), however that function doesn't use
 * VBO's or VAO's, and hence isn't compatible with core contexts.
 */
static void
draw_rect(float x, float y, float w, float h)
{
	float verts[4][4];

	verts[0][0] = x;
	verts[0][1] = y;
	verts[0][2] = 0.0;
	verts[0][3] = 1.0;
	verts[1][0] = x + w;
	verts[1][1] = y;
	verts[1][2] = 0.0;
	verts[1][3] = 1.0;
	verts[2][0] = x;
	verts[2][1] = y + h;
	verts[2][2] = 0.0;
	verts[2][3] = 1.0;
	verts[3][0] = x + w;
	verts[3][1] = y + h;
	verts[3][2] = 0.0;
	verts[3][3] = 1.0;

	draw_arrays(verts, sizeof(verts));
}


/**
 * Render using the program and verify that it outputs the proper data
 * to the transform feedback buffer.
 *
 * The program should already be linked and stored in the global \c
 * prog.
 */
static enum piglit_result
test_xfb(bool use_rasterizer_discard)
{
	GLuint buf;
	void *initial_data;
	const void *readback;
	unsigned expected_num_outputs = count_outputs();
	unsigned buf_size
		= expected_num_outputs * NUM_VERTICES * sizeof(float);
	bool pass = true;

	/* Create transform feedback buffer and pre-load it with
	 * garbage.
	 */
	glGenBuffers(1, &buf);
	initial_data = malloc(buf_size);
	memset(initial_data, 0xcc, buf_size);
	glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, buf);
	glBufferData(GL_TRANSFORM_FEEDBACK_BUFFER, buf_size, initial_data,
		     GL_STREAM_READ);
	free(initial_data);

	/* Draw a quad filling the window, with transform feedback
	 * enabled.
	 */
	glUseProgram(prog);
	glBeginTransformFeedback(GL_TRIANGLES);
	if (use_rasterizer_discard)
		glEnable(GL_RASTERIZER_DISCARD);
	draw_rect(-1, -1, 2, 2);
	if (use_rasterizer_discard)
		glDisable(GL_RASTERIZER_DISCARD);
	glEndTransformFeedback();
	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

	/* Inspect transform feedback output. */
	readback = glMapBufferRange(GL_TRANSFORM_FEEDBACK_BUFFER, 0, buf_size,
				    GL_MAP_READ_BIT);
	pass = check_outputs(readback) && pass;
	glUnmapBuffer(GL_TRANSFORM_FEEDBACK_BUFFER);

	glDeleteBuffers(1, &buf);

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}


static void
print_usage_and_exit(const char *prog_name)
{
	unsigned i;
	printf("Usage: %s <subtest> <mode> {options}\n"
	       "  where <subtest> is one of the following:\n", prog_name);
	for (i = 0; i < ARRAY_SIZE(tests); i++)
		printf("    %s\n", tests[i].name);
	printf("  <mode> is one of the following:\n"
	       "    error\n"
	       "    get\n"
	       "    run\n"
	       "    run-no-fs\n"
	       "  and possible options are:\n"
	       "    interface - use interface blocks\n");
	piglit_report_result(PIGLIT_FAIL);
}


void
piglit_init(int argc, char **argv)
{
	unsigned i;

	/* Parse first param. */
	if (argc < 3)
		print_usage_and_exit(argv[0]);
	for (i = 0; i < ARRAY_SIZE(tests); i++) {
		if (strcmp(argv[1], tests[i].name) == 0) {
			test = &tests[i];
			break;
		}
	}
	if (test == NULL)
		print_usage_and_exit(argv[0]);

	/* Parse options. */
	for (i = 3; i < argc; i++) {
		if (strcmp(argv[i], "interface") == 0)
			use_interface_blocks = true;
		else
			print_usage_and_exit(argv[0]);
	}

	/* Parse second param and setup test */
	if (strcmp(argv[2], "error") == 0) {
		report_result(test_errors());
	} else if (strcmp(argv[2], "get") == 0) {
		link_shaders(true);
		report_result(test_gets());
	} else if (strcmp(argv[2], "run") == 0) {
		link_shaders(true);
		/* Testing will occur in piglit_display */
	} else if (strcmp(argv[2], "run-no-fs") == 0) {
		link_shaders(false);
		report_result(test_xfb(true));
	} else {
		print_usage_and_exit(argv[0]);
	}
}


enum piglit_result
piglit_display(void)
{
	static const float green[4] = {0.0, 1.0, 0.0, 1.0};
	enum piglit_result result;

	glClear(GL_COLOR_BUFFER_BIT);
	result = test_xfb(false);

	/* test_xfb() sends a set of vertices down the pipeline that
	 * should cause the entire window to be drawn, so all we need
	 * to do to make sure that the correct data got to the
	 * fragment shader is verify that it painted a green window.
	 */
	if (!piglit_probe_rect_rgba(0, 0, piglit_width, piglit_height, green))
		result = PIGLIT_FAIL;

	piglit_present_results();
	report_result(result);
	return result;
}
