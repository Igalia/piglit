/*
 * Copyright 2016 VMware, Inc.
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
 * Test rendering of GS adjacency primitives, with:
 * - First and last provoking vertex
 * - Front and back-face culling
 * - glPolygonMode(GL_FRONT_AND_BACK, GL_LINE)
 * See code for command line arguments.
 *
 * Brian Paul
 * May 2016
 */


#include "piglit-util-gl.h"
#include "piglit-matrix.h"


PIGLIT_GL_TEST_CONFIG_BEGIN
	config.window_width = 800;
	config.window_height = 200;
	config.supports_gl_core_version = 32;
	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;
	config.khr_no_error_support = PIGLIT_NO_ERRORS;
PIGLIT_GL_TEST_CONFIG_END


#define VERTEX_SIZE (2 * sizeof(GLfloat))

static const float gray[4] = { 0.5, 0.5, 0.5, 1.0 };
static const float black[4] = { 0.0, 0.0, 0.0, 0.0 };

static const float colors[18][4] = {
	{1.0, 0.2, 0.2, 1.0},
	{0.2, 1.0, 0.2, 1.0},
	{0.2, 0.2, 1.0, 1.0},
	{1.0, 1.0, 1.0, 1.0},
	{0.2, 1.0, 1.0, 1.0},
	{1.0, 0.2, 1.0, 1.0},
	{1.0, 1.0, 0.2, 1.0},
	{0.5, 1.0, 1.0, 1.0},
	{1.0, 0.5, 1.0, 1.0},
	{1.0, 1.0, 0.5, 1.0},
	{0.7, 1.0, 1.0, 1.0},
	{1.0, 0.7, 1.0, 1.0},
	{1.0, 1.0, 0.7, 1.0},
	{1.0, 0.2, 0.2, 1.0},
	{0.2, 1.0, 0.2, 1.0},
	{0.2, 0.2, 1.0, 1.0},
	{1.0, 1.0, 1.0, 1.0},
	{0.5, 0.5, 0.5, 1.0}
};

static const float lines_adj_verts[8][2] = {
	// first line
	{-1, -.75},
	{-0.5, -0.25},
	{ 0.5, -0.25},
	{ 1.0, -.75},
	// second line
	{-1, 0.0},
	{-0.5, 0.5},
	{ 0.5, 0.5},
	{ 1.0, 0.0}
};

static const float line_strip_adj_verts[7][2] = {
	{-1.5, .3},
	{-1, -.3},
	{-0.5, .3},
	{ 0.0, -.3},
	{ 0.5, .3},
	{ 1.0, -.3},
	{ 1.5, .3},
};

static const float triangles_adj_verts[6][2] = {
	{0, -.5},
	{-1.2, 0},
	{-.75, 1},
	{0, 1.5},
	{0.75, 1},
	{1.2, 0},
};

static const float triangle_strip_adj_verts[][2] = {
	{-1.5, -0.5},  // 0
	{-1.9, 0.0},   // 1
	{-1.5, 0.5},   // 2
	{-1, -1},      // 3 *
	{-1, -.5},     // 4
	{-1.5, 1},     // 5 *
	{-1, 0.5},     // 6
	{-.5, -1},     // 7 *
	{-.5, -.5},    // 8
	{-1, 1},       // 9 *
	{-.5, .5},     // 10
	{0, -1},       // 11 *
	{0, -.5},      // 12
	{-.5, 1},      // 13 *
	{0, 0.5},      // 14
	{0.5, -1},     // 15 *
	{0.5, -.5},    // 16
	{1, 0},        // 17
};

#define NUM_VERTS(ARRAY)  (sizeof(ARRAY) / VERTEX_SIZE)

static GLfloat ortho_matrix[16];

static GLuint lines_adj_vao;
static GLuint line_strip_adj_vao;
static GLuint triangles_adj_vao;
static GLuint triangle_strip_adj_vao;

static GLenum polygon_mode = GL_FILL;
static GLenum cull_mode = GL_NONE;
static GLenum provoking_vertex = GL_LAST_VERTEX_CONVENTION;

static GLuint gs_lines_program;
static GLuint gs_line_strip_program;
static GLuint gs_triangles_program;
static GLuint gs_triangle_strip_program;
static GLuint ref_program;
static GLint colorUniform, modelViewProjUniform;

// if false, draw without GS, also draw the 'extra' lines/tris.  For debugging.
static bool draw_with_gs = true;


/**
 * Given a primitive type (adjacency type only), and the first/last provoking
 * vertex mode, and a primitive (line, triangle) index, return the index of
 * the vertex which will specify the primitive's flat-shaded color.
 */
static unsigned
provoking_vertex_index(GLenum prim_mode, GLenum pv_mode, unsigned prim_index)
{
	switch (prim_mode) {
	case GL_LINES_ADJACENCY:
		if (pv_mode == GL_FIRST_VERTEX_CONVENTION)
			return prim_index * 4 + 1;
		else
			return prim_index * 4 + 2;
	case GL_LINE_STRIP_ADJACENCY:
		if (pv_mode == GL_FIRST_VERTEX_CONVENTION)
			return prim_index + 1;
		else
			return prim_index + 2;
	case GL_TRIANGLES_ADJACENCY:
		if (pv_mode == GL_FIRST_VERTEX_CONVENTION)
			return prim_index * 6 + 0;
		else
			return prim_index * 6 + 4;
	case GL_TRIANGLE_STRIP_ADJACENCY:
		if (pv_mode == GL_FIRST_VERTEX_CONVENTION)
			return prim_index * 2;
		else
			return prim_index * 2 + 4;
	default:
		assert(!"Unexpected prim_mode");
		return 0;
	}
}


/**
 * Given a primitive type and a primitive (line/triangle) index, return
 * the (x,y) screen coordinate for probing.
 */
static void
compute_probe_location(GLenum prim_mode, unsigned prim_index,
		       const float verts[][2],
		       int vp_x, int vp_y, int *x, int *y)
{
	int i0, i1, i2 = -1;
	float coord[4], ndc[4], win[3];

	switch (prim_mode) {
	case GL_LINES_ADJACENCY:
		i0 = prim_index * 4 + 1;
		i1 = prim_index * 4 + 2;
		break;
	case GL_LINE_STRIP_ADJACENCY:
		i0 = prim_index + 1;
		i1 = prim_index + 2;
		break;
	case GL_TRIANGLES_ADJACENCY:
		i0 = prim_index * 6 + 0;
		i1 = prim_index * 6 + 2;
		if (polygon_mode != GL_LINE)
			i2 = prim_index * 6 + 4;
		break;
	case GL_TRIANGLE_STRIP_ADJACENCY:
		i0 = prim_index * 2;
		i1 = prim_index * 2 + 2;
		if (polygon_mode != GL_LINE)
			i2 = prim_index * 2 + 4;
		break;
	default:
		assert(!"Unexpected prim_mode");
		*x = *y = 0;
		return;
	}

	/* average of 2 or 3 points */
	if (i2 == -1) {
		coord[0] = (verts[i0][0] + verts[i1][0]) / 2.0;
		coord[1] = (verts[i0][1] + verts[i1][1]) / 2.0;
	} else {
		coord[0] = (verts[i0][0] + verts[i1][0] + verts[i2][0]) / 3.0;
		coord[1] = (verts[i0][1] + verts[i1][1] + verts[i2][1]) / 3.0;
	}
	coord[2] = 0.0;
	coord[3] = 1.0;

	piglit_matrix_mul_vector(ndc, ortho_matrix, coord);
	piglit_ndc_to_window(win, ndc, vp_x, vp_y,
			     piglit_width/4, piglit_height);

	*x = (int) win[0];
	*y = (int) win[1];
}


/**
 * Do the colors match, within an epsilon?
 */
static bool
colors_match(const float c1[4], const float c2[4])
{
	const float epsilon = 1.0 / 256.0;

	if (fabs(c1[0] - c2[0]) > epsilon ||
	    fabs(c1[1] - c2[1]) > epsilon ||
	    fabs(c1[2] - c2[2]) > epsilon ||
	    fabs(c1[3] - c2[3]) > epsilon)
		return false;
	else
		return true;
}


/**
 * Given a primitive type and a number of vertices, return the number of
 * primitives (lines/tris) that'll be drawn.
 */
static unsigned
num_gs_prims(GLenum prim_mode, unsigned num_verts)
{
	switch (prim_mode) {
	case GL_LINES_ADJACENCY:
		assert(num_verts % 4 == 0);
		return num_verts / 4;
	case GL_LINE_STRIP_ADJACENCY:
		assert(num_verts >= 4);
		return num_verts - 3;
	case GL_TRIANGLES_ADJACENCY:
		assert(num_verts % 6 == 0);
		return num_verts / 6;
	case GL_TRIANGLE_STRIP_ADJACENCY:
		assert(num_verts >= 6);
		return (num_verts - 4) / 2;
	default:
		assert(!"Unexpected prim_mode");
		return 0;
	}
}


/**
 * Check if a primitive strip was rendered correctly by doing color probing.
 * vp_pos is the viewport position (0..3).
 */
static bool
probe_prims(GLenum prim_mode, const float verts[][2], unsigned num_verts,
	    unsigned vp_pos)
{
	const int vp_w = piglit_width / 4;
	const unsigned num_prims = num_gs_prims(prim_mode, num_verts);
	unsigned prim;

	for (prim = 0; prim < num_prims; prim++) {
		bool pass = false;
		const float *expected_color = NULL;
		float bad_color[4];
		bool bad_color_found = false;
		int x, y, i;

		compute_probe_location(prim_mode, prim, verts,
				       vp_pos * vp_w, 0, &x, &y);

		if (cull_mode == GL_FRONT &&
		    (prim_mode == GL_TRIANGLES_ADJACENCY ||
		     prim_mode == GL_TRIANGLE_STRIP_ADJACENCY)) {
			// All triangles should be front facing.
			// With front culling, all should be discarded.
			// Region should be black.
			if (piglit_probe_rect_rgba(x-1, y-1, 3, 3, black)) {
				pass = true;
			}
		} else {
			GLfloat buf[9][4];
			unsigned pvi = provoking_vertex_index(prim_mode,
						     provoking_vertex, prim);
			expected_color = colors[pvi];

			// Read a 3x3 region for line probing
			glReadPixels(x-1, y-1, 3, 3, GL_RGBA, GL_FLOAT, buf);

			// look for non-black pixel
			for (i = 0; i < 9; i++) {
				if (buf[i][0] != 0 || buf[i][1] != 0 ||
				    buf[i][2] != 0 || buf[i][3] != 0) {
					// check for expected color
					if (colors_match(expected_color, buf[i]))
						pass = true;
					else {
						bad_color_found = true;
						bad_color[0] = buf[i][0];
						bad_color[1] = buf[i][1];
						bad_color[2] = buf[i][2];
						bad_color[3] = buf[i][3];
					}
				}
			}
		}

		if (!pass) {
			printf("Failure for %s, "
			       "prim %u wrong color at (%d,%d)\n",
			       piglit_get_prim_name(prim_mode), prim, x, y);
			if (expected_color && bad_color_found) {
				printf("Expected %.1g, %.1g, %.1g, %.1g\n",
				       expected_color[0],
				       expected_color[1],
				       expected_color[2],
				       expected_color[3]);
				printf("Found %.1g, %.1g, %.1g, %.1g\n",
				       bad_color[0],
				       bad_color[1],
				       bad_color[2],
				       bad_color[3]);
			}

			return false;
		}
	}

	return true;
}



static GLuint
make_gs_program(GLenum input_prim)
{
	static const char *vs_text =
		"#version 150 \n"
		"in vec4 vertex; \n"
		"in vec4 color; \n"
		"uniform mat4 modelViewProj; \n"
		"out vec4 pos;\n"
		"out vec4 vs_gs_color; \n"
		"void main() \n"
		"{ \n"
		"   gl_Position = vertex * modelViewProj; \n"
		"   pos = vertex * modelViewProj; \n"
		"   vs_gs_color = color; \n"
		"} \n";
	static const char *gs_text_lines =
		"#version 150 \n"
		"layout(lines_adjacency) in;\n"
		"layout(line_strip, max_vertices = 2) out;\n"
		"in vec4 pos[]; \n"
		"in vec4 vs_gs_color[4]; \n"
		"flat out vec4 gs_fs_color; \n"
		"void main() \n"
		"{ \n"
		"   gs_fs_color = vs_gs_color[1]; \n"
		"   gl_Position = pos[1]; \n"
		"   EmitVertex(); \n"
		"   gs_fs_color = vs_gs_color[2]; \n"
		"   gl_Position = pos[2]; \n"
		"   EmitVertex(); \n"
		"   EndPrimitive(); \n"
		"} \n";
	static const char *gs_text_triangles =
		"#version 150 \n"
		"layout(triangles_adjacency) in;\n"
		"layout(triangle_strip, max_vertices = 3) out;\n"
		"in vec4 pos[]; \n"
		"in vec4 vs_gs_color[6]; \n"
		"flat out vec4 gs_fs_color; \n"
		"void main() \n"
		"{ \n"
		"   gs_fs_color = vs_gs_color[0]; \n"
		"   gl_Position = pos[0]; \n"
		"   EmitVertex(); \n"
		"   gs_fs_color = vs_gs_color[2]; \n"
		"   gl_Position = pos[2]; \n"
		"   EmitVertex(); \n"
		"   gs_fs_color = vs_gs_color[4]; \n"
		"   gl_Position = pos[4]; \n"
		"   EmitVertex(); \n"
		"   //EndPrimitive(); \n"
		"} \n";
	static const char *fs_text =
		"#version 150 \n"
		"flat in vec4 gs_fs_color; \n"
		"void main() \n"
		"{ \n"
		"   gl_FragColor = gs_fs_color; \n"
		"} \n";
	const char *gs_text;
	GLuint program;

	switch (input_prim) {
	case GL_LINES_ADJACENCY:
	case GL_LINE_STRIP_ADJACENCY:
		gs_text = gs_text_lines;
		break;
	case GL_TRIANGLES_ADJACENCY:
	case GL_TRIANGLE_STRIP_ADJACENCY:
		gs_text = gs_text_triangles;
		break;
	default:
		assert(!"Unexpected input_prim");
		return 0;
	}

	program = piglit_build_simple_program_unlinked_multiple_shaders(
		GL_VERTEX_SHADER, vs_text,
		GL_GEOMETRY_SHADER, gs_text,
		GL_FRAGMENT_SHADER, fs_text,
		0);

	assert(program);

	glBindAttribLocation(program, 0, "vertex");
	glBindAttribLocation(program, 1, "color");

	glLinkProgram(program);

	return program;
}


static GLuint
make_ref_program(void)
{
	static const char *vs_text =
		"#version 150 \n"
		"in vec4 vertex; \n"
		"uniform vec4 color; \n"
		"uniform mat4 modelViewProj; \n"
		"out vec4 vs_fs_color; \n"
		"void main() \n"
		"{ \n"
		"   gl_Position = vertex * modelViewProj; \n"
		"   vs_fs_color = color; \n"
		"} \n";

	static const char *fs_text =
		"#version 150 \n"
		"in vec4 vs_fs_color; \n"
		"void main() \n"
		"{ \n"
		"   gl_FragColor = vs_fs_color; \n"
		"} \n";

	GLuint program = piglit_build_simple_program_unlinked_multiple_shaders(
		GL_VERTEX_SHADER, vs_text,
		GL_FRAGMENT_SHADER, fs_text,
		0);

	glBindAttribLocation(program, 0, "vertex");
	glBindAttribLocation(program, 1, "color");

	glLinkProgram(program);

	return program;
}


static void
draw_elements3(GLenum mode, unsigned v0, unsigned v1, unsigned v2)
{
	GLushort elements[3];
	elements[0] = v0;
	elements[1] = v1;
	elements[2] = v2;
	glDrawElements(mode, 3, GL_UNSIGNED_SHORT, elements);
}


static void
set_color(const GLfloat c[4])
{
	glUniform4fv(colorUniform, 1, c);
}


static void
draw_lines_adj(GLuint vao, unsigned n)
{
	assert(n % 4 == 0);

	glBindVertexArray(vao);
	{
		unsigned i;
		for (i = 0; i < n; i += 4) {
			unsigned pvi =
				provoking_vertex_index(GL_LINES_ADJACENCY,
						       provoking_vertex, i/4);
			set_color(gray);

			// draw preceeding "wing" line
			glDrawArrays(GL_LINES, i, 2);
			// draw trailing "wing" line
			glDrawArrays(GL_LINES, i+2, 2);

			set_color(colors[pvi]);
			// draw "real" line
			glDrawArrays(GL_LINES, i+1, 2);
		}
	}
}


static void
draw_line_strip_adj(GLuint vao, unsigned n)
{
	unsigned i;

	assert(n >= 4);

	glBindVertexArray(vao);

	set_color(gray);
	glDrawArrays(GL_LINES, 0, 2);
	glDrawArrays(GL_LINES, n-2, 2);

	for (i = 1; i < n-2; i++) {
		unsigned pvi =
			provoking_vertex_index(GL_LINE_STRIP_ADJACENCY,
					       provoking_vertex, i-1);
		set_color(colors[pvi]);
		glDrawArrays(GL_LINES, i, 2);
	}
}


static void
draw_triangles_adj(GLuint vao, unsigned n)
{
	unsigned i;

	assert(n % 6 == 0);

	glBindVertexArray(vao);

	for (i = 0; i < n; i += 6) {
		unsigned pvi =
			provoking_vertex_index(GL_TRIANGLES_ADJACENCY,
					       provoking_vertex, i/6);

		// draw gray outlines of "wing" triangles
		set_color(gray);
		draw_elements3(GL_LINE_LOOP, i, i+1, i+2);
		draw_elements3(GL_LINE_LOOP, i+2, i+3, i+4);
		draw_elements3(GL_LINE_LOOP, i, i+4, i+5);

		// draw "real" triangle
		set_color(colors[pvi]);
		draw_elements3(GL_TRIANGLES, i, i+2, i+4);
	}
}


static void
draw_triangle_strip_adj(GLuint vao, unsigned n)
{
	unsigned i;

	assert(n >= 6);

	glBindVertexArray(vao);

	// draw first "wing" triangle
	set_color(gray);
	glDrawArrays(GL_LINE_LOOP, 0, 3);

	for (i = 0; i < n-4; i += 2) {
		unsigned pvi =
			provoking_vertex_index(GL_TRIANGLE_STRIP_ADJACENCY,
					       provoking_vertex, i/2);

		if (i % 4 == 2) {
			// even tri
			set_color(gray);
			draw_elements3(GL_LINE_LOOP, i, i+3, i+4);
			set_color(colors[pvi]);
			draw_elements3(GL_TRIANGLES, i, i+4, i+2);
		}
		else {
			// odd tri
			set_color(gray);
			draw_elements3(GL_LINE_LOOP, i, i+4, i+3);
			set_color(colors[pvi]);
			draw_elements3(GL_TRIANGLES, i, i+2, i+4);
		}
	}

	// draw last "wing" triangle
	set_color(gray);
	draw_elements3(GL_LINE_LOOP, i, i+2, i+3);
}


static void
use_program(GLuint program)
{
	glUseProgram(program);
	modelViewProjUniform = glGetUniformLocation(program, "modelViewProj");
	colorUniform = glGetUniformLocation(program, "color");

	piglit_ortho_matrix(ortho_matrix, -2, 2, -2, 2, -1, 1);
	glUniformMatrix4fv(modelViewProjUniform, 1, GL_FALSE, ortho_matrix);
}


static void
set_viewport(unsigned pos)
{
	int vp_w = piglit_width / 4;
	assert(pos < 4);
	glViewport(pos * vp_w, 0, vp_w, piglit_height);
}


enum piglit_result
piglit_display(void)
{
	bool pass = true;

	glClear(GL_COLOR_BUFFER_BIT);

	if (draw_with_gs) {
		use_program(gs_lines_program);
		set_viewport(0);
		glBindVertexArray(lines_adj_vao);
		glDrawArrays(GL_LINES_ADJACENCY, 0,
			     NUM_VERTS(lines_adj_verts));

		use_program(gs_line_strip_program);
		set_viewport(1);
		glBindVertexArray(line_strip_adj_vao);
		glDrawArrays(GL_LINE_STRIP_ADJACENCY, 0,
			     NUM_VERTS(line_strip_adj_verts));

		use_program(gs_triangles_program);
		set_viewport(2);
		glBindVertexArray(triangles_adj_vao);
		glDrawArrays(GL_TRIANGLES_ADJACENCY, 0,
			     NUM_VERTS(triangles_adj_verts));

		use_program(gs_triangle_strip_program);
		set_viewport(3);
		glBindVertexArray(triangle_strip_adj_vao);
		glDrawArrays(GL_TRIANGLE_STRIP_ADJACENCY, 0,
			     NUM_VERTS(triangle_strip_adj_verts));
	}
	else {
		/* This path is basically for debugging and visualizing the
		 * "extra" lines and tris in adjacency primitives.
		 */
		use_program(ref_program);

		set_viewport(0);
		draw_lines_adj(lines_adj_vao, 8);

		set_viewport(1);
		draw_line_strip_adj(line_strip_adj_vao, 7);

		set_viewport(2);
		draw_triangles_adj(triangles_adj_vao, 6);

		set_viewport(3);
		draw_triangle_strip_adj(triangle_strip_adj_vao, 17);
	}

	/* check the rendering */
	pass = probe_prims(GL_LINES_ADJACENCY,
			   lines_adj_verts,
			   NUM_VERTS(lines_adj_verts), 0) && pass;

	pass = probe_prims(GL_LINE_STRIP_ADJACENCY,
			   line_strip_adj_verts,
			   NUM_VERTS(line_strip_adj_verts), 1) && pass;

	pass = probe_prims(GL_TRIANGLES_ADJACENCY,
			   triangles_adj_verts,
			   NUM_VERTS(triangles_adj_verts), 2) && pass;

	pass = probe_prims(GL_TRIANGLE_STRIP_ADJACENCY,
			   triangle_strip_adj_verts,
			   NUM_VERTS(triangle_strip_adj_verts), 3) && pass;

	piglit_present_results();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}


static GLuint
create_vao(const GLfloat (*verts)[2], GLuint numVerts)
{
	GLuint vao, vbo;

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	// positions
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, numVerts * VERTEX_SIZE,
		     verts, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, VERTEX_SIZE, NULL);
	glEnableVertexAttribArray(0);

	// colors
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(colors), colors, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 4*sizeof(GLfloat), NULL);
	glEnableVertexAttribArray(1);

	return vao;
}


void
piglit_init(int argc, char **argv)
{
	int i;

	for (i = 1; i < argc; i++) {
		if (strcmp(argv[i], "line") == 0)
			polygon_mode = GL_LINE;
		else if (strcmp(argv[i], "cull-back") == 0)
			cull_mode = GL_BACK;
		else if (strcmp(argv[i], "cull-front") == 0)
			cull_mode = GL_FRONT;
		else if (strcmp(argv[i], "ref") == 0)
			draw_with_gs = GL_FALSE;
		else if (strcmp(argv[i], "pv-last") == 0)
			provoking_vertex = GL_LAST_VERTEX_CONVENTION;
		else if (strcmp(argv[i], "pv-first") == 0)
			provoking_vertex = GL_FIRST_VERTEX_CONVENTION;
		else
			printf("Unexpected %s argument\n", argv[i]);
	}

	glPolygonMode(GL_FRONT_AND_BACK, polygon_mode);
	if (cull_mode != GL_NONE) {
		glCullFace(cull_mode);
		glEnable(GL_CULL_FACE);
		glFrontFace(GL_CW);
	}
	glProvokingVertex(provoking_vertex);

	lines_adj_vao = create_vao(lines_adj_verts,
				   NUM_VERTS(lines_adj_verts));

	line_strip_adj_vao = create_vao(line_strip_adj_verts,
					NUM_VERTS(line_strip_adj_verts));

	triangles_adj_vao = create_vao(triangles_adj_verts,
				       NUM_VERTS(triangles_adj_verts));

	triangle_strip_adj_vao = create_vao(triangle_strip_adj_verts,
					    NUM_VERTS(triangle_strip_adj_verts));

	gs_lines_program = make_gs_program(GL_LINES_ADJACENCY);
	gs_line_strip_program = make_gs_program(GL_LINE_STRIP_ADJACENCY);
	gs_triangles_program = make_gs_program(GL_TRIANGLES_ADJACENCY);
	gs_triangle_strip_program = make_gs_program(GL_TRIANGLE_STRIP_ADJACENCY);
	ref_program = make_ref_program();
}
