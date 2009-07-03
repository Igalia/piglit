// BEGIN_COPYRIGHT -*- glean -*-
// 
// Copyright (C) 1999  Allen Akin   All Rights Reserved.
// 
// Permission is hereby granted, free of charge, to any person
// obtaining a copy of this software and associated documentation
// files (the "Software"), to deal in the Software without
// restriction, including without limitation the rights to use,
// copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following
// conditions:
// 
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the
// Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY
// KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
// WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
// PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL ALLEN AKIN BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
// AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
// OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.
// 
// END_COPYRIGHT

// tfragprog.cpp:  Test GL_ARB_fragment_program extension.
// Brian Paul  22 October 2005
//
// This is pretty simple.  Specific fragment programs are run, we read back
// the framebuffer color and compare the color to the expected result.
// Pretty much any fragment program can be tested in the manner.
// Ideally, an additional fragment program test should be developed which
// exhaustively tests instruction combinations with all the various swizzle
// and masking options, etc.
// But this test is good for regression testing to be sure that particular or
// unique programs work correctly.


#include <cstring>
#include <cassert>
#include <cmath>
#include <math.h>
#include "tfragprog1.h"


namespace GLEAN {


static PFNGLPROGRAMLOCALPARAMETER4FVARBPROC glProgramLocalParameter4fvARB_func;
static PFNGLGENPROGRAMSARBPROC glGenProgramsARB_func;
static PFNGLPROGRAMSTRINGARBPROC glProgramStringARB_func;
static PFNGLBINDPROGRAMARBPROC glBindProgramARB_func;
static PFNGLISPROGRAMARBPROC glIsProgramARB_func;
static PFNGLDELETEPROGRAMSARBPROC glDeleteProgramsARB_func;
static PFNGLGETPROGRAMIVARBPROC glGetProgramivARB_func;
static PFNGLFOGCOORDFPROC glFogCoordf_func;


// Clamp X to [0, 1]
#define CLAMP01( X )  ( (X)<(0.0) ? (0.0) : ((X)>(1.0) ? (1.0) : (X)) )
// Absolute value
#define ABS(X)  ( (X) < 0.0 ? -(X) : (X) )
// Max
#define MAX( A, B )   ( (A) > (B) ? (A) : (B) )
// Min
#define MIN( A, B )   ( (A) < (B) ? (A) : (B) )
// Duplicate value four times
#define SMEAR(X)  (X), (X), (X), (X)

#define DONT_CARE_Z -1.0
#define DONT_CARE_COLOR -1.0

#define FRAGCOLOR { 0.25, 0.75, 0.5, 0.25 }
#define PARAM0 { 0.0, 0.0, 0.0, 0.0 }
#define PARAM1 { 0.5, 0.25, 1.0, 0.5 }
#define PARAM2 { -1.0, 0.0, 0.25, -0.5 }
static const GLfloat FragColor[4] = FRAGCOLOR;
static const GLfloat Param0[4] = PARAM0;
static const GLfloat Param1[4] = PARAM1;
static const GLfloat Param2[4] = PARAM2;
static GLfloat InfNan[4];
static GLfloat FogColor[4] = {1.0, 1.0, 0.0, 0.0};
static GLfloat FogStart = 10.0;
static GLfloat FogEnd = 100.0;
static GLfloat FogDensity = 0.03;
static GLfloat FogCoord = 50.0;  /* Between FogStart and FogEnd */


// These are the specific fragment programs which we'll test
// Alphabetical order, please
static const FragmentProgram Programs[] = {
	{
		"ABS test",
		"!!ARBfp1.0\n"
		"PARAM p = program.local[2]; \n"
		"ABS result.color, p; \n"
		"END \n",
		{ ABS(Param2[0]),
		  ABS(Param2[1]),
		  ABS(Param2[2]),
		  ABS(Param2[3])
		},
		DONT_CARE_Z,
	},
	{
		"ADD test",
		"!!ARBfp1.0\n"
		"PARAM p = program.local[1]; \n"
		"ADD result.color, fragment.color, p; \n"
		"END \n",
		{ CLAMP01(FragColor[0] + Param1[0]),
		  CLAMP01(FragColor[1] + Param1[1]),
		  CLAMP01(FragColor[2] + Param1[2]),
		  CLAMP01(FragColor[3] + Param1[3])
		},
		DONT_CARE_Z
	},
	{
		"CMP test",
		"!!ARBfp1.0\n"
		"PARAM zero = program.local[0]; \n"
		"PARAM p1 = program.local[1]; \n"
		"PARAM p2 = program.local[2]; \n"
		"CMP result.color, p2, zero, p1; \n"
		"END \n",
		{ Param0[0], Param1[1], Param1[2], Param0[3] },
		DONT_CARE_Z
	},
	{
		"COS test",
		"!!ARBfp1.0\n"
		"PARAM values = { 0.0, 3.14159, 0.5, 1.0 }; \n"
		"COS result.color.x, values.x; \n"
		"COS result.color.y, values.y; \n"
		"COS result.color.z, values.z; \n"
		"COS result.color.w, values.w; \n"
		"END \n",
		{ CLAMP01(1.0),
		  CLAMP01(-1.0),
		  CLAMP01(0.8775),
		  CLAMP01(0.5403)
		},
		DONT_CARE_Z
	},
	{
		"COS test 2",
		"!!ARBfp1.0\n"
		"PARAM values = { 6.78318, 7.28318, 6.28318, -5.78318 }; \n"
		"COS result.color.x, values.x; \n"
		"COS result.color.y, values.y; \n"
		"COS result.color.z, values.z; \n"
		"COS result.color.w, values.w; \n"
		"END \n",
		{ CLAMP01(0.8775),
		  CLAMP01(0.5403),
		  CLAMP01(1.0),
		  CLAMP01(0.8775)
		},
		DONT_CARE_Z
	},
	{
		"DP3 test",
		"!!ARBfp1.0\n"
		"PARAM p1 = program.local[1]; \n"
		"DP3 result.color, p1, fragment.color; \n"
		"END \n",
		{ SMEAR(CLAMP01(Param1[0] * FragColor[0] +
                                Param1[1] * FragColor[1] +
                                Param1[2] * FragColor[2]))
		},
		DONT_CARE_Z
	},
	{
		"DP4 test",
		"!!ARBfp1.0\n"
		"PARAM p1 = program.local[1]; \n"
		"DP4 result.color, p1, fragment.color; \n"
		"END \n",
		{ SMEAR(CLAMP01(Param1[0] * FragColor[0] +
                                Param1[1] * FragColor[1] +
                                Param1[2] * FragColor[2] +
                                Param1[3] * FragColor[3]))
		},
		DONT_CARE_Z
	},
	{
		"DPH test",
		"!!ARBfp1.0\n"
		"PARAM p1 = program.local[1]; \n"
                "PARAM scale = {0.1, 0.1, 0.1, 0.1}; \n"
                "TEMP t; \n"
		"DPH t, p1, fragment.color; \n"
		"MUL result.color, t, scale; \n"
		"END \n",
		{ SMEAR(CLAMP01((Param1[0] * FragColor[0] +
                                 Param1[1] * FragColor[1] +
                                 Param1[2] * FragColor[2] +
                                 FragColor[3]) * 0.1))
		},
		DONT_CARE_Z
	},
	{
		"DST test",
		"!!ARBfp1.0\n"
		"# let d = 0.4 \n"
		"PARAM v1 = {9.9, 0.16, 0.16, 9.9}; \n"
		"PARAM v2 = {9.9, 2.5, 9.9, 2.5}; \n"
		"DST result.color, v1, v2; \n"
		"END \n",
		{ 1.0,
		  0.4,           // v1.y * v2.y
		  0.16,          // v1.z
		  CLAMP01(2.5)   // v2.w
		},
		DONT_CARE_Z
	},
	{
		"EX2 test",
		"!!ARBfp1.0\n"
                "PARAM scale = {0.01, 0.01, 0.01, 0.01}; \n"
		"PARAM values = {0.0, 1.0, 4.0, -2.0 }; \n"
                "TEMP t; \n"
		"EX2 t.x, values.x; \n"
		"EX2 t.y, values.y; \n"
		"EX2 t.z, values.z; \n"
		"EX2 t.w, values.w; \n"
		"MUL result.color, t, scale; \n"
		"END \n",
		{  1.0 * 0.01,
                   2.0 * 0.01,
                  16.0 * 0.01,
                  0.25 * 0.01 },
		DONT_CARE_Z
	},
	{
		"FLR test",
		"!!ARBfp1.0\n"
		"PARAM values = {4.8, 0.3, -0.2, 1.2}; \n"
		"PARAM scale = {0.1, 0.1, 0.1, 0.1}; \n"
		"TEMP t; \n"
		"FLR t, values; \n"
		"MUL result.color, t, scale; \n"
		"END \n",
		{ 0.4,
		  0.0,
		  CLAMP01(-0.1),
		  0.1
		},
		DONT_CARE_Z
	},
	{
		"FRC test",
		"!!ARBfp1.0\n"
		"PARAM values = {-1.1, 0.1, -2.2, 2.4 }; \n"
		"FRC result.color, values; \n"
		"END \n",
		{ 0.9, 0.1, 0.8, 0.4 },
		DONT_CARE_Z
	},
	{
		"LG2 test",
		"!!ARBfp1.0\n"
		"PARAM values = {64.0, 1, 30, 4}; \n"
		"PARAM scale = {0.1, 0.1, 0.1, 0.1}; \n"
		"TEMP t; \n"
		"LG2 t.x, values.x; \n"
		"LG2 t.y, values.y; \n"
		"LG2 t.z, values.z; \n"
		"LG2 t.w, values.w; \n"
		"MUL result.color, t, scale; \n"
		"END \n",
		{ 0.6,
		  0.0,
		  0.49,
		  0.2
		},
		DONT_CARE_Z
	},
	{
		"LIT test 1",
		"!!ARBfp1.0\n"
		"PARAM values = {0.65, 0.9, 0.0, 8.0}; \n"
		"LIT result.color, values; \n"
		"END \n",
		{ 1.0,
		  0.65,    // values.x
		  0.433,   // roughly Pow(values.y, values.w)
		  1.0
		},
		DONT_CARE_Z
	},
	{
		"LIT test 2 (degenerate case: 0 ^ 0 -> 1)",
		"!!ARBfp1.0\n"
		"PARAM values = {0.65, 0.0, 0.0, 0.0}; \n"
		"LIT result.color, values; \n"
		"END \n",
		{ 1.0,
		  0.65,    // values.x
		  1.0,     // 0^0
		  1.0
		},
		DONT_CARE_Z
	},
	{
		"LIT test 3 (case x < 0)",
		"!!ARBfp1.0\n"
		"PARAM values = {-0.5, 0.0, 0.0, 0.0}; \n"
		"LIT result.color, values; \n"
		"END \n",
		{ 1.0,
		  CLAMP01(-0.5),    // values.x
		  0.0,
		  1.0
		},
		DONT_CARE_Z
	},
        {
		"LRP test",
		"!!ARBfp1.0\n"
		"PARAM p1 = program.local[1]; \n"
		"PARAM t = {0.2, 0.5, 1.0, 0.0}; \n"
		"LRP result.color, t, fragment.color, p1; \n"
		"END \n",
		{ 0.2 * FragColor[0] + (1.0 - 0.2) * Param1[0],
		  0.5 * FragColor[1] + (1.0 - 0.5) * Param1[1],
		  1.0 * FragColor[2] + (1.0 - 1.0) * Param1[2],
		  0.0 * FragColor[3] + (1.0 - 0.0) * Param1[3]
		},
		DONT_CARE_Z
	},
	{
		"MAD test",
		"!!ARBfp1.0\n"
		"PARAM p1 = program.local[1]; \n"
		"PARAM p2 = program.local[2]; \n"
		"MAD result.color, fragment.color, p1, p2; \n"
		"END \n",
		{ CLAMP01(FragColor[0] * Param1[0] + Param2[0]),
		  CLAMP01(FragColor[1] * Param1[1] + Param2[1]),
		  CLAMP01(FragColor[2] * Param1[2] + Param2[2]),
		  CLAMP01(FragColor[3] * Param1[3] + Param2[3])
		},
		DONT_CARE_Z
	},
	{
		"MAX test",
		"!!ARBfp1.0\n"
		"PARAM p1 = program.local[1]; \n"
		"PARAM p2 = program.local[2]; \n"
		"MAX result.color, p1, p2; \n"
		"END \n",
		{ MAX(Param1[0], Param2[0]),
		  MAX(Param1[1], Param2[1]),
		  MAX(Param1[2], Param2[2]),
		  MAX(Param1[3], Param2[3]),
		},
		DONT_CARE_Z
	},
	{
		"MIN test",
		"!!ARBfp1.0\n"
		"PARAM p1 = program.local[1]; \n"
		"MIN result.color, p1, fragment.color; \n"
		"END \n",
		{ MIN(Param1[0], FragColor[0]),
		  MIN(Param1[1], FragColor[1]),
		  MIN(Param1[2], FragColor[2]),
		  MIN(Param1[3], FragColor[3]),
		},
		DONT_CARE_Z
	},
	{
		"MOV test",
		"!!ARBfp1.0\n"
		"MOV result.color, fragment.color; \n"
		"END \n",
		FRAGCOLOR,
		DONT_CARE_Z,
	},
	{
		"MUL test",
		"!!ARBfp1.0\n"
		"PARAM p = program.local[1]; \n"
		"MUL result.color, fragment.color, p; \n"
		"END \n",
		{ CLAMP01(FragColor[0] * Param1[0]),
		  CLAMP01(FragColor[1] * Param1[1]),
		  CLAMP01(FragColor[2] * Param1[2]),
		  CLAMP01(FragColor[3] * Param1[3])
		},
		DONT_CARE_Z
	},
	{
		"masked MUL test",
		"!!ARBfp1.0\n"
		"PARAM zero = program.local[0]; \n"
		"PARAM p = program.local[1]; \n"
		"MOV result.color, zero; \n"
		"MUL result.color.xy, fragment.color, p; \n"
		"END \n",
		{ CLAMP01(FragColor[0] * Param1[0]),
		  CLAMP01(FragColor[1] * Param1[1]),
		  0.0,
		  0.0
		},
		DONT_CARE_Z
	},
	{
		"POW test (exponentiation)",
		"!!ARBfp1.0\n"
		"PARAM values = {0.5, 2, 3, 4}; \n"
		"POW result.color.x, values.x, values.y; \n"
		"POW result.color.y, values.x, values.z; \n"
		"POW result.color.z, values.x, values.w; \n"
		"POW result.color.w, values.w, values.x; \n"
		"END \n",
		{ 0.5 * 0.5,
		  0.5 * 0.5 * 0.5,
		  0.5 * 0.5 * 0.5 * 0.5,
		  CLAMP01(2.0) },
		DONT_CARE_Z
	},
	{
		"RCP test (reciprocal)",
		"!!ARBfp1.0\n"
		"PARAM values = {8, -10, 1, 12 }; \n"
		"RCP result.color.x, values.x; \n"
		"RCP result.color.y, values.y; \n"
		"RCP result.color.z, values.z; \n"
		"RCP result.color.w, values.w; \n"
		"END \n",
		{ 1.0 / 8.0, CLAMP01(1.0 / -10.0), 1, 1.0 / 12.0 },
		DONT_CARE_Z
	},
	{
		/* check that RCP result is replicated across XYZW */
		"RCP test 2 (reciprocal)",
		"!!ARBfp1.0\n"
		"PARAM values = {8, -10, 1, 12 }; \n"
		"MOV result.color, values; \n"
		"RCP result.color, values.x; \n"
		"END \n",
		{ 1.0 / 8.0, 1.0 / 8.0, 1.0 / 8.0, 1.0 / 8.0 },
		DONT_CARE_Z
	},
	{
		"RSQ test 1 (reciprocal square root)",
		"!!ARBfp1.0\n"
		"PARAM values = {1, 4, 9, 100 }; \n"
		"RSQ result.color.x, values.x; \n"
		"RSQ result.color.y, values.y; \n"
		"RSQ result.color.z, values.z; \n"
		"RSQ result.color.w, values.w; \n"
		"END \n",
		{ 1.0, 0.5, 0.3333, 0.1 },
		DONT_CARE_Z
	},
	{
		"RSQ test 2 (reciprocal square root of negative value)",
		"!!ARBfp1.0\n"
		"PARAM values = {0, -100, -5, -1}; \n"
		"RSQ result.color.x, values.x; \n"
		"RSQ result.color.y, values.y; \n"
		"RSQ result.color.z, values.z; \n"
		"RSQ result.color.w, values.w; \n"
		"END \n",
		{ DONT_CARE_COLOR,
		  0.1,
		  0.447,
		  1.0,
		},
		DONT_CARE_Z
	},
	{
		"SCS test",
		"!!ARBfp1.0\n"
		"PARAM values = { 0.5, 0.5, 0.0, 0.0 }; \n"
		"SCS result.color.x, values.x; \n"
		"SCS result.color.y, values.y; \n"
		"END \n",
		{ CLAMP01(0.8775),
		  CLAMP01(0.4794),
		  DONT_CARE_COLOR,
		  DONT_CARE_COLOR,
		},
		DONT_CARE_Z
	},
	{
		"SGE test",
		"!!ARBfp1.0\n"
		"PARAM p0 = program.local[0]; \n"
		"PARAM p2 = program.local[2]; \n"
		"SGE result.color, p2, p0; \n"
		"END \n",
		{ Param2[0] >= Param0[0] ? 1.0 : 0.0,
		  Param2[1] >= Param0[1] ? 1.0 : 0.0,
		  Param2[2] >= Param0[2] ? 1.0 : 0.0,
		  Param2[3] >= Param0[3] ? 1.0 : 0.0,
		},
		DONT_CARE_Z
	},
	{
		"SIN test",
		"!!ARBfp1.0\n"
		"PARAM values = { 1.57079, -1.57079, 0.5, 1.0 }; \n"
		"SIN result.color.x, values.x; \n"
		"SIN result.color.y, values.y; \n"
		"SIN result.color.z, values.z; \n"
		"SIN result.color.w, values.w; \n"
		"END \n",
		{ CLAMP01(1.0),
		  CLAMP01(-1.0),
		  CLAMP01(0.4794),
		  CLAMP01(0.8414)
		},
		DONT_CARE_Z
	},
	{
		"SIN test 2",
		"!!ARBfp1.0\n"
		"PARAM values = { 3.14159, -3.14159, 6.78319, -5.78319 }; \n"
		"SIN result.color.x, values.x; \n"
		"SIN result.color.y, values.y; \n"
		"SIN result.color.z, values.z; \n"
		"SIN result.color.w, values.w; \n"
		"END \n",
		{ CLAMP01(0.0),
		  CLAMP01(0.0),
		  CLAMP01(0.4794),
		  CLAMP01(0.4794)
		},
		DONT_CARE_Z
	},
	{
		"SLT test",
		"!!ARBfp1.0\n"
		"PARAM p1 = program.local[1]; \n"
		"SLT result.color, fragment.color, p1; \n"
		"END \n",
		{ FragColor[0] < Param1[0] ? 1.0 : 0.0,
		  FragColor[1] < Param1[1] ? 1.0 : 0.0,
		  FragColor[2] < Param1[2] ? 1.0 : 0.0,
		  FragColor[3] < Param1[3] ? 1.0 : 0.0,
		},
		DONT_CARE_Z
	},
	{
		"SUB test (with swizzle)",
		"!!ARBfp1.0\n"
		"PARAM p1 = program.local[1]; \n"
		"SUB result.color, p1.yxwz, fragment.color.yxwz; \n"
		"END \n",
		{ CLAMP01(Param1[1] - FragColor[1]),
		  CLAMP01(Param1[0] - FragColor[0]),
		  CLAMP01(Param1[3] - FragColor[3]),
		  CLAMP01(Param1[2] - FragColor[2])
		},
		DONT_CARE_Z
	},
	{
		"SWZ test",
		"!!ARBfp1.0\n"
		"PARAM p = program.local[1]; \n"
		"SWZ result.color, p, -1,-y,z,0; \n"
		"END \n",
		{ CLAMP01(-1.0),
		  CLAMP01(-Param1[1]),
		  CLAMP01(Param1[2]),
		  CLAMP01(0.0)
		},
		DONT_CARE_Z
	},
	{
		// this test checks that SOA execution is handled correctly
		"swizzled move test",
		"!!ARBfp1.0\n"
		"TEMP t; \n"
		"PARAM p = program.local[1]; \n"
		"MOV t, p; \n"
		"MOV t, t.yxwz; \n"  // "in-place" swizzle
		"MOV result.color, t; \n"
		"END \n",
		{ Param1[1], Param1[0], Param1[3], Param1[2] },
		DONT_CARE_Z
	},
	{
		// this test checks that SOA execution is handled correctly
		"swizzled add test",
		"!!ARBfp1.0\n"
		"TEMP t; \n"
		"PARAM p = program.local[1]; \n"
		"MOV t, p; \n"
		"ADD t, t, t.yxwz; \n"  // "in-place" swizzled add
		"MOV result.color, t; \n"
		"END \n",
		{ CLAMP01(Param1[0] + Param1[1]),
		  CLAMP01(Param1[1] + Param1[0]),
		  CLAMP01(Param1[2] + Param1[3]),
		  CLAMP01(Param1[3] + Param1[2]) },
		DONT_CARE_Z
	},
	{
		"XPD test 1",
		"!!ARBfp1.0\n"
		"PARAM p1 = program.local[1]; \n"
		"PARAM p2 = program.local[2]; \n"
		"XPD result.color, p1, p2; \n"
		"END \n",
		{ CLAMP01(Param1[1] * Param2[2] - Param1[2] * Param2[1]),
		  CLAMP01(Param1[2] * Param2[0] - Param1[0] * Param2[2]),
		  CLAMP01(Param1[0] * Param2[1] - Param1[1] * Param2[0]),
		  DONT_CARE_COLOR
		},
		DONT_CARE_Z
	},
	{
		"Z-write test",
		"!!ARBfp1.0\n"
		"PARAM p = program.local[1]; \n"
		"MOV result.color, p; \n"
		"MOV result.depth.z, p.y; \n"
		"END \n",
		{ Param1[0],
		  Param1[1],
		  Param1[2],
		  Param1[3]
		},
		Param1[1]
	},

	// ============= Numeric stress tests =================================
	// Basically just check that we don't crash when we do divides by
	// zero, etc.
	{
		"Divide by zero test",
		"!!ARBfp1.0\n"
		"PARAM zero = program.local[0]; \n"
		"RCP result.color.x, zero.x; \n"
		"RCP result.color.y, zero.y; \n"
		"RCP result.color.z, zero.z; \n"
		"RCP result.color.w, zero.w; \n"
		"END \n",
		{ DONT_CARE_COLOR,
		  DONT_CARE_COLOR,
		  DONT_CARE_COLOR,
		  DONT_CARE_COLOR
		},
		DONT_CARE_Z
	},
	{
		"Infinity / nan test",
		"!!ARBfp1.0\n"
		"PARAM zero = program.local[0]; \n"
		"PARAM infNan = program.local[9]; \n"
		"ADD result.color, infNan, zero; \n"
		"END \n",
		{ DONT_CARE_COLOR,
		  DONT_CARE_COLOR,
		  DONT_CARE_COLOR,
		  DONT_CARE_COLOR
		},
		DONT_CARE_Z
	},

	// ============= Fog tests ============================================
	// Linear fog
#define FOG_FACT ((FogEnd - FogCoord) / (FogEnd - FogStart))
	{
		"ARB_fog_linear test",
		"!!ARBfp1.0\n"
		"OPTION ARB_fog_linear; \n"
		"MOV result.color, fragment.color; \n"
		"END \n",
		{ FragColor[0] * FOG_FACT + FogColor[0] * (1.0 - FOG_FACT),
		  FragColor[1] * FOG_FACT + FogColor[1] * (1.0 - FOG_FACT),
		  FragColor[2] * FOG_FACT + FogColor[2] * (1.0 - FOG_FACT),
		  FragColor[3]
		},
		DONT_CARE_Z
	},
	{
		"Computed fog linear test",
		"!!ARBfp1.0\n"
		"# fogParams.x = density \n"
		"# fogParams.y = start \n"
		"# fogParams.z = end \n"
		"# fogParams.w = 1/(end-start) \n"
		"PARAM fogParams = state.fog.params; \n"
		"ATTRIB fogCoord = fragment.fogcoord; \n"
		"PARAM fogColor = state.fog.color; \n"
		"TEMP numerator, f; \n"
		"# f = (end - coord) / (end - start) \n"
		"SUB numerator, fogParams.z, fogCoord.x; \n"
		"MUL_SAT f, numerator, fogParams.w; \n"
		"LRP result.color.rgb, f, fragment.color, fogColor; \n"
		"MOV result.color.a, fragment.color.a; \n"
		"END \n",
		{ FragColor[0] * FOG_FACT + FogColor[0] * (1.0 - FOG_FACT),
		  FragColor[1] * FOG_FACT + FogColor[1] * (1.0 - FOG_FACT),
		  FragColor[2] * FOG_FACT + FogColor[2] * (1.0 - FOG_FACT),
		  FragColor[3]
		},
		DONT_CARE_Z
	},
#undef FOG_FACT

	// Exp fog
#define FOG_FACT 0.2231   // = exp(-Density * Coord)
	{
		"ARB_fog_exp test",
		"!!ARBfp1.0\n"
		"OPTION ARB_fog_exp; \n"
		"MOV result.color, fragment.color; \n"
		"END \n",
		{ FragColor[0] * FOG_FACT + FogColor[0] * (1.0 - FOG_FACT),
		  FragColor[1] * FOG_FACT + FogColor[1] * (1.0 - FOG_FACT),
		  FragColor[2] * FOG_FACT + FogColor[2] * (1.0 - FOG_FACT),
		  FragColor[3]
		},
		DONT_CARE_Z
	},
#undef FOG_FACT
#define FOG_FACT 0.3535   // = ex2(-Density * Coord)
	{
		// NOTE: we could also do this with the POW instruction
		"Computed fog exp test",
		"!!ARBfp1.0\n"
		"# fogParams.x = density \n"
		"# fogParams.y = start \n"
		"# fogParams.z = end \n"
		"# fogParams.w = 1/(end-start) \n"
		"PARAM fogParams = state.fog.params; \n"
		"ATTRIB fogCoord = fragment.fogcoord; \n"
		"PARAM fogColor = state.fog.color; \n"
		"TEMP f, dc; \n"
		"# f = exp(-density * coord) \n"
		"MUL dc.x, fogParams.x, fogCoord.x; \n"
		"EX2_SAT f, -dc.x; \n"
		"LRP result.color.rgb, f, fragment.color, fogColor; \n"
		"MOV result.color.a, fragment.color.a; \n"
		"END \n",
		{ FragColor[0] * FOG_FACT + FogColor[0] * (1.0 - FOG_FACT),
		  FragColor[1] * FOG_FACT + FogColor[1] * (1.0 - FOG_FACT),
		  FragColor[2] * FOG_FACT + FogColor[2] * (1.0 - FOG_FACT),
                  FragColor[3]
		},
		DONT_CARE_Z
	},
#undef FOG_FACT

	// Exp2 fog
#define FOG_FACT 0.1054   // = exp(-(Density * Coord)^2)
	{
		"ARB_fog_exp2 test",
		"!!ARBfp1.0\n"
		"OPTION ARB_fog_exp2; \n"
		"MOV result.color, fragment.color; \n"
		"END \n",
		{ FragColor[0] * FOG_FACT + FogColor[0] * (1.0 - FOG_FACT),
		  FragColor[1] * FOG_FACT + FogColor[1] * (1.0 - FOG_FACT),
		  FragColor[2] * FOG_FACT + FogColor[2] * (1.0 - FOG_FACT),
		  FragColor[3]
		},
		DONT_CARE_Z
	},
#undef FOG_FACT
#define FOG_FACT 0.2102   // = ex2(-(Density * Coord)^2)
	{
		// NOTE: we could also do this with the POW instruction
		"Computed fog exp2 test",
		"!!ARBfp1.0\n"
		"# fogParams.x = density \n"
		"# fogParams.y = start \n"
		"# fogParams.z = end \n"
		"# fogParams.w = 1/(end-start) \n"
		"PARAM fogParams = state.fog.params; \n"
		"ATTRIB fogCoord = fragment.fogcoord; \n"
		"PARAM fogColor = state.fog.color; \n"
		"TEMP f, dc; \n"
		"# f = exp(-(density * coord)^2) \n"
		"MUL dc.x, fogParams.x, fogCoord.x; \n"
		"MUL dc.x, dc.x, dc.x; \n"
		"EX2_SAT f, -dc.x; \n"
		"LRP result.color.rgb, f, fragment.color, fogColor; \n"
		"MOV result.color.a, fragment.color.a; \n"
		"END \n",
		{ FragColor[0] * FOG_FACT + FogColor[0] * (1.0 - FOG_FACT),
		  FragColor[1] * FOG_FACT + FogColor[1] * (1.0 - FOG_FACT),
		  FragColor[2] * FOG_FACT + FogColor[2] * (1.0 - FOG_FACT),
		  FragColor[3]
		},
		DONT_CARE_Z
	},
#undef FOG_FACT

	// XXX add lots more tests here!
	{ NULL, NULL, {0,0,0,0}, 0 } // end of list sentinal
};



void
FragmentProgramTest::setup(void)
{
	// setup Infinity, Nan values
	int nan;
	float *nanPtr;

	nan = (0xff << 23) | (1 << 0);
	nanPtr = (float *) &nan;
	InfNan[0] = HUGE_VAL;
	InfNan[1] = -HUGE_VAL;
	InfNan[2] = (float) (*nanPtr);
	InfNan[3] = 1.0 / HUGE_VAL;

	// get function pointers
	glProgramLocalParameter4fvARB_func = (PFNGLPROGRAMLOCALPARAMETER4FVARBPROC) GLUtils::getProcAddress("glProgramLocalParameter4fvARB");
	assert(glProgramLocalParameter4fvARB_func);

	glGenProgramsARB_func = (PFNGLGENPROGRAMSARBPROC) GLUtils::getProcAddress("glGenProgramsARB");
	assert(glGenProgramsARB_func);

	glProgramStringARB_func = (PFNGLPROGRAMSTRINGARBPROC) GLUtils::getProcAddress("glProgramStringARB");
	assert(glProgramStringARB_func);

	glBindProgramARB_func = (PFNGLBINDPROGRAMARBPROC) GLUtils::getProcAddress("glBindProgramARB");
	assert(glBindProgramARB_func);

	glIsProgramARB_func = (PFNGLISPROGRAMARBPROC) GLUtils::getProcAddress("glIsProgramARB");
	assert(glIsProgramARB_func);

	glDeleteProgramsARB_func = (PFNGLDELETEPROGRAMSARBPROC) GLUtils::getProcAddress("glDeleteProgramsARB");
	assert(glDeleteProgramsARB_func);

	glGetProgramivARB_func = (PFNGLGETPROGRAMIVARBPROC) GLUtils::getProcAddress("glGetProgramivARB");
	assert(glGetProgramivARB_func);

	glFogCoordf_func = (PFNGLFOGCOORDFPROC) GLUtils::getProcAddress("glFogCoordf");
	assert(glFogCoordf_func);

	GLuint progID;
	glGenProgramsARB_func(1, &progID);
	glBindProgramARB_func(GL_FRAGMENT_PROGRAM_ARB, progID);
	glEnable(GL_FRAGMENT_PROGRAM_ARB);

	// load program inputs
	glColor4fv(FragColor);
	glProgramLocalParameter4fvARB_func(GL_FRAGMENT_PROGRAM_ARB, 0, Param0);
	glProgramLocalParameter4fvARB_func(GL_FRAGMENT_PROGRAM_ARB, 1, Param1);
	glProgramLocalParameter4fvARB_func(GL_FRAGMENT_PROGRAM_ARB, 2, Param2);
	glProgramLocalParameter4fvARB_func(GL_FRAGMENT_PROGRAM_ARB, 9, InfNan);

	GLenum err = glGetError();
	assert(!err);  // should be OK

	// setup vertex transform (we'll draw a quad in middle of window)
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
#if DEVEL_MODE
	glOrtho(-1.0, 1.0, -1.0, 1.0, 0.0, 1.0);
#else
	glOrtho(-4.0, 4.0, -4.0, 4.0, 0.0, 1.0);
#endif
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glDrawBuffer(GL_FRONT);
	glReadBuffer(GL_FRONT); 

	// other GL state
	glFogf(GL_FOG_START, FogStart);
	glFogf(GL_FOG_END, FogEnd);
	glFogf(GL_FOG_DENSITY, FogDensity);
	glFogfv(GL_FOG_COLOR, FogColor);
	glFogi(GL_FOG_COORDINATE_SOURCE_EXT, GL_FOG_COORDINATE_EXT);
	glFogCoordf_func(FogCoord);

	// compute error tolerances (may need fine-tuning)
	int bufferBits[5];
	glGetIntegerv(GL_RED_BITS, &bufferBits[0]);
	glGetIntegerv(GL_GREEN_BITS, &bufferBits[1]);
	glGetIntegerv(GL_BLUE_BITS, &bufferBits[2]);
	glGetIntegerv(GL_ALPHA_BITS, &bufferBits[3]);
	glGetIntegerv(GL_DEPTH_BITS, &bufferBits[4]);

	tolerance[0] = 2.0 / (1 << bufferBits[0]);
	tolerance[1] = 2.0 / (1 << bufferBits[1]);
	tolerance[2] = 2.0 / (1 << bufferBits[2]);
	if (bufferBits[3])
		tolerance[3] = 2.0 / (1 << bufferBits[3]);
	else
		tolerance[3] = 1.0;
	if (bufferBits[4])
		tolerance[4] = 16.0 / (1 << bufferBits[4]);
	else
		tolerance[4] = 1.0;
}


void
FragmentProgramTest::reportFailure(const char *programName,
				   const GLfloat expectedColor[4],
				   const GLfloat actualColor[4] ) const
{
	env->log << "FAILURE:\n";
	env->log << "  Program: " << programName << "\n";
	env->log << "  Expected color: ";
	env->log << expectedColor[0] << ", ";
	env->log << expectedColor[1] << ", ";
	env->log << expectedColor[2] << ", ";
	env->log << expectedColor[3] << "\n";
	env->log << "  Observed color: ";
	env->log << actualColor[0] << ", ";
	env->log << actualColor[1] << ", ";
	env->log << actualColor[2] << ", ";
	env->log << actualColor[3] << "\n";
}


void
FragmentProgramTest::reportZFailure(const char *programName,
				    GLfloat expectedZ, GLfloat actualZ) const
{
	env->log << "FAILURE:\n";
	env->log << "  Program: " << programName << "\n";
	env->log << "  Expected Z: " << expectedZ << "\n";
	env->log << "  Observed Z: " << actualZ << "\n";
}


// Compare actual and expected colors
bool
FragmentProgramTest::equalColors(const GLfloat act[4], const GLfloat exp[4]) const
{
	if (fabsf(act[0] - exp[0]) > tolerance[0] && exp[0] != DONT_CARE_COLOR)
		return false;
	if (fabsf(act[1] - exp[1]) > tolerance[1] && exp[1] != DONT_CARE_COLOR)
		return false;
	if (fabsf(act[2] - exp[2]) > tolerance[2] && exp[2] != DONT_CARE_COLOR)
		return false;
	if (fabsf(act[3] - exp[3]) > tolerance[3] && exp[3] != DONT_CARE_COLOR)
		return false;
	return true;
}


bool
FragmentProgramTest::equalDepth(GLfloat z0, GLfloat z1) const
{
	if (fabsf(z0 - z1) > tolerance[4])
		return false;
	else
		return true;
}


bool
FragmentProgramTest::testProgram(const FragmentProgram &p)
{
	glProgramStringARB_func(GL_FRAGMENT_PROGRAM_ARB,
				GL_PROGRAM_FORMAT_ASCII_ARB,
				strlen(p.progString),
				(const GLubyte *) p.progString);

	GLenum err = glGetError();
	if (err) {
		env->log << "OpenGL error " << (int) err << "\n";
		env->log << "Invalid Fragment Program:\n";
		env->log << p.progString;
                env->log << glGetString(GL_PROGRAM_ERROR_STRING_ARB) << "\n";
		return false;
	}

	// to avoid potential issue with undefined result.depth.z
	if (p.expectedZ == DONT_CARE_Z)
		glDisable(GL_DEPTH_TEST);
	else
		glEnable(GL_DEPTH_TEST);

#if !DEVEL_MODE
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
#endif
	glBegin(GL_POLYGON);
	glVertex2f(-1, -1);
	glVertex2f( 1, -1);
	glVertex2f( 1,  1);
	glVertex2f(-1,  1);
	glEnd();

#if !DEVEL_MODE
	GLfloat pixel[4];
	glReadPixels(windowWidth / 2, windowHeight / 2, 1, 1,
		     GL_RGBA, GL_FLOAT, pixel);

        if (0) // debug
           printf("%s: Expect: %.3f %.3f %.3f %.3f  found: %.3f %.3f %.3f %.3f\n",
                  p.name,
                  p.expectedColor[0], p.expectedColor[1],
                  p.expectedColor[2], p.expectedColor[3], 
                  pixel[0], pixel[1], pixel[2], pixel[3]);

	if (!equalColors(pixel, p.expectedColor)) {
		reportFailure(p.name, p.expectedColor, pixel);
		return false;
	}

	if (p.expectedZ != DONT_CARE_Z) {
		GLfloat z;
		glReadPixels(windowWidth / 2, windowHeight / 2, 1, 1,
			     GL_DEPTH_COMPONENT, GL_FLOAT, &z);
		if (!equalDepth(z, p.expectedZ)) {
			reportZFailure(p.name, p.expectedZ, z);
			return false;
		}
	}
#endif
	return true;
}

void
FragmentProgramTest::runOne(MultiTestResult &r, Window &w)
{
	// to test a single sub-test, set the name here:
	const char *single = NULL;

	(void) w;
	setup();

#if DEVEL_MODE
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
#endif
	for (int i = 0; Programs[i].name; i++) {

		if (!single || strcmp(single, Programs[i].name) == 0) {

#if DEVEL_MODE
			glViewport(0, i * 20, windowWidth, 20);
#endif
			if (!testProgram(Programs[i])) {
				r.numFailed++;
			}
			else {
				r.numPassed++;
			}
		}
	}

#if DEVEL_MODE
	glFinish();
	sleep(100);
#endif
	r.pass = (r.numFailed == 0);
}


// The test object itself:
FragmentProgramTest fragmentProgramTest("fragProg1", "window, rgb, z",
	"GL_ARB_fragment_program",
	"Fragment Program test 1: test a specific set of fragment programs.\n");



} // namespace GLEAN
