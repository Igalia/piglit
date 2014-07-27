/*
 * Copyright © 2012 Intel Corporation
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
 * \file common.cpp
 *
 * This file defines the functions which can be utilized to develop new
 * multisample test cases. Functions can be utilized to:
 *
 * - Draw a test image to default framebuffer.
 * - Initialize test_fbo with specified sample count.
 * - Draw a test image to test_fbo.
 * - Draw a reference image.
 * - Verify the accuracy of multisample antialiasing in FBO.
 *
 * Accuracy verification is done by rendering a scene consisting of
 * triangles that aren't perfectly aligned to pixel coordinates. Every
 * triangle in the scene is rendered using a solid color whose color
 * components are all 0.0 or 1.0.  The scene is renederd in two ways:
 *
 * - At normal resoluation, using MSAA.
 *
 * - At very high resolution ("supersampled" by a factor of 16 in both
 *   X and Y dimensions), without MSAA.
 *
 * Then, the supersampled image is scaled down to match the resolution
 * of the MSAA image, using a fragment shader to manually blend each
 * block of 16x16 pixels down to 1 pixel.  This produces a reference
 * image, which is then compared to the MSAA image to measure the
 * error introduced by MSAA.
 *
 * (Note: the supersampled image is actually larger than the maximum
 * texture size that GL 3.0 requires all implementations to support
 * (1024x1024), so it is actually done in 1024x1024 tiles that are
 * then stitched together to form the reference image).
 *
 * In the piglit window, the MSAA image appears on the left; the
 * reference image is on the right.
 *
 * For each color component of each pixel, if the reference image has
 * a value of exactly 0.0 or 1.0, that pixel is presumed to be
 * completely covered by a triangle, so the test verifies that the
 * corresponding pixel in the MSAA image is exactly 0.0 or 1.0.  Where
 * the reference image has a value between 0.0 and 1.0, we know there
 * is a triangle boundary that MSAA should smooth out, so the test
 * estimates the accuracy of MSAA rendering by computing the RMS error
 * between the reference image and the MSAA image for these pixels.
 *
 * In addition to the above test (the "color" test), there are functions
 * which can also verify the proper behavior of the stencil MSAA buffer.
 * This can be done in two ways:
 *
 * - "stencil_draw" test: after drawing the scene, we clear the MSAA
 *   color buffer and run a "manifest" pass which uses stencil
 *   operations to make a visual representation of the contents of the
 *   stencil buffer show up in the color buffer.  The rest of the test
 *   operates as usual.  This allows us to verify that drawing
 *   operations that use the stencil buffer operate correctly in MSAA
 *   mode.
 *
 * - "stencil_resolve" test: same as above, except that we blit the
 *   MSAA stencil buffer to a single-sampled FBO before running the
 *   "manifest" pass.  This allows us to verify that the
 *   implementation properly downsamples the MSAA stencil buffer.
 *
 * There are similar variants "depth_draw" and "depth_resolve" for
 * testing the MSAA depth buffer.
 *
 * Note that when downsampling the MSAA color buffer, implementations
 * are expected to blend the values of each of the color samples;
 * but when downsampling the stencil and depth buffers, they are
 * expected to just choose one representative sample (this is because
 * an intermediate stencil or depth value would not be meaningful).
 * Therefore, the pass threshold is relaxed for the "stencil_resolve"
 * and "depth_resolve" tests.
 *
 * Functions also accepts the following flags:
 *
 * - "small": Causes the MSAA image to be renedered in extremely tiny
 *   (16x16) tiles that are then stitched together.  This verifies
 *   that MSAA works properly on very small buffers (a critical corner
 *   case on i965).
 *
 * - "depthstencil": Causes the framebuffers to use a combined
 *   depth/stencil buffer (as opposed to separate depth and stencil
 *   buffers).  On some implementations (e.g. the nVidia proprietary
 *   driver for Linux) this is necessary for framebuffer completeness.
 *   On others (e.g. i965), this is an important corner case to test.
 */

#include "common.h"
using namespace piglit_util_fbo;
using namespace piglit_util_test_pattern;

void
DownsampleProg::compile(int supersample_factor)
{
	static const char *vert =
		"#version 120\n"
		"attribute vec2 pos;\n"
		"attribute vec2 texCoord;\n"
		"varying vec2 texCoordVarying;\n"
		"void main()\n"
		"{\n"
		"  gl_Position = vec4(pos, 0.0, 1.0);\n"
		"  texCoordVarying = texCoord;\n"
		"}\n";

	static const char *frag =
		"#version 120\n"
		"uniform sampler2DRect samp;\n"
		"uniform int supersample_factor;\n"
		"varying vec2 texCoordVarying;\n"
		"void main()\n"
		"{\n"
		"  vec4 sum = vec4(0.0);\n"
		"  vec2 pixel = floor(texCoordVarying);\n"
		"  for (int i = 0; i < supersample_factor; ++i) {\n"
		"    for (int j = 0; j < supersample_factor; ++j) {\n"
		"      sum += texture2DRect(\n"
		"          samp, pixel * float(supersample_factor) + vec2(i, j));\n"
		"    }\n"
		"  }\n"
		"  gl_FragColor = sum / (supersample_factor * supersample_factor);\n"
		"}\n";

	/* Compile program */
	prog = piglit_build_simple_program_unlinked(vert, frag);
	glBindAttribLocation(prog, 0, "pos");
	glBindAttribLocation(prog, 1, "texCoord");
	glLinkProgram(prog);
	if (!piglit_link_check_status(prog)) {
		piglit_report_result(PIGLIT_FAIL);
	}

	/* Set up uniforms */
	glUseProgram(prog);
	glUniform1i(glGetUniformLocation(prog, "supersample_factor"),
		    supersample_factor);
	glUniform1i(glGetUniformLocation(prog, "samp"), 0);

	/* Set up vertex array object */
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	/* Set up vertex input buffer */
	glGenBuffers(1, &vertex_buf);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buf);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float),
			      (void *) 0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float),
			      (void *) (2*sizeof(float)));

	/* Set up element input buffer to tessellate a quad into
	 * triangles
	 */
	unsigned int indices[6] = { 0, 1, 2, 0, 2, 3 };
	GLuint element_buf;
	glGenBuffers(1, &element_buf);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_buf);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices,
		     GL_STATIC_DRAW);
}

void
DownsampleProg::run(const Fbo *src_fbo, int dest_width, int dest_height,
		    bool srgb)
{
	float w = dest_width;
	float h = dest_height;

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_RECTANGLE, src_fbo->color_tex);

	glUseProgram(prog);
	glBindVertexArray(vao);

	float vertex_data[4][4] = {
		{ -1, -1, 0, 0 },
		{ -1,  1, 0, h },
		{  1,  1, w, h },
		{  1, -1, w, 0 }
	};
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buf);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_data), vertex_data,
		     GL_STREAM_DRAW);

	if (srgb) {
		/* If we're testing sRGB color, instruct OpenGL to
		 * convert the output of the fragment shader from
		 * linear color space to sRGB color space.
		 */
		glEnable(GL_FRAMEBUFFER_SRGB);
	}
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void *) 0);
	glDisable(GL_FRAMEBUFFER_SRGB);
}

Stats::Stats()
	: count(0), sum_squared_error(0.0)
{
}

void
Stats::summarize()
{
	printf("  count = %d\n", count);
	if (count != 0) {
		if (sum_squared_error != 0.0) {
			printf("  RMS error = %f\n",
			       sqrt(sum_squared_error / count));
		} else {
			printf("  Perfect output\n");
		}
	}
}

bool
Stats::is_perfect()
{
	return sum_squared_error == 0.0;
}

bool
Stats::is_better_than(double rms_error_threshold)
{
	return sqrt(sum_squared_error / count) < rms_error_threshold;
}

Test::Test(TestPattern *pattern, ManifestProgram *manifest_program,
	   bool test_resolve, GLbitfield blit_type, bool srgb)
	: pattern(pattern),
	  manifest_program(manifest_program),
	  test_resolve(test_resolve),
	  blit_type(blit_type),
	  num_samples(0),
	  pattern_width(0),
	  pattern_height(0),
	  supersample_factor(0),
	  srgb(srgb),
	  downsample_prog(),
	  filter_mode(GL_NONE)
{
}

void
Test::init(int num_samples, bool small, bool combine_depth_stencil,
	   int pattern_width, int pattern_height, int supersample_factor,
	   GLenum filter_mode)
{
	this->num_samples = num_samples;
	this->pattern_width = pattern_width;
	this->pattern_height = pattern_height;
	this->supersample_factor = supersample_factor;
	this->filter_mode = filter_mode;

	FboConfig test_fbo_config(0,
				  small ? 16 : pattern_width,
				  small ? 16 : pattern_height);
	if (srgb)
		test_fbo_config.color_internalformat = GL_SRGB8_ALPHA8;
	test_fbo_config.combine_depth_stencil = combine_depth_stencil;
	test_fbo.setup(test_fbo_config);

	FboConfig multisample_fbo_config = test_fbo_config;
	multisample_fbo_config.num_samples = num_samples;
	multisample_fbo.setup(multisample_fbo_config);

	resolve_fbo.setup(test_fbo_config);

	FboConfig supersample_fbo_config = test_fbo_config;
	supersample_fbo_config.width = 1024;
	supersample_fbo_config.height = 1024;
	supersample_fbo_config.attach_texture = true;
	supersample_fbo.setup(supersample_fbo_config);

	FboConfig downsample_fbo_config = test_fbo_config;
	downsample_fbo_config.width = 1024 / supersample_factor;
	downsample_fbo_config.height = 1024 / supersample_factor;
	downsample_fbo.setup(downsample_fbo_config);

	pattern->compile();
	downsample_prog.compile(supersample_factor);
	if (manifest_program)
		manifest_program->compile();

	/* Only do depth testing in those parts of the test where we
	 * explicitly want it
	 */
	glDisable(GL_DEPTH_TEST);
}

/**
 * Blit the data from multisample_fbo to resolve_fbo, forcing the
 * implementation to do an MSAA resolve.
 */
void
Test::resolve(Fbo *fbo, GLbitfield which_buffers)
{
	glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo->handle);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, resolve_fbo.handle);
	resolve_fbo.set_viewport();
	glBlitFramebuffer(0, 0, fbo->config.width, fbo->config.height,
			  0, 0, resolve_fbo.config.width,
			  resolve_fbo.config.height,
			  which_buffers, filter_mode);
}

/**
 * Use downsample_prog to blend 16x16 blocks of samples in
 * supersample_fbo, to produce a reference image in downsample_fbo.
 */
void
Test::downsample_color(int downsampled_width, int downsampled_height)
{

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, downsample_fbo.handle);
	downsample_fbo.set_viewport();
	downsample_prog.run(&supersample_fbo,
			    downsample_fbo.config.width,
			    downsample_fbo.config.height, srgb);
}

/**
 * Blit the color data from src_fbo to the given location in the
 * windowsystem buffer, so that the user can see it and we can read it
 * using glReadPixels.
 */
void
Test::show(Fbo *src_fbo, int x_offset, int y_offset)
{
	glBindFramebuffer(GL_READ_FRAMEBUFFER, src_fbo->handle);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, piglit_winsys_fbo);
	glViewport(0, 0, piglit_width, piglit_height);
	glBlitFramebuffer(0, 0, src_fbo->config.width, src_fbo->config.height,
			  x_offset, y_offset,
			  x_offset + src_fbo->config.width,
			  y_offset + src_fbo->config.height,
			  GL_COLOR_BUFFER_BIT, GL_NEAREST);
}

/**
 * Draw a portion of the test pattern by setting up an appropriate
 * projection matrix to map that portion of the test pattern to the
 * full FBO.
 */
void
Test::draw_pattern(int x_offset, int y_offset, int width, int height)
{
	/* Need a projection matrix such that:
	 * xc = ((xe + 1) * pattern_width/2 - x_offset) * 2/width - 1
	 * yc = ((ye + 1) * pattern_height/2 - y_offset) * 2/height - 1
	 * zc = ze
	 * wc = we = 1.0
	 *
	 * Therefore
	 * xc = pattern_width / width * xe
	 *    + pattern_width / width - x_offset * 2 / width - 1
	 * yc = pattern_height / height * ye
	 *    + pattern_height / height - y_offset * 2 / height - 1
	 * zc = ze
	 * wc = we = 1.0
	 */
	float x_scale = float(pattern_width) / width;
	float x_delta = x_scale - x_offset * 2.0 / width - 1.0;
	float y_scale = float(pattern_height) / height;
	float y_delta = y_scale - y_offset * 2.0 / height - 1.0;
	float proj[4][4] = {
		{ x_scale, 0, 0, x_delta },
		{ 0, y_scale, 0, y_delta },
		{ 0, 0, 1, 0 },
		{ 0, 0, 0, 1 }
	};

	pattern->draw(proj);
}

/**
 * Draw the entire test image, rendering it a piece at a time if
 * multisample_fbo is very small.
 */
void
Test::draw_test_image(Fbo *fbo)
{
	int num_h_tiles = pattern_width / fbo->config.width;
	int num_v_tiles = pattern_height / fbo->config.height;
	for (int h = 0; h < num_h_tiles; ++h) {
		for (int v = 0; v < num_v_tiles; ++v) {
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER,
					  fbo->handle);
			fbo->set_viewport();
			int x_offset = h * fbo->config.width;
			int y_offset = v * fbo->config.height;
			draw_pattern(x_offset, y_offset,
				     fbo->config.width,
				     fbo->config.height);
			if (test_resolve) {
				resolve(fbo, blit_type);
				if (manifest_program)
					manifest_program->run();
			} else {
				if (manifest_program)
					manifest_program->run();
				resolve(fbo,
					GL_COLOR_BUFFER_BIT);
			}

			show(&resolve_fbo, x_offset, y_offset);
		}
	}
}

/**
 * Draw the test image to the default framebuffer
 */
void
Test::draw_to_default_framebuffer()
{
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, piglit_winsys_fbo);
	glViewport(0, 0, pattern_width, pattern_height);
	draw_pattern(0, 0, pattern_width, pattern_height);
}

/**
 * Draw the entire test image, rendering it a piece at a time.
 */
void
Test::draw_reference_image()
{
	int downsampled_width =
		supersample_fbo.config.width / supersample_factor;
	int downsampled_height =
		supersample_fbo.config.height / supersample_factor;
	int num_h_tiles = pattern_width / downsampled_width;
	int num_v_tiles = pattern_height / downsampled_height;
	for (int h = 0; h < num_h_tiles; ++h) {
		for (int v = 0; v < num_v_tiles; ++v) {
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER,
					  supersample_fbo.handle);
			supersample_fbo.set_viewport();
			int x_offset = h * downsampled_width;
			int y_offset = v * downsampled_height;
			draw_pattern(x_offset, y_offset,
				     downsampled_width, downsampled_height);

			if (manifest_program)
				manifest_program->run();

			downsample_color(downsampled_width, downsampled_height);
			show(&downsample_fbo,
			     pattern_width + x_offset, y_offset);
		}
	}
}

/**
 * Measure the accuracy of MSAA downsampling.  Pixels that are fully
 * on or off in the reference image are required to be fully on or off
 * in the test image.  Pixels that are not fully on or off in the
 * reference image may be at any grayscale level; we mesaure the RMS
 * error between the reference image and the test image.
 */
bool
Test::measure_accuracy()
{
	bool pass = true;

	glBindFramebuffer(GL_READ_FRAMEBUFFER, piglit_winsys_fbo);
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, piglit_winsys_fbo);
			glViewport(0, 0, piglit_width, piglit_height);

	float *reference_data = new float[pattern_width * pattern_height * 4];
	glReadPixels(pattern_width, 0, pattern_width, pattern_height, GL_RGBA,
		     GL_FLOAT, reference_data);

	float *test_data = new float[pattern_width * pattern_height * 4];
	glReadPixels(0, 0, pattern_width, pattern_height, GL_RGBA,
		     GL_FLOAT, test_data);

	Stats unlit_stats;
	Stats partially_lit_stats;
	Stats totally_lit_stats;
	for (int y = 0; y < pattern_height; ++y) {
		for (int x = 0; x < pattern_width; ++x) {
			for (int c = 0; c < 4; ++c) {
				int pixel_pos = 4*(y*pattern_width + x) + c;
				float ref = reference_data[pixel_pos];
				float test = test_data[pixel_pos];
				/* When testing sRGB, compare pixels
				 * linearly so that the measured error
				 * is comparable to the non-sRGB case.
				 */
				if (srgb && c < 3) {
					ref = piglit_srgb_to_linear(ref);
					test = piglit_srgb_to_linear(test);
				}
				if (ref <= 0.0)
					unlit_stats.record(test - ref);
				else if (ref >= 1.0)
					totally_lit_stats.record(test - ref);
				else
					partially_lit_stats.record(test - ref);
			}
		}
	}

	printf("Pixels that should be unlit\n");
	unlit_stats.summarize();
	pass = unlit_stats.is_perfect() && pass;
	printf("Pixels that should be totally lit\n");
	totally_lit_stats.summarize();
	pass = totally_lit_stats.is_perfect() && pass;
	printf("Pixels that should be partially lit\n");
	partially_lit_stats.summarize();

	double error_threshold;
	if (test_resolve) {
		/* For depth and stencil resolves, the implementation
		 * typically just picks one of the N multisamples, so
		 * we have to allow for a generous amount of error.
		 */
		error_threshold = 0.4;
	} else {
		/* Empirically, the RMS error for no oversampling is
		 * about 0.25, and each additional factor of 2
		 * overampling reduces the error by a factor of about
		 * 0.6.  Leaving some room for variation, we'll set
		 * the error threshold to 0.333 * 0.6 ^
		 * log2(num_samples).
		 */
		int effective_num_samples = num_samples == 0 ? 1 : num_samples;
		error_threshold = 0.333 *
			pow(0.6, log((double)effective_num_samples) / log(2.0));
	}
	printf("The error threshold for this test is %f\n", error_threshold);
	pass = partially_lit_stats.is_better_than(error_threshold) && pass;
	// TODO: deal with sRGB.
	return pass;
}

bool
Test::run()
{
	draw_test_image(&multisample_fbo);
	draw_reference_image();
	return measure_accuracy();
}


Test *
create_test(test_type_enum test_type, int n_samples, bool small,
	    bool combine_depth_stencil, int pattern_width, int pattern_height,
	    int supersample_factor, GLenum filter_mode)
{
	Test *test = NULL;
	switch (test_type) {
	case TEST_TYPE_COLOR:
		test = new Test(new Triangles(), NULL, false, 0, false);
		break;
	case TEST_TYPE_SRGB:
		test = new Test(new Triangles(), NULL, false, 0, true);
		break;
	case TEST_TYPE_STENCIL_DRAW:
		test = new Test(new StencilSunburst(),
				new ManifestStencil(),
				false, 0, false);
		break;
	case TEST_TYPE_STENCIL_RESOLVE:
		test = new Test(new StencilSunburst(),
				new ManifestStencil(),
				true,
				GL_STENCIL_BUFFER_BIT, false);
		break;
	case TEST_TYPE_DEPTH_DRAW:
		test = new Test(new DepthSunburst(),
				new ManifestDepth(),
				false, 0, false);
		break;
	case TEST_TYPE_DEPTH_RESOLVE:
		test = new Test(new DepthSunburst(),
				new ManifestDepth(),
				true,
				GL_DEPTH_BUFFER_BIT, false);
		break;
	default:
		printf("Unrecognized test type\n");
		piglit_report_result(PIGLIT_FAIL);
		break;
	}

	test->init(n_samples, small, combine_depth_stencil, pattern_width,
		   pattern_height, supersample_factor, filter_mode);
	return test;
}
