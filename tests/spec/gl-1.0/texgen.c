/*
 * Copyright (C) 1999  Allen Akin   All Rights Reserved.
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
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY
 * KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 * PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL ALLEN AKIN BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
 * AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 */

/**
 * @file texgen.c:  Basic test of GL texture coordinate generation.
 * Author: Brian Sharp (brian@maniacal.org) December 2000
 *
 * This test does a basic test of the glTexGen functions, including
 * object_linear, eye_linear, and sphere_map.  We draw an icosahedron
 * and map a checkerboard texture onto it.
 * We use an ortho projection to keep it simple.  The result should be a 1:1
 * mapping of the check texture for all three modes (sphere map maps 1:1
 * because mapping it onto a sphere inverts the spheremap math).
 *
 * Note that accuracy issues might cause this test to fail if the
 * texcoords near the center are a little warped; I've specifically tried
 * to keep the matrices as "pure" as possible (no rotations) to
 * keep the numerical precision high.  So far it seems to work fine.
 * Introducing a rotation by 90 degrees about the x axis resulted,
 * on one driver, in a warping at the center of the sphere which caused
 * the test to fail.
 *
 * For the second test of the three, we offset the texture by 0.5,
 * so that each test's rendering is visually distinct from the
 * previous.
 *
 * To test for pass/fail we examine the color buffer for green and blue,
 * (the check colors) in the appropriate places.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;
	config.window_visual = PIGLIT_GL_VISUAL_RGB;
	config.khr_no_error_support = PIGLIT_NO_ERRORS;
	config.window_width = 50;
	config.window_height = 50;

PIGLIT_GL_TEST_CONFIG_END

/* Colors of checkerboard texture */
static float green[] = {0, 1, 0, 1};
static float blue[] = {0, 0, 1, 1};

static int num_vertices;
static int num_indices;
static float *vertices;
static float *normals;
static unsigned short *indices;

static void
generate_sphere(float radius, int slices, int stacks)
{
	int v = 0, n = 0, i = 0;
	/* Can't have a sphere of less than 2 slices or stacks. */
	assert(slices >= 2 && stacks >= 2);

	/* We have 2 verts for the top and bottom point, and then
	 * slices*(stacks-1) more for the middle rings (it's stacks-1 since
	 * the top and bottom points each count in the stack count). */
	num_vertices = 2 + (slices * (stacks - 1));
	vertices = malloc(sizeof(float) * num_vertices * 3);
	normals = malloc(sizeof(float) * num_vertices * 3);

	/* The top and bottom slices have <slices> tris in them, and the ones
	 * in the middle (since they're made of quads) have 2*<slices> each.
	 */
	num_indices = 3 * (2 * slices + 2 * (stacks - 2) * slices);
	indices = malloc(sizeof(unsigned short) * num_indices);

#define VX(i) vertices[3 * (i) + 0]
#define VY(i) vertices[3 * (i) + 1]
#define VZ(i) vertices[3 * (i) + 2]
#define VINDEX(st, sl) (3 * (1 + (((st)-1) * slices) + (sl)))
#ifndef M_PI
#define M_PI 3.14159
#endif

	/* Generate the verts.  The bottom and top verts are kind of special
	 * cases (they occupy the first and last vertex slots, respectively).
	 */
	vertices[v++] = 0;
	vertices[v++] = 0;
	vertices[v++] = -radius;
	normals[n++] = 0;
	normals[n++] = 0;
	normals[n++] = -1;

	/* Now the inner rings; I can't decide whether it spreads the tri area
	 * out better to do this by increments in the spherical coordinate phi
	 * or in the cartesian z, but I think phi is a better bet. */
	for (int cur_stack = 1; cur_stack < stacks; cur_stack++) {
		float phi = M_PI - ((cur_stack / (float)stacks) * M_PI);
		float zVal = cos(phi);
		float sliceRadius = sqrt(1.0 - zVal * zVal);
		for (int cur_slice = 0; cur_slice < slices; cur_slice++) {
			float theta = 2 * M_PI * ((float)cur_slice / slices);

			float xVal = sliceRadius * cos(theta);
			float yVal = sliceRadius * sin(theta);

			normals[v++] = xVal;
			normals[v++] = yVal;
			normals[v++] = zVal;
			vertices[n++] = xVal * radius;
			vertices[n++] = yVal * radius;
			vertices[n++] = zVal * radius;
		}
	}

	vertices[v++] = 0;
	vertices[v++] = 0;
	vertices[v++] = radius;
	normals[n++] = 0;
	normals[n++] = 0;
	normals[n++] = 1;

	/* Now to assemble them into triangles.  Do the top and bottom slices
	 * first. */
	for (int cur_slice = 0; cur_slice < slices; cur_slice++) {
		indices[i++] = 0;
		indices[i++] = (cur_slice + 1 % slices + 1);
		indices[i++] = cur_slice + 1;

		indices[i++] = num_vertices - 1;
		indices[i++] = num_vertices - 2 - ((cur_slice + 1 % slices));
		indices[i++] = num_vertices - 2 - cur_slice;
	}

	/* Now for the inner rings.  We're already done with 2*slices
	 * triangles, so start after that. */
	for (int cur_stack = 1; cur_stack < stacks - 1; cur_stack++) {
		for (int cur_slice = 0; cur_slice < slices; cur_slice++) {
			int nextStack = cur_stack + 1;
			int nextSlice = (cur_slice + 1) % slices;
			indices[i++] = VINDEX(cur_stack, cur_slice);
			indices[i++] = VINDEX(cur_stack, nextSlice);
			indices[i++] = VINDEX(nextStack, nextSlice);

			indices[i++] = VINDEX(cur_stack, cur_slice);
			indices[i++] = VINDEX(nextStack, nextSlice);
			indices[i++] = VINDEX(nextStack, cur_slice);
		}
	}

	assert(v == num_vertices * 3);
	assert(i == num_indices);

#undef VX
#undef VY
#undef VZ
#undef VINDEX
}

static void
render_sphere(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glBegin(GL_TRIANGLES);
	for (int i = 0; i < num_indices; ++i) {
		glNormal3fv(&normals[indices[i]]);
		glVertex3fv(&vertices[indices[i]]);
	}
	glEnd();
}

static bool
verify_checkers(float *upper_left, float *upper_right)
{
	/* It's a piglit_width x piglit_height pixel block; since we drew a
	 * sphere that doesn't quite touch the edges, we need to be careful
	 * not to sample from what should be background.  These pairs are
	 * hand-picked coordinates on the image that fall on the bottom-left
	 * quadrant of the sphere.
	 * XXX FIX ME: these sample coordinates assume that
	 * piglit_width == piglit_height == 50. */
	unsigned int samples[6][2] = {
		{13,13}, {4,22}, {22,4}, {20,20}, {20,10}, {10,20}
	};

	/* Run through those sample points in the bottom-left corner and make
	 * sure they're all the right color. */
	for (int samp=0; samp<6; samp++) {
		int x = samples[samp][0];
		int y = samples[samp][1];
		if (!piglit_probe_pixel_rgb(x, y, upper_right))
			return false;
	}

	/* Run through those sample points in the bottom-right corner and make
	 * sure they're all the right color.  Note the "piglit_width -
	 * samples[samp][0]" to flip it to the bottom-right quadrant. */
	for (int samp=0; samp<6; samp++) {
		int x = piglit_width - samples[samp][0];
		int y = samples[samp][1];
		if (!piglit_probe_pixel_rgb(x, y, upper_left))
			return false;
	}

	/* Run through those sample points in the upper-right corner and make
	 * sure they're all the right color. */
	for (int samp=0; samp<6; samp++) {
		int x = piglit_width - samples[samp][0];
		int y = piglit_height - samples[samp][1];
		if (!piglit_probe_pixel_rgb(x, y, upper_right))
			return false;
	}

	/* Run through those sample points in the upper-left corner and make
	 * sure they're all the right color. */
	for (int samp=0; samp<6; samp++) {
		int x = samples[samp][0];
		int y = piglit_height - samples[samp][1];
		if (!piglit_probe_pixel_rgb(x, y, upper_left))
			return false;
	}

	return true;
}

enum piglit_result
piglit_display(void)
{
	/* GL_SPHERE_MAP: with spheremap, the UL corner is blue */
	glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);
	glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP);

	render_sphere();

	if (!verify_checkers(blue, green)) {
		printf("GL_SPHERE_MAP\n");
		return PIGLIT_FAIL;
	}

	/* GL_OBJECT_LINEAR: with object linear and the below planes, the
	 * UL corner is green. */
	glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
	glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
	/* We flip the checker by setting W to 1.5 (phases by half a period)
	 */
	const float s_obj_plane[4] = {0, 0.05, 0, 1.5};
	const float t_obj_plane[4] = {0.05, 0, 0, 1};
	glTexGenfv(GL_S, GL_OBJECT_PLANE, s_obj_plane);
	glTexGenfv(GL_T, GL_OBJECT_PLANE, t_obj_plane);

	render_sphere();

	if (!verify_checkers(green, blue)) {
		printf("GL_OBJECT_LINEAR\n");
		return PIGLIT_FAIL;
	}

	/* GL_EYE_LINEAR: with eye linear and the below planes, the UL corner is blue. */
	glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
	glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
	const float s_eye_plane[4] = {0, 0.05, 0, 1};
	const float t_eye_plane[4] = {0.05, 0, 0, 1};
	glTexGenfv(GL_S, GL_EYE_PLANE, s_eye_plane);
	glTexGenfv(GL_T, GL_EYE_PLANE, t_eye_plane);

	render_sphere();

	if (!verify_checkers(blue, green)) {
		printf("GL_EYE_LINEAR\n");
		return PIGLIT_FAIL;
	}

	/* success */
	return PIGLIT_PASS;
}

void
piglit_init(int argc, char **argv)
{
	generate_sphere(9.9, 32, 16);

	/* Setup the projection. */
	piglit_gen_ortho_projection(-10, 10, -10, 10, -10, 10, false);

	/* Set up our texture. */
	glEnable(GL_TEXTURE_2D);
	GLuint checker_texture_handle;
	glGenTextures(1, &checker_texture_handle);
	glBindTexture(GL_TEXTURE_2D, checker_texture_handle);

	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glEnable(GL_TEXTURE_GEN_S);
	glEnable(GL_TEXTURE_GEN_T);

	piglit_checkerboard_texture(checker_texture_handle, 0, 256, 256, 128,
	                            128, green, blue);
}
