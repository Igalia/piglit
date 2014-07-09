/* Copyright © 2013 Intel Corporation
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
 * \file piglit-util-test-pattern.h
 * This file declares classes and functions which can be utilized to draw
 * test patterns in to color, depth or stencil buffers.
 */

#include "piglit-util-gl.h"
#include "math.h"

namespace piglit_util_test_pattern
{
	/**
	 * There are two programs used to "manifest" an auxiliary buffer,
	 * turning it into visible colors: one for manifesting the stencil
	 * buffer, and one for manifesting the depth buffer.  This is the base
	 * class that they both derive from.
	 */
	class ManifestProgram
	{
	public:
		virtual void compile() = 0;
		virtual void run() = 0;
	};

	/**
	 * Program we use to manifest the stencil buffer.
	 *
	 * This program operates by repeatedly drawing over the entire buffer
	 * using the stencil function "EQUAL", and a different color each
	 * time.  This causes stencil values from 0 to 7 to manifest as colors
	 * (black, blue, green, cyan, red, magenta, yellow, white).
	 */
	class ManifestStencil : public ManifestProgram
	{
	public:
		virtual void compile();
		virtual void run();

	private:
		GLint prog;
		GLint color_loc;
		GLuint vertex_buf;
		GLuint vao;
	};

	/**
	 * Program we use to manifest the depth buffer.
	 *
	 * This program operates by repeatedly drawing over the entire buffer
	 * at decreasing depth values with depth test enabled; the stencil
	 * function is configured to "EQUAL" with a stencil op of "INCR", so
	 * that after a sample passes the depth test, its stencil value will
	 * be incremented and it will fail the stencil test on later draws.
	 * As a result, depth values from back to front will manifest as
	 * colors (black, blue, green, cyan, red, magenta, yellow, white).
	 */
	class ManifestDepth : public ManifestProgram
	{
	public:
		virtual void compile();
		virtual void run();

	private:
		GLint prog;
		GLint color_loc;
		GLint depth_loc;
		GLuint vertex_buf;
		GLuint vao;
	};

	/**
	 * There are three programs used to draw a test pattern, depending on
	 * whether we are testing the color buffer, the depth buffer, or the
	 * stencil buffer.  This is the base class that they all derive from.
	 */
	class TestPattern
	{
	public:
		virtual void compile() = 0;

		/**
		 * Draw the test pattern, applying the given projection matrix
		 * to vertex coordinates.  The projection matrix is in
		 * row-major order.
		 *
		 * If no projection transformation is needed, pass
		 * TestPattern::no_projection for \c proj.
		 */
		virtual void draw(const float (*proj)[4]) = 0;

		static const float no_projection[4][4];
	};

	/**
	 * Program we use to draw a test pattern into the color buffer.
	 *
	 * This program draws a grid of small disjoint triangles, each rotated
	 * at a different angle.  This ensures that the image will have a
	 * large number of edges at different angles, so that we'll thoroughly
	 * exercise antialiasing.
	 */
	class Triangles : public TestPattern
	{
	public:
		virtual void compile();
		virtual void draw(const float (*proj)[4]);

	protected:
		GLint prog;
		GLuint vertex_buf;
		GLuint vao;
		GLint proj_loc;
		GLint tri_num_loc;
		int num_tris;
	};


	/**
	 * Program we use to test that interpolation works properly.
	 *
	 * This program draws the same sequence of small triangles as the
	 * Triangles program, but it's capable of coloring the triangles in
	 * various ways based on the fragment program provided to the
	 * constructor.
	 *
	 * The fragment program has access to the following variables:
	 *
	 * - in vec3 barycentric_coords: barycentric coordinates of the
	 *   triangle being drawn, normally interpolated.
	 *
	 * - centroid in vec3 barycentric_coords_centroid: same as
	 *   barycentric_coords, but centroid interpolated.
	 *
	 * - in vec2 pixel_pos: pixel coordinate ((0,0) to (viewport_width,
	 *   viewport_height)), normally interpolated.
	 *
	 * - centroid in vec2 pixel_pos_centroid: same as pixel_pos, but
	 *   centroid interpolated.
	 */
	class InterpolationTestPattern : public Triangles
	{
	public:
		explicit InterpolationTestPattern(const char *frag);
		virtual void compile();
		virtual void draw(const float (*proj)[4]);

	private:
		const char *frag;
		GLint viewport_size_loc;
	};


	/**
	 * Program we use to draw a test pattern into the color buffer.
	 *
	 * This program draws a sequence of points with varied sizes. This ensures
	 * antialiasing works well with all point sizes.
	 */
	class Points : public TestPattern
	{
	public:
		virtual void compile();
		virtual void draw(const float (*proj)[4]);

	private:
		GLint prog;
		GLuint vao;
		GLint proj_loc;
		GLint depth_loc;
		GLint point_num_loc;
		GLuint vertex_buf;
		int num_points;
	};

	/**
	 * Program we use to draw a test pattern into the color buffer.
	 *
	 * This program draws a sequence of lines with varied width. This ensures
	 * antialiasing works well with all line widths.
	 */
	class Lines : public TestPattern
	{
	public:
		virtual void compile();
		virtual void draw(const float (*proj)[4]);

	private:
		GLint prog;
		GLuint vao;
		GLint proj_loc;
		GLint line_num_loc;
		GLuint vertex_buf;
		int num_lines;
	};

	/**
	 * Program we use to draw a test pattern into the depth and stencil
	 * buffers.
	 *
	 * This program draws a "sunburst" pattern consisting of 7 overlapping
	 * triangles, each at a different angle.  This ensures that the
	 * triangles overlap in a complex way, with the edges between them
	 * covering a a large number of different angles, so that we'll
	 * thoroughly exercise antialiasing.
	 *
	 * This program is further specialized into depth and stencil variants.
	 */
	class Sunburst : public TestPattern
	{
	public:
		Sunburst();

		virtual void compile();

		/**
		 * Type of color buffer being rendered into.  Should be one of
		 * the following enum values: GL_FLOAT,
		 * GL_UNSIGNED_NORMALIZED, GL_UNSIGNED_INT, or GL_INT.
		 *
		 * Defaults to GL_UNSIGNED_NORMALIZED.
		 */
		GLenum out_type;

		/**
		 * Whether or not the fragment shader should output a depth
		 * value.
		 *
		 * Defaults to false.
		 */
		bool compute_depth;

	protected:
		GLint prog;
		GLint rotation_loc;
		GLint vert_depth_loc;
		GLint frag_depth_loc;
		GLint proj_loc;
		GLint draw_colors_loc;
		GLuint vao;
		int num_tris;

	private:
		const char *get_out_type_glsl() const;

		GLuint vertex_buf;
	};

	/**
	 * Program that draws a test pattern into the color buffer.
	 *
	 * This program draws triangles using a variety of colors and
	 * gradients.
	 *
	 * This program is capable of drawing to floating point, integer, and
	 * unsigned integer framebuffers, controlled by the out_type
	 * constructor parameter, which should be GL_FLOAT,
	 * GL_UNSIGNED_NORMALIZED, GL_UNSIGNED_INT, or GL_INT.
	 */
	class ColorGradientSunburst : public Sunburst
	{
	public:
		explicit ColorGradientSunburst(GLenum out_type);

		virtual void draw(const float (*proj)[4]);

		void draw_with_scale_and_offset(const float (*proj)[4],
						float scale, float offset);
	};

	/**
	 * Program we use to draw a test pattern into the stencil buffer.
	 *
	 * The triangles in this sunburst are drawn back-to-front, using no
	 * depth testing.  Each triangle is drawn using a different stencil
	 * value.
	 */
	class StencilSunburst : public Sunburst
	{
	public:
		virtual void draw(const float (*proj)[4]);
	};

	/**
	 * Program we use to draw a test pattern into the depth buffer.
	 *
	 * The triangles in this sunburst are drawn at a series of different
	 * depth values, with depth testing enabled.  They are drawn in an
	 * arbitrary non-consecutive order, to verify that depth testing
	 * properly sorts the surfaces into front-to-back order.
	 *
	 * If the constructor parameter compute_depth is true, the depth value
	 * is determined using a fragment shader output.  If it is false, it
	 * is determined by the z value of the vertex shader gl_Position
	 * output.
	 */
	class DepthSunburst : public Sunburst
	{
	public:
		explicit DepthSunburst(bool compute_depth = false);

		virtual void draw(const float (*proj)[4]);
	};
}
