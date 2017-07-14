/*
 * Copyright © 2009, 2013 VMware, Inc.
 * Copyright © 2015 Intel Corporation
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

/*
 * Test that the correct provoking vertex is used when a tri/quad/polygon
 * is clipped for glShadeModel(GL_FLAT).
 *
 * This is based on the glean clipFlat test.
 *
 * Test with glDrawArrays and glBegin/End.  Test GL_CCW and GL_CW winding.
 * Back-face polygon culling is enabled so if the winding order of any
 * primitive is incorrect, nothing may be drawn.
 *
 * XXX We should also test with two-sided lighting.
 *
 * If GL_ARB/EXT_provoking_vertex is supported, that feature is tested as well.
 *
 * Author: Brian Paul
 */


#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN
	config.supports_gl_compat_version = 10;
	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;
	config.khr_no_error_support = PIGLIT_NO_ERRORS;
PIGLIT_GL_TEST_CONFIG_END


/* Note: all correctly rendered tris/quad/polygons will be green.
 * Any other color indicates that the wrong vertex color was used.
 */

// GL_TRIANGLES: provoking vertex = last of tri
static const GLfloat TriVerts[6][5] =
	{
		// R  G  B     X   Y
		{ 1, 0, 0,   -1, -1 },
		{ 0, 0, 1,    1, -1 },
		{ 0, 1, 0,    1,  1 }, // PV

		{ 0, 0, 1,    1,  1 },
		{ 1, 0, 0,   -1,  1 },
		{ 0, 1, 0,   -1, -1 } // PV
	};

// GL_TRIANGLES: first provoking vertex
static const GLfloat TriVertsFirstPV[6][5] =
	{
		{ 0, 1, 0,    1,  1 }, // PV
		{ 1, 0, 0,   -1, -1 },
		{ 0, 0, 1,    1, -1 },

		{ 0, 1, 0,   -1, -1 }, // PV
		{ 0, 0, 1,    1,  1 },
		{ 1, 0, 0,   -1,  1 }
	};


// GL_TRIANGLE_STRIP: provoking vertex = last of tri
static const GLfloat TriStripVerts[6][5] =
	{
		{ 1, 0, 0,   -1, -1 },
		{ 0, 0, 1,    1, -1 },
		{ 0, 1, 0,   -1,  0 }, // PV
		{ 0, 1, 0,    1,  0 }, // PV
		{ 0, 1, 0,   -1,  1 }, // PV
		{ 0, 1, 0,    1,  1 }  // PV
	};

// GL_TRIANGLE_STRIP: first provoking vertex
static const GLfloat TriStripVertsFirstPV[6][5] =
	{
		{ 0, 1, 0,   -1, -1 }, // PV
		{ 0, 1, 0,    1, -1 }, // PV
		{ 0, 1, 0,   -1,  0 }, // PV
		{ 0, 1, 0,    1,  0 }, // PV
		{ 1, 0, 0,   -1,  1 },
		{ 0, 0, 1,    1,  1 }
	};


// GL_TRIANGLE_FAN: provoking vertex = last of tri
static const GLfloat TriFanVerts[4][5] =
	{
		{ 1, 0, 0,   -1, -1 },
		{ 0, 0, 1,    1, -1 },
		{ 0, 1, 0,    1,  1 }, // PV
		{ 0, 1, 0,   -1,  1 }  // PV
	};

// GL_TRIANGLE_FAN: first provoking vertex
static const GLfloat TriFanVertsFirstPV[4][5] =
	{
		{ 0, 0, 1,    1, -1 },
		{ 0, 1, 0,    1,  1 }, // PV
		{ 0, 1, 0,   -1,  1 }, // PV
		{ 1, 0, 0,   -1, -1 }
	};


// GL_QUADS: provoking vertex = last of quad
static const GLfloat QuadVerts[4][5] =
	{
		{ 1, 0, 0,   -1, -1 },
		{ 0, 0, 1,    1, -1 },
		{ 1, 1, 0,    1,  1 },
		{ 0, 1, 0,   -1,  1 }  // PV
	};

// GL_QUADS: first provoking vertex
static const GLfloat QuadVertsFirstPV[4][5] =
	{
		{ 0, 1, 0,   -1,  1 }, // PV
		{ 1, 0, 0,   -1, -1 },
		{ 0, 0, 1,    1, -1 },
		{ 1, 1, 0,    1,  1 }
	};


// GL_QUAD_STRIP: provoking vertex = last of quad
static const GLfloat QuadStripVerts[6][5] =
	{
		{ 1, 0, 0,   -1, -1 },
		{ 0, 0, 1,    1, -1 },
		{ 1, 1, 0,   -1,  0 },
		{ 0, 1, 0,    1,  0 }, // PV
		{ 1, 1, 0,   -1,  1 },
		{ 0, 1, 0,    1,  1 }  // PV
	};

// GL_QUAD_STRIP: first provoking vertex
static const GLfloat QuadStripVertsFirstPV[6][5] =
	{
		{ 0, 1, 0,   -1, -1 }, // PV
		{ 1, 1, 0,    1, -1 },
		{ 0, 1, 0,   -1,  0 }, // PV
		{ 1, 0, 0,    1,  0 },
		{ 0, 0, 1,   -1,  1 },
		{ 1, 0, 0,    1,  1 }
	};


// GL_POLYGON: provoking vertex = first vertex
static const GLfloat PolygonVerts[4][5] =
	{
		{ 0, 1, 0,   -1, -1 }, // PV
		{ 1, 0, 0,    1, -1 },
		{ 0, 0, 1,    1,  1 },
		{ 1, 1, 0,   -1,  1 }
	};


enum draw_mode {
	BEGIN_END,
	DRAW_ARRAYS,
	DRAW_ELEMENTS,
	NUM_DRAW_MODES
};


static bool provoking_vertex_first;
static bool quads_follows_pv_convention;
static bool testing_first_pv;


void
piglit_init(int argc, char **argv)
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-1.25, 1.25, -1.25, 1.25, -1, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glShadeModel(GL_FLAT);
	glPixelStorei(GL_PACK_ALIGNMENT, 1);

	glFrontFace(GL_CW);
	glCullFace(GL_FRONT);
	glEnable(GL_CULL_FACE);

	if (piglit_is_extension_supported("GL_ARB_provoking_vertex")) {
		provoking_vertex_first = true;
	}
	else if (piglit_is_extension_supported("GL_EXT_provoking_vertex")) {
		provoking_vertex_first = true;
	}

	printf("Have GL_ARB/EXT_provoking_vertex: %s\n",
	       provoking_vertex_first ? "yes" : "no");

	if (provoking_vertex_first) {
		GLboolean k;
		glGetBooleanv(GL_QUADS_FOLLOW_PROVOKING_VERTEX_CONVENTION_EXT, &k);
		quads_follows_pv_convention = k;

		printf("Quads follow provoking vertex convention: %s\n",
		       k ? "yes" : "no");
	}
}


// Draw with glDrawArrays()
static void
drawArrays(GLenum mode, const GLfloat *verts, GLuint count)
{
	glColorPointer(3, GL_FLOAT, 5 * sizeof(GLfloat), verts + 0);
	glVertexPointer(2, GL_FLOAT, 5 * sizeof(GLfloat), verts + 3);
	glEnableClientState(GL_COLOR_ARRAY);
	glEnableClientState(GL_VERTEX_ARRAY);

	glDrawArrays(mode, 0, count);

	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
}


// Draw with glDrawElements()
static void
drawElements(GLenum mode, const GLfloat *verts, GLuint count)
{
	static const GLuint elements[6] = { 0, 1, 2, 3, 4, 5 };
	glColorPointer(3, GL_FLOAT, 5 * sizeof(GLfloat), verts + 0);
	glVertexPointer(2, GL_FLOAT, 5 * sizeof(GLfloat), verts + 3);
	glEnableClientState(GL_COLOR_ARRAY);
	glEnableClientState(GL_VERTEX_ARRAY);

	assert(count <= ARRAY_SIZE(elements));

	glDrawElements(mode, count, GL_UNSIGNED_INT, elements);

	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_VERTEX_ARRAY);
}


// Draw with glBegin/End()
static void
drawBeginEnd(GLenum mode, const GLfloat *verts, GLuint count)
{
	GLuint i;

	glBegin(mode);
	for (i = 0; i < count; i++) {
		glColor3fv(verts + i * 5);
		glVertex2fv(verts + i * 5 + 3);
	}
	glEnd();
}


// Read pixels and check pixels.  All pixels should be green or black.
// Any other color indicates a failure.
static bool
checkResult(GLfloat badColor[3])
{
	GLubyte *image;
	GLuint i, j;
	bool anyGreen = false;

	image = malloc(piglit_width * piglit_height * 3);

	badColor[0] = badColor[1] = badColor[2] = 0.0f;

	glReadPixels(0, 0, piglit_width, piglit_height,
		     GL_RGB, GL_UNSIGNED_BYTE, image);

	if (!piglit_automatic)
		piglit_present_results();

	for (i = 0; i < piglit_height; i++) {
		for (j = 0; j < piglit_width; j++) {
			GLuint k = (i * piglit_width + j) * 3;

			if (image[k + 0] == 0 &&
			    image[k + 1] == 0 &&
			    image[k + 2] == 0) {
				// black - OK
			}
			else if (image[k + 0] == 0 &&
				 image[k + 1] >= 254 &&
				 image[k + 2] == 0) {
				// green - OK
				anyGreen = true;
			}
			else {
				// any other color = failure
				badColor[0] = image[k + 0] / 255.0;
				badColor[1] = image[k + 1] / 255.0;
				badColor[2] = image[k + 2] / 255.0;
				free(image);
				return false;
			}
		}
	}

	free(image);

	return anyGreen;
}


char *
calcQuadrant(GLfloat x, GLfloat y)
{
	const char *strx, *stry;
	char *ret;
	int _x, _y;
	_x = (int) x;
	_y = (int) y;

	switch (_x) {
	case -1:
		strx = "left";
		break;
	case 0:
		strx = "center";
		break;
	case 1:
		strx = "right";
		break;
	default:
		assert(0);
	}

	switch (_y) {
	case 1:
		stry = "top";
		break;
	case 0:
		stry = "middle";
		break;
	case -1:
		stry = "bottom";
		break;
	default:
		assert(0);
	}

	(void)!asprintf(&ret, "%s %s", strx, stry);
	return ret;
}


static void
reportSubtest(GLenum mode, int drawMode, GLuint facing,
              GLuint fill,
              const GLfloat badColor[3], GLfloat x, GLfloat y,
              bool pass)
{
	const char *m, *d, *f, *p;
	char *q;

	switch (mode) {
	case GL_TRIANGLES:
		m = "GL_TRIANGLES";
		break;
	case GL_TRIANGLE_STRIP:
		m = "GL_TRIANGLE_STRIP";
		break;
	case GL_TRIANGLE_FAN:
		m = "GL_TRIANGLE_FAN";
		break;
	case GL_QUADS:
		m = "GL_QUADS";
		break;
	case GL_QUAD_STRIP:
		m = "GL_QUAD_STRIP";
		break;
	case GL_POLYGON:
		m = "GL_POLYGON";
		break;
	default:
		m = "???";
	}

	switch (drawMode) {
	case BEGIN_END:
		d = "glBegin/End";
		break;
	case DRAW_ARRAYS:
		d = "glDrawArrays";
		break;
	case DRAW_ELEMENTS:
		d = "glDrawElements";
		break;
	default:
		assert(0);
		d = "???";
	}

	if (facing == 0)
		f = "GL_CCW";
	else
		f = "GL_CW";

	if (fill == 0)
		p = "GL_FILL";
	else
		p = "GL_LINE";

	q = calcQuadrant(x, y);

	if (!pass) {
		printf("clipflat: Failure for %s(%s), glFrontFace(%s), "
		       "glPolygonMode(%s), quadrant: %s\n",
		       d, m, f, p, q);

		if (testing_first_pv) {
			printf("\tGL_EXT_provoking_vertex test: "
			       "GL_FIRST_VERTEX_CONVENTION_EXT mode\n");
		}

		printf("Expected color (0, 1, 0) but found (%g, %g, %g)\n",
		       badColor[0], badColor[1], badColor[2]);
		printf("\n");
	}

	piglit_report_subtest_result(pass ? PIGLIT_PASS : PIGLIT_FAIL,
				     "%s(%s), glFrontFace(%s), glPolygonMode(%s), "
				     "quadrant: %s",
				     d, m, f, p, q);

	free(q);
}


// Test a particular primitive mode for one drawing mode, filled/unfilled
// state and CW/CCW winding.
static bool
testPrimCombo(GLenum mode, const GLfloat *verts, GLuint count,
			  bool fill, enum draw_mode drawMode, GLuint facing)
{
	GLfloat x, y;
	bool pass = true;

	glPolygonMode(GL_FRONT_AND_BACK, fill ? GL_LINE : GL_FILL);

	if (facing == 0) {
		glFrontFace(GL_CCW);
		glCullFace(GL_BACK);
	}
	else {
		glFrontFace(GL_CW);
		glCullFace(GL_FRONT);
	}

	// Position the geometry at 9 different locations to test
	// clipping against the left, right, bottom and top edges of
	// the window.
	// Only the center location will be unclipped.
	for (y = -1.0; y <= 1.0; y += 1.0) {
		for (x = -1.0; x <= 1.0; x += 1.0) {
			bool quad_pass;
			GLfloat badColor[3];

			glPushMatrix();
			glTranslatef(x, y, 0.0);

			glClear(GL_COLOR_BUFFER_BIT);

			switch (drawMode) {
			case BEGIN_END:
				drawBeginEnd(mode, verts, count);
				break;
			case DRAW_ARRAYS:
				drawArrays(mode, verts, count);
				break;
			case DRAW_ELEMENTS:
				drawElements(mode, verts, count);
				break;
			default:
				assert(0);
			}

			glPopMatrix();

			quad_pass = checkResult(badColor);
			pass = pass && quad_pass;
			reportSubtest(mode, drawMode, facing, fill,
						  badColor, x, y, quad_pass);
		}
	}

	return pass;
}


// Test a particular primitive mode for all drawing modes, filled/unfilled
// and CW/CCW winding.
static bool
testPrim(GLenum mode, const GLfloat *verts, GLuint count)
{
	GLuint facing, fill;
	int drawMode;
	bool pass = true;

	// Loop over polygon mode: filled vs. outline
	for (fill = 0; fill < 2; fill++) {

		// Loop over drawing mode: glBegin/End vs glDrawArrays vs glDrawElements
		for (drawMode = 0; drawMode < NUM_DRAW_MODES; drawMode++) {

			// Loop over CW vs. CCW winding (should make no difference)
			for (facing = 0; facing < 2; facing++) {
				pass = testPrimCombo(mode, verts, count,
									 fill, drawMode, facing) && pass;
			}
		}
	}
	return pass;
}


enum piglit_result
piglit_display(void)
{
	bool pass = true;

	testing_first_pv = false;

	pass = testPrim(GL_TRIANGLES,
			(GLfloat *) TriVerts,
			ARRAY_SIZE(TriVerts)) && pass;

	pass = testPrim(GL_TRIANGLE_STRIP,
			(GLfloat *) TriStripVerts,
			ARRAY_SIZE(TriStripVerts)) && pass;

	pass = testPrim(GL_TRIANGLE_FAN,
			(GLfloat *) TriFanVerts,
			ARRAY_SIZE(TriFanVerts)) && pass;

	pass = testPrim(GL_QUADS,
			(GLfloat *) QuadVerts,
			ARRAY_SIZE(QuadVerts)) && pass;

	pass = testPrim(GL_QUAD_STRIP,
			(GLfloat *) QuadStripVerts,
			ARRAY_SIZE(QuadStripVerts)) && pass;

	pass = testPrim(GL_POLYGON,
			(GLfloat *) PolygonVerts,
			ARRAY_SIZE(PolygonVerts)) && pass;

	if (provoking_vertex_first) {
		glProvokingVertex(GL_FIRST_VERTEX_CONVENTION_EXT);
		testing_first_pv = true;

		pass = testPrim(GL_TRIANGLES,
				(GLfloat *) TriVertsFirstPV,
				ARRAY_SIZE(TriVertsFirstPV)) && pass;

		pass = testPrim(GL_TRIANGLE_STRIP,
				(GLfloat *) TriStripVertsFirstPV,
				ARRAY_SIZE(TriStripVertsFirstPV)) && pass;

		pass = testPrim(GL_TRIANGLE_FAN,
				(GLfloat *) TriFanVertsFirstPV,
				ARRAY_SIZE(TriFanVertsFirstPV)) && pass;

		if (quads_follows_pv_convention)
			pass = testPrim(GL_QUADS,
					(GLfloat *) QuadVertsFirstPV,
					ARRAY_SIZE(QuadVertsFirstPV)) && pass;
		else
			pass = testPrim(GL_QUADS,
					(GLfloat *) QuadVerts,
					ARRAY_SIZE(QuadVerts)) && pass;

		if (quads_follows_pv_convention)
			pass = testPrim(GL_QUAD_STRIP,
					(GLfloat *) QuadStripVertsFirstPV,
					ARRAY_SIZE(QuadStripVertsFirstPV)) && pass;
		else
			pass = testPrim(GL_QUAD_STRIP,
					(GLfloat *) QuadStripVerts,
					ARRAY_SIZE(QuadStripVerts)) && pass;

		pass  = testPrim(GL_POLYGON,
				 (GLfloat *) PolygonVerts,
				 ARRAY_SIZE(PolygonVerts)) && pass;
	}

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}
