/*
 * BEGIN_COPYRIGHT -*- glean -*-
 * 
 * Copyright (C) 2000  Allen Akin   All Rights Reserved.
 * Copyright (C) 2014  Intel Corporation.
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
 * 
 * END_COPYRIGHT
 */

/** @file orthopos.c
 * 
 *  Test positioning of primitives in orthographic projection.
 *
 *  Some applications use OpenGL extensively for 2D rendering:  portable
 *  GUI toolkits, heads-up display generators, etc.  These apps require
 *  primitives to be drawn with reliable position and size in orthographic
 *  projections.  There are some potential pitfalls; for a good discussion,
 *  see the OpenGL Programming Guide (the Red Book).  In the second edition,
 *  see the OpenGL Correctness Tips on page 601.
 */

#include "piglit-util-gl.h"

#include <stdlib.h>
#include <stdio.h>

#define window_size piglit_width
#define drawing_size window_size - 2

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 13;

	config.window_visual = PIGLIT_GL_VISUAL_RGB | 
		PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

struct orthpos_result {
	bool has_gaps, has_overlaps, has_bad_edges;
};

static int img_bytes;
static GLubyte* img;

void
piglit_init(int argc, char **argv)
{
	srand(0);
	img_bytes = window_size * window_size * 
		piglit_num_components(GL_RGB) * sizeof(GLubyte);
	img = malloc(img_bytes);

	/* Common setup */
	piglit_ortho_projection(window_size, window_size, GL_FALSE);
	glTranslatef(0.375f, 0.375f, 0);

	glFrontFace(GL_CCW);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	glDisable(GL_DITHER);
	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);
	glShadeModel(GL_FLAT);
}

bool
log_results(const char* title, struct orthpos_result* r) 
{
	bool pass = true;
	printf("\t%s: ", title);
	if (r->has_gaps || r->has_overlaps || r->has_bad_edges) {
		printf("%s %s %s\n", (r->has_gaps? "Gaps.": ""),
			(r->has_overlaps? " Overlaps.": ""),
			(r->has_bad_edges? " Incorrect edges.": ""));
		pass = false;
	} else {
		printf(" No gaps, overlaps, or incorrect edges.\n");
	}
	return pass;
} /* log_results */

GLubyte
logical_sum(GLubyte* start, int stride, int count)
{
	GLubyte* p = start;
	GLubyte sum = 0;
	int i;
	for (i = 0; i < count; ++i) {
		sum |= p[0];
		sum |= p[1];
		sum |= p[2];
		p += stride;
	}
	return sum;
}

bool
verify_orth_pos(const GLubyte* img, GLsizei img_row_size_in_bytes, 
	const char* title) 
{

	/*
	 * All of the tests in this group are constructed so that the
	 * "correct" image covers a square of exactly drawing_size by
	 * drawing_size pixels, embedded in a window that's two pixels
	 * larger in both dimensions.  The border consists of pixels
	 * with all components set to zero.  Within the image, all
	 * pixels should be either red (only the red component is
	 * nonzero) or green (only the green component is nonzero).  If
	 * any pixels with all zero components are found, that indicates
	 * the presence of gaps.  If any pixels with both red and green
	 * nonzero components are found, that indicates the presence of
	 * overlaps.
	 */

	/* For logging results */
	struct orthpos_result res;

	/* For examining edges */
	GLubyte* row0 = (GLubyte*) img;
	GLubyte* row1 = row0 + img_row_size_in_bytes;
	GLubyte* row_last = row0 + (window_size - 1) * img_row_size_in_bytes;
	GLubyte* row_next_last = row_last - img_row_size_in_bytes;

	/* For examining the drawing area */
	int i, j, idx;
	GLubyte red, green, blue;

	/* Initialize results */
	res.has_gaps = false;
	res.has_overlaps = false;
	res.has_bad_edges = false;

	/* Check the bottom horizontal edge; it must be all zero. */
	if (logical_sum(row0, 3, window_size)) {
		printf("\t%s:  bottom border (at Y==0) was touched\n", title);
		res.has_bad_edges = true;
	}
	/* Repeat the process for the top horizontal edge. */
	if (logical_sum(row_last, 3, window_size)) {
		printf("\t%s:  top border (at Y==%i) was touched\n",
			title, window_size - 1);
		res.has_bad_edges = true;
	}
	/*
	 * Check the second row; there must be at least one nonzero
	 * pixel in the "drawn" region (excluding the first and last
	 * column).
	 */
	if (!logical_sum(row1 + 3/*skip 1st pixel's RGB*/, 3, drawing_size)) {
		printf("\t%s:  first row (at Y==1) was not drawn\n", title);
		res.has_bad_edges = true;
	}
	/* Repeat the process for the last row. */
	if (!logical_sum(row_next_last + 3, 3, drawing_size)) {
		printf("\t%s:  last row (at Y==%i) was not drawn\n",
			title, window_size - 2);
		res.has_bad_edges = true;
	}

	/* Check the left-hand vertical edge; it must be all zero. */
	if (logical_sum(row0, img_row_size_in_bytes, window_size)) {
		printf("\t%s:  left border (at X==0) was touched\n", title);
		res.has_bad_edges = true;
	}
	/* Repeat for the right-hand vertical edge. */
	if (logical_sum(row0 + 3 * (window_size - 1), img_row_size_in_bytes,
	    window_size)) {
		printf("\t%s:  right border (at X==%i) was touched\n",
			title, window_size - 1);
		res.has_bad_edges = true;
	}
	/* Check the left-hand column; something must be nonzero. */
	if (!logical_sum(row1 + 3, img_row_size_in_bytes, drawing_size)) {
		printf("\t%s:  first column (at X==1) was not drawn\n", 
			title);
		res.has_bad_edges = true;
	}
	/* And repeat for the right-hand column: */
	if (!logical_sum(row1 + 3 * (drawing_size - 1), img_row_size_in_bytes,
	    drawing_size)) {
		printf("\t%s:  last column (at X==%i) was not drawn\n",
			title, window_size - 2);
		res.has_bad_edges = true;
	}
	
	/*
	 * Scan the drawing area.  Anytime we find a pixel with all zero
	 * components, that's a gap.  Anytime we find a pixel with both
	 * red and green components nonzero, that's an overlap.
	 */
	/* Not sure what was wrong with the original, but this works. */
	for (i = 1; i < window_size - 1; ++i) {
		for (j = 1; j < window_size - 1; ++j) {
			idx = 3*(window_size*i + j);
			red = img[idx + 0];
			green = img[idx + 1];
			blue = img[idx + 2];

			if (!red && !green && !blue) {
				if (!res.has_gaps) {
					printf("\t%s:  found first ", title);
					printf("gap at X==%i, Y==%i\n",
						j, i);
					res.has_gaps = true;
				}
			}
			if (red && green) {
				if (!res.has_overlaps) {
					printf("\t%s:  found first ", title);
					printf("overlap at X==%i, Y==%i\n",
						j + 1, i + 1);
					res.has_overlaps = true;
				}
			}
		}
	}

	return log_results(title, &res);
} /* verify_orth_pos */

void
subdivide_rects(int minx, int maxx, int miny, int maxy,
    bool split_horiz, bool draw_in_red) 
{
	/*
	 * Basically we're just splitting the input rectangle
	 * recursively.  At each step we alternate between splitting
	 * horizontally (dividing along Y) or vertically (along X).  We
	 * also toggle colors (between red and green) at various times,
	 * in order to give us some adjacent edges of different colors
	 * that we can check for overlaps.  Recursion bottoms out when
	 * the axis of interest drops below 30 pixels in length.
	 */
	int split;

	int min = split_horiz? miny: minx;
	int max = split_horiz? maxy: maxx;
	if (min + 30 > max) {
		glColor4f(draw_in_red? 1.0: 0.0, draw_in_red? 0.0: 1.0,
			0.0, 0.5);
		glBegin(GL_QUADS);
		glVertex2i(minx, miny);
		glVertex2i(maxx, miny);
		glVertex2i(maxx, maxy);
		glVertex2i(minx, maxy);
		glEnd();
		return;
	}

	split = min + (int) ((max - min) * 
		((float) rand() / RAND_MAX)); /* Float in [0.0f, 1.0f] */
	if (split_horiz) {
		subdivide_rects(minx, maxx, miny, split,
			!split_horiz, draw_in_red);
		subdivide_rects(minx, maxx, split, maxy,
			!split_horiz, !draw_in_red);
	} else {
		subdivide_rects(minx, split, miny, maxy,
			!split_horiz, draw_in_red);
		subdivide_rects(split, maxx, miny, maxy,
			!split_horiz, !draw_in_red);
	}
}

/**
 * This test checks the positioning of unit-sized points under
 * orthographic projection.  (This is important for apps that
 * want to use OpenGL for precise 2D drawing.)  It fills in an
 * entire rectangle one pixel at a time, drawing adjacent pixels
 * with different colors and with blending enabled.  If there are
 * gaps (pixels that are the background color, and thus haven't
 * been filled), overlaps (pixels that show a blend of more than
 * one color), or improper edges (pixels around the edge of the
 * rectangle that haven't been filled, or pixels just outside the
 * edge that have), then the test fails.
 * 
 * This test generally fails for one of several reasons.  First,
 * the coordinate transformation process may have an incorrect bias;
 * this usually will cause a bad edge.  Second, the coordinate
 * transformation process may round pixel coordinates incorrectly;
 * this will usually cause gaps and/or overlaps.  Third, the point
 * rasterization process may not be filling the correct pixels;
 * this can cause gaps, overlaps, or bad edges.
 */

bool
ortho_pos_points(void) 
{
	int x, y;

	/* Draw the image */
	glClear(GL_COLOR_BUFFER_BIT);
	glBegin(GL_POINTS);
	for (x = 1; x <= drawing_size; ++x) {
		for (y = 1; y <= drawing_size; ++y) {
			if ((x ^ y) & 1) {
				glColor4f(0.0, 1.0, 0.0, 0.5);
			}
			else {
				glColor4f(1.0, 0.0, 0.0, 0.5);
			}
			glVertex2i(x, y);
		}
	}
	glEnd();

	/* Read the image */
	glReadPixels(0, 0, window_size, window_size, GL_RGB, 
		GL_UNSIGNED_BYTE, img);

	/* Show the image */
	if (!piglit_automatic) {
		piglit_present_results();
	}

	/* Check the results */
	return verify_orth_pos(img, img_bytes/window_size, 
		"Immediate-mode points");
} /* ortho_pos_points */

/**
 * This test checks the positioning of unit-width vertical lines
 * under orthographic projection.	(This is important for apps
 * that want to use OpenGL for precise 2D drawing.)  It fills in
 * an entire rectangle with a collection of vertical lines, drawing
 * adjacent lines with different colors and with blending enabled.
 * If there are gaps (pixels that are the background color, and
 * thus haven't been filled), overlaps (pixels that show a blend
 * of more than one color), or improper edges (pixels around the
 * edge of the rectangle that haven't been filled, or pixels just
 * outside the edge that have), then the test fails.
 * 
 * This test generally fails for one of several reasons.  First,
 * the coordinate transformation process may have an incorrect bias;
 * this usually will cause a bad edge.  Second, the coordinate
 * transformation process may round pixel coordinates incorrectly;
 * this will usually cause gaps and/or overlaps.  Third, the
 * line rasterization process may not be filling the correct
 * pixels; this can cause gaps, overlaps, or bad edges.  Fourth,
 * the OpenGL implementation may not handle the diamond-exit rule
 * (section 3.4.1 in version 1.2.1 of the OpenGL spec) correctly;
 * this should cause a bad border or bad top edge.
 * 
 * It can be argued that this test is more strict that the OpenGL
 * specification requires.  However, it is necessary to be this
 * strict in order for the results to be useful to app developers
 * using OpenGL for 2D drawing.
 */
bool
ortho_pos_vlines(void) 
{
	/*
	 * Immediate-mode vertical lines
	 * 	Note that these are a little tricky, because of
	 * 	OpenGL's "diamond-exit rule" line semantics.  In
	 * 	this case, we can safely treat them as half-open
	 * 	lines, where the terminal point isn't drawn.  Thus
	 * 	we need to specify a terminal coordinate one pixel
	 * 	beyond the last pixel we wish to be drawn.
	 */

	int x;

	/* Draw the image */
	glClear(GL_COLOR_BUFFER_BIT);
	glBegin(GL_LINES);
	for (x = 1; x <= drawing_size; ++x) {
		if (x & 1)
			glColor4f(0.0, 1.0, 0.0, 0.5);
		else
			glColor4f(1.0, 0.0, 0.0, 0.5);
		glVertex2i(x, 1);
		glVertex2i(x, drawing_size + 1);
	}
	glEnd();

	/* Read the image */
	glReadPixels(0, 0, window_size, window_size, GL_RGB, 
		GL_UNSIGNED_BYTE, img);

	/* Show the image */
	if (!piglit_automatic) {
		piglit_present_results();
	}

	/* Check the results */
	return verify_orth_pos(img, img_bytes/window_size, 
		"Immediate-mode vertical lines");
} /* ortho_pos_vlines */

/**
 * This test checks the positioning of unit-width horizontal lines
 * under orthographic projection. (This is important for apps
 * that want to use OpenGL for precise 2D drawing.)  It fills in
 * an entire rectangle with a stack of horizontal lines, drawing
 * adjacent lines with different colors and with blending enabled.
 * If there are gaps (pixels that are the background color, and
 * thus haven't been filled), overlaps (pixels that show a blend
 * of more than one color), or improper edges (pixels around the
 * edge of the rectangle that haven't been filled, or pixels just
 * outside the edge that have), then the test fails.
 * 
 * This test generally fails for one of several reasons.  First,
 * the coordinate transformation process may have an incorrect bias;
 * this usually will cause a bad edge.  Second, the coordinate
 * transformation process may round pixel coordinates incorrectly;
 * this will usually cause gaps and/or overlaps.  Third, the
 * line rasterization process may not be filling the correct
 * pixels; this can cause gaps, overlaps, or bad edges.  Fourth,
 * the OpenGL implementation may not handle the diamond-exit rule
 * (section 3.4.1 in version 1.2.1 of the OpenGL spec) correctly;
 * this should cause a bad border or bad right edge.
 * 
 * It can be argued that this test is more strict that the OpenGL
 * specification requires.  However, it is necessary to be this
 * strict in order for the results to be useful to app developers
 * using OpenGL for 2D drawing.
 */
bool
ortho_pos_hlines(void) 
{
	/*
	 * Immediate-mode horizontal lines
	 * See the comments in the vertical line case above.
	 */
	int y;
	
	/* Draw the image */
	glClear(GL_COLOR_BUFFER_BIT);
	glBegin(GL_LINES);
	for (y = 1; y <= drawing_size; ++y) {
		if (y & 1)
			glColor4f(0.0, 1.0, 0.0, 0.5);
		else
			glColor4f(1.0, 0.0, 0.0, 0.5);
		glVertex2i(1, y);
		glVertex2i(drawing_size + 1, y);
	}
	glEnd();

	/* Read the image */
	glReadPixels(0, 0, window_size, window_size, GL_RGB, 
		GL_UNSIGNED_BYTE, img);

	/* Show the image */
	if (!piglit_automatic) {
		piglit_present_results();
	}

	/* Check the results */
	return verify_orth_pos(img, img_bytes/window_size, 
		"Immediate-mode horizontal lines");
} /* ortho_pos_hlines */

/**
 * This test checks the positioning of 1x1-pixel quadrilaterals
 * under orthographic projection.	(This is important for apps
 * that want to use OpenGL for precise 2D drawing.)  It fills in
 * an entire rectangle with an array of quadrilaterals, drawing
 * adjacent quads with different colors and with blending enabled.
 * If there are gaps (pixels that are the background color, and
 * thus haven't been filled), overlaps (pixels that show a blend
 * of more than one color), or improper edges (pixels around the
 * edge of the rectangle that haven't been filled, or pixels just
 * outside the edge that have), then the test fails.
 * 
 * This test generally fails for one of several reasons.  First,
 * the coordinate transformation process may have an incorrect bias;
 * this usually will cause a bad edge.  Second, the coordinate
 * transformation process may round pixel coordinates incorrectly;
 * this will usually cause gaps and/or overlaps.  Third, the
 * quad rasterization process may not be filling the correct
 * pixels; this can cause gaps, overlaps, or bad edges.
 */
bool
ortho_pos_tiny_quads(void) 
{
	/*
	 * Immediate-mode 1x1-pixel quads
	 */

	int x, y;
	
	/* Draw the image */
	glClear(GL_COLOR_BUFFER_BIT);
	glBegin(GL_QUADS);
	for (x = 1; x <= drawing_size; ++x)
		for (y = 1; y <= drawing_size; ++y) {
			if ((x ^ y) & 1)
				glColor4f(0.0, 1.0, 0.0, 0.5);
			else
				glColor4f(1.0, 0.0, 0.0, 0.5);
			glVertex2i(x, y);
			glVertex2i(x + 1, y);
			glVertex2i(x + 1, y + 1);
			glVertex2i(x, y + 1);
	}
	glEnd();

	/* Read the image */
	glReadPixels(0, 0, window_size, window_size, GL_RGB, 
		GL_UNSIGNED_BYTE, img);

	/* Show the image */
	if (!piglit_automatic) {
		piglit_present_results();
	}

	/* Check the results */
	return verify_orth_pos(img, img_bytes/window_size, 
		"Immediate-mode 1x1 quads");
} /* ortho_pos_tiny_quads */

/**
 * This test checks the positioning of axis-aligned rectangles
 * under orthographic projection.	(This is important for apps
 * that want to use OpenGL for precise 2D drawing.)  It fills in
 * an entire rectangle with an array of smaller rects, drawing
 * adjacent rects with different colors and with blending enabled.
 * If there are gaps (pixels that are the background color, and
 * thus haven't been filled), overlaps (pixels that show a blend
 * of more than one color), or improper edges (pixels around the
 * edge of the rectangle that haven't been filled, or pixels just
 * outside the edge that have), then the test fails.
 * 
 * This test generally fails for one of several reasons.  First,
 * the coordinate transformation process may have an incorrect bias;
 * this usually will cause a bad edge.  Second, the coordinate
 * transformation process may round pixel coordinates incorrectly;
 * this will usually cause gaps and/or overlaps.  Third, the
 * rectangle rasterization process may not be filling the correct
 * pixels; this can cause gaps, overlaps, or bad edges.
 */
bool
ortho_pos_rand_rects(void) 
{
	/*
	 * Immediate-mode random axis-aligned rectangles
	 */

	/* Draw the image */
	glClear(GL_COLOR_BUFFER_BIT);
	subdivide_rects(1, drawing_size + 1, 1, drawing_size + 1,
		true, true);

	/* Read the image */
	glReadPixels(0, 0, window_size, window_size, GL_RGB, 
		GL_UNSIGNED_BYTE, img);

	/* Show the image */
	if (!piglit_automatic) {
		piglit_present_results();
	}

	/* Check the results */
	return verify_orth_pos(img, img_bytes/window_size, 
		"Immediate-mode random axis-aligned rectangles");
} /* ortho_pos_rand_rects */

/* Factory for generating random mesh just like Glean's RandomMesh2D class */
float*
random_mesh_2d(float minx, float maxx, int xpoints,
	float miny, float maxy, int ypoints)
{
	int x, y, idx;
	float* mesh = malloc(xpoints * ypoints * 2 * sizeof(float));
	double deltax = 0.7 * (maxx - minx) / (xpoints - 1);
	double deltay = 0.7 * (maxy - miny) / (ypoints - 1);
	float rand_no;

	for (y = 0; y < ypoints; ++y) {
		for (x = 0; x < xpoints; ++x) {

			idx = 2 * (xpoints * y + x);

			/* Generate an unperturbed, uniform mesh */
			mesh[idx + 0] = minx + (x * (maxx - minx)) / 
				(xpoints - 1);
			mesh[idx + 1] = miny + (y * (maxy - miny)) / 
				(ypoints - 1);

			/* Perturb the interior points of the mesh */
			if ((x != 0) && (y != 0) && (x != xpoints - 1) && 
				(y != ypoints - 1))
			{
				/* Float in range [0.0f, 1.0f] */
				rand_no = (float) rand() / RAND_MAX; 
				mesh[idx + 0] += deltax * (rand_no - 0.5);
				rand_no = (float) rand() / RAND_MAX; 
				mesh[idx + 1] += deltay * (rand_no - 0.5);
			}
		}
	}

	return mesh;
} /* random_mesh_2d */

/**
 * This test checks the positioning of random triangles under
 * orthographic projection.  (This is important for apps that
 * want to use OpenGL for precise 2D drawing.)  It fills in an
 * entire rectangle with an array of randomly-generated triangles,
 * drawing adjacent triangles with different colors and with blending
 * enabled.  If there are gaps (pixels that are the background color,
 * and thus haven't been filled), overlaps (pixels that show a blend
 * of more than one color), or improper edges (pixels around the
 * edge of the rectangle that haven't been filled, or pixels just
 * outside the edge that have), then the test fails.
 * 
 * This test generally fails for one of several reasons.  First,
 * the coordinate transformation process may have an incorrect bias;
 * this usually will cause a bad edge.  Second, the coordinate
 * transformation process may round pixel coordinates incorrectly;
 * this will usually cause gaps and/or overlaps.  Third, the
 * triangle rasterization process may not be filling the correct
 * pixels; this can cause gaps, overlaps, or bad edges.
 */
bool
ortho_pos_rand_tris(void) 
{
	/*
	 * Immediate-mode random triangles
	 */
	int i, j;
	int npoints = 10;
	float* mesh;

	/* Draw the image */
	glClear(GL_COLOR_BUFFER_BIT);
	mesh = random_mesh_2d(1, drawing_size + 1, npoints,
		1, drawing_size + 1, npoints);
	for (i = npoints - 1; i > 0; --i) {
		glBegin(GL_TRIANGLE_STRIP);
		for (j = 0; j < npoints; ++j) {
			glColor4f(1.0, 0.0, 0.0, 0.5);
			/* mesh[i, j] */
			glVertex2fv(mesh + 2 * (npoints * i + j)); 
			glColor4f(0.0, 1.0, 0.0, 0.5);
			/* mesh[i - 1, j] */
			glVertex2fv(mesh + 2 * (npoints * (i - 1) + j)); 
		}
		glEnd();
	}
	free(mesh);

	/* Read the image */
	glReadPixels(0, 0, window_size, window_size, GL_RGB, 
		GL_UNSIGNED_BYTE, img);

	/* Show the image */
	if (!piglit_automatic) {
		piglit_present_results();
	}

	/* Check the results */
	return verify_orth_pos(img, img_bytes/window_size, 
		"Immediate-mode random triangles");
} /* ortho_pos_rand_tris */

enum piglit_result
piglit_display(void)
{
	bool pass = true;

	pass &= ortho_pos_points();
	pass &= ortho_pos_vlines();
	pass &= ortho_pos_hlines();
	pass &= ortho_pos_tiny_quads();
	pass &= ortho_pos_rand_rects();
	pass &= ortho_pos_rand_tris();

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}
