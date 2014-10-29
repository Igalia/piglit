/*
 * BEGIN_COPYRIGHT -*- glean -*-
 * 
 * Copyright (C) 2001  Allen Akin   All Rights Reserved.
 * Copyright (C) 2014  Intel Corporation   All Rights Reserved.  
 * 
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 * 
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the
 * Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 * 
 * END_COPYRIGHT
 */

/** @file polygon-offset.c
 *
 * Implementation of polygon offset tests.
 *
 * This test verifies glPolygonOffset.  It is run on every
 * OpenGL-capable drawing surface configuration that supports
 * creation of a window, has a depth buffer, and is RGB.
 * 
 * The first subtest verifies that the OpenGL implementation is
 * using a plausible value for the \"minimum resolvable
 * difference\" (MRD).  This is the offset in window coordinates
 * that is sufficient to provide separation in depth (Z) for any
 * two parallel surfaces.  The subtest searches for the MRD by
 * drawing two surfaces at a distance from each other and
 * checking the resulting image to see if they were cleanly
 * separated.  The distance is then modified (using a binary
 * search) until a minimum value is found.  This is the so-called
 * \"ideal\" MRD.  Then two surfaces are drawn using
 * glPolygonOffset to produce a separation that should equal one
 * MRD.  The depth values at corresponding points on each surface
 * are subtracted to form the \"actual\" MRD.  The subtest performs
 * these checks twice, once close to the viewpoint and once far
 * away from it, and passes if the largest of the ideal MRDs and
 * the largest of the actual MRDs are nearly the same.
 * 
 * The second subtest verifies that the OpenGL implementation is
 * producing plausible values for slope-dependent offsets.  The
 * OpenGL spec requires that the depth slope of a surface be
 * computed by an approximation that is at least as large as
 * max(abs(dz/dx),abs(dz/dy)) and no larger than
 * sqrt((dz/dx)**2+(dz/dy)**2).  The subtest draws a quad rotated
 * by various angles along various axes, samples three points on
 * the quad's surface, and computes dz/dx and dz/dy.  Then it
 * draws two additional quads offset by one and two times the
 * depth slope, respectively.  The base quad and the two new
 * quads are sampled and their actual depths read from the depth
 * buffer.  The subtest passes if the quads are offset by amounts
 * that are within one and two times the allowable range,
 * respectively.
 *
 * Derived in part from tests written by Angus Dorbie <dorbie@sgi.com>
 * in September 2000 and Rickard E. (Rik) Faith <faith@valinux.com> in
 * October 2000.
 *
 * Ported to Piglit by Laura Ekstrand.
 */

#include "piglit-util-gl.h"

#if defined(__APPLE__)
#  include <OpenGL/glu.h>
#else
#  include <GL/glu.h>
#endif

#include <math.h>

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 11;

	config.window_visual = PIGLIT_GL_VISUAL_RGBA | 
		PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_DEPTH;

PIGLIT_GL_TEST_CONFIG_END


struct angle_axis {
	GLfloat angle;
	GLfloat axis[3];
};

void
draw_quad_at_distance(GLdouble dist) 
{
	glBegin(GL_QUADS);
		glVertex3d(-dist, -dist, -dist);
		glVertex3d( dist, -dist, -dist);
		glVertex3d( dist,  dist, -dist);
		glVertex3d(-dist,  dist, -dist);
	glEnd();
}

GLdouble
window_coord_depth(GLdouble dist) 
{
	/*
	 * Assumes we're using the "far at infinity" projection matrix
	 * and simple viewport transformation.
	 */
	return 0.5 * (dist - 2.0) / dist + 0.5;
}

bool
red_quad_was_drawn(void) 
{
	float expected[] = {1.0f, 0.0f, 0.0f};
	return piglit_probe_rect_rgb_silent(0, 0, piglit_width, 
		piglit_height, expected);
}

void
piglit_init(int argc, char **argv)
{

}

void
find_ideal_mrd(GLdouble* ideal_mrd_near, GLdouble* ideal_mrd_far, 
	GLdouble* next_to_near, GLdouble* next_to_far) 
{
	/*
	 * MRD stands for Minimum Resolvable Difference, the smallest
	 * distance in depth that suffices to separate any two
	 * polygons (or a polygon and the near or far clipping
	 * planes).
	 *
	 * This function tries to determine the "ideal" MRD for the
	 * current rendering context.  It's expressed in window
	 * coordinates, because the value in model or clipping
	 * coordinates depends on the scale factors in the modelview
	 * and projection matrices and on the distances to the near
	 * and far clipping planes.
	 *
	 * For simple unsigned-integer depth buffers that aren't too
	 * deep (so that precision isn't an issue during coordinate
	 * transformations), it should be about one least-significant
	 * bit.  For deep or floating-point or compressed depth
	 * buffers the situation may be more complicated, so we don't
	 * pass or fail an implementation solely on the basis of its
	 * ideal MRD.
	 *
	 * There are two subtle parts of this function.  The first is
	 * the projection matrix we use for rendering.  This matrix
	 * places the far clip plane at infinity (so that we don't run
	 * into arbitrary limits during our search process).  The
	 * second is the method used for drawing the polygon.  We
	 * scale the x and y coords of the polygon vertices by the
	 * polygon's depth, so that it always occupies the full view
	 * frustum.  This makes it easier to verify that the polygon
	 * was resolved completely -- we just read back the entire
	 * window and see if any background pixels appear.
	 *
	 * To insure that we get reasonable results on machines with
	 * unusual depth buffers (floating-point, or compressed), we
	 * determine the MRD twice, once close to the near clipping
	 * plane and once as far away from the eye as possible.  On a
	 * simple integer depth buffer these two values should be
	 * essentially the same.  For other depth-buffer formats, the
	 * ideal MRD is simply the largest of the two.
	 */

	GLdouble near_dist, far_dist, half_dist;
	int i;

	/*
	 * First, find a distance that is as far away as possible, yet
	 * a quad at that distance can be distinguished from the
	 * background.  Start by pushing quads away from the eye until
	 * we find an interval where the closer quad can be resolved,
	 * but the farther quad cannot.  Then binary-search to find
	 * the threshold.
	 */

	glDepthFunc(GL_LESS);
	glClearDepth(1.0);
	glColor3f(1.0, 0.0, 0.0); /* red */
	near_dist = 1.0;
	far_dist = 2.0;
	for (;;) {
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		draw_quad_at_distance(far_dist);
		if (!red_quad_was_drawn())
			break;
		piglit_present_results();
		near_dist = far_dist;
		far_dist *= 2.0;
	}
	for (i = 0; i < 64; ++i) {
		half_dist = 0.5 * (near_dist + far_dist);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		draw_quad_at_distance(half_dist);
		if (red_quad_was_drawn())
			near_dist = half_dist;
		else
			far_dist = half_dist;
		piglit_present_results();
	}
	*next_to_far = near_dist;

	/*
	 * We can derive a resolvable difference from the value
	 * next_to_far, but it's not necessarily the one we want. 
	 * Consider mapping the object coordinate range [0,1] onto the
	 * integer window coordinate range [0,2].  A natural way to do
	 * this is with a linear function, windowCoord =
	 * 2*objectCoord.  With rounding, this maps [0,0.25) to 0,
	 * [0.25,0.75) to 1, and [0.75,1] to 2.  Note that the
	 * intervals at either end are 0.25 wide, but the one in the
	 * middle is 0.5 wide.  The difference we can derive from
	 * next_to_far is related to the width of the final interval. 
	 * We want to back up just a bit so that we can get a
	 * (possibly much larger) difference that will work for the
	 * larger interval.  To do this we need to find a difference
	 * that allows us to distinguish two quads when the more
	 * distant one is at distance next_to_far.
	 */

	near_dist = 1.0;
	far_dist = *next_to_far;
	for (i = 0; i < 64; ++i) {
		half_dist = 0.5 * (near_dist + far_dist);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glColor3f(0.0, 0.0, 0.0); /* black */
		glDepthFunc(GL_ALWAYS);
		draw_quad_at_distance(*next_to_far);

		glColor3f(1.0, 0.0, 0.0); /* red */
		glDepthFunc(GL_LESS);
		draw_quad_at_distance(half_dist);

		if (red_quad_was_drawn())
			near_dist = half_dist;
		else
			far_dist = half_dist;
		piglit_present_results();
	}

	*ideal_mrd_far = window_coord_depth(*next_to_far)
		- window_coord_depth(near_dist);

	/*
	 * Now we apply a similar strategy at the near end of the
	 * depth range, but swapping the senses of various comparisons
	 * so that we approach the near clipping plane rather than the
	 * far.
	 */

	glClearDepth(0.0);
	glDepthFunc(GL_GREATER);
	glColor3f(1.0, 0.0, 0.0); /* red */
	near_dist = 1.0;
	far_dist = *next_to_far;
	for (i = 0; i < 64; ++i) {
		half_dist = 0.5 * (near_dist + far_dist);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		draw_quad_at_distance(half_dist);
		if (red_quad_was_drawn())
			far_dist = half_dist;
		else
			near_dist = half_dist;
		piglit_present_results();
	}
	*next_to_near = far_dist;

	near_dist = *next_to_near;
	far_dist = *next_to_far;
	for (i = 0; i < 64; ++i) {
		half_dist = 0.5 * (near_dist + far_dist);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glColor3f(0.0, 0.0, 0.0); /* black */
		glDepthFunc(GL_ALWAYS);
		draw_quad_at_distance(*next_to_near);

		glColor3f(1.0, 0.0, 0.0); /* red */
		glDepthFunc(GL_GREATER);
		draw_quad_at_distance(half_dist);

		if (red_quad_was_drawn())
			far_dist = half_dist;
		else
			near_dist = half_dist;
		piglit_present_results();
	}

	*ideal_mrd_near = window_coord_depth(far_dist)
		- window_coord_depth(*next_to_near);
} /* find_ideal_mrd */

double
read_depth(int x, int y) 
{
	GLuint depth;
	glReadPixels(x, y, 1, 1, GL_DEPTH_COMPONENT, 
		GL_UNSIGNED_INT, &depth);

	/*
	 * This normalization of "depth" is correct even on 64-bit
	 * machines because GL types have machine-independent ranges.
	 */
	return ((double) depth) / 4294967295.0;
}

void
find_actual_mrd(GLdouble* next_to_near, GLdouble* next_to_far,
	GLdouble* actual_mrd_near, GLdouble* actual_mrd_far) 
{
	/*
	 * Here we use polygon offset to determine the
	 * implementation's actual MRD.
	 */

	double base_depth;

	glDepthFunc(GL_ALWAYS);

	/* Draw a quad far away from the eye and read the depth at its
	 * center: */
	glDisable(GL_POLYGON_OFFSET_FILL);
	draw_quad_at_distance(*next_to_far);
	base_depth = read_depth(piglit_width/2, piglit_height/2);

	/* Now draw a quad that's one MRD closer to the eye: */
	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(0.0, -1.0);
	draw_quad_at_distance(*next_to_far);

	/*
	 * The difference between the depths of the two quads is the
	 * value the implementation is actually using for one MRD:
	 */
	*actual_mrd_far = base_depth
		- read_depth(piglit_width/2, piglit_height/2);

	/* Repeat the process for a quad close to the eye: */
	glDisable(GL_POLYGON_OFFSET_FILL);
	draw_quad_at_distance(*next_to_near);
	base_depth = read_depth(piglit_width/2, piglit_height/2);
	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(0.0, 1.0);	/* 1 MRD further away */
	draw_quad_at_distance(*next_to_near);
	*actual_mrd_near = read_depth(piglit_width/2, piglit_height/2)
		- base_depth;
} /* find_actual_mrd */

void
draw_2x2_quad(void) 
{
	glBegin(GL_QUADS);
		glVertex2f(-1.0, -1.0);
		glVertex2f( 1.0, -1.0);
		glVertex2f( 1.0,  1.0);
		glVertex2f(-1.0,  1.0);
	glEnd();
}

bool
check_slope_offset(struct angle_axis* aa, GLdouble* ideal_mrd_near) 
{
	/*
	 * This function checks for correct slope-based offsets for
	 * a quad rotated to a given angle around a given axis.
	 *
	 * The basic strategy is to:
	 *	Draw the quad.  (Note: the quad's size and position
	 *		are chosen so that it won't ever be clipped.)
	 *	Sample three points in the quad's interior.
	 *	Compute dz/dx and dz/dy based on those samples.
	 *	Compute the range of allowable offsets; must be between
	 *		max(abs(dz/dx), abs(dz/dy)) and
	 *		sqrt((dz/dx)**2, (dz/dy)**2)
	 *	Sample the depth of the quad at its center.
	 *	Use PolygonOffset to produce an offset equal to one
	 *		times the depth slope of the base quad.
	 *	Draw another quad with the same orientation as the first.
	 *	Sample the second quad at its center.
	 *	Compute the difference in depths between the first quad
	 *		and the second.
	 *	Verify that the difference is within the allowable range.
	 *	Repeat for a third quad at twice the offset from the first.
	 *		(This verifies that the implementation is scaling
	 *		the depth offset correctly.)
	 */

	const GLfloat quad_dist = 2.5;	/* must be > 1+sqrt(2) to avoid */
					/* clipping by the near plane */
	GLdouble modelview_mat[16];
	GLdouble projection_mat[16];
	GLint viewport[4];
	GLdouble centerw[3];
	GLdouble base_depth;
	GLdouble p0[3];
	GLdouble p1[3];
	GLdouble p2[3];
	double det, dzdx, dzdy, mmax, mmin;
	GLdouble offset_depth, offset;


	glClearDepth(1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	glColor3f(1.0, 0.0, 0.0); /* red */

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(0.0, 0.0, -quad_dist);
	glRotatef(aa->angle, aa->axis[0], aa->axis[1], aa->axis[2]);

	glGetDoublev(GL_MODELVIEW_MATRIX, modelview_mat);
	glGetDoublev(GL_PROJECTION_MATRIX, projection_mat);
	glGetIntegerv(GL_VIEWPORT, viewport);

	glDisable(GL_POLYGON_OFFSET_FILL);

	draw_2x2_quad();

	gluProject(0.0, 0.0, 0.0, modelview_mat, projection_mat, viewport,
		centerw + 0, centerw + 1, centerw + 2);
	base_depth = read_depth(centerw[0], centerw[1]);

	gluProject(-0.9, -0.9, 0.0, modelview_mat, projection_mat, viewport,
		p0 + 0, p0 + 1, p0 + 2);
	p0[2] = read_depth(p0[0], p0[1]);
	gluProject( 0.9, -0.9, 0.0, modelview_mat, projection_mat, viewport,
		p1 + 0, p1 + 1, p1 + 2);
	p1[2] = read_depth(p1[0], p1[1]);
	gluProject( 0.9,  0.9, 0.0, modelview_mat, projection_mat, viewport,
		p2 + 0, p2 + 1, p2 + 2);
	p2[2] = read_depth(p2[0], p2[1]);

	det = (p0[0] - p1[0]) * (p0[1] - p2[1])
		- (p0[0] - p2[0]) * (p0[1] - p1[1]);
	if (fabs(det) < 0.001)
		return false;	/* too close to colinear to evaluate */

	dzdx = ((p0[2] - p1[2]) * (p0[1] - p2[1])
		- (p0[2] - p2[2]) * (p0[1] - p1[1])) / det;
	dzdy = ((p0[0] - p1[0]) * (p0[2] - p2[2])
		- (p0[0] - p2[0]) * (p0[2] - p1[2])) / det;

	mmax = 1.1 * sqrt(dzdx * dzdx + dzdy * dzdy) + (*ideal_mrd_near);
		/* (adding ideal_mrd_near is a fudge for roundoff error */
		/* when the slope is extremely close to zero) */
	mmin = 0.9 * fmax(fabs(dzdx), fabs(dzdy));

	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(-1.0, 0.0);
	piglit_present_results();
	draw_2x2_quad();
	offset_depth = read_depth(centerw[0], centerw[1]);
	offset = fmax(base_depth - offset_depth, 0.0);
	if (offset < mmin || offset > mmax) {
		if (offset < mmin)
			printf("\tDepth-slope related offset was too small");
		else
			printf("\tDepth-slope related offset was too large");
		printf("; first failure at:\n");
		printf("\t\tAngle = %f degrees, axis = (%f, %f, %f)\n",
			aa->angle, aa->axis[0], aa->axis[1], aa->axis[2]);
		printf("\t\tFailing offset was %.16f\n", offset);
		printf("\t\tAllowable range is (%f, %f)\n", mmin, mmax);
		return false;
	}

	glPolygonOffset(-2.0, 0.0);
	piglit_present_results();
	draw_2x2_quad();
	offset_depth = read_depth(centerw[0], centerw[1]);
	offset = fmax(base_depth - offset_depth, 0.0);
	if (offset < 2.0 * mmin || offset > 2.0 * mmax) {
		if (offset < 2.0 * mmin)
			printf("\tDepth-slope related offset was too small");
		else
			printf("\tDepth-slope related offset was too large");
		printf("; first failure at:\n");
		printf("\t\tAngle = %f degrees, axis = (%f, %f, %f)\n",
			aa->angle, aa->axis[0], aa->axis[1], aa->axis[2]);
		printf("\t\tFailing offset was %.16f\n", offset);
		printf("\t\tAllowable range is (%f, %f)\n", 2.0 * mmin, 
			2.0 * mmax);
		return false;
	}

	return true;
}

bool
check_slope_offsets(GLdouble* ideal_mrd_near) 
{
	/*
	 * This function checks that the implementation is offsetting
	 * primitives correctly according to their depth slopes.
	 * (Note that it uses some values computed by find_ideal_mrd, so
	 * that function must be run first.)
	 */
	bool pass = true;
	int i;

	/*
	 * Rotation angles (degrees)
	 * and axes for which offset will be checked
	 */
	struct angle_axis aa[] = {
		{ 0,	{1, 0, 0}},
		{30,	{1, 0, 0}},
		{45,	{1, 0, 0}},
		{60,	{1, 0, 0}},
		{80,	{1, 0, 0}},
		{ 0,	{0, 1, 0}},
		{30,	{0, 1, 0}},
		{45,	{0, 1, 0}},
		{60,	{0, 1, 0}},
		{80,	{0, 1, 0}},
		{ 0,	{1, 1, 0}},
		{30,	{1, 1, 0}},
		{45,	{1, 1, 0}},
		{60,	{1, 1, 0}},
		{80,	{1, 1, 0}},
		{ 0,	{2, 1, 0}},
		{30,	{2, 1, 0}},
		{45,	{2, 1, 0}},
		{60,	{2, 1, 0}},
		{80,	{2, 1, 0}}
	};

	for (i = 0; pass && i < ARRAY_SIZE(aa); ++i)
		pass &= check_slope_offset(aa + i, ideal_mrd_near);
	
	return pass;
} /* check_slope_offsets */

void
log_mrd(double mrd, GLint dbits) 
{
	int bits;
	bits = (int)(0.5 + (pow(2.0, dbits) - 1.0) * mrd);
	printf("%e (nominally %i %s)\n", mrd, bits, 
		(bits == 1)? "bit": "bits");
} /* log_mrd */

enum piglit_result
piglit_display(void)
{
	bool pass = true;
	double ideal_mrd, actual_mrd;
	GLdouble ideal_mrd_near, ideal_mrd_far, next_to_near, next_to_far;
	GLdouble actual_mrd_near, actual_mrd_far;
	bool big_enough_mrd, small_enough_mrd;
	GLint dbits;

	/*
	 * The following projection matrix places the near clipping
	 * plane at distance 1.0, and the far clipping plane at
	 * infinity.  This allows us to stress depth-buffer resolution
	 * as far away from the eye as possible, without introducing
	 * code that depends on the size or format of the depth
	 * buffer.
	 *
	 * (To derive this matrix, start with the matrix generated by
	 * glFrustum with near-plane distance equal to 1.0, and take
	 * the limit of the matrix elements as the far-plane distance
	 * goes to infinity.)
	 */

	static GLfloat near_1_far_infinity[] = {
		1.0,  0.0,  0.0,  0.0,
		0.0,  1.0,  0.0,  0.0,
		0.0,  0.0, -1.0, -1.0,
		0.0,  0.0, -2.0,  0.0
	};
	
	glViewport(0, 0, piglit_width, piglit_height);
	glDepthRange(0.0, 1.0);

	glMatrixMode(GL_PROJECTION);
	glLoadMatrixf(near_1_far_infinity);

	glDisable(GL_LIGHTING);

	glFrontFace(GL_CCW);
	glDisable(GL_NORMALIZE);
	glDisable(GL_COLOR_MATERIAL);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glDisable(GL_TEXTURE_2D);

	glDisable(GL_FOG);

	glDisable(GL_SCISSOR_TEST);
	glDisable(GL_ALPHA_TEST);
	glDisable(GL_STENCIL_TEST);
	glDepthFunc(GL_LESS);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
	glDisable(GL_DITHER);
	glDisable(GL_COLOR_LOGIC_OP);

	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glDepthMask(GL_TRUE);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glCullFace(GL_BACK);
	glEnable(GL_CULL_FACE);
	glDisable(GL_POLYGON_STIPPLE);
	glDisable(GL_POLYGON_OFFSET_FILL);

	glShadeModel(GL_FLAT);

	glClearColor(0.0, 0.0, 0.0, 0.0);
	glClearDepth(1.0);
	
	find_ideal_mrd(&ideal_mrd_near, &ideal_mrd_far, 
		&next_to_near, &next_to_far);
	find_actual_mrd(&next_to_near, &next_to_far, 
		&actual_mrd_near, &actual_mrd_far);
	ideal_mrd = fmax(ideal_mrd_near, ideal_mrd_far);
	actual_mrd = fmax(actual_mrd_near, actual_mrd_far);
	big_enough_mrd = (actual_mrd >= 0.99 * ideal_mrd);
	small_enough_mrd = (actual_mrd <= 2.0 * ideal_mrd);

	pass &= big_enough_mrd;
	pass &= small_enough_mrd;
	pass &= check_slope_offsets(&ideal_mrd_near);

	/* Print the results */
	if (!big_enough_mrd)
	{
		printf("\tActual MRD is too small ");
		printf("(may cause incorrect results)\n");
	}
	if (!small_enough_mrd)
	{
		printf("\tActual MRD is too large ");
		printf("(may waste depth-buffer range)\n\n");
	}

	glGetIntegerv(GL_DEPTH_BITS, &dbits);
	printf("\tIdeal  MRD at near plane is ");
	log_mrd(ideal_mrd_near, dbits);
	printf("\tActual MRD at near plane is ");
	log_mrd(actual_mrd_near, dbits);
	printf("\tIdeal  MRD at infinity is ");
	log_mrd(ideal_mrd_far, dbits);
	printf("\tActual MRD at infinity is ");
	log_mrd(actual_mrd_far, dbits);
	printf("\n");

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
} /* piglit_display */
