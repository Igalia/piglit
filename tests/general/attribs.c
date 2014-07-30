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
 * Test for immediate-mode style commands like glVertexAttrib, glColor, etc.
 * with vertex arrays, immediate mode and display lists.
 * Most of the GL2 and GL3 commands are covered.
 *
 * glVertex and the commands taking a pointer (e.g. glColor*v) are not
 * tested here.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_width = 512;
	config.window_height = 512;
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;

PIGLIT_GL_TEST_CONFIG_END

static GLboolean snorm_equation_23;

enum {
	R,
	RG,
	RGB,
	RGBA
};

enum {
	FLOAT_TYPE,
	INT_TYPE,
	UINT_TYPE
};

enum {
	VERTEX_ARRAYS,
	IMMEDIATE_MODE,
	DISPLAY_LIST,
	NUM_MODES
};

static const char *mode_to_str[NUM_MODES] = {
	"vertex arrays",
	"immediate mode",
	"display list"
};

static void draw_quad(unsigned mode, float *v,
		      void (*attrib)(float x, float y, float z, float w))
{
	static const float verts[] = {
		0,   0,
		0,  10,
		10, 10,
		10,  0
	};
	int i;

	switch (mode) {
	case VERTEX_ARRAYS:
		attrib(v[0], v[1], v[2], v[3]);
		glEnableClientState(GL_VERTEX_ARRAY);
		glVertexPointer(2, GL_FLOAT, 0, verts);
		glDrawArrays(GL_QUADS, 0, 4);
		glDisableClientState(GL_VERTEX_ARRAY);
		break;
	case IMMEDIATE_MODE:
		glBegin(GL_QUADS);
		for (i = 0; i < 4; i++) {
			attrib(v[0], v[1], v[2], v[3]);
			glVertex2fv(&verts[i*2]);
		}
		glEnd();
		break;
	case DISPLAY_LIST:
		glNewList(1, GL_COMPILE);
		glBegin(GL_QUADS);
		for (i = 0; i < 4; i++) {
			attrib(v[0], v[1], v[2], v[3]);
			glVertex2fv(&verts[i*2]);
		}
		glEnd();
		glEndList();
		glCallList(1);
		break;
	default:
		assert(0);
	}
}

static GLboolean test(int x, int y, const char *shaderfunc,
		      unsigned mask, unsigned type, unsigned mode,
		      void (*attrib)(float x, float y, float z, float w),
		      const char *info)
{
	static const char *templ = {
		"%s \n"
		"#extension GL_ARB_explicit_attrib_location : require \n"
		"layout(location = 1) in %s attr; \n"
		"void main() { \n"
		"  gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex; \n"
		"  gl_FrontColor = (%s) * vec4(1.0, 1.0, 1.0, 0.5); \n"
		"} \n"
	};
	GLuint prog, vs;
	GLboolean pass = GL_TRUE;
	char vstext[1024];
	int i;

	float color[3][4] = {
		{0.2, 0.4, 0.6, 0.8},
		{0, 1, 0, 1},
		{0.5, 0.3, 0.9, 0.2}
	};

	sprintf(vstext, templ,
		type != FLOAT_TYPE ? "#version 130" : "",
		type == INT_TYPE ? "ivec4" : type == UINT_TYPE ? "uvec4" : "vec4",
		shaderfunc);

	/* Create the shader. */
	vs = piglit_compile_shader_text(GL_VERTEX_SHADER, vstext);
	if (!vs)
		piglit_report_result(PIGLIT_FAIL);
	prog = piglit_link_simple_program(vs, 0);
	if (!prog)
		piglit_report_result(PIGLIT_FAIL);

	/* Render. */
	glUseProgram(prog);
	glLoadIdentity();
	glTranslatef(x, y, 0);

	draw_quad(mode, color[0], attrib);
	glDrawArrays(GL_QUADS, 0, 4);

	glTranslatef(10, 0, 0);
	draw_quad(mode, color[1], attrib);

	glTranslatef(10, 0, 0);
	draw_quad(mode, color[2], attrib);

	switch (mask) {
	case R:
		for (i = 0; i < 3; i++)
			color[i][1] = 0;
		/* fall through */
	case RG:
		for (i = 0; i < 3; i++)
			color[i][2] = 0;
		/* fall through */
	case RGB:
		for (i = 0; i < 3; i++)
			color[i][3] = 1;
		/* fall through */
	case RGBA:
		break;
	default:
		assert(0);
	}

	if (strstr(info, "GL_INT_2_10_10_10_REV-norm")) {
		if (snorm_equation_23) {
			for (i = 0; i < 3; i++) {
				if (color[i][3] < 1)
					color[i][3] = 0;
			}
		}
		else {
			for (i = 0; i < 3; i++) {
				if (color[i][3] < 0.333)
					color[i][3] = 0;
				else if (color[i][3] < 1)
					color[i][3] = 0.333;
			}
		}
	} else if (strstr(info, "GL_INT_2_10_10_10_REV")) {
		for (i = 0; i < 3; i++) {
			if (color[i][3] < 1)
				color[i][3] = 0;
		}
	} else if (strstr(info, "GL_UNSIGNED_INT_2_10_10_10_REV")) {
		for (i = 0; i < 3; i++) {
			if (color[i][3] < 0.333)
				color[i][3] = 0;
			else if (color[i][3] < 0.666)
				color[i][3] = 0.333;
			else if (color[i][3] < 1)
				color[i][3] = 0.666;
		}
	}

	/* We scale alpha in the shader to ensure it's not > 1 when it should be 1. */
	for (i = 0; i < 3; i++)
		color[i][3] *= 0.5;

	/* Probe pixels. */
	pass = piglit_probe_pixel_rgba(x+5,  y+5, color[0]) && pass;
	pass = piglit_probe_pixel_rgba(x+15, y+5, color[1]) && pass;
	pass = piglit_probe_pixel_rgba(x+25, y+5, color[2]) && pass;
	return pass;
}

typedef GLboolean (*test_func)(int x, int y, unsigned mode);

#define DEFINE_TEST(func, funcsuffix, params, shaderfunc, mask, type, info) \
	static void invoke_##func##funcsuffix(float x, float y, float z, float w) \
	{ \
		func params; \
	} \
	\
	static GLboolean test_##func##funcsuffix(int x, int y, unsigned mode) \
	{ \
		printf("Testing %s, %s\n", #func "(" info ")", mode_to_str[mode]); \
		return test(x, y, shaderfunc, mask, type, mode, invoke_##func##funcsuffix, info); \
	}

#define B(f) ((f) * 0x7F)
#define UB(f) ((f) * 0xFF)
#define S(f) ((f) * 0x7FFF)
#define US(f) ((f) * 0xFFFF)
#define I(f) ((double)(f) * 0x7FFFFFFF)
#define UI(f) ((double)(f) * 0xFFFFFFFF)

/* GL 2.0 */
/* XXX This list is incomplete. */
DEFINE_TEST(glColor3b,,  (B(x), B(y), B(z)),		"gl_Color", RGB, FLOAT_TYPE, "")
DEFINE_TEST(glColor3d,,  (x, y, z),			"gl_Color", RGB, FLOAT_TYPE, "")
DEFINE_TEST(glColor3f,,  (x, y, z),			"gl_Color", RGB, FLOAT_TYPE, "")
DEFINE_TEST(glColor3i,,  (I(x), I(y), I(z)),		"gl_Color", RGB, FLOAT_TYPE, "")
DEFINE_TEST(glColor3s,,  (S(x), S(y), S(z)),		"gl_Color", RGB, FLOAT_TYPE, "")
DEFINE_TEST(glColor3ub,, (UB(x), UB(y), UB(z)),		"gl_Color", RGB, FLOAT_TYPE, "")
DEFINE_TEST(glColor3ui,, (UI(x), UI(y), UI(z)),		"gl_Color", RGB, FLOAT_TYPE, "")
DEFINE_TEST(glColor3us,, (US(x), US(y), US(z)),		"gl_Color", RGB, FLOAT_TYPE, "")
DEFINE_TEST(glColor4b,,  (B(x), B(y), B(z), B(w)),	"gl_Color", RGBA, FLOAT_TYPE, "")
DEFINE_TEST(glColor4d,,  (x, y, z, w),			"gl_Color", RGBA, FLOAT_TYPE, "")
DEFINE_TEST(glColor4f,,  (x, y, z, w),			"gl_Color", RGBA, FLOAT_TYPE, "")
DEFINE_TEST(glColor4i,,  (I(x), I(y), I(z), I(w)),	"gl_Color", RGBA, FLOAT_TYPE, "")
DEFINE_TEST(glColor4s,,  (S(x), S(y), S(z), S(w)),	"gl_Color", RGBA, FLOAT_TYPE, "")
DEFINE_TEST(glColor4ub,, (UB(x), UB(y), UB(z), UB(w)),	"gl_Color", RGBA, FLOAT_TYPE, "")
DEFINE_TEST(glColor4ui,, (UI(x), UI(y), UI(z), UI(w)),	"gl_Color", RGBA, FLOAT_TYPE, "")
DEFINE_TEST(glColor4us,, (US(x), US(y), US(z), US(w)),	"gl_Color",RGBA, FLOAT_TYPE, "")
DEFINE_TEST(glVertexAttrib1d,, (1, x),			"attr",	R, FLOAT_TYPE, "")
DEFINE_TEST(glVertexAttrib1f,, (1, x),			"attr", R, FLOAT_TYPE, "")
DEFINE_TEST(glVertexAttrib1s,, (1, S(x)),		"attr * vec4(1.0/32768.0, 1.0, 1.0, 1.0)", R, FLOAT_TYPE, "")
DEFINE_TEST(glVertexAttrib2d,, (1, x, y),		"attr", RG, FLOAT_TYPE, "")
DEFINE_TEST(glVertexAttrib2f,, (1, x, y),		"attr", RG, FLOAT_TYPE, "")
DEFINE_TEST(glVertexAttrib2s,, (1, S(x), S(y)),		"attr * vec4(vec2(1.0/32768.0), 1.0, 1.0)", RG, FLOAT_TYPE, "")
DEFINE_TEST(glVertexAttrib3d,, (1, x, y, z),		"attr", RGB, FLOAT_TYPE, "")
DEFINE_TEST(glVertexAttrib3f,, (1, x, y, z),		"attr", RGB, FLOAT_TYPE, "")
DEFINE_TEST(glVertexAttrib3s,, (1, S(x), S(y), S(z)),	"attr * vec4(vec3(1.0/32768.0), 1.0)", RGB, FLOAT_TYPE, "")
DEFINE_TEST(glVertexAttrib4Nub,, (1, UB(x), UB(y), UB(z), UB(w)), "attr", RGBA, FLOAT_TYPE, "")
DEFINE_TEST(glVertexAttrib4d,, (1, x, y, z, w),		"attr", RGBA, FLOAT_TYPE, "")
DEFINE_TEST(glVertexAttrib4f,, (1, x, y, z, w),		"attr", RGBA, FLOAT_TYPE, "")
DEFINE_TEST(glVertexAttrib4s,, (1, S(x), S(y), S(z), S(w)), "attr * vec4(1.0/32768.0)", RGBA, FLOAT_TYPE, "")

static test_func tests_GL2[] = {
	test_glColor3b,
	test_glColor3d,
	test_glColor3f,
	test_glColor3i,
	test_glColor3s,
	test_glColor3ub,
	test_glColor3ui,
	test_glColor3ui,
	test_glColor3us,
	test_glColor4b,
	test_glColor4d,
	test_glColor4f,
	test_glColor4i,
	test_glColor4s,
	test_glColor4ub,
	test_glColor4ui,
	test_glColor4us,
	test_glVertexAttrib1d,
	test_glVertexAttrib1d,
	test_glVertexAttrib1f,
	test_glVertexAttrib1s,
	test_glVertexAttrib2d,
	test_glVertexAttrib2f,
	test_glVertexAttrib2s,
	test_glVertexAttrib3d,
	test_glVertexAttrib3f,
	test_glVertexAttrib3s,
	test_glVertexAttrib4Nub,
	test_glVertexAttrib4d,
	test_glVertexAttrib4f,
	test_glVertexAttrib4s,
};

/* GL 3.0 */
DEFINE_TEST(glVertexAttribI1i,, (1, I(x)),
	    "vec4(attr) * vec4(1.0/2147483647.0, 1.0, 1.0, 1.0)", R, INT_TYPE, "")
DEFINE_TEST(glVertexAttribI2i,, (1, I(x), I(y)),
	    "vec4(attr) * vec4(vec2(1.0/2147483647.0), 1.0, 1.0)", RG, INT_TYPE, "")
DEFINE_TEST(glVertexAttribI3i,, (1, I(x), I(y), I(z)),
	    "vec4(attr) * vec4(vec3(1.0/2147483647.0), 1.0)",	RGB, INT_TYPE, "")
DEFINE_TEST(glVertexAttribI4i,, (1, I(x), I(y), I(z), I(w)),
	    "vec4(attr) * vec4(1.0/2147483647.0)",		RGBA, INT_TYPE, "")
DEFINE_TEST(glVertexAttribI1ui,, (1, UI(x)),
	    "vec4(attr) * vec4(1.0/4294967295.0, 1.0, 1.0, 1.0)", R, UINT_TYPE, "")
DEFINE_TEST(glVertexAttribI2ui,, (1, UI(x), UI(y)),
	    "vec4(attr) * vec4(vec2(1.0/4294967295.0), 1.0, 1.0)", RG, UINT_TYPE, "")
DEFINE_TEST(glVertexAttribI3ui,, (1, UI(x), UI(y), UI(z)),
	    "vec4(attr) * vec4(vec3(1.0/4294967295.0), 1.0)",	RGB, UINT_TYPE, "")
DEFINE_TEST(glVertexAttribI4ui,, (1, UI(x), UI(y), UI(z), UI(w)),
	    "vec4(attr) * vec4(1.0/4294967295.0)",		RGBA, UINT_TYPE, "")

static test_func tests_GL3[] = {
	test_glVertexAttribI1i,
	test_glVertexAttribI2i,
	test_glVertexAttribI3i,
	test_glVertexAttribI4i,
	test_glVertexAttribI1ui,
	test_glVertexAttribI2ui,
	test_glVertexAttribI3ui,
	test_glVertexAttribI4ui,
};

/* ARB_vertex_type_2_10_10_10_rev */
/* Packing functions for a signed normalized 2-bit component.
 * These are based on equation 2.2 and 2.3 from the opengl specification, see:
 * http://lists.freedesktop.org/archives/mesa-dev/2013-August/042680.html
 */
#define PN2_22(f) ((int)((f) < 0.333 ? 3 /* = -0.333 */ : (f) < 1 ? 0 /* = 0.333 */ : 1))
#define PN2_23(f) ((int)((f) < 1 ? 0 : 1))
#define PN2(f) (snorm_equation_23 ? PN2_23(f) : PN2_22(f))
/* other packing functions */
#define P10(f) ((int)((f) * 0x1FF))
#define UP10(f) ((int)((f) * 0x3FF))
#define P2(f) ((int)(f)) /* unnormalized */
#define UP2(f) ((int)((f) * 0x3))
#define P1010102(x,y,z,w) (P10(x) | (P10(y) << 10) | (P10(z) << 20) | (P2(w) << 30))
#define PN1010102(x,y,z,w) (P10(x) | (P10(y) << 10) | (P10(z) << 20) | (PN2(w) << 30))
#define UP1010102(x,y,z,w) (UP10(x) | (UP10(y) << 10) | (UP10(z) << 20) | (UP2(w) << 30))

/* GL_INT_2_10_10_10_REV */
DEFINE_TEST(glTexCoordP1ui,, (GL_INT_2_10_10_10_REV, P1010102(x,y,z,w)),
	    "gl_MultiTexCoord0 * vec4(1.0/511.0, 1.0, 1.0, 1.0)", R, FLOAT_TYPE, "GL_INT_2_10_10_10_REV")
DEFINE_TEST(glTexCoordP2ui,, (GL_INT_2_10_10_10_REV, P1010102(x,y,z,w)),
	    "gl_MultiTexCoord0 * vec4(vec2(1.0/511.0), 1.0, 1.0)", RG, FLOAT_TYPE, "GL_INT_2_10_10_10_REV")
DEFINE_TEST(glTexCoordP3ui,, (GL_INT_2_10_10_10_REV, P1010102(x,y,z,w)),
	    "gl_MultiTexCoord0 * vec4(vec3(1.0/511.0), 1.0)", RGB, FLOAT_TYPE, "GL_INT_2_10_10_10_REV")
DEFINE_TEST(glTexCoordP4ui,, (GL_INT_2_10_10_10_REV, P1010102(x,y,z,w)),
	    "gl_MultiTexCoord0 * vec4(vec3(1.0/511.0), 1.0)", RGBA, FLOAT_TYPE, "GL_INT_2_10_10_10_REV")
DEFINE_TEST(glMultiTexCoordP1ui,, (GL_TEXTURE1, GL_INT_2_10_10_10_REV, P1010102(x,y,z,w)),
	    "gl_MultiTexCoord1 * vec4(1.0/511.0, 1.0, 1.0, 1.0)", R, FLOAT_TYPE, "GL_INT_2_10_10_10_REV")
DEFINE_TEST(glMultiTexCoordP2ui,, (GL_TEXTURE1, GL_INT_2_10_10_10_REV, P1010102(x,y,z,w)),
	    "gl_MultiTexCoord1 * vec4(vec2(1.0/511.0), 1.0, 1.0)", RG, FLOAT_TYPE, "GL_INT_2_10_10_10_REV")
DEFINE_TEST(glMultiTexCoordP3ui,, (GL_TEXTURE1, GL_INT_2_10_10_10_REV, P1010102(x,y,z,w)),
	    "gl_MultiTexCoord1 * vec4(vec3(1.0/511.0), 1.0)", RGB, FLOAT_TYPE, "GL_INT_2_10_10_10_REV")
DEFINE_TEST(glMultiTexCoordP4ui,, (GL_TEXTURE1, GL_INT_2_10_10_10_REV, P1010102(x,y,z,w)),
	    "gl_MultiTexCoord1 * vec4(vec3(1.0/511.0), 1.0)", RGBA, FLOAT_TYPE, "GL_INT_2_10_10_10_REV")
DEFINE_TEST(glNormalP3ui,, (GL_INT_2_10_10_10_REV, PN1010102(x,y,z,w)),
	    "vec4(gl_Normal, 1.0)", RGB, FLOAT_TYPE, "GL_INT_2_10_10_10_REV-norm")
DEFINE_TEST(glColorP3ui,, (GL_INT_2_10_10_10_REV, PN1010102(x,y,z,w)),
	    "gl_Color", RGB, FLOAT_TYPE, "GL_INT_2_10_10_10_REV-norm")
DEFINE_TEST(glColorP4ui,, (GL_INT_2_10_10_10_REV, PN1010102(x,y,z,w)),
	    "gl_Color", RGBA, FLOAT_TYPE, "GL_INT_2_10_10_10_REV-norm")
DEFINE_TEST(glSecondaryColorP3ui,, (GL_INT_2_10_10_10_REV, PN1010102(x,y,z,w)),
	    "gl_SecondaryColor", RGB, FLOAT_TYPE, "GL_INT_2_10_10_10_REV-norm")
/* GL_INT_2_10_10_10_REV unnormalized */
DEFINE_TEST(glVertexAttribP1ui,, (1, GL_INT_2_10_10_10_REV, 0, P1010102(x,y,z,w)),
	    "attr * vec4(1.0/511.0, 1.0, 1.0, 1.0)", R, FLOAT_TYPE, "GL_INT_2_10_10_10_REV")
DEFINE_TEST(glVertexAttribP2ui,, (1, GL_INT_2_10_10_10_REV, 0, P1010102(x,y,z,w)),
	    "attr * vec4(vec2(1.0/511.0), 1.0, 1.0)", RG, FLOAT_TYPE, "GL_INT_2_10_10_10_REV")
DEFINE_TEST(glVertexAttribP3ui,, (1, GL_INT_2_10_10_10_REV, 0, P1010102(x,y,z,w)),
	    "attr * vec4(vec3(1.0/511.0), 1.0)", RGB, FLOAT_TYPE, "GL_INT_2_10_10_10_REV")
DEFINE_TEST(glVertexAttribP4ui,, (1, GL_INT_2_10_10_10_REV, 0, P1010102(x,y,z,w)),
	    "attr * vec4(vec3(1.0/511.0), 1.0)", RGBA, FLOAT_TYPE, "GL_INT_2_10_10_10_REV")
/* GL_INT_2_10_10_10_REV normalized */
DEFINE_TEST(glVertexAttribP1ui,_norm, (1, GL_INT_2_10_10_10_REV, 1, PN1010102(x,y,z,w)),
	    "attr", R, FLOAT_TYPE, "GL_INT_2_10_10_10_REV-norm")
DEFINE_TEST(glVertexAttribP2ui,_norm, (1, GL_INT_2_10_10_10_REV, 1, PN1010102(x,y,z,w)),
	    "attr", RG, FLOAT_TYPE, "GL_INT_2_10_10_10_REV-norm")
DEFINE_TEST(glVertexAttribP3ui,_norm, (1, GL_INT_2_10_10_10_REV, 1, PN1010102(x,y,z,w)),
	    "attr", RGB, FLOAT_TYPE, "GL_INT_2_10_10_10_REV-norm")
DEFINE_TEST(glVertexAttribP4ui,_norm, (1, GL_INT_2_10_10_10_REV, 1, PN1010102(x,y,z,w)),
	    "attr", RGBA, FLOAT_TYPE, "GL_INT_2_10_10_10_REV-norm")

/* GL_UNSIGNED_INT_2_10_10_10_REV */
DEFINE_TEST(glTexCoordP1ui,_uint, (GL_UNSIGNED_INT_2_10_10_10_REV, UP1010102(x,y,z,w)),
	    "gl_MultiTexCoord0 * vec4(1.0/1023.0, 1.0, 1.0, 1.0)", R, FLOAT_TYPE, "GL_UNSIGNED_INT_2_10_10_10_REV")
DEFINE_TEST(glTexCoordP2ui,_uint, (GL_UNSIGNED_INT_2_10_10_10_REV, UP1010102(x,y,z,w)),
	    "gl_MultiTexCoord0 * vec4(vec2(1.0/1023.0), 1.0, 1.0)", RG, FLOAT_TYPE, "GL_UNSIGNED_INT_2_10_10_10_REV")
DEFINE_TEST(glTexCoordP3ui,_uint, (GL_UNSIGNED_INT_2_10_10_10_REV, UP1010102(x,y,z,w)),
	    "gl_MultiTexCoord0 * vec4(vec3(1.0/1023.0), 1.0)", RGB, FLOAT_TYPE, "GL_UNSIGNED_INT_2_10_10_10_REV")
DEFINE_TEST(glTexCoordP4ui,_uint, (GL_UNSIGNED_INT_2_10_10_10_REV, UP1010102(x,y,z,w)),
	    "gl_MultiTexCoord0 * vec4(vec3(1.0/1023.0), 1.0/3.0)", RGBA, FLOAT_TYPE, "GL_UNSIGNED_INT_2_10_10_10_REV")
DEFINE_TEST(glMultiTexCoordP1ui,_uint, (GL_TEXTURE1, GL_UNSIGNED_INT_2_10_10_10_REV, UP1010102(x,y,z,w)),
	    "gl_MultiTexCoord1 * vec4(1.0/1023.0, 1.0, 1.0, 1.0)", R, FLOAT_TYPE, "GL_UNSIGNED_INT_2_10_10_10_REV")
DEFINE_TEST(glMultiTexCoordP2ui,_uint, (GL_TEXTURE1, GL_UNSIGNED_INT_2_10_10_10_REV, UP1010102(x,y,z,w)),
	    "gl_MultiTexCoord1 * vec4(vec2(1.0/1023.0), 1.0, 1.0)", RG, FLOAT_TYPE, "GL_UNSIGNED_INT_2_10_10_10_REV")
DEFINE_TEST(glMultiTexCoordP3ui,_uint, (GL_TEXTURE1, GL_UNSIGNED_INT_2_10_10_10_REV, UP1010102(x,y,z,w)),
	    "gl_MultiTexCoord1 * vec4(vec3(1.0/1023.0), 1.0)", RGB, FLOAT_TYPE, "GL_UNSIGNED_INT_2_10_10_10_REV")
DEFINE_TEST(glMultiTexCoordP4ui,_uint, (GL_TEXTURE1, GL_UNSIGNED_INT_2_10_10_10_REV, UP1010102(x,y,z,w)),
	    "gl_MultiTexCoord1 * vec4(vec3(1.0/1023.0), 1.0/3.0)", RGBA, FLOAT_TYPE, "GL_UNSIGNED_INT_2_10_10_10_REV")
DEFINE_TEST(glNormalP3ui,_uint, (GL_UNSIGNED_INT_2_10_10_10_REV, UP1010102(x,y,z,w)),
	    "vec4(gl_Normal, 1.0)", RGB, FLOAT_TYPE, "GL_UNSIGNED_INT_2_10_10_10_REV-norm")
DEFINE_TEST(glColorP3ui,_uint, (GL_UNSIGNED_INT_2_10_10_10_REV, UP1010102(x,y,z,w)),
	    "gl_Color", RGB, FLOAT_TYPE, "GL_UNSIGNED_INT_2_10_10_10_REV-norm")
DEFINE_TEST(glColorP4ui,_uint, (GL_UNSIGNED_INT_2_10_10_10_REV, UP1010102(x,y,z,w)),
	    "gl_Color", RGBA, FLOAT_TYPE, "GL_UNSIGNED_INT_2_10_10_10_REV-norm")
DEFINE_TEST(glSecondaryColorP3ui,_uint, (GL_UNSIGNED_INT_2_10_10_10_REV, UP1010102(x,y,z,w)),
	    "gl_SecondaryColor", RGB, FLOAT_TYPE, "GL_UNSIGNED_INT_2_10_10_10_REV-norm")
/* GL_UNSIGNED_INT_2_10_10_10_REV unnormalized */
DEFINE_TEST(glVertexAttribP1ui,_uint, (1, GL_UNSIGNED_INT_2_10_10_10_REV, 0, UP1010102(x,y,z,w)),
	    "attr * vec4(1.0/1023.0, 1.0, 1.0, 1.0)", R, FLOAT_TYPE, "GL_UNSIGNED_INT_2_10_10_10_REV")
DEFINE_TEST(glVertexAttribP2ui,_uint, (1, GL_UNSIGNED_INT_2_10_10_10_REV, 0, UP1010102(x,y,z,w)),
	    "attr * vec4(vec2(1.0/1023.0), 1.0, 1.0)", RG, FLOAT_TYPE, "GL_UNSIGNED_INT_2_10_10_10_REV")
DEFINE_TEST(glVertexAttribP3ui,_uint, (1, GL_UNSIGNED_INT_2_10_10_10_REV, 0, UP1010102(x,y,z,w)),
	    "attr * vec4(vec3(1.0/1023.0), 1.0)", RGB, FLOAT_TYPE, "GL_UNSIGNED_INT_2_10_10_10_REV")
DEFINE_TEST(glVertexAttribP4ui,_uint, (1, GL_UNSIGNED_INT_2_10_10_10_REV, 0, UP1010102(x,y,z,w)),
	    "attr * vec4(vec3(1.0/1023.0), 1.0/3.0)", RGBA, FLOAT_TYPE, "GL_UNSIGNED_INT_2_10_10_10_REV")
/* GL_UNSIGNED_INT_2_10_10_10_REV normalized */
DEFINE_TEST(glVertexAttribP1ui,_uint_norm, (1, GL_UNSIGNED_INT_2_10_10_10_REV, 1, UP1010102(x,y,z,w)),
	    "attr", R, FLOAT_TYPE, "GL_UNSIGNED_INT_2_10_10_10_REV-norm")
DEFINE_TEST(glVertexAttribP2ui,_uint_norm, (1, GL_UNSIGNED_INT_2_10_10_10_REV, 1, UP1010102(x,y,z,w)),
	    "attr", RG, FLOAT_TYPE, "GL_UNSIGNED_INT_2_10_10_10_REV-norm")
DEFINE_TEST(glVertexAttribP3ui,_uint_norm, (1, GL_UNSIGNED_INT_2_10_10_10_REV, 1, UP1010102(x,y,z,w)),
	    "attr", RGB, FLOAT_TYPE, "GL_UNSIGNED_INT_2_10_10_10_REV-norm")
DEFINE_TEST(glVertexAttribP4ui,_uint_norm, (1, GL_UNSIGNED_INT_2_10_10_10_REV, 1, UP1010102(x,y,z,w)),
	    "attr", RGBA, FLOAT_TYPE, "GL_UNSIGNED_INT_2_10_10_10_REV-norm")

static test_func tests_GL_ARB_vertex_type_2_10_10_10_rev[] = {
	test_glTexCoordP1ui,
	test_glTexCoordP2ui,
	test_glTexCoordP3ui,
	test_glTexCoordP4ui,
	test_glMultiTexCoordP1ui,
	test_glMultiTexCoordP2ui,
	test_glMultiTexCoordP3ui,
	test_glMultiTexCoordP4ui,
	test_glNormalP3ui,
	test_glColorP3ui,
	test_glColorP4ui,
	test_glSecondaryColorP3ui,
	test_glVertexAttribP1ui,
	test_glVertexAttribP2ui,
	test_glVertexAttribP3ui,
	test_glVertexAttribP4ui,
	test_glVertexAttribP1ui_norm,
	test_glVertexAttribP2ui_norm,
	test_glVertexAttribP3ui_norm,
	test_glVertexAttribP4ui_norm,
	test_glTexCoordP1ui_uint,
	test_glTexCoordP2ui_uint,
	test_glTexCoordP3ui_uint,
	test_glTexCoordP4ui_uint,
	test_glMultiTexCoordP1ui_uint,
	test_glMultiTexCoordP2ui_uint,
	test_glMultiTexCoordP3ui_uint,
	test_glMultiTexCoordP4ui_uint,
	test_glNormalP3ui_uint,
	test_glColorP3ui_uint,
	test_glColorP4ui_uint,
	test_glSecondaryColorP3ui_uint,
	test_glVertexAttribP1ui_uint,
	test_glVertexAttribP2ui_uint,
	test_glVertexAttribP3ui_uint,
	test_glVertexAttribP4ui_uint,
	test_glVertexAttribP1ui_uint_norm,
	test_glVertexAttribP2ui_uint_norm,
	test_glVertexAttribP3ui_uint_norm,
	test_glVertexAttribP4ui_uint_norm,
};

#define TESTS(name) #name, tests_##name, ARRAY_SIZE(tests_##name)

struct test_set {
	const char *name;
	test_func *tests;
	int num_tests;
	int gl_version;
	const char *extension;
	int alpha_bits;
} test_sets[] = {
	{ TESTS(GL2) },
	{ TESTS(GL3), 30 },
	{ TESTS(GL_ARB_vertex_type_2_10_10_10_rev), 0, "GL_ARB_vertex_type_2_10_10_10_rev" },
};

struct test_set *test_set = &test_sets[0];

enum piglit_result piglit_display(void)
{
	GLboolean pass = GL_TRUE;
	int i, mode;
	int x = 0, y = 0;

	glClear(GL_COLOR_BUFFER_BIT);
	printf("Testing %s\n", test_set->name);

	for (mode = 0; mode < NUM_MODES; mode++) {
		printf("\n");
		for (i = 0; i < test_set->num_tests; i++) {
			pass = test_set->tests[i](x, y, mode) && pass;
			x += 40;
			if (x+40 > piglit_width) {
				x = 0;
				y += 20;
			}
		}
	}

	assert(glGetError() == 0);
	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void piglit_init(int argc, char **argv)
{
	int a,i;

	piglit_require_gl_version(20);
	piglit_require_extension("GL_ARB_explicit_attrib_location");
	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);

	snorm_equation_23 = piglit_get_gl_version() >= 42;

	glClearColor(0.2, 0.2, 0.2, 1.0);

	for (a = 1; a < argc; a++) {
		for (i = 0; i < ARRAY_SIZE(test_sets); i++) {
			if (strcmp(argv[a], test_sets[i].name) == 0) {
				if (test_sets[i].gl_version)
					piglit_require_gl_version(test_sets[i].gl_version);
				if (test_sets[i].extension)
					piglit_require_extension(test_sets[i].extension);

				test_set = &test_sets[i];
				return;
			}
		}
	}
}
