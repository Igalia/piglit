// BEGIN_COPYRIGHT -*- glean -*-
// 
// Copyright (C) 1999  Allen Akin   All Rights Reserved.
// Copyright (C) 2008  VMware, Inc.  All Rights Reserved.
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

// tglsl1.h:  Test OpenGL shading language
// Brian Paul  6 March 2007

#define GL_GLEXT_PROTOTYPES

#include <stdlib.h>
#include <cassert>
#include <cstring>
#include <math.h>
#include "tglsl1.h"


namespace GLEAN {

#define FLAG_NONE             0x0
#define FLAG_LOOSE            0x1 // to indicate a looser tolerance test is needed
#define FLAG_ILLEGAL_SHADER   0x2  // the shader test should not compile
#define FLAG_ILLEGAL_LINK     0x4  // the shaders should not link
#define FLAG_VERSION_1_20     0x8  // GLSL 1.20 test
#define FLAG_VERSION_1_30     0x10  // GLSL 1.30 test
#define FLAG_WINDING_CW       0x20  // clockwise-winding polygon
#define FLAG_VERTEX_TEXTURE   0x40
#define FLAG_ARB_DRAW_BUFFERS 0x80

#define DONT_CARE_Z -1.0

#define NO_VERTEX_SHADER NULL
#define NO_FRAGMENT_SHADER NULL

#define PRIMARY_R 0.25
#define PRIMARY_G 0.75
#define PRIMARY_B 0.5
#define PRIMARY_A 0.25
#define SECONDARY_R 0.0
#define SECONDARY_G 0.25
#define SECONDARY_B 0.25
#define SECONDARY_A 1.0

#define AMBIENT { 0.2, 0.4, 0.6, 0.8 }
#define LIGHT_DIFFUSE { 0.1, 0.3, 0.5, 0.7 }
#define MAT_DIFFUSE { 0.1, 0.3, 0.5, 0.7 }
#define DIFFUSE_PRODUCT { 0.01, 0.09, 0.25, 0.7 } // note alpha!

#define UNIFORM1 {1.0, 0.25, 0.75, 0.0 }  // don't change!

#define PSIZE 3.0
#define PSIZE_MIN 2.0
#define PSIZE_MAX 8.0
#define PSIZE_THRESH 1.5
#define PSIZE_ATTEN0 4.0
#define PSIZE_ATTEN1 5.0
#define PSIZE_ATTEN2 6.0

#define FOG_START 100.0
#define FOG_END   200.0
#define FOG_R 1.0
#define FOG_G 0.5
#define FOG_B 1.0
#define FOG_A 0.0

static const GLfloat PrimaryColor[4] = { PRIMARY_R, PRIMARY_G,
					 PRIMARY_B, PRIMARY_A };
static const GLfloat SecondaryColor[4] = { SECONDARY_R, SECONDARY_G,
					   SECONDARY_B, SECONDARY_A };

static const GLfloat Ambient[4] = AMBIENT;
static const GLfloat MatDiffuse[4] = MAT_DIFFUSE;
static const GLfloat LightDiffuse[4] = LIGHT_DIFFUSE;

static const GLfloat Uniform1[4] = UNIFORM1;
static const GLfloat UniformArray[4] = { 0.1, 0.25, 0.5, 0.75 };
static const GLfloat UniformArray4[4][4] = {
   { 0.1, 0.2, 0.3, 0.4 },
   { 0.9, 0.8, 0.7, 0.6 },
   { 0.5, 0.6, 0.7, 0.5 },
   { 0.3, 0.4, 0.5, 0.6 }
};

static const GLfloat PointAtten[3] = { PSIZE_ATTEN0, PSIZE_ATTEN1, PSIZE_ATTEN2 };
static const GLfloat FogColor[4] = { FOG_R, FOG_G, FOG_B, FOG_A };

// Shader program test cases
static const ShaderProgram Programs[] = {
	// Simple tests =======================================================
	{
		"Directly set fragment color",  // name
		NO_VERTEX_SHADER,  // vertex shader
		// fragment shader:
		"void main() { \n"
		"   gl_FragColor = vec4(1.0, 0.5, 0.25, 0.0); \n"
		"} \n",
		{ 1.0, 0.5, 0.25, 0.0 }, // expectedColor
		DONT_CARE_Z,  // expectedZ
		FLAG_NONE  // flags
	},

	{
		"Directly set vertex color",
		"void main() { \n"
		"   gl_Position = ftransform(); \n"
		"   gl_FrontColor = vec4(0.5, 1.0, 0.25, 0.0); \n"
		"} \n",
		NO_FRAGMENT_SHADER,
		{ 0.5, 1.0, 0.25, 0.0 },
		DONT_CARE_Z,
		FLAG_NONE
	},

	{
		"Pass-through vertex color",
		// vert shader:
		"void main() { \n"
		"   gl_Position = ftransform(); \n"
		"   gl_FrontColor = vec4(0.25, 1.0, 0.75, 0.0); \n"
		"} \n",
		// frag shader:
		"void main() { \n"
		"   gl_FragColor = gl_Color; \n"
		"} \n",
		{ 0.25, 1.0, 0.75, 0.0 },
		DONT_CARE_Z,
		FLAG_NONE
	},

	{
		"Primary plus secondary color",
		// vert shader:
		"void main() { \n"
		"   gl_Position = ftransform(); \n"
		"   gl_FrontColor = gl_Color + gl_SecondaryColor; \n"
		"} \n",
		// frag shader:
		"void main() { \n"
		"   gl_FragColor = gl_Color; \n"
		"} \n",
		{ PRIMARY_R + SECONDARY_R,
		  PRIMARY_G + SECONDARY_G,
		  PRIMARY_B + SECONDARY_B,
		  1.0 /*PRIMARY_A + SECONDARY_A*/ },
		DONT_CARE_Z,
		FLAG_NONE
	},

	{
		"Empty blocks ({}), nil (;) statements",
		NO_VERTEX_SHADER,
		"void main() { \n"
		"   {} \n"   // empty block
		"   ; \n"    // nil statement
		"   gl_FragColor = vec4(1.0, 0.5, 0.25, 0.0); \n"
		"} \n",
		{ 1.0, 0.5, 0.25, 0.0 },
		DONT_CARE_Z,
		FLAG_NONE
	},

	{
		"Global vars and initializers",
		NO_VERTEX_SHADER,
		"vec4 c = vec4(1.0, 0.5, 0.25, 0.0); \n"
		"void main() { \n"
		"   gl_FragColor = c; \n"
		"} \n",
		{ 1.0, 0.5, 0.25, 0.0 },
		DONT_CARE_Z,
		FLAG_NONE
	},

	{
		"Global vars and initializers (2)",
		NO_VERTEX_SHADER,
		"vec4 c1 = vec4(0.4, 0.5, 0.25, 0.0); \n"
		"vec4 c2 = vec4(0.3, 0.5, 0.5,  0.4); \n"
		"vec4 c3 = c1 + c2; \n"
		"void main() { \n"
		"   gl_FragColor = c3; \n"
		"} \n",
		{ 0.7, 1.0, 0.75, 0.4 },
		DONT_CARE_Z,
		FLAG_NONE
	},

	{
		"Integer Literals",
		"void main() { \n"
		"   int i = 16;   // Decimal \n"
		"   int j = 0x10; // Hexadecimal \n"
		"   int k = 020;  // Octal \n"
		"   gl_FrontColor = vec4(i, j, k, 16) / 32.0; \n"
		"   gl_Position = ftransform(); \n"
		"} \n",
		NO_FRAGMENT_SHADER,
		{ 0.5, 0.5, 0.5, 0.5 },
		DONT_CARE_Z,
		FLAG_NONE
	},

	{
		"Float Literals",
		"void main() { \n"
		"   float x = 0.5e0; \n"
		"   float y = 5.0e-1; \n"
		"   float z = -(-0.05e1); \n"
		"   float w = 0.5; \n"
		"   gl_FrontColor = vec4(x, y, z, w); \n"
		"   gl_Position = ftransform(); \n"
		"} \n",
		NO_FRAGMENT_SHADER,
		{ 0.5, 0.5, 0.5, 0.5 },
		DONT_CARE_Z,
		FLAG_NONE
	},

	// Swizzle, writemask =================================================
	{
		"Swizzle",
		NO_VERTEX_SHADER,
		"void main() { \n"
		"   vec4 a = vec4(0.5,  0.25, 0.0, 1.0); \n"
		"   gl_FragColor = a.yxxz; \n"
		"} \n",
		{ 0.25, 0.5, 0.5, 0.0 },
		DONT_CARE_Z,
		FLAG_NONE
	},

	{
		"Swizzle (rgba)",
		NO_VERTEX_SHADER,
		"void main() { \n"
		"   vec4 a = vec4(0.5,  0.25, 0.0, 1.0); \n"
		"   gl_FragColor = a.grrb; \n"
		"} \n",
		{ 0.25, 0.5, 0.5, 0.0 },
		DONT_CARE_Z,
		FLAG_NONE
	},

	{
		"Swizzle (stpq)",
		NO_VERTEX_SHADER,
		"void main() { \n"
		"   vec4 a = vec4(0.5,  0.25, 0.0, 1.0); \n"
		"   gl_FragColor = a.tssp; \n"
		"} \n",
		{ 0.25, 0.5, 0.5, 0.0 },
		DONT_CARE_Z,
		FLAG_NONE
	},

	{
		"Writemask",
		NO_VERTEX_SHADER,
		"void main() { \n"
		"   gl_FragColor = vec4(1.0); \n"
		"   gl_FragColor.x = 0.5; \n"
		"   gl_FragColor.z = 0.25; \n"
		"} \n",
		{ 0.5, 1.0, 0.25, 1.0 },
		DONT_CARE_Z,
		FLAG_NONE
	},

	{
		"Swizzled writemask",
		NO_VERTEX_SHADER,
		"void main() { \n"
		"   gl_FragColor.zwxy = vec4(1.0, 0.5, 0.25, 0.75); \n"
		"} \n",
		{ 0.25, 0.75, 1.0, 0.5 },
		DONT_CARE_Z,
		FLAG_NONE
	},

	{
		"Swizzled writemask (2)",
		NO_VERTEX_SHADER,
		"void main() { \n"
		"   gl_FragColor.zy = vec2(1.0, 0.5); \n"
		"   gl_FragColor.wx = vec2(0.25, 0.75); \n"
		"} \n",
		{ 0.75, 0.5, 1.0, 0.25 },
		DONT_CARE_Z,
		FLAG_NONE
	},

	{
		"Swizzled writemask (rgba)",
		NO_VERTEX_SHADER,
		"void main() { \n"
		"   gl_FragColor.bg = vec2(1.0, 0.5); \n"
		"   gl_FragColor.ar = vec2(0.25, 0.75); \n"
		"} \n",
		{ 0.75, 0.5, 1.0, 0.25 },
		DONT_CARE_Z,
		FLAG_NONE
	},

	{
		"Swizzled writemask (stpq)",
		NO_VERTEX_SHADER,
		"void main() { \n"
		"   gl_FragColor.pt = vec2(1.0, 0.5); \n"
		"   gl_FragColor.qs = vec2(0.25, 0.75); \n"
		"} \n",
		{ 0.75, 0.5, 1.0, 0.25 },
		DONT_CARE_Z,
		FLAG_NONE
	},

	{
		"Swizzled expression",
		NO_VERTEX_SHADER,
		"void main() { \n"
		"   vec4 a = vec4(1, 1, 1, 1); \n"
		"   vec4 b = vec4(0.5, 0.2, 0.1, 0.8); \n"
		"   vec4 c = (a * b).wzyx; \n"
		"   gl_FragColor = c; \n"
		"} \n",
		{ 0.8, 0.1, 0.2, 0.5 },
		DONT_CARE_Z,
		FLAG_NONE
	},

	{
		// This test targets SOA implementations where we have to
		// check for SOA dependencies.
		"Swizzle in-place",
		NO_VERTEX_SHADER,
		"void main() { \n"
		"   vec4 a = vec4(0.5, 0.2, 0.1, 0.8); \n"
		"   a = a.yxwz; \n"
		"   gl_FragColor = a; \n"
		"} \n",
		{ 0.2, 0.5, 0.8, 0.1 },
		DONT_CARE_Z,
		FLAG_NONE
	},

	{
		"Swizzled swizzle",
		NO_VERTEX_SHADER,
		"void main() { \n"
		"   vec4 a = vec4(0.1, 0.2, 0.3, 0.4); \n"
		"   vec4 b = a.wzyx.yxwz; \n"
		"   gl_FragColor = b; \n"
		"} \n",
		{ 0.3, 0.4, 0.1, 0.2 },
		DONT_CARE_Z,
		FLAG_NONE
	},

	{
		"Swizzled swizzled swizzle",
		NO_VERTEX_SHADER,
		"void main() { \n"
		"   vec4 a = vec4(0.1, 0.2, 0.3, 0.4); \n"
		"   vec4 b = a.wzyx.yxwz.xxyz; \n"
		"   gl_FragColor = b; \n"
		"} \n",
		{ 0.3, 0.3, 0.4, 0.1 },
		DONT_CARE_Z,
		FLAG_NONE
	},


	// Z-write ============================================================
	{
		"gl_FragDepth writing",
		NO_VERTEX_SHADER,
		"void main() { \n"
		"   gl_FragColor = vec4(0.5); \n"
		"   gl_FragDepth = 0.25; \n"
		"} \n",
		{ 0.5, 0.5, 0.5, 0.5 },
		0.25,  // Z value
		FLAG_NONE
	},

	// Basic arithmetic ===================================================
	{
		"chained assignment",
		NO_VERTEX_SHADER,
		"void main() { \n"
		"   float x, y, z; \n"
		"   x = y = z = 0.25; \n"
		"   gl_FragColor = vec4(x + y + z); \n"
		"} \n",
		{ 0.75, 0.75, 0.75, 0.75 },
		DONT_CARE_Z,
		FLAG_NONE
	},

	{
		"assignment operators",
		NO_VERTEX_SHADER,
		"void main() { \n"
		"   vec4 v = vec4(0.0, 0.25, 0.5, 0.75); \n"
		"   v *= 2.0; \n"
		"   v -= vec4(-0.5, 0.0, 0.25, 1.0); \n"
		"   gl_FragColor = v; \n"
		"} \n",
		{ 0.5, 0.5, 0.75, 0.5 },
		DONT_CARE_Z,
		FLAG_NONE
	},

	// built-in functions ================================================
	{
		// This is a Mesa regression test (bump.c)
		"cross() function, in-place",
		NO_VERTEX_SHADER,
		"void main() { \n"
		"   vec3 u,v ; \n"
		"   u.x = 0.8; \n"
		"   u.y = -0.5; \n"
		"   u.z = 1.0; \n"
		"   v.x = 0.1; \n"
		"   v.y = 0.5; \n"
		"   v.z = -2.0; \n"
		"   u = cross(u, v); \n"
		"   gl_FragColor.xyz = u; \n"
		"   gl_FragColor.w = 1.0; \n"
		"} \n",
		{ 0.502, 1.0, 0.4509, 1.0 },
		DONT_CARE_Z,
		FLAG_NONE
	},

	// Floating Point Precision ==========================================
	{
		"precision exp2",
		NO_VERTEX_SHADER,
		"uniform vec4 uniform1; \n"
		"void main() { \n"
		"   vec4 vals = vec4(-0.999992, -0.988281, -0.535149, -0.496090); \n"
		"   vals *= uniform1.xxxx; // multply by one \n"
		"   vec4 actual = exp2(vals); \n"
		"   vec4 expected = vec4(0.500003, 0.504078, 0.690087, 0.709026); \n"
		"   vec4 error = abs((actual - expected) / expected); \n"
		"   gl_FragColor = vec4(lessThan(error, vec4(1e-04))); \n"
		"} \n",
		{ 1.0, 1.0, 1.0, 1.0 },
		DONT_CARE_Z,
		FLAG_NONE
	},

	{
		"precision log2",
		NO_VERTEX_SHADER,
		"uniform vec4 uniform1; \n"
		"void main() { \n"
		"   vec4 vals = vec4(0.125096, 0.250265, 0.500301, 2.001205); \n"
		"   vals *= uniform1.xxxx; // multiply by one \n"
		"   vec4 actual = log2(vals); \n"
		"   vec4 expected = vec4(-2.998889, -1.998471, -0.999131, 1.000869); \n"
		"   vec4 error = abs(actual - expected); \n"
		"   gl_FragColor = vec4(lessThan(error, vec4(1e-05))); \n"
		"} \n",
		{ 1.0, 1.0, 1.0, 1.0 },
		DONT_CARE_Z,
		FLAG_NONE
	},

	// Flow Control ======================================================
	{
		"simple if statement, fragment shader",
		NO_VERTEX_SHADER,
		"void main() { \n"
		"   // this should always be true \n"
		"   if (gl_FragCoord.x >= 0.0) { \n"
		"      gl_FragColor = vec4(0.5, 0.0, 0.5, 0.0); \n"
		"   } \n"
		"} \n",
		{ 0.5, 0.0, 0.5, 0.0 },
		DONT_CARE_Z,
		FLAG_NONE
	},

	{
		"simple if statement, vertex shader",
		"uniform vec4 uniform1; \n"
		"void main() { \n"
		"   gl_Position = ftransform(); \n"
		"   gl_FrontColor = vec4(0.0); \n"
		"   // this should always be true \n"
		"   if (uniform1.x >= 0.0) { \n"
		"      gl_FrontColor = vec4(0.5, 0.0, 0.5, 0.0); \n"
		"   } \n"
		"} \n",
		NO_FRAGMENT_SHADER,
		{ 0.5, 0.0, 0.5, 0.0 },
		DONT_CARE_Z,
		FLAG_NONE
	},

	{
		"simple if statement (scalar test)",
		NO_VERTEX_SHADER,
		"void main() { \n"
		"   float x = 1.0; \n"
		"   if (x != 0.0) { \n"
		"      gl_FragColor = vec4(0.5, 0.0, 0.5, 0.0); \n"
		"   } \n"
		"} \n",
		{ 0.5, 0.0, 0.5, 0.0 },
		DONT_CARE_Z,
		FLAG_NONE
	},

	{
		"simple if-else statement, fragment shader",
		NO_VERTEX_SHADER,
		"void main() { \n"
		"   // this should always be false \n"
		"   if (gl_FragCoord.x < 0.0) { \n"
		"      gl_FragColor = vec4(0.0, 0.0, 0.0, 0.0); \n"
		"   } else { \n"
		"      gl_FragColor = vec4(0.5, 0.25, 0.5, 0.0); \n"
		"   } \n"
		"} \n",
		{ 0.5, 0.25, 0.5, 0.0 },
		DONT_CARE_Z,
		FLAG_NONE
	},

	{
		"simple if-else statement, vertex shader",
		"uniform vec4 uniform1; \n"
		"void main() { \n"
		"   gl_Position = ftransform(); \n"
		"   // this should always be true \n"
		"   if (uniform1.x >= 0.0) { \n"
		"      gl_FrontColor = vec4(0.0, 1.0, 0.0, 0.0); \n"
		"   } else { \n"
		"      gl_FrontColor = vec4(1.0, 0.0, 0.0, 0.0); \n"
		"   } \n"
		"} \n",
		NO_FRAGMENT_SHADER,
		{ 0.0, 1.0, 0.0, 0.0 },
		DONT_CARE_Z,
		FLAG_NONE
	},

	{
		"while-loop",
		NO_VERTEX_SHADER,
		"void main() { \n"
		"   float sum = 0.0; \n"
		"   while (sum < 0.499999) { \n"
		"      sum += 0.1; \n"
		"   } \n"
		"   gl_FragColor = vec4(sum); \n"
		"} \n",
		{ 0.5, 0.5, 0.5, 0.5 },
		DONT_CARE_Z,
		FLAG_NONE
	},

	{
		"do-loop",
		NO_VERTEX_SHADER,
		"void main() { \n"
		"   float sum = 0.0; \n"
		"   do { \n"
		"      sum += 0.1; \n"
		"   } while (sum < 0.499999); \n"
		"   gl_FragColor = vec4(sum); \n"
		"} \n",
		{ 0.5, 0.5, 0.5, 0.5 },
		DONT_CARE_Z,
		FLAG_NONE
	},

	{
		"for-loop",
		NO_VERTEX_SHADER,
		"void main() { \n"
		"   vec4 sum = vec4(0.0); \n"
		"   int i; \n"
		"   for (i = 0; i < 5; ++i) { \n"
		"      sum += vec4(0.1); \n"
		"   } \n"
		"   gl_FragColor = sum; \n"
		"} \n",
		{ 0.5, 0.5, 0.5, 0.5 },
		DONT_CARE_Z,
		FLAG_NONE
	},

	{
		"while-loop with continue",
		NO_VERTEX_SHADER,
		"void main() { \n"
		"   int i = 0; \n"
		"   float sum = 0.0; \n"
		"   while (i < 20) { \n"
		"      ++i; \n"
		"      if (i > 5) \n"
		"         continue; \n"
		"      sum += 0.1; \n"
		"   } \n"
		"   gl_FragColor = vec4(sum); \n"
		"} \n",
		{ 0.5, 0.5, 0.5, 0.5 },
		DONT_CARE_Z,
		FLAG_NONE
	},

	{
		"for-loop with continue",
		NO_VERTEX_SHADER,
		"void main() { \n"
		"   int i; \n"
		"   float sum = 0.0; \n"
		"   for (i = 0; i < 20; ++i) { \n"
		"      if (i > 4) \n"
		"         continue; \n"
		"      sum += 0.1; \n"
		"   } \n"
		"   gl_FragColor = vec4(sum); \n"
		"} \n",
		{ 0.5, 0.5, 0.5, 0.5 },
		DONT_CARE_Z,
		FLAG_NONE
	},

	{
		"do-loop with break",
		NO_VERTEX_SHADER,
		"void main() { \n"
		"   float sum = 0.0; \n"
		"   do { \n"
		"      sum += 0.1; \n"
		"      if (sum >= 0.499999) \n"
		"         break; \n"
		"   } while (true); \n"
		"   gl_FragColor = vec4(sum); \n"
		"} \n",
		{ 0.5, 0.5, 0.5, 0.5 },
		DONT_CARE_Z,
		FLAG_NONE
	},

	{
		"do-loop with continue and break",
		NO_VERTEX_SHADER,
		"void main() { \n"
		"   float sum = 0.0; \n"
		"   do { \n"
		"      sum += 0.1; \n"
		"      if (sum < 0.499999) \n"
		"         continue; \n"
		"      break; \n"
		"   } while (true); \n"
		"   gl_FragColor = vec4(sum); \n"
		"} \n",
		{ 0.5, 0.5, 0.5, 0.5 },
		DONT_CARE_Z,
		FLAG_NONE
	},

	{
		"discard statement (1)",
		NO_VERTEX_SHADER,
		"void main() { \n"
		"   gl_FragColor = vec4(1.0); \n"
		"   if (gl_TexCoord[0].x < 0.5) \n"
		"      discard; \n"
		"} \n",
		{ 0.0, 0.0, 0.0, 0.0 },  // glClear color
		DONT_CARE_Z,
		FLAG_NONE
	},

	{
		"discard statement (2)",
		NO_VERTEX_SHADER,
		"void main() { \n"
		"   gl_FragColor = vec4(1.0); \n"
		"   if (gl_TexCoord[0].x > 0.5) \n"
		"      discard; \n"
		"} \n",
		{ 1.0, 1.0, 1.0, 1.0 },  // fragment color
		DONT_CARE_Z,
		FLAG_NONE
	},

	{
		"discard statement in for loop",
		NO_VERTEX_SHADER,
		"void main() { \n"
		"   gl_FragColor = vec4(1.0); \n"
		"   int i; \n"
		"   for (i = 0; i < 1000; i++) { \n"
		"      if (i == 9) { \n"
		"         discard; \n"
		"      } \n"
		"   } \n"
		"} \n",
		{ 0.0, 0.0, 0.0, 0.0 },  // glClear color
		DONT_CARE_Z,
		FLAG_NONE
	},

	{
		"conditional expression",
		NO_VERTEX_SHADER,
		"void main() { \n"
		"   gl_FragColor = gl_FragCoord.x < 0.0 ? vec4(0.0) : vec4(0.5); \n"
		"} \n",
		{ 0.5, 0.5, 0.5, 0.5 },
		DONT_CARE_Z,
		FLAG_NONE
	},

	{
		"conditional expression (2)",
		NO_VERTEX_SHADER,
		"void main() { \n"
		"   gl_FragColor = vec4(0.0); \n"
		"   bool b = true; \n"
		"   gl_FragColor.y = b ? 1.0 : 0.5; \n"
		"} \n",
		{ 0.0, 1.0, 0.0, 0.0 },
		DONT_CARE_Z,
		FLAG_NONE
	},

	{
		"sequence (comma) operator",
		NO_VERTEX_SHADER,
		"void main() { \n"
		"   float x, y, z; \n"
		"   x = 1.0, y = 0.5, z = x * y; \n"
		"   gl_FragColor = vec4(z); \n"
		"} \n",
		{ 0.5, 0.5, 0.5, 0.5 },
		DONT_CARE_Z,
		FLAG_NONE
	},

	{
		"constant array with constant indexing, fragment shader",
		NO_VERTEX_SHADER,
		"uniform float uniformArray[4]; \n"
		"void main() { \n"
		"   gl_FragColor.x = uniformArray[0]; \n"
		"   gl_FragColor.y = uniformArray[1]; \n"
		"   gl_FragColor.z = uniformArray[2]; \n"
		"   gl_FragColor.w = uniformArray[3]; \n"
		"} \n",
		{ UniformArray[0], UniformArray[1],
		  UniformArray[2], UniformArray[3] },
		DONT_CARE_Z,
		FLAG_NONE
	},

	{
		"temp array with constant indexing, fragment shader",
		NO_VERTEX_SHADER,
		"void main() { \n"
		"   float ar[4]; \n"
		"   ar[0] = 0.5; \n"
		"   ar[1] = 1.0; \n"
		"   ar[2] = 0.25; \n"
		"   ar[3] = 0.2; \n"
		"   gl_FragColor.x = ar[0]; \n"
		"   gl_FragColor.y = ar[1]; \n"
		"   gl_FragColor.z = ar[2]; \n"
		"   gl_FragColor.w = ar[3]; \n"
		"} \n",
		{ 0.5, 1.0, 0.25, 0.2 },
		DONT_CARE_Z,
		FLAG_NONE
	},

	{
		"constant array with constant indexing, vertex shader",
		"uniform float uniformArray[4]; \n"
		"void main() { \n"
		"   gl_FrontColor.x = uniformArray[0]; \n"
		"   gl_FrontColor.y = uniformArray[1]; \n"
		"   gl_FrontColor.z = uniformArray[2]; \n"
		"   gl_FrontColor.w = uniformArray[3]; \n"
		"   gl_Position = ftransform(); \n"
		"} \n",
		NO_FRAGMENT_SHADER,
		{ UniformArray[0], UniformArray[1],
		  UniformArray[2], UniformArray[3] },
		DONT_CARE_Z,
		FLAG_NONE
	},

	{
		"temp array with constant indexing, vertex shader",
		"void main() { \n"
		"   float ar[4]; \n"
		"   ar[0] = 0.5; \n"
		"   ar[1] = 1.0; \n"
		"   ar[2] = 0.25; \n"
		"   ar[3] = 0.2; \n"
		"   gl_FrontColor.x = ar[0]; \n"
		"   gl_FrontColor.y = ar[1]; \n"
		"   gl_FrontColor.z = ar[2]; \n"
		"   gl_FrontColor.w = ar[3]; \n"
		"   gl_Position = ftransform(); \n"
		"} \n",
		NO_FRAGMENT_SHADER,
		{ 0.5, 1.0, 0.25, 0.2 },
		DONT_CARE_Z,
		FLAG_NONE
	},

#if 0
	{
		"temp array with variable indexing, fragment shader",
		NO_VERTEX_SHADER,
		"uniform vec4 uniform1; \n"
		"void main() { \n"
		"   float ar[4]; \n"
		"   ar[0] = 0.0; \n"
		"   ar[1] = 0.1; \n"
		"   ar[2] = 0.5; \n"
		"   ar[3] = 0.7; \n"
		"   int indx = int(uniform1.y * 8.0);  // should be 2 \n"
		"   gl_FragColor = vec4(ar[indx]); \n"
		"} \n",
		{ 0.5, 0.5, 0.5, 0.5 },
		DONT_CARE_Z,
		FLAG_NONE
	},

	{
		"temp array with variable indexing, vertex shader",
		"uniform vec4 uniform1; \n"
		"void main() { \n"
		"   float ar[4]; \n"
		"   ar[0] = 0.0; \n"
		"   ar[1] = 0.1; \n"
		"   ar[2] = 0.5; \n"
		"   ar[3] = 0.7; \n"
		"   int indx = int(uniform1.y * 8.0);  // should be 2 \n"
		"   gl_FrontColor = vec4(ar[indx]); \n"
		"   gl_Position = ftransform(); \n"
		"} \n",
		NO_FRAGMENT_SHADER,
		{ 0.5, 0.5, 0.5, 0.5 },
		DONT_CARE_Z,
		FLAG_NONE
	},
#endif
	{
		"constant array with variable indexing, vertex shader",
		"uniform float uniformArray[4]; \n"
		"uniform vec4 uniform1; \n"
		"void main() { \n"
		"   int indx = int(uniform1.y * 8.0);  // should be 2 \n"
		"   gl_FrontColor = vec4(uniformArray[indx]); \n"
		"   gl_Position = ftransform(); \n"
		"} \n",
		NO_FRAGMENT_SHADER,
		{ 0.5, 0.5, 0.5, 0.5 },
		DONT_CARE_Z,
		FLAG_NONE
	},

	{
		"constant array of vec4 with variable indexing, vertex shader",
		"uniform vec4 uniform1; \n"
		"uniform float uniformArray[4]; \n"
		"uniform vec4 uniformArray4[4]; \n"
		"void main() { \n"
		"   int i0 = int(gl_TexCoord[0].x); \n"
		"   int i1 = int(gl_TexCoord[0].y); \n"
		"   int i2 = int(gl_TexCoord[0].z); \n"
		"   int i3 = int(gl_TexCoord[0].w); \n"

		"   int indx0 = int(uniform1.y * 3.0);  // should be 2 \n"
		"   int indx = int(uniform1.y * 8.0);  // should be 2 \n"
		"   gl_FrontColor.z = uniformArray4[indx].z; \n"
		"   gl_FrontColor.x = uniformArray4[indx].x; \n"
		"   gl_FrontColor.w = uniformArray4[indx].w; \n"
		"   gl_FrontColor.y = uniformArray4[indx].y; \n"
		"   gl_Position = ftransform(); \n"
		"} \n",
		NO_FRAGMENT_SHADER,
		{ 0.5, 0.6, 0.7, 0.5 },
		DONT_CARE_Z,
		FLAG_NONE
	},

	{
		// This one tests that a different array index per vertex
		// works as expected.  The left edge of the polygon should
		// have a gray value = uniformArray[2] while the right
		// edge of the polygon should have a gray value =
		// uniformArray[3].
		"constant array with variable indexing, vertex shader (2)",
		"uniform float uniformArray[4]; \n"
		"void main() { \n"
		"   int indx = int(gl_MultiTexCoord0.x + 2.0);  // 2 or 3 \n"
		"   gl_FrontColor = vec4(uniformArray[indx]); \n"
		"   gl_Position = ftransform(); \n"
		"} \n",
		NO_FRAGMENT_SHADER,
		// If we read the center pixel we'd get the average of
		// the Uniform[2] and Uniform[3] values here.  But we read
		// an off-center pixel so this result was found emperically.
		{ 0.6, 0.6, 0.6, 0.6 },
		DONT_CARE_Z,
		FLAG_NONE
	},

	{
		"temp array with swizzled variable indexing",
		NO_VERTEX_SHADER,
		"uniform vec4 uniform1; \n"
		"void main() { \n"
		"   float ar[4]; \n"
		"   ar[0] = 0.0; \n"
		"   ar[1] = 0.8; \n"
		"   ar[2] = 0.5; \n"
		"   ar[3] = 0.7; \n"
		"   ivec2 indx; \n"
		"   indx.x = 1; \n"
		"   indx.y = int(uniform1.y * 8.0);  // should be 2 \n"
		"   float p = ar[indx.x] * ar[indx.y]; \n"
		"   gl_FragColor = vec4(p); \n"
		"} \n",
		{ 0.4, 0.4, 0.4, 0.4 },
		DONT_CARE_Z,
		FLAG_NONE
	},

#if 0 // XXX enable someday
	{
		"vector subscript *=",
		NO_VERTEX_SHADER,
		"void main() { \n"
		"   vec4 c = vec4(0.25, 0.5, 0.1, 0.1); \n"
		"   c[0] *= 4.0; \n"
		"   c[1] *= 2.0; \n"
		"   gl_FragColor = c; \n"
		"} \n",
		{ 1.0, 1.0, 0.1, 0.1 },
		DONT_CARE_Z,
		FLAG_NONE
	},
#endif

	// Logical operators =================================================
	{
		"&& operator, short-circuit",
		NO_VERTEX_SHADER,
		"void main() { \n"
		"   float x = 0.75; \n"
		"   // this should always be false \n"
		"   if (x <= 0.5 && ++x > 0.0) { \n"
		"      x += 0.1; \n"
		"   } \n"
		"   gl_FragColor = vec4(x); \n"
		"} \n",
		{ 0.75, 0.75, 0.75, 0.75 },
		DONT_CARE_Z,
		FLAG_NONE
	},

	{
		"|| operator, short-circuit",
		NO_VERTEX_SHADER,
		"void main() { \n"
		"   float x = 0.75; \n"
		"   // this should always be true \n"
		"   if (x >= 0.5 || ++x >= 0.0) { \n"
		"      x += 0.1; \n"
		"   } \n"
		"   gl_FragColor = vec4(x); \n"
		"} \n",
		{ 0.85, 0.85, 0.85, 0.85 },
		DONT_CARE_Z,
		FLAG_NONE
	},


	// Uniform & Varying vars ============================================
	{
		"uniform variable (fragment shader)",
		NO_VERTEX_SHADER,
		"uniform vec4 uniform1; \n"
		"void main() { \n"
		"   gl_FragColor = uniform1; \n"
		"} \n",
		UNIFORM1,
		DONT_CARE_Z,
		FLAG_NONE
	},

	{
		"uniform variable (vertex shader)",
		"uniform vec4 uniform1; \n"
		"void main() { \n"
		"   gl_FrontColor = uniform1; \n"
		"   gl_Position = ftransform(); \n"
		"} \n",
		NO_FRAGMENT_SHADER,
		UNIFORM1,
		DONT_CARE_Z,
		FLAG_NONE
	},

	{
		"varying variable",
		// vertex program:
		"varying vec4 var1; \n"
		"void main() { \n"
		"   var1 = vec4(1.0, 0.5, 0.25, 0.0); \n"
		"   gl_Position = ftransform(); \n"
		"} \n",
		// fragment program:
		"varying vec4 var1; \n"
		"void main() { \n"
		"   gl_FragColor = var1; \n"
		"} \n",
		{ 1.0, 0.5, 0.25, 0.0 },
		DONT_CARE_Z,
		FLAG_NONE
	},

	{
		// Test that reads of varying vars in vertex shaders works.
		// Mesa's GLSL compiler replaces some varying vars with temp regs
		// so that they can be read.  The vertex shader here does some
		// arithmetic so that additional temp regs are used.  If any
		// temp regs are mis-used, this test should fail.
		// This is a regression test for fd.o bug 26317
		// Note: var3 = gl_Color
		// Note: var1 = -var2
		// Final fragment color should be equal to gl_Color
		"varying variable read-write",
		// vertex program:
		"varying vec4 var1, var2, var3; \n"
		"void main() { \n"
		"   gl_Position = ftransform(); \n"
		"   var1 = 2.0 * (vec4(0.0) - gl_Position); \n"
		"   var2 = 2.0 * gl_Color; \n"
		"   var3 = 0.5 * var2 + (2.0 * gl_Position + var1); \n"
		"   var1 = -var2; \n"
		"} \n",
		// fragment program:
		"varying vec4 var1; \n"
		"varying vec4 var2; \n"
		"varying vec4 var3; \n"
		"void main() { \n"
		"   gl_FragColor = var1 + var2 + var3; \n"
		"} \n",
		{ PRIMARY_R, PRIMARY_G, PRIMARY_B, PRIMARY_A },
		DONT_CARE_Z,
		FLAG_NONE
	},


	// GL state refs =====================================================
	{
		"GL state variable reference (gl_FrontMaterial.ambient)",
		NO_VERTEX_SHADER,
		"void main() { \n"
		"   gl_FragColor = gl_FrontMaterial.ambient; \n"
		"} \n",
		AMBIENT,
		DONT_CARE_Z,
		FLAG_NONE
	},
	{
		"GL state variable reference (gl_LightSource[0].diffuse)",
		NO_VERTEX_SHADER,
		"void main() { \n"
		"   gl_FragColor = gl_LightSource[0].diffuse; \n"
		"} \n",
		LIGHT_DIFFUSE,
		DONT_CARE_Z,
		FLAG_NONE
	},

	{
		"GL state variable reference (diffuse product)",
		NO_VERTEX_SHADER,
		"void main() { \n"
		"   gl_FragColor = gl_FrontLightProduct[0].diffuse; \n"
		"} \n",
		DIFFUSE_PRODUCT,
		DONT_CARE_Z,
		FLAG_NONE
	},

	{
		"GL state variable reference (point size)",
		"void main() { \n"
		"   gl_Position = ftransform(); \n"
		"   gl_FrontColor.x = gl_Point.size * 0.1; \n"
		"   gl_FrontColor.y = gl_Point.sizeMin * 0.1; \n"
		"   gl_FrontColor.z = gl_Point.sizeMax * 0.1; \n"
		"   gl_FrontColor.w = 0.0; \n"
		"} \n",
		NO_FRAGMENT_SHADER,
		{ PSIZE * 0.1, PSIZE_MIN * 0.1, PSIZE_MAX * 0.1, 0.0 },
		DONT_CARE_Z,
		FLAG_NONE
	},

	{
		"GL state variable reference (point attenuation)",
		"void main() { \n"
		"   gl_Position = ftransform(); \n"
		"   gl_FrontColor.x = gl_Point.distanceConstantAttenuation * 0.1; \n"
		"   gl_FrontColor.y = gl_Point.distanceLinearAttenuation * 0.1; \n"
		"   gl_FrontColor.z = gl_Point.distanceQuadraticAttenuation * 0.1; \n"
		"   gl_FrontColor.w = 0.0; \n"
		"} \n",
		NO_FRAGMENT_SHADER,
		{ PSIZE_ATTEN0 * 0.1, PSIZE_ATTEN1 * 0.1,
		  PSIZE_ATTEN2 * 0.1, 0.0 },
		DONT_CARE_Z,
		FLAG_NONE
	},

	{
		"linear fog",
		// vertex prog:
		"void main() { \n"
		"   gl_Position = ftransform(); \n"
		"   gl_FogFragCoord = 125.0; \n"
		"   gl_FrontColor = gl_Color; \n"
		"} \n",
		// fragment prog:
		"void main() { \n"
		"   float bf = (gl_FogFragCoord - gl_Fog.start) * gl_Fog.scale; \n"
		"   gl_FragColor = mix(gl_Color, gl_Fog.color, bf); \n"
		"} \n",
#define BF (125.0 - FOG_START) / (FOG_END - FOG_START)  // Blend Factor
		{ PRIMARY_R + BF * (FOG_R - PRIMARY_R),
		  PRIMARY_G + BF * (FOG_G - PRIMARY_G),
		  PRIMARY_B + BF * (FOG_B - PRIMARY_B),
		  PRIMARY_A + BF * (FOG_A - PRIMARY_A) },
#undef BF
		DONT_CARE_Z,
		FLAG_NONE
	},

	{
		"built-in constants",
		// vertex shader:
		"void main() { \n"
		"   gl_Position = ftransform(); \n"
		"   // front color values should all be >= 1.0 \n"
		"   gl_FrontColor = vec4(gl_MaxLights, gl_MaxClipPlanes,\n"
		"        		gl_MaxTextureUnits, \n"
		"        		gl_MaxTextureCoords); \n"
		"} \n",
		NO_FRAGMENT_SHADER,
		{ 1.0, 1.0, 1.0, 1.0 },
		DONT_CARE_Z,
		FLAG_NONE
	},

	{
		"gl_FrontFacing var (1)",
		NO_VERTEX_SHADER,
		"void main() { \n"
		"   gl_FragColor = vec4(0.5 * float(gl_FrontFacing)); \n"
		"} \n",
		{ 0.5, 0.5, 0.5, 0.5 },
		DONT_CARE_Z,
		FLAG_NONE
	},

	{
		"gl_FrontFacing var (2)",
		NO_VERTEX_SHADER,
		"void main() { \n"
		"   gl_FragColor = vec4(0.25 + float(gl_FrontFacing)); \n"
		"} \n",
		{ 0.25, 0.25, 0.25, 0.25 },
		DONT_CARE_Z,
		FLAG_WINDING_CW
	},

	// Texture functions ==================================================
	{
		"texture2D()",
		NO_VERTEX_SHADER,
		"uniform sampler2D tex2d; \n"
		"void main() { \n"
		"   gl_FragColor = texture2D(tex2d, gl_TexCoord[0].xy);\n"
		"} \n",
		{ 1.0, 0.0, 0.0, 1.0 },
		DONT_CARE_Z,
		FLAG_NONE
	},

	{
		"texture2D(), computed coordinate",
		NO_VERTEX_SHADER,
		"uniform sampler2D tex2d; \n"
		"void main() { \n"
		"   vec2 coord = gl_TexCoord[0].xy + vec2(0.5); \n"
		"   gl_FragColor = texture2D(tex2d, coord, 0.0); \n"
		"} \n",
		{ 1.0, 1.0, 1.0, 1.0 },  // upper-right tex color
		DONT_CARE_Z,
		FLAG_NONE
	},

	{
		"texture2D(), with bias",
		NO_VERTEX_SHADER,
		"uniform sampler2D tex2d; \n"
		"void main() { \n"
		"   gl_FragColor = texture2D(tex2d, gl_TexCoord[0].xy, 1.0);\n"
		"} \n",
		{ 0.5, 0.0, 0.0, 0.5 },
		DONT_CARE_Z,
		FLAG_NONE
	},

	{
		"2D Texture lookup with explicit lod (Vertex shader)",
		"uniform sampler2D tex2d; \n"
		"void main() { \n"
		"   gl_FrontColor = texture2DLod(tex2d, gl_MultiTexCoord0.xy, 2.0);\n"
		"   gl_Position = ftransform(); \n"
		"} \n",
		NO_FRAGMENT_SHADER,
		{ 0.25, 0.0, 0.0, 0.25 },
		DONT_CARE_Z,
		FLAG_VERTEX_TEXTURE
	},

	{
		"texture2DProj()",
		NO_VERTEX_SHADER,
		"uniform sampler2D tex2d; \n"
		"void main() { \n"
		"   vec4 coord = gl_TexCoord[0] * vec4(2.25); \n"
		"   // 'proj' will divide components by w (=2.25) \n"
		"   gl_FragColor = texture2DProj(tex2d, coord);\n"
		"} \n",
		{ 1.0, 0.0, 0.0, 1.0 },
		DONT_CARE_Z,
		FLAG_NONE
	},

	{
		"texture1D()",
		NO_VERTEX_SHADER,
		"uniform sampler1D tex1d; \n"
		"void main() { \n"
		"   gl_FragColor = texture1D(tex1d, gl_TexCoord[0].x);\n"
		"} \n",
		{ 1.0, 0.0, 0.0, 1.0 },
		DONT_CARE_Z,
		FLAG_NONE
	},

	{
		"texture3D()",
		NO_VERTEX_SHADER,
		"uniform sampler3D tex3d; \n"
		"void main() { \n"
		"   gl_FragColor = texture3D(tex3d, gl_TexCoord[0].xyz);\n"
		"} \n",
		{ 1.0, 0.0, 0.0, 1.0 },
		DONT_CARE_Z,
		FLAG_NONE
	},

	{
		"texture3D(), computed coord",
		NO_VERTEX_SHADER,
		"uniform sampler3D tex3d; \n"
		"void main() { \n"
		"   vec3 coord = gl_TexCoord[0].xyz; \n"
		"   coord.y = 0.75; \n"
		"   coord.z = 0.75; \n"
		"   gl_FragColor = texture3D(tex3d, coord); \n"
		"} \n",
		{ 0.0, 0.0, 0.5, 0.5 },
		DONT_CARE_Z,
		FLAG_NONE
	},

	{
		"shadow2D(): 1",
		NO_VERTEX_SHADER,
		"uniform sampler2DShadow texZ; \n"
		"void main() { \n"
		"   vec3 coord = vec3(0.1, 0.1, 0.5); \n"
		"   // shadow map value should be 0.25 \n"
		"   gl_FragColor = shadow2D(texZ, coord) + vec4(0.25); \n"
		"   // color = (0.5 <= 0.25) ? 1.25 : 0.25\n"
		"} \n",
		{ 0.25, 0.25, 0.25, 1.0 },
		DONT_CARE_Z,
		FLAG_NONE
	},

	{
		"shadow2D(): 2",
		NO_VERTEX_SHADER,
		"uniform sampler2DShadow texZ; \n"
		"void main() { \n"
		"   vec3 coord = vec3(0.1, 0.1, 0.2); \n"
		"   // shadow map value should be 0.25 \n"
		"   gl_FragColor = shadow2D(texZ, coord); \n"
		"   // color = (0.2 <= 0.25) ? 1 : 0\n"
		"} \n",
		{ 1.0, 1.0, 1.0, 1.0 },
		DONT_CARE_Z,
		FLAG_NONE
	},

	{
		"shadow2D(): 3",
		NO_VERTEX_SHADER,
		"uniform sampler2DShadow texZ; \n"
		"void main() { \n"
		"   vec3 coord = vec3(0.9, 0.9, 0.95); \n"
		"   // shadow map value should be 0.75 \n"
		"   gl_FragColor = shadow2D(texZ, coord) + vec4(0.25); \n"
		"   // color = (0.95 <= 0.75) ? 1.25 : 0.25\n"
		"} \n",
		{ 0.25, 0.25, 0.25, 1.0 },
		DONT_CARE_Z,
		FLAG_NONE
	},

	{
		"shadow2D(): 4",
		NO_VERTEX_SHADER,
		"uniform sampler2DShadow texZ; \n"
		"void main() { \n"
		"   vec3 coord = vec3(0.9, 0.9, 0.65); \n"
		"   // shadow map value should be 0.75 \n"
		"   gl_FragColor = shadow2D(texZ, coord); \n"
		"   // color = (0.65 <= 0.75) ? 1 : 0\n"
		"} \n",
		{ 1.0, 1.0, 1.0, 1.0 },
		DONT_CARE_Z,
		FLAG_NONE
	},

	// Function calls ====================================================
	{
		"nested function calls (1)",
		NO_VERTEX_SHADER,
		"float Half(const in float x) { \n"
		"   return 0.5 * x; \n"
		"} \n"
		"\n"
		"float square(const in float x) { \n"
		"   return x * x; \n"
		"} \n"
		"\n"
		"void main() { \n"
		"   float a = 0.5; \n"
		"   float b = square(Half(1.0)); \n"
		"   gl_FragColor = vec4(b); \n"
		"} \n",
		{ 0.25, 0.25, 0.25, 0.25 },
		DONT_CARE_Z,
		FLAG_NONE
	},

	{
		"nested function calls (2)",
		NO_VERTEX_SHADER,
		"float Half(const in float x) { \n"
		"   return 0.5 * x; \n"
		"} \n"
		"\n"
		"float square_half(const in float x) { \n"
		"   float y = Half(x); \n"
		"   return y * y; \n"
		"} \n"
		"\n"
		"void main() { \n"
		"   float a = 1.0; \n"
		"   float b = square_half(a); \n"
		"   gl_FragColor = vec4(b); \n"
		"} \n",
		{ 0.25, 0.25, 0.25, 0.25 },
		DONT_CARE_Z,
		FLAG_NONE
	},

	{
		"nested function calls (3)",
		NO_VERTEX_SHADER,
		"float Half(const in float x) { \n"
		"   return 0.5 * x; \n"
		"} \n"
		"\n"
		"void main() { \n"
		"   float a = 0.5; \n"
		"   float b = Half(Half(a)); \n"
		"   gl_FragColor = vec4(b); \n"
		"} \n",
		{ 0.125, 0.125, 0.125, 0.125 },
		DONT_CARE_Z,
		FLAG_NONE
	},

	{
		"TPPStreamCompiler::assignOperands",
		NO_VERTEX_SHADER,
		"struct S { \n"
		"   float f; \n"
		"}; \n"
		"\n"
		"void F(S s) {} \n"
		"\n"
		"const S s = S(0.0); \n"
		"\n"
		"void F() { \n"
		"   F(s); \n"
		"} \n"
		"\n"
		"void main() { \n"
		"   gl_FragColor = vec4(0.0, 0.0, 0.0, 0.0); \n"
		"} \n",
		{ 0.0, 0.0, 0.0, 0.0 },
		DONT_CARE_Z,
		FLAG_NONE
	},

	// Matrix tests ======================================================
	{
		"matrix column check (1)",
		NO_VERTEX_SHADER,
		"void main() { \n"
		"   mat4 m = gl_TextureMatrix[1]; \n"
		"   gl_FragColor = m[0]; \n"
		"} \n",
		{ 1.0, 0.5, 0.6, 0.0 },
		DONT_CARE_Z,
		FLAG_NONE
	},

	{
		"matrix column check (2)",
		NO_VERTEX_SHADER,
		"void main() { \n"
		"   mat4 m = gl_TextureMatrix[1]; \n"
		"   gl_FragColor = m[3]; \n"
		"} \n",
		{ 0.1, 0.2, 0.3, 1.0 },
		DONT_CARE_Z,
		FLAG_NONE
	},

	{
		"matrix, vector multiply (1)",
		NO_VERTEX_SHADER,
		"void main() { \n"
		"   mat4 m = mat4(0.5); // scale by 0.5 \n"
		"   vec4 color = gl_Color * m; \n"
		"   gl_FragColor = color; \n"
		"} \n",
		{ 0.5 * PRIMARY_R, 0.5 * PRIMARY_G,
		  0.5 * PRIMARY_B, 0.5 * PRIMARY_A },
		DONT_CARE_Z,
		FLAG_NONE
	},

	{
		"matrix, vector multiply (2)",
		NO_VERTEX_SHADER,
		"void main() { \n"
		"   vec4 color = gl_TextureMatrix[1] * gl_Color; \n"
		"   gl_FragColor = color; \n"
		"} \n",
		{ 0.2745, 0.9255, 0.7294, 1.0 },
		DONT_CARE_Z,
		FLAG_NONE
	},

	{
		"matrix, vector multiply (3)",
		NO_VERTEX_SHADER,
		"void main() { \n"
		"   vec4 color = gl_Color * gl_TextureMatrix[1]; \n"
		"   gl_FragColor = color; \n"
		"} \n",
		{ 0.925, 0.925, 0.6999, .5750 },
		DONT_CARE_Z,
		FLAG_NONE
	},

	{
		"uniform matrix",
		NO_VERTEX_SHADER,
		"uniform mat4 uniformMat4; \n"
		"void main() { \n"
		"   gl_FragColor = uniformMat4[3]; \n"
		"} \n",
		{ 0.6, 0.7, 0.8, 1.0 },
		DONT_CARE_Z,
		FLAG_NONE
	},

	{
		"uniform matrix, transposed",
		NO_VERTEX_SHADER,
		"uniform mat4 uniformMat4t; \n"
		"void main() { \n"
		"   gl_FragColor = uniformMat4t[2]; \n"
		"} \n",
		{ 0.2, 0.0, 1.0, 0.8 },
		DONT_CARE_Z,
		FLAG_NONE
	},

	// Struct tests ======================================================
	{
		"struct (1)",
		NO_VERTEX_SHADER,
		"struct s1 { \n"
		"  float f1; \n"
		"  vec4 v4; \n"
		"}; \n"
		"\n"
		"void main() { \n"
		"   s1 a, b; \n"
		"   a.v4 = vec4(0.25, 0.5, 0.75, 1.0); \n"
		"   a.f1 = 0.0; \n"
		"   b = a; \n"
		"   gl_FragColor = b.v4; \n"
		"} \n",
		{ 0.25, 0.5, 0.75, 1.0 },
		DONT_CARE_Z,
		FLAG_NONE
	},

	{
		"struct (2)",
		NO_VERTEX_SHADER,
		"struct s1 { \n"
		"  float f1; \n"
		"  vec4 v4; \n"
		"}; \n"
		"\n"
		"void main() { \n"
		"   s1 a[2]; \n"
		"   a[0].v4 = vec4(0.25, 0.5, 0.75, 1.0); \n"
		"   a[0].f1 = 0.0; \n"
		"   a[1] = a[0]; \n"
		"   gl_FragColor = a[1].v4; \n"
		"} \n",
		{ 0.25, 0.5, 0.75, 1.0 },
		DONT_CARE_Z,
		FLAG_NONE
	},

	{
		"struct (3)",
		NO_VERTEX_SHADER,
		"struct s1 { \n"
		"  float f1; \n"
		"  vec4 v4; \n"
		"}; \n"
		"\n"
		"void main() { \n"
		"   vec4 scale = vec4(0.5); \n"
		"   vec4 bias = vec4(0.1); \n"
		"   s1 a; \n"
		"   a.v4 = vec4(0.25, 0.5, 0.75, 1.0); \n"
		"   a.f1 = 0.0; \n"
		"   gl_FragColor = a.v4 * scale + bias; \n"
		"} \n",
		{ 0.225, 0.35, 0.475, 0.6 },
		DONT_CARE_Z,
		FLAG_NONE
	},

	{
		"struct (4)",
		NO_VERTEX_SHADER,
		"struct s1 { \n"
		"  float foo; \n"
		"  vec4 v4; \n"
		"}; \n"
		"struct s2 { \n"
		"  float bar; \n"
		"  s1 s; \n"
		"  float baz; \n"
		"}; \n"
		"\n"
		"void main() { \n"
		"   s2 a; \n"
		"   a.s.v4 = vec4(0.25, 0.5, 0.75, 1.0); \n"
		"   a.bar = 0.0; \n"
		"   a.baz = 0.0; \n"
		"   a.s.foo = 0.0; \n"
		"   gl_FragColor = a.s.v4; \n"
		"} \n",
		{ 0.25, 0.5, 0.75, 1.0 },
		DONT_CARE_Z,
		FLAG_NONE
	},

	// Preprocessor tests ================================================
	{
		"Preprocessor test 1 (#if 0)",
		NO_VERTEX_SHADER,
		"void main() { \n"
		"#if 0 \n"
		"   gl_FragColor = vec4(0.5); \n"
		"#else \n"
		"   gl_FragColor = vec4(0.3); \n"
		"#endif \n"
		"} \n",
		{ 0.3, 0.3, 0.3, 0.3 },
		DONT_CARE_Z,
		FLAG_NONE
	},

	{
		"Preprocessor test 2 (#if 1)",
		NO_VERTEX_SHADER,
		"void main() { \n"
		"#if 1 \n"
		"   gl_FragColor = vec4(0.5); \n"
		"#else \n"
		"   gl_FragColor = vec4(0.3); \n"
		"#endif \n"
		"} \n",
		{ 0.5, 0.5, 0.5, 0.5 },
		DONT_CARE_Z,
		FLAG_NONE
	},

	{
		"Preprocessor test 3 (#if ==)",
		NO_VERTEX_SHADER,
		"void main() { \n"
		"#define SYMBOL 3 \n"
		"#if SYMBOL == 3 \n"
		"   gl_FragColor = vec4(0.5); \n"
		"#else \n"
		"   gl_FragColor = vec4(0.3); \n"
		"#endif \n"
		"} \n",
		{ 0.5, 0.5, 0.5, 0.5 },
		DONT_CARE_Z,
		FLAG_NONE
	},

	{
		"Preprocessor test 4 (#if 1, #define macro)",
		NO_VERTEX_SHADER,
		"#if 1 \n"
		"#define FOO(x) x \n"
		"#else \n"
		"#define FOO(x) (0.5 * (x)) \n"
		"#endif \n"
		"void main() { \n"
		"   gl_FragColor = vec4(FOO(0.25)); \n"
		"} \n",
		{ 0.25, 0.25, 0.25, 0.25 },
		DONT_CARE_Z,
		FLAG_NONE
	},

	{
		"Preprocessor test 5 (#if 1, #define macro)",
		NO_VERTEX_SHADER,
		"#define BAR(x) x \n"
		"#if 1 \n"
		"#define FOO(x) BAR(x) \n"
		"#else \n"
		"#define FOO(x) (BAR(x) + BAR(x)) \n"
		"#endif \n"
		"void main() { \n"
		"   gl_FragColor = vec4(FOO(0.25)); \n"
		"} \n",
		{ 0.25, 0.25, 0.25, 0.25 },
		DONT_CARE_Z,
		FLAG_NONE
	},

	{
		"Preprocessor test 6 (#if 0, #define macro)",
		NO_VERTEX_SHADER,
		"#define BAR(x) x \n"
		"#if 0 \n"
		"#define FOO(x) BAR(x) \n"
		"#else \n"
		"#define FOO(x) (BAR(x) + BAR(x)) \n"
		"#endif \n"
		"void main() { \n"
		"   gl_FragColor = vec4(FOO(0.25)); \n"
		"} \n",
		{ 0.5, 0.5, 0.5, 0.5 },
		DONT_CARE_Z,
		FLAG_NONE
	},

	{
		"Preprocessor test 7 (multi-line #define)",
		NO_VERTEX_SHADER,
		"#define FOO(x) \\\n"
		" ((x) + (x)) \n"
		"void main() { \n"
		"   gl_FragColor = vec4(FOO(0.25)); \n"
		"} \n",
		{ 0.5, 0.5, 0.5, 0.5 },
		DONT_CARE_Z,
		FLAG_NONE
	},

	{
		"Preprocessor test 8 (#ifdef)",
		NO_VERTEX_SHADER,
		"#define FOO \n"
		"void main() { \n"
		"#ifdef FOO \n"
		"   gl_FragColor = vec4(0.0, 1.0, 0.0, 0.0); \n"
		"#else \n"
		"   gl_FragColor = vec4(1.0, 0.0, 0.0, 0.0); \n"
		"#endif \n"
		"} \n",
		{ 0.0, 1.0, 0.0, 0.0 },
		DONT_CARE_Z,
		FLAG_NONE
	},

	{
		"Preprocessor test 9 (#ifndef)",
		NO_VERTEX_SHADER,
		"#define FOO \n"
		"void main() { \n"
		"#ifndef FOO \n"
		"   gl_FragColor = vec4(0.0, 1.0, 0.0, 0.0); \n"
		"#else \n"
		"   gl_FragColor = vec4(1.0, 0.0, 0.0, 0.0); \n"
		"#endif \n"
		"} \n",
		{ 1.0, 0.0, 0.0, 0.0 },
		DONT_CARE_Z,
		FLAG_NONE
	},

	{
		"Preprocessor test 10 (#if defined())",
		NO_VERTEX_SHADER,
		"#define FOO \n"
		"void main() { \n"
		"#if defined(FOO) \n"
		"   gl_FragColor = vec4(0.0, 1.0, 0.0, 0.0); \n"
		"#else \n"
		"   gl_FragColor = vec4(1.0, 0.0, 0.0, 0.0); \n"
		"#endif \n"
		"} \n",
		{ 0.0, 1.0, 0.0, 0.0 },
		DONT_CARE_Z,
		FLAG_NONE
	},

	{
		"Preprocessor test 11 (#elif)",
		NO_VERTEX_SHADER,
		"#define FOO 1\n"
		"void main() { \n"
		"#if FOO == 1 \n"
		"   vec4 r = vec4(0.0, 1.0, 0.0, 0.0); \n"
		"#elif FOO == 2\n"
		"   vec4 r = vec4(1.0, 0.0, 0.0, 0.0); \n"
		"#else \n"
		"   vec4 r = vec4(1.0, 1.0, 0.0, 0.0); \n"
		"#endif \n"
		"   gl_FragColor = r; \n"
		"} \n",
		{ 0.0, 1.0, 0.0, 0.0 },
		DONT_CARE_Z,
		FLAG_NONE
	},

	{
		"Preprocessor test 12 (#elif)",
		NO_VERTEX_SHADER,
		"#define FOO 2\n"
		"void main() { \n"
		"#if FOO == 1 \n"
		"   vec4 r = vec4(0.0, 1.0, 0.0, 0.0); \n"
		"#elif FOO == 2\n"
		"   vec4 r = vec4(1.0, 0.0, 0.0, 0.0); \n"
		"#else \n"
		"   vec4 r = vec4(1.0, 1.0, 0.0, 0.0); \n"
		"#endif \n"
		"   gl_FragColor = r; \n"
		"} \n",
		{ 1.0, 0.0, 0.0, 0.0 },
		DONT_CARE_Z,
		FLAG_NONE
	},

	{
		"Preprocessor test 13 (nested #if)",
		NO_VERTEX_SHADER,
		"#define FOO 1\n"
		"#define BAR 0\n"
		"void main() { \n"
		"#if FOO == 1 \n"
		"#if BAR == 1 \n"
		"   vec4 r = vec4(1.0, 0.0, 0.0, 0.0); \n"
		"#else \n"
		"   vec4 r = vec4(0.0, 1.0, 0.0, 0.0); \n"
		"#endif \n"
		"#else \n"
		"   vec4 r = vec4(0.0, 0.0, 1.0, 0.0); \n"
		"#endif \n"
		"   gl_FragColor = r; \n"
		"} \n",
		{ 0.0, 1.0, 0.0, 0.0 },
		DONT_CARE_Z,
		FLAG_NONE
	},

	{
		"Preprocessor test 14 (nested #if)",
		NO_VERTEX_SHADER,
		"#define FOO 0\n"
		"#define BAR 0\n"
		"void main() { \n"
		"#if FOO == 1 \n"
		"   vec4 r = vec4(0.0, 0.0, 1.0, 0.0); \n"
		"#else \n"
		"#if BAR == 1 \n"
		"   vec4 r = vec4(1.0, 0.0, 0.0, 0.0); \n"
		"#else \n"
		"   vec4 r = vec4(0.0, 1.0, 0.0, 0.0); \n"
		"#endif \n"
		"#endif \n"
		"   gl_FragColor = r; \n"
		"} \n",
		{ 0.0, 1.0, 0.0, 0.0 },
		DONT_CARE_Z,
		FLAG_NONE
	},

	{
		"Preprocessor test 15 (nested #if, #elif)",
		NO_VERTEX_SHADER,
		"#define FOO 0\n"
		"#define BAR 2\n"
		"void main() { \n"
		"#if FOO == 1 \n"
		"   vec4 r = vec4(0.0, 0.0, 1.0, 0.0); \n"
		"#else \n"
		"#if BAR == 1 \n"
		"   vec4 r = vec4(1.0, 0.0, 0.0, 0.0); \n"
		"#elif BAR == 2 \n"
		"   vec4 r = vec4(1.0, 0.0, 0.0, 0.0); \n"
		"#else \n"
		"   vec4 r = vec4(0.0, 1.0, 0.0, 0.0); \n"
		"#endif \n"
		"#endif \n"
		"   gl_FragColor = r; \n"
		"} \n",
		{ 1.0, 0.0, 0.0, 0.0 },
		DONT_CARE_Z,
		FLAG_NONE
	},

	{
		// This test will only be run if we have the GL_ARB_draw_buffers
		// extension.  Note the FLAG_ARB_DRAW_BUFFERS flag.
		"Preprocessor test (extension test 1)",
		NO_VERTEX_SHADER,
		"#extension GL_ARB_draw_buffers: enable\n"
		"void main() { \n"
		"#if defined(GL_ARB_draw_buffers) \n"
		"   gl_FragData[0] = vec4(0.0, 1.0, 0.0, 0.0); \n"
		"#else \n"
		"   gl_FragColor = vec4(1.0, 0.0, 0.0, 0.0); \n"
		"#endif \n"
		"} \n",
		{ 0.0, 1.0, 0.0, 0.0 },
		DONT_CARE_Z,
		FLAG_ARB_DRAW_BUFFERS
	},

	{
		// As above, but use #if == 1 test.
		"Preprocessor test (extension test 2)",
		NO_VERTEX_SHADER,
		"#extension GL_ARB_draw_buffers: enable\n"
		"void main() { \n"
		"#if GL_ARB_draw_buffers == 1\n"
		"   gl_FragData[0] = vec4(0.0, 1.0, 0.0, 0.0); \n"
		"#else \n"
		"   gl_FragColor = vec4(1.0, 0.0, 0.0, 0.0); \n"
		"#endif \n"
		"} \n",
		{ 0.0, 1.0, 0.0, 0.0 },
		DONT_CARE_Z,
		FLAG_ARB_DRAW_BUFFERS
	},

	{
		// Test using a non-existant function.  Should not compile.
		"Preprocessor test (extension test 3)",
		NO_VERTEX_SHADER,
		"#extension GL_FOO_bar: require\n"
		"void main() { \n"
		"   gl_FragColor = vec4(1.0, 0.0, 0.0, 0.0); \n"
		"} \n",
		{ 0.0, 1.0, 0.0, 0.0 },
		DONT_CARE_Z,
		FLAG_ILLEGAL_SHADER
	},

	{
		"Preprocessor test (11)",
		NO_VERTEX_SHADER,
		"#define FOO \n"
		"void main() { \n"
		"#if !defined(FOO) \n"
		"   gl_FragColor = vec4(0.0, 1.0, 0.0, 0.0); \n"
		"#else \n"
		"   gl_FragColor = vec4(1.0, 0.0, 0.0, 0.0); \n"
		"#endif \n"
		"} \n",
		{ 1.0, 0.0, 0.0, 0.0 },
		DONT_CARE_Z,
		FLAG_NONE
	},

	// Illegal shaders ==================================================
	{
		"undefined variable",
		NO_VERTEX_SHADER,
		"void main() { \n"
		"   vec3 v = u; \n"
		"   gl_FragColor = vec4(0.5); \n"
		"} \n",
		{ 0.5, 0.5, 0.5, 0.5 },
		DONT_CARE_Z,
		FLAG_ILLEGAL_SHADER
	},

	{
		"if (boolean-scalar) check",
		NO_VERTEX_SHADER,
		"void main() { \n"
		"   vec3 v; \n"
		"   if (v) { \n"
		"   } \n"
		"   gl_FragColor = vec4(0.5); \n"
		"} \n",
		{ 0.5, 0.5, 0.5, 0.5 },
		DONT_CARE_Z,
		FLAG_ILLEGAL_SHADER
	},

	{
		"break with no loop",
		NO_VERTEX_SHADER,
		"void main() { \n"
		"   break; \n"
		"   gl_FragColor = vec4(0.5); \n"
		"} \n",
		{ 0.5, 0.5, 0.5, 0.5 },
		DONT_CARE_Z,
		FLAG_ILLEGAL_SHADER
	},

	{
		"continue with no loop",
		NO_VERTEX_SHADER,
		"void main() { \n"
		"   continue; \n"
		"   gl_FragColor = vec4(0.5); \n"
		"} \n",
		{ 0.5, 0.5, 0.5, 0.5 },
		DONT_CARE_Z,
		FLAG_ILLEGAL_SHADER
	},

	{
		"illegal assignment",
		NO_VERTEX_SHADER,
		"void main() { \n"
		"   float x = main; \n"
		"   gl_FragColor = vec4(0.5); \n"
		"} \n",
		{ 0.5, 0.5, 0.5, 0.5 },
		DONT_CARE_Z,
		FLAG_ILLEGAL_SHADER
	},

	{
		"syntax error check (1)",
		NO_VERTEX_SHADER,
		"void main() { \n"
		"   float x = ; \n"
		"   gl_FragColor = vec4(0.5); \n"
		"} \n",
		{ 0.5, 0.5, 0.5, 0.5 },
		DONT_CARE_Z,
		FLAG_ILLEGAL_SHADER
	},

	{
		"syntax error check (2)",
		NO_VERTEX_SHADER,
		"main() { \n"
		"   gl_FragColor = vec4(0.5); \n"
		"} \n",
		{ 0.5, 0.5, 0.5, 0.5 },
		DONT_CARE_Z,
		FLAG_ILLEGAL_SHADER
	},

	{
		"syntax error check (3)",
		NO_VERTEX_SHADER,
		"main() { \n"
		"   float x = 1.0 2.0; \n"
		"   gl_FragColor = vec4(0.5); \n"
		"} \n",
		{ 0.5, 0.5, 0.5, 0.5 },
		DONT_CARE_Z,
		FLAG_ILLEGAL_SHADER
	},

	{
		"TIntermediate::addUnaryMath",
		NO_VERTEX_SHADER,
		"void main() { \n"
		"   -vec4(x ? 1.0 : -1.0); \n"
		"} \n",
		{ 0.5, 0.5, 0.5, 0.5 },
		DONT_CARE_Z,
		FLAG_ILLEGAL_SHADER
	},

	// Other new GLSL 1.20, 1.30 features (just parse/compile tests)
	{
		"GLSL 1.30 precision qualifiers",
		NO_VERTEX_SHADER,
		"#version 130 \n"
		"highp float f1; \n"
		"mediump float f2; \n"
		"lowp float f3; \n"
		"precision mediump float; \n"
		"precision lowp int; \n"
		"precision highp float; \n"
		"void main() { \n"
		"   gl_FragColor = vec4(1); \n"
		"} \n",
		{ 1.0, 1.0, 1.0, 1.0 },
		DONT_CARE_Z,
		FLAG_VERSION_1_30
	},
	{
		"GLSL 1.20 invariant, centroid qualifiers",
		NO_VERTEX_SHADER,
		"#version 120 \n"
		"invariant varying vec4 v1; \n"
		"centroid varying vec4 v2; \n"
		"invariant centroid varying vec4 v3; \n"
		"varying vec4 v4; \n"
		"invariant v4; \n"
		"void main() { \n"
		"   gl_FragColor = vec4(1); \n"
		"} \n",
		{ 1.0, 1.0, 1.0, 1.0 },
		DONT_CARE_Z,
		FLAG_VERSION_1_20
	},

#if 0
	// Check behaviour of inf/nan =========================================
	{
		"Divide by zero",
		NO_VERTEX_SHADER,
		"uniform vec4 uniform1; \n"
		"void main() { \n"
		"   float div = uniform1.y / uniform1.w; // div by zero\n"
		"   div = div * uniform1.w; // mul by zero \n"
		"   gl_FragColor = vec4(0.5 + div); \n"
		"} \n",
		{ 0.5, 0.5, 0.5, 0.5 },
		DONT_CARE_Z,
		FLAG_NONE
	},
#endif

	// Illegal link test ==================================================
	{
		"gl_Position not written check",
		"void main() { \n"
		"   gl_FrontColor = vec4(0.3); \n"
		"} \n",
		NO_FRAGMENT_SHADER,
		{ 0.5, 0.5, 0.5, 0.5 },
		DONT_CARE_Z,
		FLAG_ILLEGAL_LINK
	},

	{
		"varying var mismatch",
		// vert shader:
		"varying vec4 foo; \n"
		"void main() { \n"
		"   foo = gl_Color; \n"
		"   gl_Position = ftransform(); \n"
		"} \n",
		// frag shader:
		"varying vec4 bar; \n"
		"void main() { \n"
		"   gl_FragColor = bar; \n"
		"} \n",
		{ 0.0, 0.0, 0.0, 0.0 },
		DONT_CARE_Z,
		FLAG_ILLEGAL_LINK
	},

	{
		"varying read but not written",
		// vert shader:
		"varying vec4 foo; \n"
		"void main() { \n"
		"   gl_Position = ftransform(); \n"
		"} \n",
		// frag shader:
		"varying vec4 foo; \n"
		"void main() { \n"
		"   gl_FragColor = foo; \n"
		"} \n",
		{ 0.0, 0.0, 0.0, 0.0 },
		DONT_CARE_Z,
		FLAG_ILLEGAL_LINK
	},

	{
		"texcoord varying",
		// Does the linker correctly recognize that texcoord[1] is
		// written by the vertex shader and read by the fragment shader?
		// vert shader:
		"varying vec4 gl_TexCoord[4]; \n"
		"void main() { \n"
		"   int i = 1; \n"
		"   gl_TexCoord[i] = vec4(0.5, 0, 0, 0); \n"
		"   gl_Position = ftransform(); \n"
		"} \n",
		// frag shader:
		"varying vec4 gl_TexCoord[4]; \n"
		"void main() { \n"
		"   gl_FragColor = gl_TexCoord[1]; \n"
		"} \n",
		{ 0.5, 0.0, 0.0, 0.0 },
		DONT_CARE_Z,
		FLAG_NONE
	},

	{ NULL, NULL, NULL, {0,0,0,0}, 0, FLAG_NONE } // end of list sentinal
};


void
GLSLTest::setupTextures(void)
{
	GLubyte teximage0[16][16][4];
	GLubyte teximage1[8][8][4];
	GLubyte teximage2[4][4][4];
	GLubyte teximage3D[16][16][16][4];
	GLfloat teximageZ[16][16];
	GLint i, j, k;
	GLuint obj1D, obj2D, obj3D, objZ;

	glGenTextures(1, &obj1D);
	glGenTextures(1, &obj2D);
	glGenTextures(1, &obj3D);
	glGenTextures(1, &objZ);

	glActiveTexture(GL_TEXTURE0);

	//
	// 2D texture, w/ mipmap
	//
	glBindTexture(GL_TEXTURE_2D, obj2D);
	//  +-------+-------+
	//  | blue  | white |
	//  +-------+-------+
	//  | red   | green |
	//  +-------+-------+
	for (i = 0; i < 16; i++) {
		for (j = 0; j < 16; j++) {
			if (i < 8) {
				// bottom half
				if (j < 8) {
					// red
					teximage0[i][j][0] = 255;
					teximage0[i][j][1] = 0;
					teximage0[i][j][2] = 0;
					teximage0[i][j][3] = 255;
				}
				else {
					// green
					teximage0[i][j][0] = 0;
					teximage0[i][j][1] = 255;
					teximage0[i][j][2] = 0;
					teximage0[i][j][3] = 255;
				}
			}
			else {
				// top half
				if (j < 8) {
					// blue
					teximage0[i][j][0] = 0;
					teximage0[i][j][1] = 0;
					teximage0[i][j][2] = 255;
					teximage0[i][j][3] = 255;
				}
				else {
					// white
					teximage0[i][j][0] = 255;
					teximage0[i][j][1] = 255;
					teximage0[i][j][2] = 255;
					teximage0[i][j][3] = 255;
				}
			}
		}
	}
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 16, 16, 0,
		     GL_RGBA, GL_UNSIGNED_BYTE, teximage0);

	// level 1: same colors, half intensity
	for (i = 0; i < 8; i++) {
		for (j = 0; j < 8; j++) {
			teximage1[i][j][0] = teximage0[i*2][j*2][0] / 2;
			teximage1[i][j][1] = teximage0[i*2][j*2][1] / 2;
			teximage1[i][j][2] = teximage0[i*2][j*2][2] / 2;
			teximage1[i][j][3] = teximage0[i*2][j*2][3] / 2;
		}
	}
	glTexImage2D(GL_TEXTURE_2D, 1, GL_RGBA, 8, 8, 0,
		     GL_RGBA, GL_UNSIGNED_BYTE, teximage1);

	// level 2: 1/4 intensity
	for (i = 0; i < 4; i++) {
		for (j = 0; j < 4; j++) {
			teximage2[i][j][0] = teximage0[i*4][j*4][0] / 4;
			teximage2[i][j][1] = teximage0[i*4][j*4][1] / 4;
			teximage2[i][j][2] = teximage0[i*4][j*4][2] / 4;
			teximage2[i][j][3] = teximage0[i*4][j*4][3] / 4;
		}
	}
	glTexImage2D(GL_TEXTURE_2D, 2, GL_RGBA, 4, 4, 0,
		     GL_RGBA, GL_UNSIGNED_BYTE, teximage2);

	// level 3, 4: don't care
	glTexImage2D(GL_TEXTURE_2D, 3, GL_RGBA, 2, 2, 0,
		     GL_RGBA, GL_UNSIGNED_BYTE, teximage0);
	glTexImage2D(GL_TEXTURE_2D, 4, GL_RGBA, 1, 1, 0,
		     GL_RGBA, GL_UNSIGNED_BYTE, teximage0);


	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	//
	// 1D texture: just bottom row of the 2D texture
	//
	glBindTexture(GL_TEXTURE_1D, obj1D);
	glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA, 16, 0,
		     GL_RGBA, GL_UNSIGNED_BYTE, teximage0);

	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	//
	// 3D texture: 2D texture, depth = 1
	//
	for (i = 0; i < 16; i++) {
		for (j = 0; j < 16; j++) {
			for (k = 0; k < 16; k++) {
				if (i < 8) {
					teximage3D[i][j][k][0] = teximage0[j][k][0];
					teximage3D[i][j][k][1] = teximage0[j][k][1];
					teximage3D[i][j][k][2] = teximage0[j][k][2];
					teximage3D[i][j][k][3] = teximage0[j][k][3];
				}
				else {
					// back half: half intensity
					teximage3D[i][j][k][0] = teximage0[j][k][0] / 2;
					teximage3D[i][j][k][1] = teximage0[j][k][1] / 2;
					teximage3D[i][j][k][2] = teximage0[j][k][2] / 2;
					teximage3D[i][j][k][3] = teximage0[j][k][3] / 2;
				}
			}
		}
	}
	glBindTexture(GL_TEXTURE_3D, obj3D);
	glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA, 16, 16, 16, 0,
		     GL_RGBA, GL_UNSIGNED_BYTE, teximage3D);

	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	//
	// 2D GL_DEPTH_COMPONENT texture (for shadow sampler tests)
	// Left half = 0.25, right half = 0.75
	//
	for (i = 0; i < 16; i++) {
		for (j = 0; j < 16; j++) {
			if (j < 8)
				teximageZ[i][j] = 0.25;
			else
				teximageZ[i][j] = 0.75;
		}
	}
	glActiveTexture(GL_TEXTURE1); // NOTE: Unit 1
	glBindTexture(GL_TEXTURE_2D, objZ);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 16, 16, 0,
		     GL_DEPTH_COMPONENT, GL_FLOAT, teximageZ);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE_ARB,
					GL_COMPARE_R_TO_TEXTURE_ARB);
	
	glActiveTexture(GL_TEXTURE0);
}


void
GLSLTest::setupTextureMatrix1(void)
{
	// This matrix is used by some of the general matrix tests
	static const GLfloat m[16] = {
		1.0, 0.5, 0.6, 0.0,  // col 0
		0.0, 1.0, 0.0, 0.7,  // col 1
		0.0, 0.0, 1.0, 0.8,  // col 2
		0.1, 0.2, 0.3, 1.0   // col 3
	};
	glMatrixMode(GL_TEXTURE);
	glActiveTexture(GL_TEXTURE1);
	glLoadMatrixf(m);
	glActiveTexture(GL_TEXTURE0);
	glMatrixMode(GL_MODELVIEW);
}


bool
GLSLTest::setup(void)
{
	// check GLSL version
#ifdef GL_SHADING_LANGUAGE_VERSION
	const char *glslVersion = (const char *) glGetString(GL_SHADING_LANGUAGE_VERSION);
#else
	const char *glslVersion = NULL;
#endif
	const float version = atof(glslVersion);
	if (version < 1.00) {
		env->log << "GLSL 1.x not supported\n";
		return false;
	}
	glsl_120 = version >= 1.20;
	glsl_130 = version >= 1.30;

	setupTextures();
	setupTextureMatrix1();

	// load program inputs
	glColor4fv(PrimaryColor);
	glSecondaryColor3fv(SecondaryColor);

	// other GL state
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, Ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, LightDiffuse);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, MatDiffuse);
	glPointSize(PSIZE);
	glPointParameterf(GL_POINT_SIZE_MIN, PSIZE_MIN);
	glPointParameterf(GL_POINT_SIZE_MAX, PSIZE_MAX);
	glPointParameterf(GL_POINT_FADE_THRESHOLD_SIZE, PSIZE_THRESH);
	glPointParameterfv(GL_POINT_DISTANCE_ATTENUATION, PointAtten);
	glFogf(GL_FOG_START, FOG_START);
	glFogf(GL_FOG_END, FOG_END);
	glFogfv(GL_FOG_COLOR, FogColor);

	GLenum err = glGetError();
	assert(!err);  // should be OK

	// setup vertex transform (we'll draw a quad in middle of window)
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-4.0, 4.0, -4.0, 4.0, 0.0, 1.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glDrawBuffer(GL_FRONT);
	glReadBuffer(GL_FRONT); 

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

	// Some tests request a looser tolerance:
	// XXX a factor of 4 may be too much...
	for (int i = 0; i < 5; i++)
		looseTolerance[i] = 4.0 * tolerance[i];

	return true;
}


void
GLSLTest::reportFailure(const char *programName,
				   const GLfloat expectedColor[4],
				   const GLfloat actualColor[4] ) const
{
	env->log << "FAILURE:\n";
	env->log << "  Shader test: " << programName << "\n";
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
GLSLTest::reportZFailure(const char *programName,
				  GLfloat expectedZ, GLfloat actualZ) const
{
	env->log << "FAILURE:\n";
	env->log << "  Shader test: " << programName << "\n";
	env->log << "  Expected Z: " << expectedZ << "\n";
	env->log << "  Observed Z: " << actualZ << "\n";
}


// Compare actual and expected colors
bool
GLSLTest::equalColors(const GLfloat act[4], const GLfloat exp[4], int flags) const
{
	const GLfloat *tol;
	if (flags & FLAG_LOOSE)
		tol = looseTolerance;
	else
		tol = tolerance;
	if ((fabsf(act[0] - exp[0]) > tol[0]) ||
	    (fabsf(act[1] - exp[1]) > tol[1]) ||
	    (fabsf(act[2] - exp[2]) > tol[2]) ||
	    (fabsf(act[3] - exp[3]) > tol[3]))
		return false;
	else
		return true;
}


bool
GLSLTest::equalDepth(GLfloat z0, GLfloat z1) const
{
	if (fabsf(z0 - z1) > tolerance[4])
		return false;
	else
		return true;
}


GLuint
GLSLTest::loadAndCompileShader(GLenum target, const char *str)
{
	GLuint shader;
	shader = glCreateShader(target);
	glShaderSource(shader, 1, (const GLchar **) &str, NULL);
	glCompileShader(shader);
	return shader;
}


// Check the compile status of the just compiled shader.
// If the outcome is unexpected, report an error.
bool
GLSLTest::checkCompileStatus(GLenum target, GLuint shader,
			     const ShaderProgram &p)
{
	GLint stat;
	GLchar infoLog[1000];
	GLsizei len;

	glGetShaderiv(shader, GL_COMPILE_STATUS, &stat);
	if (!stat) {
		glGetShaderInfoLog(shader, 1000, &len, infoLog);
		// env->log << infoLog << "\n";
	}

	if (!stat && (p.flags & FLAG_ILLEGAL_SHADER) == 0) {
		// this _should_ have compiled
		env->log << "FAILURE:\n";
		env->log << "  Shader test: " << p.name << "\n";
		if (target == GL_FRAGMENT_SHADER)
			env->log << "Fragment shader did not compile:\n";
		else
			env->log << "Vertex shader did not compile:\n";
		env->log << infoLog;
		return false;
	}
	else if (stat && (p.flags & FLAG_ILLEGAL_SHADER)) {
		// this should _not_ have compiled!
		env->log << "FAILURE:\n";
		env->log << "  Shader test: " << p.name << "\n";
		env->log << "  Shader should not have compiled, but it did.\n";
		return false;
	}
	return true;
}


bool
GLSLTest::testProgram(const ShaderProgram &p)
{
	static const GLfloat uniformMatrix[16] = {
		1.0, 0.1, 0.2, 0.3,  // col 0
		0.0, 1.0, 0.0, 0.4,  // col 1
		0.0, 1.0, 1.0, 0.5,  // col 2
		0.6, 0.7, 0.8, 1.0   // col 3
	};
	static const GLfloat uniformMatrix2x4[8] = {
		0.0, 0.1, 0.2, 0.3,  // col 0
		0.4, 0.5, 0.6, 0.7   // col 1
	};
	static const GLfloat uniformMatrix4x3[12] = {
		0.0, 0.1, 0.2,  // col 0
		0.3, 0.4, 0.5,  // col 1
		0.6, 0.7, 0.8,  // col 2
		0.9, 1.0, 0.0   // col 3
	};
	const GLfloat r = 0.62; // XXX draw 16x16 pixel quad
	GLuint fragShader = 0, vertShader = 0, program = 0;
	GLint u1, uArray, uArray4, utex1d, utex2d, utex3d, utexZ, umat4, umat4t;
	GLint umat2x4, umat2x4t, umat4x3, umat4x3t;
	bool retVal = false;

	if (p.flags & FLAG_ARB_DRAW_BUFFERS &&
	    !GLUtils::haveExtensions("GL_ARB_draw_buffers")) {
		// skip
		retVal = true;
		goto cleanup;
	}


	if (p.fragShaderString) {
		fragShader = loadAndCompileShader(GL_FRAGMENT_SHADER,
						  p.fragShaderString);
		if (!checkCompileStatus(GL_FRAGMENT_SHADER, fragShader, p)) {
			retVal = false;
			goto cleanup;
		}
	}
	if (p.vertShaderString) {
		vertShader = loadAndCompileShader(GL_VERTEX_SHADER,
						  p.vertShaderString);
		if (!checkCompileStatus(GL_VERTEX_SHADER, vertShader, p)) {
			retVal = false;
			goto cleanup;
		}
	}
	if (!fragShader && !vertShader) {
		// must have had a compilation errror
		retVal = false;
		goto cleanup;
	}

	if (p.flags & FLAG_ILLEGAL_SHADER) {
		// don't render/test
		retVal = true;
		goto cleanup;
	}

	program = glCreateProgram();
	if (fragShader)
		glAttachShader(program, fragShader);
	if (vertShader)
		glAttachShader(program, vertShader);
	glLinkProgram(program);

	// check link
	{
		GLint stat;
		glGetProgramiv(program, GL_LINK_STATUS, &stat);
		if (!stat) {
			if (p.flags & FLAG_ILLEGAL_LINK) {
				// this is the expected outcome
				retVal = true;
				goto cleanup;
			}
			else {
				GLchar log[1000];
				GLsizei len;
				glGetProgramInfoLog(program, 1000, &len, log);
				env->log << "FAILURE:\n";
				env->log << "  Shader test: " << p.name << "\n";
				env->log << "  Link error: ";
				env->log << log;
				retVal = false;
				goto cleanup;
			}
		}
		else {
			// link successful
			if (p.flags & FLAG_ILLEGAL_LINK) {
				// the shaders should _not_ have linked
				env->log << "FAILURE:\n";
				env->log << "  Shader test: " << p.name << "\n";
				env->log << "  Program linked, but shouldn't have.\n";
				retVal = false;
				goto cleanup;
			}
		}
	}

	glUseProgram(program);

	if (p.flags & FLAG_VERTEX_TEXTURE) {
		// check if vertex texture units are available
		GLint n;
		glGetIntegerv(GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS_ARB, &n);
		if (n == 0) {
			// can't run the test
			retVal = true;
			goto cleanup;
		}
	}

	// load uniform vars
	u1 = glGetUniformLocation(program, "uniform1");
	if (u1 >= 0)
		glUniform4fv(u1, 1, Uniform1);

	uArray = glGetUniformLocation(program, "uniformArray");
	if (uArray >= 0)
		glUniform1fv(uArray, 4, UniformArray);

	uArray4 = glGetUniformLocation(program, "uniformArray4");
	if (uArray4 >= 0)
		glUniform4fv(uArray4, 4, (float *) UniformArray4);

	utex1d = glGetUniformLocation(program, "tex1d");
	if (utex1d >= 0)
		glUniform1i(utex1d, 0);  // bind to tex unit 0

	utex2d = glGetUniformLocation(program, "tex2d");
	if (utex2d >= 0)
		glUniform1i(utex2d, 0);  // bind to tex unit 0

	utex3d = glGetUniformLocation(program, "tex3d");
	if (utex3d >= 0)
		glUniform1i(utex3d, 0);  // bind to tex unit 0

	utexZ = glGetUniformLocation(program, "texZ");
	if (utexZ >= 0)
		glUniform1i(utexZ, 1);  // bind to tex unit 1

	umat4 = glGetUniformLocation(program, "uniformMat4");
	if (umat4 >= 0)
		glUniformMatrix4fv(umat4, 1, GL_FALSE, uniformMatrix);

	umat4t = glGetUniformLocation(program, "uniformMat4t");
	if (umat4t >= 0)
		glUniformMatrix4fv(umat4t, 1, GL_TRUE, uniformMatrix);

	umat2x4 = glGetUniformLocation(program, "uniformMat2x4");
	if (umat2x4 >= 0)
		glUniformMatrix2x4fv(umat2x4, 1, GL_FALSE, uniformMatrix2x4);

	umat2x4t = glGetUniformLocation(program, "uniformMat2x4t");
	if (umat2x4t >= 0)
		glUniformMatrix2x4fv(umat2x4t, 1, GL_TRUE, uniformMatrix2x4);

	umat4x3 = glGetUniformLocation(program, "uniformMat4x3");
	if (umat4x3 >= 0)
		glUniformMatrix4x3fv(umat4x3, 1, GL_FALSE, uniformMatrix4x3);

	umat4x3t = glGetUniformLocation(program, "uniformMat4x3t");
	if (umat4x3t >= 0)
		glUniformMatrix4x3fv(umat4x3t, 1, GL_TRUE, uniformMatrix4x3);


	// to avoid potential issue with undefined result.depth.z
	if (p.expectedZ == DONT_CARE_Z)
		glDisable(GL_DEPTH_TEST);
	else
		glEnable(GL_DEPTH_TEST);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	if (p.flags & FLAG_WINDING_CW) {
		/* Clockwise */
		glBegin(GL_POLYGON);
		glTexCoord2f(0, 0);  glVertex2f(-r, -r);
		glTexCoord2f(0, 1);  glVertex2f(-r,  r);
		glTexCoord2f(1, 1);  glVertex2f( r,  r);
		glTexCoord2f(1, 0);  glVertex2f( r, -r);
		glEnd();
	}
	else {
		/* Counter Clockwise */
		glBegin(GL_POLYGON);
		glTexCoord2f(0, 0);  glVertex2f(-r, -r);
		glTexCoord2f(1, 0);  glVertex2f( r, -r);
		glTexCoord2f(1, 1);  glVertex2f( r,  r);
		glTexCoord2f(0, 1);  glVertex2f(-r,  r);
		glEnd();
	}

	// env->log << "  Shader test: " << p.name << "\n";

	// read a pixel from lower-left corder of rendered quad
	GLfloat pixel[4];
	glReadPixels(windowSize / 2 - 2, windowSize / 2 - 2, 1, 1,
		     GL_RGBA, GL_FLOAT, pixel);
	if (0) // debug
		printf("%s: Expect: %.3f %.3f %.3f %.3f  found: %.3f %.3f %.3f %.3f\n",
		       p.name,
		       p.expectedColor[0], p.expectedColor[1],
		       p.expectedColor[2], p.expectedColor[3], 
		       pixel[0], pixel[1], pixel[2], pixel[3]);

	if (!equalColors(pixel, p.expectedColor, p.flags)) {
		reportFailure(p.name, p.expectedColor, pixel);
		retVal = false;
		goto cleanup;
	}

	if (p.expectedZ != DONT_CARE_Z) {
		GLfloat z;
		// read z at center of quad
		glReadPixels(windowSize / 2, windowSize / 2, 1, 1,
			     GL_DEPTH_COMPONENT, GL_FLOAT, &z);
		if (!equalDepth(z, p.expectedZ)) {
			reportZFailure(p.name, p.expectedZ, z);
			retVal = false;
			goto cleanup;
		}
	}

	// passed!
	retVal = true;

	if (0) // debug
	   printf("%s passed\n", p.name);

 cleanup:
	if (fragShader)
		glDeleteShader(fragShader);
	if (vertShader)
		glDeleteShader(vertShader);
	glDeleteProgram(program);

	return retVal;
}


void
GLSLTest::runOne(MultiTestResult &r, Window &w)
{
	(void) w;
	if (!setup()) {
		r.pass = false;
		return;
	}

	// If you just want to run a single sub-test, assign the name to singleTest.
	const char *singleTest = getenv("PIGLIT_TEST");
	if (singleTest) {
		env->log << "glsl1: Running single test: " << singleTest << "\n";
		for (int i = 0; Programs[i].name; i++) {
			if (strcmp(Programs[i].name, singleTest) == 0) {

				if ((Programs[i].flags & FLAG_VERSION_1_20) && !glsl_120)
					break; // skip non-applicable tests
				if ((Programs[i].flags & FLAG_VERSION_1_30) && !glsl_130)
					break; // skip non-applicable tests

				r.numPassed = testProgram(Programs[i]);
				r.numFailed = 1 - r.numPassed;
				break;
			}
		}
	}
	else {
		// loop over all tests
		for (int i = 0; Programs[i].name; i++) {
			if ((Programs[i].flags & FLAG_VERSION_1_20) && !glsl_120)
				continue; // skip non-applicable tests
			if ((Programs[i].flags & FLAG_VERSION_1_30) && !glsl_130)
				continue; // skip non-applicable tests
			if (testProgram(Programs[i])) {
				r.numPassed++;
			}
			else {
				r.numFailed++;
			}
		}
	}
	r.pass = (r.numFailed == 0);
}


// We need OpenGL 2.0, 2.1 or 3.0
bool
GLSLTest::isApplicable() const
{
	const char *version = (const char *) glGetString(GL_VERSION);
	const float v = atof(version);
	if (v >= 2.0) {
		return true;
	}
	else {
		env->log << name
				 << ":  skipped.  Requires GL 2.0 or later.\n";
		return false;
	}
}


// The test object itself:
GLSLTest glslTest("glsl1", "window, rgb, z",
		  "",  // no extension filter but see isApplicable()
		  "GLSL test 1: test basic Shading Language functionality.\n"
		  );



} // namespace GLEAN
