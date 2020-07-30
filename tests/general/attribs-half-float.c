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
 * Test for immediate-mode style commands like glNormal, glColor, etc.
 * with vertex arrays, immediate mode and display lists for GL_NV_half_float
 * extension.
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
	config.khr_no_error_support = PIGLIT_NO_ERRORS;

PIGLIT_GL_TEST_CONFIG_END

#define ftoh(f) piglit_half_from_float(f)

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

DEFINE_TEST(glNormal3hNV,, (ftoh(x),ftoh(y),ftoh(z)),   "vec4(gl_Normal, 1.0)", RGB, FLOAT_TYPE, "")
DEFINE_TEST(glColor3hNV,,  (ftoh(x), ftoh(y), ftoh(z)), "gl_Color", RGB, FLOAT_TYPE, "")
DEFINE_TEST(glColor4hNV,,  (ftoh(x), ftoh(y), ftoh(z), ftoh(w)), "gl_Color", RGBA, FLOAT_TYPE, "")
DEFINE_TEST(glTexCoord1hNV,, (ftoh(x)),                 "gl_MultiTexCoord0", R, FLOAT_TYPE, "")
DEFINE_TEST(glTexCoord2hNV,, (ftoh(x),ftoh(y)),         "gl_MultiTexCoord0", RG, FLOAT_TYPE, "")
DEFINE_TEST(glTexCoord3hNV,, (ftoh(x),ftoh(y),ftoh(z)), "gl_MultiTexCoord0", RGB, FLOAT_TYPE, "")
DEFINE_TEST(glTexCoord4hNV,, (ftoh(x),ftoh(y),ftoh(z),ftoh(w)), "gl_MultiTexCoord0", RGBA, FLOAT_TYPE, "")
DEFINE_TEST(glMultiTexCoord1hNV,, (GL_TEXTURE1, ftoh(x)), "gl_MultiTexCoord1", R, FLOAT_TYPE, "")
DEFINE_TEST(glMultiTexCoord2hNV,, (GL_TEXTURE1, ftoh(x),ftoh(y)), "gl_MultiTexCoord1", RG, FLOAT_TYPE, "")
DEFINE_TEST(glMultiTexCoord3hNV,, (GL_TEXTURE1, ftoh(x),ftoh(y),ftoh(z)), "gl_MultiTexCoord1", RGB, FLOAT_TYPE, "")
DEFINE_TEST(glMultiTexCoord4hNV,, (GL_TEXTURE1, ftoh(x),ftoh(y),ftoh(z),ftoh(w)), "gl_MultiTexCoord1", RGBA, FLOAT_TYPE, "")
DEFINE_TEST(glSecondaryColor3hNV,, (ftoh(x),ftoh(y),ftoh(z)), "gl_SecondaryColor", RGB, FLOAT_TYPE, "")

static test_func tests[] = {
	test_glNormal3hNV,
	test_glColor3hNV,
	test_glColor4hNV,
	test_glTexCoord1hNV,
	test_glTexCoord2hNV,
	test_glTexCoord3hNV,
	test_glTexCoord4hNV,
	test_glMultiTexCoord1hNV,
	test_glMultiTexCoord2hNV,
	test_glMultiTexCoord3hNV,
	test_glMultiTexCoord4hNV,
	test_glSecondaryColor3hNV,
};

enum piglit_result piglit_display(void)
{
	GLboolean pass = GL_TRUE;
	int i, mode;
	int x = 0, y = 0;

	glClear(GL_COLOR_BUFFER_BIT);

	for (mode = 0; mode < NUM_MODES; mode++) {
		printf("\n");
		for (i = 0; i < ARRAY_SIZE(tests); i++) {
			pass = tests[i](x, y, mode) && pass;
			x += 40;
			if (x+40 > piglit_width) {
				x = 0;
				y += 20;
			}
		}
	}

	if (!piglit_check_gl_error(GL_NO_ERROR))
		piglit_report_result(PIGLIT_FAIL);
	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

void piglit_init(int argc, char **argv)
{
	piglit_require_gl_version(20);
	piglit_require_extension("GL_ARB_explicit_attrib_location");
	piglit_require_extension("GL_NV_half_float");
	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);

	snorm_equation_23 = piglit_get_gl_version() >= 42;

	glClearColor(0.2, 0.2, 0.2, 1.0);
}
