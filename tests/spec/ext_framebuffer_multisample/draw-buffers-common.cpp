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

#include "draw-buffers-common.h"
using namespace piglit_util_fbo;

/**
 * \file draw-buffers-common.cpp
 *
 * This file provides utility functions to draw a test pattern to multiple draw
 * buffers attached to a FBO with GL_SAMPLE_ALPHA_TO_{COVERAGE, ONE}
 * enabled / disabled.
 *
 * Expected color values are computed for each draw buffer based on the enabled
 * GL_SAMPLE_ALPHA_TO_{COVERAGE, ONE} flags and coverage value used to draw the
 * test pattern.
 *
 * Reference image for each draw buffer is drawn in to right half of default
 * framebuffer. It is used to verify the accuracy of test image as well as to
 * visually compare the difference caused by enabling above flags.
 *
 * Test image is drawn with the same test pattern in multisample buffer with
 * GL_SAMPLE_ALPHA_TO_{COVERAGE, ONE} enabled. All multisample draw buffers
 * are sequentially resolved by  blitting them to a single sample FBO. resolve_fbo
 * is then blitted to left half of window system framebuffer with appropriate y
 * offset. This produces three test images in the left half, each corresponds to
 * a color attachment.
 *
 * Test image is verified by comparing it with the corresponding reference
 * image in the right half
 *
 * For sample coverage and sample alpha to coverage, test image should be
 * verified by probing the rectangles in left half of window system framebuffer
 * and comparing with expected color values. OpenGL 3.0 specification intends to
 * allow (but not require) the implementation to produce a dithering effect when
 * the coverage value is not a strict multiple of 1 / num_samples. We will skip
 * computing expected values and probing for such rectangles. They are drawn
 * just to look for dithering by human inspection.
 *
 * Note:
 * At present, the test always uses three draw buffers.  To test other
 * numbers of draw buffers, we would have to modify the fragment shader in
 * nontrivial ways at run time.
 *
 * Also, the test always uses GL_RGBA8I as integer format or GL_RGBA as float
 * format for draw buffer zero.
 *
 * Author: Anuj Phogat <anuj.phogat@gmail.com>
 */

static Fbo ms_fbo, resolve_fbo, resolve_int_fbo;
static GLbitfield buffer_to_test;

static float *coverage = NULL;
static float *color = NULL;
static float *depth = NULL;
static float *expected_color = NULL;
static float *expected_depth = NULL;

static int num_draw_buffers;
static int num_samples;
static int num_rects;
static int prog;
static int color_loc;
static int depth_loc;
static int frag_0_color_loc;
static int alpha_to_coverage_loc;
static int pattern_width;
static int pattern_height;

static bool is_buffer_zero_integer_format = false;
static bool is_dual_src_blending = false;
static GLenum draw_buffer_zero_format;

static const int num_components = 4; /* for RGBA formats */
static const int num_color_bits = 8; /* for GL_RGBA & GL_RGBA8I formats */

static const float bg_depth = 0.8;
static const float bg_color[4] = {
	0.0, 0.6, 0.0, 0.4 };

/* Testing for three draw buffers is supported */
static const GLenum draw_buffers[] = {
	GL_COLOR_ATTACHMENT0_EXT,
	GL_COLOR_ATTACHMENT1_EXT,
	GL_COLOR_ATTACHMENT2_EXT };

/* Offset the viewport transformation on depth value passed to the vertex
 * shader by setting it to (2 * depth - 1.0).
 */
static const char *vert_template =
	"#version %s\n"
	"attribute vec2 pos;\n"
	"uniform float depth;\n"
	"void main()\n"
	"{\n"
	"  vec4 eye_pos = gl_ModelViewProjectionMatrix * vec4(pos, 0.0, 1.0);\n"
	"  gl_Position = vec4(eye_pos.xy, 2 * depth - 1.0, 1.0);\n"
	"}\n";

/* Fragment shader generates three different color outputs. Different color
 * values are generated based on if sample_alpha_to_coverage / dual_src_blend
 * are enabled or not.
 */
static const char *frag_template =
	"#version %s\n"
	"#define NUM_ATTACHMENTS %d\n"
	"#define DUAL_SRC_BLEND %d\n"
	"#define ALPHA_TO_COVERAGE %d\n"
	"#define OUT_TYPE %s\n"
	"#define FRAG_OUT_ZERO_WRITE %d\n"
	"#if __VERSION__ == 130\n"
	"out OUT_TYPE frag_out_0;\n"
	"#if DUAL_SRC_BLEND\n"
	"out vec4 frag_out_1;\n"
	"#elif NUM_ATTACHMENTS > 1\n"
	"out vec4 frag_out_1;\n"
	"out vec4 frag_out_2;\n"
	"#endif\n"
	"#else\n"
	"#define frag_out_0 gl_FragData[0]\n"
	"#if NUM_ATTACHMENTS > 1\n"
	"#define frag_out_1 gl_FragData[1]\n"
	"#define frag_out_2 gl_FragData[2]\n"
	"#endif\n"
	"#endif\n"
	"uniform OUT_TYPE frag_0_color;\n"
	"uniform vec4 color;\n"
	"void main()\n"
	"{\n"
	"  #if FRAG_OUT_ZERO_WRITE\n"
	"    frag_out_0 = frag_0_color;\n"
	"  #endif\n"
	"  #if DUAL_SRC_BLEND\n"
	"    frag_out_1 = vec4(color.rgb, 1.0 - color.a / 2.0);\n"
	"  #elif ALPHA_TO_COVERAGE && NUM_ATTACHMENTS > 1\n"
	"    frag_out_1 = vec4(color.rgb, color.a / 2);\n"
	"    frag_out_2 = vec4(color.rgb, color.a / 4);\n"
	"  #elif NUM_ATTACHMENTS > 1\n"
	"    frag_out_1 = frag_out_2 = color;\n"
	"  #endif\n"
	"}\n";

const char *
get_out_type_glsl(void)
{
	if (is_buffer_zero_integer_format)
		return "ivec4";
	else
		return "vec4";
}
void
shader_compile(bool sample_alpha_to_coverage,
	       bool dual_src_blend,
	       bool frag_out_zero_write)
{
	bool need_glsl130 = is_buffer_zero_integer_format || dual_src_blend;

	if (need_glsl130) {
		piglit_require_gl_version(30);
	}

	is_dual_src_blending = dual_src_blend;

	/* Compile program */
	unsigned vert_alloc_len = strlen(vert_template) + 4;
	char *vert = (char *) malloc(vert_alloc_len);
	sprintf(vert, vert_template, need_glsl130 ? "130" : "120");
	GLint vs = piglit_compile_shader_text(GL_VERTEX_SHADER, vert);
	free(vert);

	/* Generate appropriate fragment shader program */
	const char *out_type_glsl = get_out_type_glsl();
	unsigned frag_alloc_len = strlen(frag_template) +
				  strlen(out_type_glsl) + 4;
	char *frag = (char *) malloc(frag_alloc_len);
	sprintf(frag, frag_template, need_glsl130 ? "130" : "120",
		num_draw_buffers,
		is_dual_src_blending,
		sample_alpha_to_coverage,
		out_type_glsl,
		frag_out_zero_write);

	GLint fs = piglit_compile_shader_text(GL_FRAGMENT_SHADER, frag);
	prog = piglit_link_simple_program(vs, fs);

	if (!piglit_link_check_status(prog)) {
		piglit_report_result(PIGLIT_FAIL);
	}
	free(frag);

	if (need_glsl130) {
		if (is_dual_src_blending) {
			glBindFragDataLocationIndexed(prog, 0, 0, "frag_out_0");
			glBindFragDataLocationIndexed(prog, 0, 1, "frag_out_1");

		}
		else if (num_draw_buffers > 1) {
			glBindFragDataLocation(prog, 0, "frag_out_0");
			glBindFragDataLocation(prog, 1, "frag_out_1");
			glBindFragDataLocation(prog, 2, "frag_out_2");
		}
		else
			glBindFragDataLocation(prog, 0, "frag_out_0");
	}

	glBindAttribLocation(prog, 0, "pos");
	glEnableVertexAttribArray(0);

	/* Linking is rquired after glBindFragDataLocation */
	glLinkProgram(prog);

	/* Set up uniforms */
	glUseProgram(prog);
	color_loc = glGetUniformLocation(prog, "color");
	depth_loc = glGetUniformLocation(prog, "depth");
	frag_0_color_loc = glGetUniformLocation(prog, "frag_0_color");
	alpha_to_coverage_loc = glGetUniformLocation(prog, "alphatocoverage");
}

void
allocate_data_arrays(void)
{
	float alpha_scale;
	/* Draw 2N + 1 rectangles for N samples, each with a unique color
	 * and coverage value
	 */
	if (num_samples) {
		num_rects = 2 * num_samples + 1;
		alpha_scale = (1.0 / (2.0 * num_samples));
	}
	else {
		num_rects = 9;
		alpha_scale = 0.125;
	}

	/* Allocate data arrays based on number of samples used */
	color = (float *) malloc(num_rects *
				 num_components *
				 sizeof(float));
	expected_color = (float *) malloc(num_draw_buffers *
				  num_rects *
				  num_components *
				  sizeof(float));
	depth = (float *) malloc(num_rects * sizeof(float));
	expected_depth = (float *) malloc(num_draw_buffers *
					  num_rects *
					  sizeof(float));
	coverage = (float *) malloc(num_rects * sizeof(float));

	for (int i = 0; i < num_rects; i++) {
		unsigned rect_idx = i * num_components;
		for (int j = 0; j < num_components - 1; j++) {
			color[rect_idx + j] =
			(sin((float)(rect_idx + j)) + 1) / 2;
		}

		/* In case of alpha-to-coverage enabled, alpha values will be
		 * directly used as coverage.
		 */
		if (buffer_to_test == GL_DEPTH_BUFFER_BIT)
			/* For depth buffer testing with alpha-to-coverage,
			 * set more rects with alpha = 1.0.
			 */
			color[rect_idx + 3] = 2 * i * alpha_scale;
		else
			color[rect_idx + 3] = i * alpha_scale;

		depth[i] =  i * (alpha_scale / 2.0);
	}
}

void
free_data_arrays(void)
{
	free(color);
	color = NULL;
	free(depth);
	depth = NULL;
	free(coverage);
	coverage = NULL;
	free(expected_color);
	expected_color = NULL;
}

void
float_color_to_int_color(int *dst, float *src)
{
	float offset = 1 - (1 << (num_color_bits - 1));
	float scale = -2.0 * offset;

	for (int j = 0; j < num_rects; ++j) {
		for (int k = 0; k < num_components; ++k) {
			dst[j * num_components + k] =
			scale * src[j * num_components + k] + offset;
		}
	}
}

void
draw_pattern(bool sample_alpha_to_coverage,
	     bool sample_alpha_to_one,
	     bool is_reference_image,
	     float *float_color)
{
	glUseProgram(prog);
	if (buffer_to_test == GL_COLOR_BUFFER_BIT)
		glClearColor(bg_color[0], bg_color[1],
			     bg_color[2], bg_color[3]);
	else if (buffer_to_test == GL_DEPTH_BUFFER_BIT)
		glClearDepth(bg_depth);
	glClear(buffer_to_test);

	if (!is_reference_image) {
		if (sample_alpha_to_coverage)
			glEnable(GL_SAMPLE_ALPHA_TO_COVERAGE);
		if (sample_alpha_to_one)
			glEnable(GL_SAMPLE_ALPHA_TO_ONE);
	}
	glUniform1i(alpha_to_coverage_loc, sample_alpha_to_coverage);

	unsigned indices[6] = {0, 1, 2, 0, 2, 3};
	int *integer_color = (int *) malloc(num_rects *
					    num_components *
					    sizeof(int));

	/* For integer color buffers convert the color data to integer format */
	if (is_buffer_zero_integer_format) {
		float_color_to_int_color(integer_color, float_color);
	}

	for (int i = 0; i < num_rects; ++i) {
		float vertices[4][2] = {
		{ 0.0f, 0.0f + i * (pattern_height / num_rects) },
		{ 0.0f, (i + 1.0f) * (pattern_height / num_rects) },
		{ pattern_width, (i + 1.0f) * (pattern_height / num_rects) },
		{ pattern_width, 0.0f + i * (pattern_height / num_rects) } };

		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE,
				      sizeof(vertices[0]),
				      (void *) vertices);

		glUniform4fv(color_loc, 1, (float_color + i * num_components));
		if (is_buffer_zero_integer_format) {
			glUniform4iv(frag_0_color_loc, 1,
				     integer_color + i * num_components);
		}
		else {
			glUniform4fv(frag_0_color_loc, 1,
				     (float_color + i * num_components));
		}
		glUniform1f(depth_loc, depth[i]);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT,
			       (void *) indices);
	}
	glDisable(GL_SAMPLE_ALPHA_TO_COVERAGE);
	glDisable(GL_SAMPLE_ALPHA_TO_ONE);
	free(integer_color);
}

float
get_alpha_blend_factor(float src0_alpha, float src1_alpha,
		       bool compute_src)
{
	GLint blend_func;
	if (compute_src)
		glGetIntegerv(GL_BLEND_SRC_RGB, &blend_func);
	else
		glGetIntegerv(GL_BLEND_DST_RGB, &blend_func);

	switch(blend_func) {
		case GL_SRC_ALPHA:
			return src0_alpha;
			break;
		case GL_ONE_MINUS_SRC_ALPHA:
			return (1.0 - src0_alpha);
			break;
		case GL_SRC1_ALPHA:
			return src1_alpha;
			break;
		case GL_ONE_MINUS_SRC1_ALPHA:
			return (1.0 - src1_alpha);
			break;
		default:
			printf("Blend function is not supported"
			       " by test case\n");
	}
	return -1;
}

void
compute_blend_color(float *frag_color, int rect_count,
		    bool sample_alpha_to_one)
{
	float src_blend_factor, dst_blend_factor;
	/* Taking in to account alpha values output by
	 * fragment shader.
	 */
	float src0_alpha = color[rect_count * num_components + 3];
	float src1_alpha =  1.0 - src0_alpha / 2.0;

	if (sample_alpha_to_one && num_samples) {
		/* Set fragment src0_alpha, src1_alpha to 1.0 and use them
		 * to compute blending factors.
		 */
		src0_alpha = 1.0;
		src1_alpha = 1.0;
	}

	src_blend_factor = get_alpha_blend_factor(src0_alpha,
						  src1_alpha,
						  true);
	dst_blend_factor = get_alpha_blend_factor(src0_alpha,
						  src1_alpha,
						  false);
	/* Using default BlendEquation, blend_color is:
	 * src0_color * src_blend_factor + dst_color * dst_blend_factor
	 */
	for (int j = 0; j < num_components; j++) {
		float blend_color=
		color[rect_count * num_components + j] *
		src_blend_factor +
		bg_color[j] *
		dst_blend_factor;

		frag_color[rect_count * num_components + j] =
			(blend_color > 1) ? 1.0 : blend_color;
	}
}

void
compute_expected_color(bool sample_alpha_to_coverage,
		       bool sample_alpha_to_one,
		       int draw_buffer_count)
{
	unsigned buffer_idx_offset = draw_buffer_count *
				     num_rects *
				     num_components;
	for (int i = 0; i < num_rects; i++) {

		float *frag_color = NULL;
		float samples_used = coverage[i] * num_samples;
		/* Expected color values are computed only for integer
		 * number of samples_used. Non-integer values may result
		 * in dithering effect.
		 */
		if (samples_used == (int) samples_used) {
			int rect_idx_offset = buffer_idx_offset +
					      i * num_components;
			frag_color = (float *) malloc(num_rects *
						      num_components *
						      sizeof(float));

			/* Do dual source blending computations */
			if (is_dual_src_blending) {
				compute_blend_color(frag_color,
						    i /* rect_count */,
						    sample_alpha_to_one);
			}
			else {
				memcpy(frag_color, color,
				       num_rects * num_components *
				       sizeof(float));
			}

			/* Coverage value decides the number of samples in
			 * multisample buffer covered by an incoming fragment,
			 * which will then receive the fragment data. When the
			 * multisample buffer is resolved it gets blended with
			 * the background color which is written to the
			 * remaining samples. Page 254 (page 270 of the PDF) of
			 * the OpenGL 3.0 spec says: "The method of combination
			 * is not specified, though a simple average computed
			 * independently for each color component is recommended."
			 * This is followed by NVIDIA and AMD in their proprietary
			 * linux drivers.
			 */
			for (int j = 0; j < num_components - 1 ; j++) {

				expected_color[rect_idx_offset + j] =
				frag_color[i * num_components + j] * coverage[i] +
				bg_color[j] * (1 - coverage[i]);
			}

			/* Compute expected alpha values of draw buffers */
			float frag_alpha = frag_color[i * num_components + 3];
			int alpha_idx = rect_idx_offset + 3;

			if ((!num_samples &&
			     !sample_alpha_to_coverage) ||
			    is_buffer_zero_integer_format) {
				/* Taking in to account alpha values output by
				 * fragment shader.
				 */
				expected_color[alpha_idx] =
					is_buffer_zero_integer_format ?
					frag_alpha / (1 << draw_buffer_count) :
					frag_alpha;
			}
			else if (sample_alpha_to_coverage) {
				/* Taking in to account alpha values output by
				 * fragment shader.
				 */
				frag_alpha /= (1 << draw_buffer_count);
				if (sample_alpha_to_one) {
					expected_color[alpha_idx] =
					1.0 * coverage[i] +
					bg_color[3] * (1 - coverage[i]);
				}
				else {
					expected_color[alpha_idx] =
					frag_alpha * coverage[i] +
					bg_color[3] * (1 - coverage[i]);
				}
			}
			else {
				expected_color[alpha_idx] =
					sample_alpha_to_one ? 1.0 : frag_alpha;
			}
		}
		free(frag_color);
	}

}

void
compute_expected_depth(void)
{
	/* Compute the expected depth values only for coverage value equal to
	 * 0.0 and 1.0. Expected depth is not defined by OpenGL specification
	 * when coverage value is between 0.0 and 1.0 */
	for (int i = 0; i < num_rects; i++) {
		if (coverage[i] == 0.0)
			expected_depth[i] = bg_depth;
		else if (coverage[i] == 1.0)
			expected_depth[i] = (depth[i] < 1.0) ? depth[i] : 1.0;
	}
}

void
compute_expected(bool sample_alpha_to_coverage,
		 bool sample_alpha_to_one,
		 int draw_buffer_count)
{
	int i;
	/* Compute the coverage value used in the test */
	if (num_samples &&
	    sample_alpha_to_coverage &&
	    !is_buffer_zero_integer_format) {

		for (i = 0; i < num_rects; i++) {
			/* Coverage value for all the draw buffers comes from
			 * the fragment alpha values of draw buffer zero
			 */
			float frag_alpha = color[i * num_components + 3];
			coverage[i] = (frag_alpha < 1.0) ? frag_alpha : 1.0;
		}
	}
	else {
		for (i = 0; i < num_rects; i++)
			coverage[i] = 1.0;
	}

	if (buffer_to_test == GL_COLOR_BUFFER_BIT) {
		/* Don't compute expected color for color buffer zero
		 * if no renderbuffer is attached to it.
		 */
		if (draw_buffer_count == 0 && draw_buffer_zero_format == GL_NONE)
			return;
		compute_expected_color(sample_alpha_to_coverage,
				       sample_alpha_to_one,
				       draw_buffer_count);
	}
	else if (buffer_to_test == GL_DEPTH_BUFFER_BIT)
		compute_expected_depth();

}

/* This function probes all the draw buffers blitted to downsampled FBO
 * (resolve_fbo / resolve_int_fbo) and compare against expected color values.
 */
bool
probe_framebuffer_color(void)
{
	bool result = true;
	int * expected_int_color = NULL;
	int rect_width = pattern_width;
	int rect_height = pattern_height / num_rects;

	for (int i = 0; i < num_draw_buffers; i++) {
		/* Don't probe color buffer zero if no renderbuffer is
		 * attached to it.
		 */
		if (i == 0 && draw_buffer_zero_format == GL_NONE)
			continue;
		bool is_integer_operation = is_buffer_zero_integer_format && !i;

		if (is_integer_operation) {
			glBindFramebuffer(GL_READ_FRAMEBUFFER,
					  resolve_int_fbo.handle);
			expected_int_color = (int*) malloc(num_rects *
							   num_components *
							   sizeof(int));
		}
		else {
			glBindFramebuffer(GL_READ_FRAMEBUFFER,
					  resolve_fbo.handle);
		}

		for (int j = 0; j < num_rects; j++) {
			float samples_used = coverage[j] * num_samples;
			int rect_x = 0;
			int rect_y = i * pattern_height +
				     j * rect_height;
			int rect_idx_offset = (i * num_rects + j) *
					      num_components;

			/* Only probe rectangles with coverage value which is a
			 * strict  multiple of 1 / num_samples.
			 */
			if (samples_used == (int)samples_used) {
				if (is_integer_operation) {
					float_color_to_int_color(expected_int_color,
								 expected_color);
					result = piglit_probe_rect_rgba_int(
						 rect_x,
						 rect_y,
						 rect_width,
						 rect_height,
						 expected_int_color +
						 rect_idx_offset)
						 && result;
				}
				else {
					result = piglit_probe_rect_rgba(
						 rect_x,
						 rect_y,
						 rect_width,
						 rect_height,
						 expected_color + rect_idx_offset)
						 && result;
				}
			}
		}
	}
	if (expected_int_color)
		free(expected_int_color);
	return result;
}

bool
probe_framebuffer_depth(void)
{
	bool result = true;
	int rect_width = pattern_width;
	int rect_height = pattern_height / num_rects;

	glBindFramebuffer(GL_READ_FRAMEBUFFER, resolve_fbo.handle);
	for (int i = 0; i < num_rects; i++) {
		if (coverage[i] == 0.0 || coverage[i] == 1.0) {
			int rect_x = 0;
			int rect_y = i * rect_height;
			int rect_idx = i;

			result = piglit_probe_rect_depth(
				 rect_x,
				 rect_y,
				 rect_width,
				 rect_height,
				 expected_depth[rect_idx])
				 && result;
		}
		else {
		/*Skip probing polygons which are drawn with fractional
		* coverage value (between 0.0 and 1.0)*/
		       continue;
		}
	}
	return result;
}

void
draw_image_to_window_system_fb(int draw_buffer_count, bool rhs)
{
	unsigned rect_x = 0;
	unsigned rect_y = draw_buffer_count * pattern_height;
	unsigned array_size = num_components * pattern_width * pattern_height;
	float *image = (float *) malloc(sizeof(float) * array_size);

	if (is_buffer_zero_integer_format && draw_buffer_count == 0) {
		glBindFramebuffer(GL_READ_FRAMEBUFFER, resolve_int_fbo.handle);
		int *tmp = (int *) malloc(sizeof(int) * array_size);
		glReadPixels(rect_x, rect_y,
			     pattern_width, pattern_height,
			     GL_RGBA_INTEGER,
			     GL_INT, tmp);
		for (unsigned i = 0; i < array_size; ++i) {
			image[i] = tmp[i];
		}

		/* Convert integer color data to float color data */
		float color_offset = 1.0 - (1 << (num_color_bits - 1));
		float color_scale = -2.0 * color_offset;

		for (unsigned i = 0; i < array_size; ++i) {
			image[i] = (image[i] - color_offset) / color_scale;
		}
		free(tmp);
	}
	else{
		glBindFramebuffer(GL_READ_FRAMEBUFFER, resolve_fbo.handle);
		glReadPixels(rect_x, rect_y,
			     pattern_width,
			     pattern_height,
			     GL_RGBA,
			     GL_FLOAT, image);
	}

	/* Rendering using gldrawPixels() with dual source blending enabled
	 * produces undefined results. So, disable blending in
	 * piglit_visualize_image function to avoid undefined behavior.
	 */
	GLboolean isBlending;
	glGetBooleanv(GL_BLEND, &isBlending);
	glDisable(GL_BLEND);
	piglit_visualize_image(image, GL_RGBA,
			       pattern_width, pattern_height,
			       draw_buffer_count + 1, rhs);
	if (isBlending)
		glEnable(GL_BLEND);
	free(image);
}

void
draw_test_image(bool sample_alpha_to_coverage, bool sample_alpha_to_one)
{
	/* Draw test pattern in multisample ms_fbo with
	 * GL_SAMPLE_ALPHA_TO_COVERAGE enabled
	 */
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, ms_fbo.handle);
	glDrawBuffers(num_draw_buffers, draw_buffers);
	ms_fbo.set_viewport();

	draw_pattern(sample_alpha_to_coverage,
		     sample_alpha_to_one,
		     false /* is_reference_image */,
		     color);

	for (int i = 0; i < num_draw_buffers; i++) {

		/* Blit ms_fbo to singlesample FBO to resolve multisample
		 * buffer.
		 */
		glBindFramebuffer(GL_READ_FRAMEBUFFER, ms_fbo.handle);
		if (buffer_to_test == GL_COLOR_BUFFER_BIT)
			glReadBuffer(GL_COLOR_ATTACHMENT0_EXT + i);

		if (is_buffer_zero_integer_format && !i)
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER,
					  resolve_int_fbo.handle);
		else
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER,
					  resolve_fbo.handle);

		/* Blit all the draw buffers to resolve_fbo / resolve_int_fbo
		 * with different y_offset.
		 */
		unsigned y_offset = i * pattern_height;
		glBlitFramebuffer(0, 0,
				  pattern_width, pattern_height,
				  0, y_offset,
				  pattern_width, pattern_height + y_offset,
				  buffer_to_test, GL_NEAREST);

		if (buffer_to_test == GL_COLOR_BUFFER_BIT) {
			draw_image_to_window_system_fb(i /* draw_buffer_count */,
						       false /* rhs */);
		}

		/* Expected color values for all the draw buffers are computed
		 * to aid probe_framebuffer_color() and probe_framebuffer_depth()
		 * in verification.
		 */
		if (sample_alpha_to_coverage || is_dual_src_blending) {
			/* Expected color is different for different draw
			 * buffers
			 */
			compute_expected(sample_alpha_to_coverage,
					 sample_alpha_to_one,
					 i /* draw_buffer_count */);
		}
	}
}

void
draw_reference_image(bool sample_alpha_to_coverage, bool sample_alpha_to_one)
{
	/* Draw test pattern in multisample ms_fbo with
	 * GL_SAMPLE_ALPHA_TO_COVERAGE disabled.
	 */
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, ms_fbo.handle);
	glDrawBuffers(num_draw_buffers, draw_buffers);
	ms_fbo.set_viewport();

	if (sample_alpha_to_coverage) {
		draw_pattern(sample_alpha_to_coverage,
			     sample_alpha_to_one,
			     true /* is_reference_image */,
			     color);
	}
	else {
		/* Value of draw_buffer_count doesn't matter in this case */
		compute_expected(sample_alpha_to_coverage,
				 sample_alpha_to_one,
				 0 /* draw_buffer_count */);
		draw_pattern(sample_alpha_to_coverage,
			     sample_alpha_to_one,
			     true /* is_reference_image */,
			     expected_color);
	}

	for (int i = 0; i < num_draw_buffers; i++) {

		/* Blit ms_fbo to resolve_fbo to resolve multisample buffer */
		glBindFramebuffer(GL_READ_FRAMEBUFFER, ms_fbo.handle);
		if (buffer_to_test == GL_COLOR_BUFFER_BIT)
			glReadBuffer(GL_COLOR_ATTACHMENT0_EXT + i);
		if (is_buffer_zero_integer_format && !i) {
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER,
					  resolve_int_fbo.handle);
		}
		else {
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER,
					  resolve_fbo.handle);
		}

		/* Blit all the draw buffers to resolve_fbo with different
		 * y_offset.
		 */
		unsigned y_offset = i * pattern_height;
		glBlitFramebuffer(0, 0,
				  pattern_width, pattern_height,
				  0, y_offset,
				  pattern_width, pattern_height + y_offset,
				  buffer_to_test, GL_NEAREST);

		if (buffer_to_test == GL_COLOR_BUFFER_BIT) {
			draw_image_to_window_system_fb(i /* draw_buffer_count */,
						       true /* rhs */);
		}
	}
}

void
ms_fbo_and_draw_buffers_setup(int samples,
			      int width,
			      int height,
			      int n_attachments,
			      GLenum test_buffer,
			      GLenum color_buffer_zero_format)
{
	int maxBuffers;
	glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &maxBuffers);

	/* Ensure that requested number of color attachments are
	 * supported by the implementation and fragment shader.
	 */
	if (n_attachments <= (int) ARRAY_SIZE(draw_buffers) &&
	    n_attachments <= maxBuffers)
		num_draw_buffers = n_attachments;
	else {
		printf("Number of attachments requested are not supported\n");
		piglit_report_result(PIGLIT_SKIP);
	}

	pattern_width = width;
	pattern_height = height;
	draw_buffer_zero_format = color_buffer_zero_format;

	/* Setup frame buffer objects with required configuration */
	FboConfig ms_config(samples, pattern_width, pattern_height);
	ms_config.color_internalformat = color_buffer_zero_format;
	ms_fbo.setup(ms_config);

	/* Create resolve_fbo with dimensions large enough to accomodate
	 * all the draw buffers
	 */
	FboConfig resolve_config(0, pattern_width,
				 num_draw_buffers * pattern_height);
	resolve_config.color_internalformat = GL_RGBA;
	resolve_fbo.setup(resolve_config);

	/* Create resolve_int_fbo to store downsampled integer draw buffer */
	if (color_buffer_zero_format == GL_RGBA8I) {
		resolve_config.color_internalformat = GL_RGBA8I;
		/* Assuming single integer buffer */
		resolve_config.height = pattern_height;
		resolve_int_fbo.setup(resolve_config);
		is_buffer_zero_integer_format = true;
	}
	else if (color_buffer_zero_format != GL_RGBA &&
		 color_buffer_zero_format != GL_NONE) {
		printf("Draw buffer zero format is not"
		       " supported by test functions.\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	if (!piglit_check_gl_error(GL_NO_ERROR)) {
		printf("Error setting up frame buffer objects\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	/* Query the number of samples used in ms_fbo. OpenGL implementation
	 * may create FBO with more samples per pixel than what is requested.
	 */
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, ms_fbo.handle);
	glGetIntegerv(GL_SAMPLES, &num_samples);

	/* Attach additional color buffers to multisample FBO with default
	 * non-integer format (GL_RGBA.)
	 */
	GLuint *color_rb = (GLuint *)malloc((num_draw_buffers - 1) *
					    sizeof(GLuint));
	glGenRenderbuffers(num_draw_buffers - 1, color_rb);

	for (int i = 0; i < num_draw_buffers - 1; i++) {
		glBindRenderbuffer(GL_RENDERBUFFER, color_rb[i]);
		glRenderbufferStorageMultisample(GL_RENDERBUFFER,
					ms_fbo.config.num_samples,
					GL_RGBA,
					ms_fbo.config.width,
					ms_fbo.config.height);

		glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER,
					  GL_COLOR_ATTACHMENT0 + (i + 1),
					  GL_RENDERBUFFER,
					  color_rb[i]);
	}

	GLenum status = glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE) {
		printf("Error attaching additional color buffers\n");
		piglit_report_result(PIGLIT_FAIL);
	}
	buffer_to_test = test_buffer;
	free(color_rb);
}
