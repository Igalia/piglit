/* Copyright © 2012 Intel Corporation
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
 * \file common.h
 * This file declares functions which can be utilized to develop new multisample
 * test cases.
 */

#include "piglit-util.h"
#include "math.h"

enum test_type_enum {
	TEST_TYPE_COLOR,
	TEST_TYPE_STENCIL_DRAW,
	TEST_TYPE_STENCIL_RESOLVE,
	TEST_TYPE_DEPTH_DRAW,
	TEST_TYPE_DEPTH_RESOLVE,
};

/**
 * Information needed to configure a framebuffer object for MSAA
 * testing.
 */
class FboConfig
{
public:
	FboConfig(int num_samples, int width, int height);

	int num_samples;
	int width;
	int height;

	/**
	 * True if a single renderbuffer should be used as the backing
	 * store for both the depth and stencil attachment points.
	 * Defaults to true.
	 */
	bool combine_depth_stencil;

	/**
	 * True if a texture should be used as the backing store for
	 * the color attachment point, false if a renderbuffer should
	 * be used.  Defaults to false.
	 */
	bool attach_texture;
};

/**
 * Data structure representing one of the framebuffer objects used in
 * the test.
 *
 * For the supersampled framebuffer object we use a texture as the
 * backing store for the color buffer so that we can use a fragment
 * shader to blend down to the reference image.
 */
class Fbo
{
public:
	Fbo();

	void init(const FboConfig &initial_config);
	void generate_gl_objects();
	void set_samples(int num_samples);
	void setup();

	void set_viewport();

	FboConfig config;
	GLuint handle;

	/**
	 * If config.attach_texture is true, the backing store for the
	 * color buffer.
	 */
	GLuint color_tex;

	/**
	 * If config.attach_texture is false, the backing store for
	 * the color buffer.
	 */
	GLuint color_rb;

	/**
	 * If config.combine_depth_stencil is true, the backing store
	 * for the depth/stencil buffer.  If
	 * config.combine_depth_stencil is false, the backing store
	 * for the depth buffer.
	 */
	GLuint depth_rb;

	/**
	 * If config.combine_depth_stencil is false, the backing store
	 * for the stencil buffer.
	 */
	GLuint stencil_rb;
};

/**
 * Fragment shader program we apply to the supersampled color buffer
 * to produce the reference image.  This program manually blends each
 * 16x16 block of samples in the supersampled color buffer down to a
 * single sample in the downsampled buffer.
 */
class DownsampleProg
{
public:
	void compile(int supersample_factor);
	void run(const Fbo *src_fbo, int dest_width, int dest_height);

private:
	GLint prog;
	GLuint vertex_buf;
	GLuint vao;
};

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
	 */
	virtual void draw(float (*proj)[4]) = 0;
};

/**
 * Program we use to draw a test pattern into the color buffer.
 *
 * This program draws a sequence of small triangles, each rotated at a
 * different angle.  This ensures that the image will have a large
 * number of edges at different angles, so that we'll thoroughly
 * exercise antialiasing.
 */
class Triangles : public TestPattern
{
public:
	virtual void compile();
	virtual void draw(float (*proj)[4]);

private:
	GLint prog;
	GLuint vertex_buf;
	GLuint vao;
	GLint proj_loc;
	GLint tri_num_loc;
	int num_tris;
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
	virtual void draw(float (*proj)[4]);

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
	virtual void draw(float (*proj)[4]);

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
	virtual void compile();

protected:
	GLint prog;
	GLint rotation_loc;
	GLint depth_loc;
	GLint proj_loc;
	GLint draw_colors_loc;
	GLuint vao;
	int num_tris;

private:
	GLuint vertex_buf;
};

/**
 * Program that draws a test pattern into the color buffer.
 *
 * This program draws triangles using a variety of colors and
 * gradients.
 */
class ColorGradientSunburst : public Sunburst
{
public:
	virtual void draw(const float (*proj)[4]);
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
	virtual void draw(float (*proj)[4]);
};

/**
 * Program we use to draw a test pattern into the depth buffer.
 *
 * The triangles in this sunburst are drawn at a series of different
 * depth values, with depth testing enabled.  They are drawn in an
 * arbitrary non-consecutive order, to verify that depth testing
 * properly sorts the surfaces into front-to-back order.
 */
class DepthSunburst : public Sunburst
{
public:
	virtual void draw(float (*proj)[4]);
};

/**
 * Data structure for keeping track of statistics on pixel accuracy.
 *
 * We keep track of the number of pixels tested, and the sum of the
 * squared error, so that we can summarize the RMS error at the
 * conclusion of the test.
 */
class Stats
{
public:
	Stats();

	void record(float error)
	{
		++count;
		sum_squared_error += error * error;
	}

	void summarize();

	bool is_perfect();

	bool is_better_than(double rms_error_threshold);

private:
	int count;
	double sum_squared_error;
};

/**
 * This data structure wraps up all the data we need to keep track of
 * to run the test.
 */
class Test
{
public:
	Test(TestPattern *pattern, ManifestProgram *manifest_program,
	     bool test_resolve, GLbitfield blit_type);
	void init(int num_samples, bool small, bool combine_depth_stencil,
		  int pattern_width, int pattern_height,
		  int supersample_factor);
	bool run();
	void draw_test_image(Fbo *fbo);
	void draw_to_default_framebuffer();
	void draw_reference_image();
	bool measure_accuracy();

	/**
	 * Fbo that we use to just draw test image
	 */
	Fbo test_fbo;

private:
	void resolve(Fbo *fbo, GLbitfield which_buffers);
	void downsample_color(int downsampled_width, int downsampled_height);
	void show(Fbo *src_fbo, int x_offset, int y_offset);
	void draw_pattern(int x_offset, int y_offset, int width, int height);

	/** The test pattern to draw. */
	TestPattern *pattern;

	/**
	 * The program to use to manifest depth or stencil into color,
	 * or NULL if we're just testing color rendering.
	 */
	ManifestProgram *manifest_program;

	/**
	 * True if we are testing the resolve pass, so we should
	 * downsample before manifesting; false if we should manifest
	 * before downsampling.
	 */
	bool test_resolve;

	/**
	 * The buffer under test--this should be compatible with the
	 * "mask" argument of
	 * glBlitFramebuffer--i.e. GL_COLOR_BUFFER_BIT,
	 * GL_STENCIL_BUFFER_BIT, or GL_DEPTH_BUFFER_BIT.
	 */
	GLbitfield blit_type;

	/**
	 * Fbo that we perform MSAA rendering into.
	 */
	Fbo multisample_fbo;

	/**
	 * Single-sampled fbo that we blit into to force the
	 * implementation to resolve MSAA buffers.
	 */
	Fbo resolve_fbo;

	/**
	 * Large fbo that we perform high-resolution ("supersampled")
	 * rendering into.
	 */
	Fbo supersample_fbo;

	/**
	 * Normal-sized fbo that we manually downsample the
	 * supersampled render result into, to create the reference
	 * image.
	 */
	Fbo downsample_fbo;

	int num_samples;
	int pattern_width;
	int pattern_height;
	int supersample_factor;
	DownsampleProg downsample_prog;
};

Test *
create_test(test_type_enum test_type, int n_samples, bool small,
	    bool combine_depth_stencil, int pattern_width,
	    int pattern_height, int supersample_factor);
