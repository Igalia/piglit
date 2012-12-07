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

FboConfig::FboConfig(int num_samples, int width, int height)
	: num_samples(num_samples),
	  width(width),
	  height(height),
	  combine_depth_stencil(true),
	  attach_texture(false),
	  color_internalformat(GL_RGBA),
	  depth_internalformat(GL_DEPTH_COMPONENT24),
	  stencil_internalformat(GL_STENCIL_INDEX8)
{
}

Fbo::Fbo()
	: config(0, 0, 0), /* will be overwritten on first call to setup() */
	  gl_objects_generated(false)
{
}

void
Fbo::generate_gl_objects(void)
{
	glGenFramebuffers(1, &handle);
	glGenTextures(1, &color_tex);
	glGenRenderbuffers(1, &color_rb);
	glGenRenderbuffers(1, &depth_rb);
	glGenRenderbuffers(1, &stencil_rb);
	gl_objects_generated = true;
}

void
Fbo::set_samples(int num_samples)
{
	FboConfig new_config = this->config;
	new_config.num_samples = num_samples;
	setup(new_config);
}

/**
 * Modify the state of the framebuffer object to reflect the state in
 * new_config.  if the resulting framebuffer is incomplete, terminate
 * the test.
 */
void
Fbo::setup(const FboConfig &new_config)
{
	if (!try_setup(new_config)) {
		printf("Framebuffer not complete\n");
		if (!config.combine_depth_stencil) {
			/* Some implementations do not support
			 * separate depth and stencil attachments, so
			 * don't consider it an error if we fail to
			 * make a complete framebuffer using separate
			 * depth and stencil attachments.
			 */
			piglit_report_result(PIGLIT_SKIP);
		} else {
			piglit_report_result(PIGLIT_FAIL);
		}
	}
}


/**
 * Modify the state of the framebuffer object to reflect the state in
 * config.  Return true if the resulting framebuffer is complete,
 * false otherwise.
 */
bool
Fbo::try_setup(const FboConfig &new_config)
{
	this->config = new_config;

	if (!gl_objects_generated)
		generate_gl_objects();

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, handle);

	/* Color buffer */
	if (config.color_internalformat != GL_NONE) {
		if (!config.attach_texture) {
			glBindRenderbuffer(GL_RENDERBUFFER, color_rb);
			glRenderbufferStorageMultisample(GL_RENDERBUFFER,
							 config.num_samples,
							 config.color_internalformat,
							 config.width,
							 config.height);
			glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER,
						  GL_COLOR_ATTACHMENT0,
						  GL_RENDERBUFFER, color_rb);
		} else {
			glBindTexture(GL_TEXTURE_2D, color_tex);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
					GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
					GL_NEAREST);
			glTexImage2D(GL_TEXTURE_2D,
				     0 /* level */,
				     config.color_internalformat,
				     config.width,
				     config.height,
				     0 /* border */,
				     GL_RGBA /* format */,
				     GL_BYTE /* type */,
				     NULL /* data */);
			glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER,
					       GL_COLOR_ATTACHMENT0,
					       GL_TEXTURE_2D,
					       color_tex,
					       0 /* level */);
		}
	}

	/* Depth/stencil buffer(s) */
	if (config.combine_depth_stencil) {
		glBindRenderbuffer(GL_RENDERBUFFER, depth_rb);
		glRenderbufferStorageMultisample(GL_RENDERBUFFER,
						 config.num_samples,
						 GL_DEPTH_STENCIL,
						 config.width,
						 config.height);
		glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER,
					  GL_DEPTH_STENCIL_ATTACHMENT,
					  GL_RENDERBUFFER, depth_rb);
	} else {
		if (config.stencil_internalformat != GL_NONE) {
			glBindRenderbuffer(GL_RENDERBUFFER, stencil_rb);
			glRenderbufferStorageMultisample(GL_RENDERBUFFER,
							 config.num_samples,
							 config.stencil_internalformat,
							 config.width,
							 config.height);
			glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER,
						  GL_STENCIL_ATTACHMENT,
						  GL_RENDERBUFFER, stencil_rb);
		}

		if (config.depth_internalformat != GL_NONE) {
			glBindRenderbuffer(GL_RENDERBUFFER, depth_rb);
			glRenderbufferStorageMultisample(GL_RENDERBUFFER,
							 config.num_samples,
							 config.depth_internalformat,
							 config.width,
							 config.height);
			glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER,
						  GL_DEPTH_ATTACHMENT,
						  GL_RENDERBUFFER, depth_rb);
		}
	}

	bool success = glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER)
		== GL_FRAMEBUFFER_COMPLETE;

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

	return success;
}



void
Fbo::set_viewport()
{
	glViewport(0, 0, config.width, config.height);
}

void
DownsampleProg::compile(int supersample_factor)
{
	static const char *vert =
		"#version 130\n"
		"in vec2 pos;\n"
		"in vec2 texCoord;\n"
		"out vec2 texCoordVarying;\n"
		"void main()\n"
		"{\n"
		"  gl_Position = vec4(pos, 0.0, 1.0);\n"
		"  texCoordVarying = texCoord;\n"
		"}\n";

	static const char *frag =
		"#version 130\n"
		"uniform sampler2D samp;\n"
		"uniform int supersample_factor;\n"
		"in vec2 texCoordVarying;\n"
		"void main()\n"
		"{\n"
		"  vec4 sum = vec4(0.0);\n"
		"  ivec2 pixel = ivec2(texCoordVarying);\n"
		"  for (int i = 0; i < supersample_factor; ++i) {\n"
		"    for (int j = 0; j < supersample_factor; ++j) {\n"
		"      sum += texelFetch(\n"
		"          samp, pixel * supersample_factor + ivec2(i, j), 0);\n"
		"    }\n"
		"  }\n"
		"  gl_FragColor = sum / (supersample_factor * supersample_factor);\n"
		"}\n";

	/* Compile program */
	prog = glCreateProgram();
	GLint vs = piglit_compile_shader_text(GL_VERTEX_SHADER, vert);
	glAttachShader(prog, vs);
	GLint fs = piglit_compile_shader_text(GL_FRAGMENT_SHADER, frag);
	glAttachShader(prog, fs);
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

	/* Set up element input buffer to tesselate a quad into
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
	glBindTexture(GL_TEXTURE_2D, src_fbo->color_tex);

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

void
ManifestStencil::compile()
{
	static const char *vert =
		"#version 130\n"
		"in vec2 pos;\n"
		"void main()\n"
		"{\n"
		"  gl_Position = vec4(pos, 0.0, 1.0);\n"
		"}\n";

	static const char *frag =
		"#version 130\n"
		"uniform vec4 color;\n"
		"void main()\n"
		"{\n"
		"  gl_FragColor = color;\n"
		"}\n";

	/* Compile program */
	prog = glCreateProgram();
	GLint vs = piglit_compile_shader_text(GL_VERTEX_SHADER, vert);
	glAttachShader(prog, vs);
	GLint fs = piglit_compile_shader_text(GL_FRAGMENT_SHADER, frag);
	glAttachShader(prog, fs);
	glBindAttribLocation(prog, 0, "pos");
	glLinkProgram(prog);
	if (!piglit_link_check_status(prog)) {
		piglit_report_result(PIGLIT_FAIL);
	}

	/* Set up uniforms */
	glUseProgram(prog);
	color_loc = glGetUniformLocation(prog, "color");

	/* Set up vertex array object */
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	/* Set up vertex input buffer */
	float vertex_data[4][2] = {
		{ -1, -1 },
		{ -1,  1 },
		{  1,  1 },
		{  1, -1 }
	};
	glGenVertexArrays(1, &vertex_buf);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buf);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_data), vertex_data,
		     GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(vertex_data[0]),
			      (void *) 0);

	/* Set up element input buffer to tesselate a quad into
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
ManifestStencil::run()
{
	static float colors[8][4] = {
		{ 0.0, 0.0, 0.0, 1.0 },
		{ 0.0, 0.0, 1.0, 1.0 },
		{ 0.0, 1.0, 0.0, 1.0 },
		{ 0.0, 1.0, 1.0, 1.0 },
		{ 1.0, 0.0, 0.0, 1.0 },
		{ 1.0, 0.0, 1.0, 1.0 },
		{ 1.0, 1.0, 0.0, 1.0 },
		{ 1.0, 1.0, 1.0, 1.0 }
	};

	glUseProgram(prog);
	glBindVertexArray(vao);

	glEnable(GL_STENCIL_TEST);
	glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

	/* Clear the color buffer to 0, in case the stencil buffer
	 * contains any values outside the range 0..7
	 */
	glClear(GL_COLOR_BUFFER_BIT);

	for (int i = 0; i < 8; ++i) {
		glStencilFunc(GL_EQUAL, i, 0xff);
		glUniform4fv(color_loc, 1, colors[i]);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void *) 0);
	}

	glDisable(GL_STENCIL_TEST);
}

void
ManifestDepth::compile()
{
	static const char *vert =
		"#version 130\n"
		"in vec2 pos;\n"
		"uniform float depth;\n"
		"void main()\n"
		"{\n"
		"  gl_Position = vec4(pos, depth, 1.0);\n"
		"}\n";

	static const char *frag =
		"#version 130\n"
		"uniform vec4 color;\n"
		"void main()\n"
		"{\n"
		"  gl_FragColor = color;\n"
		"}\n";

	/* Compile program */
	prog = glCreateProgram();
	GLint vs = piglit_compile_shader_text(GL_VERTEX_SHADER, vert);
	glAttachShader(prog, vs);
	GLint fs = piglit_compile_shader_text(GL_FRAGMENT_SHADER, frag);
	glAttachShader(prog, fs);
	glBindAttribLocation(prog, 0, "pos");
	glLinkProgram(prog);
	if (!piglit_link_check_status(prog)) {
		piglit_report_result(PIGLIT_FAIL);
	}

	/* Set up uniforms */
	glUseProgram(prog);
	color_loc = glGetUniformLocation(prog, "color");
	depth_loc = glGetUniformLocation(prog, "depth");

	/* Set up vertex array object */
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	/* Set up vertex input buffer */
	float vertex_data[4][2] = {
		{ -1, -1 },
		{ -1,  1 },
		{  1,  1 },
		{  1, -1 }
	};
	glGenVertexArrays(1, &vertex_buf);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buf);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_data), vertex_data,
		     GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(vertex_data[0]),
			      (void *) 0);

	/* Set up element input buffer to tesselate a quad into
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
ManifestDepth::run()
{
	static float colors[8][4] = {
		{ 0.0, 0.0, 0.0, 1.0 },
		{ 0.0, 0.0, 1.0, 1.0 },
		{ 0.0, 1.0, 0.0, 1.0 },
		{ 0.0, 1.0, 1.0, 1.0 },
		{ 1.0, 0.0, 0.0, 1.0 },
		{ 1.0, 0.0, 1.0, 1.0 },
		{ 1.0, 1.0, 0.0, 1.0 },
		{ 1.0, 1.0, 1.0, 1.0 }
	};

	glUseProgram(prog);
	glBindVertexArray(vao);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glEnable(GL_STENCIL_TEST);
	glStencilOp(GL_KEEP, GL_KEEP, GL_INCR);
	glStencilFunc(GL_EQUAL, 0, 0xff);

	/* Clear the stencil buffer to 0, leaving depth and color
	 * buffers unchanged.
	 */
	glClear(GL_STENCIL_BUFFER_BIT);

	for (int i = 0; i < 8; ++i) {
		glUniform4fv(color_loc, 1, colors[i]);
		glUniform1f(depth_loc, float(7 - 2*i)/8);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void *) 0);
	}

	glDisable(GL_STENCIL_TEST);
	glDisable(GL_DEPTH_TEST);
}


const float TestPattern::no_projection[4][4] = {
	{ 1, 0, 0, 0 },
	{ 0, 1, 0, 0 },
	{ 0, 0, 1, 0 },
	{ 0, 0, 0, 1 }
};


void Triangles::compile()
{
	/* Triangle coords within (-1,-1) to (1,1) rect */
	static const float pos_within_tri[][2] = {
		{ -0.5, -1.0 },
		{  0.0,  1.0 },
		{  0.5, -1.0 }
	};

	/* Number of triangle instances across (and down) */
	int tris_across = 8;

	/* Total number of triangles drawn */
	num_tris = tris_across * tris_across;

	/* Scaling factor uniformly applied to triangle coords */
	float tri_scale = 0.8 / tris_across;

	/* Amount each triangle should be rotated compared to prev */
	float rotation_delta = M_PI * 2.0 / num_tris;

	/* Final scaling factor */
	float final_scale = 0.95;

	static const char *vert =
		"#version 130\n"
		"in vec2 pos_within_tri;\n"
		"uniform float tri_scale;\n"
		"uniform float rotation_delta;\n"
		"uniform int tris_across;\n"
		"uniform float final_scale;\n"
		"uniform mat4 proj;\n"
		"uniform int tri_num; /* [0, num_tris) */\n"
		"\n"
		"void main()\n"
		"{\n"
		"  vec2 pos = tri_scale * pos_within_tri;\n"
		"  float rotation = rotation_delta * tri_num;\n"
		"  pos = mat2(cos(rotation), sin(rotation),\n"
		"             -sin(rotation), cos(rotation)) * pos;\n"
		"  int i = tri_num % tris_across;\n"
		"  int j = tris_across - 1 - tri_num / tris_across;\n"
		"  pos += (vec2(i, j) * 2.0 + 1.0) / tris_across - 1.0;\n"
		"  pos *= final_scale;\n"
		"  gl_Position = proj * vec4(pos, 0.0, 1.0);\n"
		"}\n";

	static const char *frag =
		"#version 130\n"
		"void main()\n"
		"{\n"
		"  gl_FragColor = vec4(1.0);\n"
		"}\n";

	/* Compile program */
	prog = glCreateProgram();
	GLint vs = piglit_compile_shader_text(GL_VERTEX_SHADER, vert);
	glAttachShader(prog, vs);
	GLint fs = piglit_compile_shader_text(GL_FRAGMENT_SHADER, frag);
	glAttachShader(prog, fs);
	glBindAttribLocation(prog, 0, "pos_within_tri");
	glLinkProgram(prog);
	if (!piglit_link_check_status(prog)) {
		piglit_report_result(PIGLIT_FAIL);
	}

	/* Set up uniforms */
	glUseProgram(prog);
	glUniform1f(glGetUniformLocation(prog, "tri_scale"), tri_scale);
	glUniform1f(glGetUniformLocation(prog, "rotation_delta"),
		    rotation_delta);
	glUniform1i(glGetUniformLocation(prog, "tris_across"), tris_across);
	glUniform1f(glGetUniformLocation(prog, "final_scale"), final_scale);
	proj_loc = glGetUniformLocation(prog, "proj");
	tri_num_loc = glGetUniformLocation(prog, "tri_num");

	/* Set up vertex array object */
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	/* Set up vertex input buffer */
	glGenBuffers(1, &vertex_buf);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buf);
	glBufferData(GL_ARRAY_BUFFER, sizeof(pos_within_tri), pos_within_tri,
		     GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, ARRAY_SIZE(pos_within_tri[0]), GL_FLOAT,
			      GL_FALSE, sizeof(pos_within_tri[0]), (void *) 0);
}

void Triangles::draw(const float (*proj)[4])
{
	glClear(GL_COLOR_BUFFER_BIT);

	glUseProgram(prog);
	glUniformMatrix4fv(proj_loc, 1, GL_TRUE, &proj[0][0]);
	glBindVertexArray(vao);
	for (int tri_num = 0; tri_num < num_tris; ++tri_num) {
		glUniform1i(tri_num_loc, tri_num);
		glDrawArrays(GL_TRIANGLES, 0, 3);
	}
}


InterpolationTestPattern::InterpolationTestPattern(const char *frag)
	: frag(frag), viewport_size_loc(0)
{
}


void
InterpolationTestPattern::compile()
{
	static struct vertex_attributes {
		float pos_within_tri[2];
		float barycentric_coords[3];
	} vertex_data[] = {
		{ { -0.5, -1.0 }, { 1, 0, 0 } },
		{ {  0.0,  1.0 }, { 0, 1, 0 } },
		{ {  0.5, -1.0 }, { 0, 0, 1 } }
	};

	/* Number of triangle instances across (and down) */
	int tris_across = 8;

	/* Total number of triangles drawn */
	num_tris = tris_across * tris_across;

	/* Scaling factor uniformly applied to triangle coords */
	float tri_scale = 0.8 / tris_across;

	/* Amount each triangle should be rotated compared to prev */
	float rotation_delta = M_PI * 2.0 / num_tris;

	/* Final scaling factor */
	float final_scale = 0.95;

	static const char *vert =
		"#version 130\n"
		"in vec2 pos_within_tri;\n"
		"in vec3 in_barycentric_coords;\n"
		"out vec3 barycentric_coords;\n"
		"centroid out vec3 barycentric_coords_centroid;\n"
		"out vec2 pixel_pos;\n"
		"centroid out vec2 pixel_pos_centroid;\n"
		"uniform float tri_scale;\n"
		"uniform float rotation_delta;\n"
		"uniform int tris_across;\n"
		"uniform float final_scale;\n"
		"uniform mat4 proj;\n"
		"uniform int tri_num; /* [0, num_tris) */\n"
		"uniform ivec2 viewport_size;\n"
		"\n"
		"void main()\n"
		"{\n"
		"  vec2 pos = tri_scale * pos_within_tri;\n"
		"  float rotation = rotation_delta * tri_num;\n"
		"  pos = mat2(cos(rotation), sin(rotation),\n"
		"             -sin(rotation), cos(rotation)) * pos;\n"
		"  int i = tri_num % tris_across;\n"
		"  int j = tris_across - 1 - tri_num / tris_across;\n"
		"  pos += (vec2(i, j) * 2.0 + 1.0) / tris_across - 1.0;\n"
		"  pos *= final_scale;\n"
		"  gl_Position = proj * vec4(pos, 0.0, 1.0);\n"
		"  barycentric_coords = barycentric_coords_centroid =\n"
		"    in_barycentric_coords;\n"
		"  pixel_pos = pixel_pos_centroid =\n"
		"    vec2(viewport_size) * (pos + 1.0) / 2.0;\n"
		"}\n";

	/* Compile program */
	prog = glCreateProgram();
	GLint vs = piglit_compile_shader_text(GL_VERTEX_SHADER, vert);
	glAttachShader(prog, vs);
	GLint fs = piglit_compile_shader_text(GL_FRAGMENT_SHADER, frag);
	glAttachShader(prog, fs);
	glBindAttribLocation(prog, 0, "pos_within_tri");
	glBindAttribLocation(prog, 1, "in_barycentric_coords");
	glLinkProgram(prog);
	if (!piglit_link_check_status(prog)) {
		piglit_report_result(PIGLIT_FAIL);
	}

	/* Set up uniforms */
	glUseProgram(prog);
	glUniform1f(glGetUniformLocation(prog, "tri_scale"), tri_scale);
	glUniform1f(glGetUniformLocation(prog, "rotation_delta"),
		    rotation_delta);
	glUniform1i(glGetUniformLocation(prog, "tris_across"), tris_across);
	glUniform1f(glGetUniformLocation(prog, "final_scale"), final_scale);
	proj_loc = glGetUniformLocation(prog, "proj");
	tri_num_loc = glGetUniformLocation(prog, "tri_num");
	viewport_size_loc = glGetUniformLocation(prog, "viewport_size");

	/* Set up vertex array object */
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	/* Set up vertex input buffer */
	glGenBuffers(1, &vertex_buf);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buf);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_data), vertex_data,
		     GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, ARRAY_SIZE(vertex_data[0].pos_within_tri),
			      GL_FLOAT, GL_FALSE, sizeof(vertex_data[0]),
			      (void *) offsetof(vertex_attributes,
						pos_within_tri));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, ARRAY_SIZE(vertex_data[0].barycentric_coords),
			      GL_FLOAT, GL_FALSE, sizeof(vertex_data[0]),
			      (void *) offsetof(vertex_attributes,
						barycentric_coords));
}


void
InterpolationTestPattern::draw(const float (*proj)[4])
{
	glUseProgram(prog);

	/* Depending what the fragment shader does, it's possible that
	 * viewport_size might get optimized away.  Only set it if it
	 * didn't.
	 */
	if (viewport_size_loc != -1) {
		GLint viewport_dims[4];
		glGetIntegerv(GL_VIEWPORT, viewport_dims);
		glUniform2i(viewport_size_loc, viewport_dims[2], viewport_dims[3]);
	}

	Triangles::draw(proj);
}


void Lines::compile()
{
	/* Line coords within (-1,-1) to (1,1) rect */
	static const float pos_line[][2] = {
		{ -0.8, -0.5 },
		{  0.8, -0.5 }
	};

	/* Number of line instances across (and down) */
	int lines_across = 4;

	/* Total number of lines drawn */
	num_lines = lines_across * lines_across;

	/* Amount each line should be rotated compared to prev */
	float rotation_delta = M_PI * 2.0 / num_lines;

	/* Scaling factor uniformly applied to line coords */
	float line_scale = 0.8 / lines_across;

	/* Final scaling factor */
	float final_scale = 0.95;

	static const char *vert =
		"#version 130\n"
		"in vec2 pos_line;\n"
		"uniform float line_scale;\n"
		"uniform float rotation_delta;\n"
		"uniform int lines_across;\n"
		"uniform float final_scale;\n"
		"uniform mat4 proj;\n"
		"uniform int line_num;\n"
		"\n"
		"void main()\n"
		"{\n"
		"  vec2 pos = line_scale * pos_line;\n"
		"  float rotation = rotation_delta * line_num;\n"
		"  pos = mat2(cos(rotation), sin(rotation),\n"
		"             -sin(rotation), cos(rotation)) * pos;\n"
		"  int i = line_num % lines_across;\n"
		"  int j = lines_across - 1 - line_num / lines_across;\n"
		"  pos += (vec2(i, j) * 2.0 + 1.0) / lines_across - 1.0;\n"
		"  pos *= final_scale;\n"
		"  gl_Position = proj * vec4(pos, 0.0, 1.0);\n"
		"}\n";

	static const char *frag =
		"#version 130\n"
		"void main()\n"
		"{\n"
		"  gl_FragColor = vec4(1.0);\n"
		"}\n";

	/* Compile program */
	prog = glCreateProgram();
	GLint vs = piglit_compile_shader_text(GL_VERTEX_SHADER, vert);
	glAttachShader(prog, vs);
	GLint fs = piglit_compile_shader_text(GL_FRAGMENT_SHADER, frag);
	glAttachShader(prog, fs);
	glBindAttribLocation(prog, 0, "pos_line");
	glLinkProgram(prog);
	if (!piglit_link_check_status(prog)) {
		piglit_report_result(PIGLIT_FAIL);
	}

	/* Set up uniforms */
	glUseProgram(prog);
	glUniform1f(glGetUniformLocation(prog, "line_scale"), line_scale);
	glUniform1f(glGetUniformLocation(prog, "rotation_delta"),
		    rotation_delta);
	glUniform1i(glGetUniformLocation(prog, "lines_across"), lines_across);
	glUniform1f(glGetUniformLocation(prog, "final_scale"), final_scale);
	proj_loc = glGetUniformLocation(prog, "proj");
	line_num_loc = glGetUniformLocation(prog, "line_num");

	/* Set up vertex array object */
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	/* Set up vertex input buffer */
	glGenBuffers(1, &vertex_buf);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buf);
	glBufferData(GL_ARRAY_BUFFER, sizeof(pos_line), pos_line,
		     GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, ARRAY_SIZE(pos_line[0]), GL_FLOAT,
			      GL_FALSE, sizeof(pos_line[0]), (void *) 0);
}

void Lines::draw(const float (*proj)[4])
{
	glClear(GL_COLOR_BUFFER_BIT);
	glUseProgram(prog);
	glUniformMatrix4fv(proj_loc, 1, GL_TRUE, &proj[0][0]);
	glBindVertexArray(vao);
	for (int line_num = 0; line_num < num_lines; ++line_num) {
		/* Draws with line width = 0.25, 0.75, 1.25,
		 * 1.75, 2.25, 2.75, 3.25, 3.75
		 */
		glLineWidth((1 + 2 * line_num) / 4.0);
		glUniform1i(line_num_loc, line_num);
		glDrawArrays(GL_LINES, 0, 2);
	}
}

void Points::compile()
{
	/* Point coords within (-1,-1) to (1,1) rect */
	static const float pos_point[2] = { -0.5, -0.5 };

	/* Number of point instances across (and down) */
	int points_across = 4;

	/* Total number of points drawn */
	num_points = points_across * points_across;

	/* Scaling factor uniformly applied to point coords */
	float point_scale = 0.8 / points_across;

	/* Final scaling factor */
	float final_scale = 0.95;

	static const char *vert =
		"#version 130\n"
		"in vec2 pos_point;\n"
		"uniform float point_scale;\n"
		"uniform int points_across;\n"
		"uniform float final_scale;\n"
		"uniform mat4 proj;\n"
		"uniform int point_num;\n"
		"uniform float depth;\n"
		"\n"
		"void main()\n"
		"{\n"
		"  vec2 pos = point_scale * pos_point;\n"
		"  int i = point_num % points_across;\n"
		"  int j = points_across - 1 - point_num / points_across;\n"
		"  pos += (vec2(i, j) * 2.0 + 1.0) / points_across - 1.0;\n"
		"  pos *= final_scale;\n"
		"  gl_Position = proj * vec4(pos, depth, 1.0);\n"
		"}\n";

	static const char *frag =
		"#version 130\n"
		"void main()\n"
		"{\n"
		"  gl_FragColor = vec4(1.0);\n"
		"}\n";

	/* Compile program */
	prog = glCreateProgram();
	GLint vs = piglit_compile_shader_text(GL_VERTEX_SHADER, vert);
	glAttachShader(prog, vs);
	GLint fs = piglit_compile_shader_text(GL_FRAGMENT_SHADER, frag);
	glAttachShader(prog, fs);
	glBindAttribLocation(prog, 0, "pos_point");
	glLinkProgram(prog);
	if (!piglit_link_check_status(prog)) {
		piglit_report_result(PIGLIT_FAIL);
	}

	/* Set up uniforms */
	glUseProgram(prog);
	glUniform1f(glGetUniformLocation(prog, "point_scale"), point_scale);
	glUniform1i(glGetUniformLocation(prog, "points_across"), points_across);
	glUniform1f(glGetUniformLocation(prog, "final_scale"), final_scale);
	proj_loc = glGetUniformLocation(prog, "proj");
	point_num_loc = glGetUniformLocation(prog, "point_num");
	depth_loc = glGetUniformLocation(prog, "depth");

	/* Set up vertex array object */
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	/* Set up vertex input buffer */
	glGenBuffers(1, &vertex_buf);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buf);
	glBufferData(GL_ARRAY_BUFFER, sizeof(pos_point), pos_point,
		     GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, ARRAY_SIZE(pos_point), GL_FLOAT,
			      GL_FALSE, 0, (void *) 0);
}

void Points::draw(const float (*proj)[4])
{
	glClear(GL_COLOR_BUFFER_BIT);
	glUseProgram(prog);
	glUniformMatrix4fv(proj_loc, 1, GL_TRUE, &proj[0][0]);
	glBindVertexArray(vao);
	glUniform1f(depth_loc, 0.0);
	for (int point_num = 0; point_num < num_points; ++point_num) {
		glPointSize((1.0 + 4 * point_num) / 4.0);
		glUniform1i(point_num_loc, point_num);
		glDrawArrays(GL_POINTS, 0, 1);
	}
}

Sunburst::Sunburst()
	: out_type(GL_UNSIGNED_NORMALIZED),
	  compute_depth(false)
{
}


/**
 * Determine the GLSL type that should be used for rendering, based on
 * out_type.
 */
const char *
Sunburst::get_out_type_glsl() const
{
	switch(out_type) {
	case GL_INT:
		return "ivec4";
	case GL_UNSIGNED_INT:
		return "uvec4";
	case GL_UNSIGNED_NORMALIZED:
	case GL_FLOAT:
		return "vec4";
	default:
		printf("Unrecognized out_type: %s\n",
		       piglit_get_gl_enum_name(out_type));
		piglit_report_result(PIGLIT_FAIL);
		return "UNKNOWN";
	}
}


void Sunburst::compile()
{
	static struct vertex_attributes {
		float pos_within_tri[2];
		float barycentric_coords[3];
	} vertex_data[] = {
		{ { -0.3, -0.8 }, { 1, 0, 0 } },
		{ {  0.0,  1.0 }, { 0, 1, 0 } },
		{ {  0.3, -0.8 }, { 0, 0, 1 } }
	};

	/* Total number of triangles drawn */
	num_tris = 7;

	static const char *vert =
		"#version 130\n"
		"in vec2 pos_within_tri;\n"
		"in vec3 in_barycentric_coords;\n"
		"out vec3 barycentric_coords;\n"
		"uniform float rotation;\n"
		"uniform float vert_depth;\n"
		"uniform mat4 proj;\n"
		"\n"
		"void main()\n"
		"{\n"
		"  vec2 pos = pos_within_tri;\n"
		"  pos = mat2(cos(rotation), sin(rotation),\n"
		"             -sin(rotation), cos(rotation)) * pos;\n"
		"  gl_Position = proj * vec4(pos, vert_depth, 1.0);\n"
		"  barycentric_coords = in_barycentric_coords;\n"
		"}\n";

	static const char *frag_template =
		"#version 130\n"
		"#define OUT_TYPE %s\n"
		"#define COMPUTE_DEPTH %s\n"
		"uniform float frag_depth;\n"
		"in vec3 barycentric_coords;\n"
		"uniform mat3x4 draw_colors;\n"
		"out OUT_TYPE frag_out;\n"
		"\n"
		"void main()\n"
		"{\n"
		"  frag_out = OUT_TYPE(draw_colors * barycentric_coords);\n"
		"#if COMPUTE_DEPTH\n"
		"  gl_FragDepth = (frag_depth + 1.0) / 2.0;\n"
		"#endif\n"
		"}\n";

	/* Compile program */
	prog = glCreateProgram();
	GLint vs = piglit_compile_shader_text(GL_VERTEX_SHADER, vert);
	glAttachShader(prog, vs);
	const char *out_type_glsl = get_out_type_glsl();
	unsigned frag_alloc_len =
		strlen(frag_template) + strlen(out_type_glsl) + 1;
	char *frag = (char *) malloc(frag_alloc_len);
	sprintf(frag, frag_template, out_type_glsl,
		compute_depth ? "1" : "0");
	GLint fs = piglit_compile_shader_text(GL_FRAGMENT_SHADER, frag);
	free(frag);
	glAttachShader(prog, fs);
	glBindAttribLocation(prog, 0, "pos_within_tri");
	glBindAttribLocation(prog, 1, "in_barycentric_coords");
	glBindFragDataLocation(prog, 0, "frag_out");
	glLinkProgram(prog);
	if (!piglit_link_check_status(prog)) {
		piglit_report_result(PIGLIT_FAIL);
	}

	/* Set up uniforms */
	glUseProgram(prog);
	rotation_loc = glGetUniformLocation(prog, "rotation");
	vert_depth_loc = glGetUniformLocation(prog, "vert_depth");
	frag_depth_loc = glGetUniformLocation(prog, "frag_depth");
	glUniform1f(vert_depth_loc, 0.0);
	glUniform1f(frag_depth_loc, 0.0);
	proj_loc = glGetUniformLocation(prog, "proj");
	draw_colors_loc = glGetUniformLocation(prog, "draw_colors");

	/* Set up vertex array object */
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	/* Set up vertex input buffer */
	glGenBuffers(1, &vertex_buf);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buf);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_data), vertex_data,
		     GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, ARRAY_SIZE(vertex_data[0].pos_within_tri),
			      GL_FLOAT, GL_FALSE, sizeof(vertex_data[0]),
			      (void *) 0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, ARRAY_SIZE(vertex_data[0].barycentric_coords),
			      GL_FLOAT, GL_FALSE, sizeof(vertex_data[0]),
			      (void *) offsetof(vertex_attributes,
						barycentric_coords));
}


ColorGradientSunburst::ColorGradientSunburst(GLenum out_type)
{
	this->out_type = out_type;
}


/**
 * Draw the color gradient sunburst, but instead of using color
 * components that range from 0.0 to 1.0, apply the given scaling
 * factor and offset to each color component.
 *
 * The offset is also applied when clearing the color buffer.
 */
void
ColorGradientSunburst::draw_with_scale_and_offset(const float (*proj)[4],
						  float scale, float offset)
{
	switch (out_type) {
	case GL_INT: {
		int clear_color[4] = { offset, offset, offset, offset };
		glClearBufferiv(GL_COLOR, 0, clear_color);
		break;
	}
	case GL_UNSIGNED_INT: {
		unsigned clear_color[4] = { offset, offset, offset, offset };
		glClearBufferuiv(GL_COLOR, 0, clear_color);
		break;
	}
	case GL_UNSIGNED_NORMALIZED:
	case GL_FLOAT: {
		float clear_color[4] = { offset, offset, offset, offset };
		glClearBufferfv(GL_COLOR, 0, clear_color);
		break;
	}
	default:
		printf("Unrecognized out_type: %s\n",
		       piglit_get_gl_enum_name(out_type));
		piglit_report_result(PIGLIT_FAIL);
		break;
	}

	glUseProgram(prog);
	glUniformMatrix4fv(proj_loc, 1, GL_TRUE, &proj[0][0]);
	float draw_colors[3][4] =
		{ { 1, 0, 0, 1.0 }, { 0, 1, 0, 0.5 }, { 0, 0, 1, 1.0 } };
	for (int i = 0; i < 3; ++i) {
		for (int j = 0; j < 4; ++j) {
			draw_colors[i][j] = scale * draw_colors[i][j] + offset;
		}
	}
	glUniformMatrix3x4fv(draw_colors_loc, 1, GL_FALSE,
			     &draw_colors[0][0]);
	glBindVertexArray(vao);
	for (int i = 0; i < num_tris; ++i) {
		glUniform1f(rotation_loc, M_PI * 2.0 * i / num_tris);
		glDrawArrays(GL_TRIANGLES, 0, 3);
	}
}


void
ColorGradientSunburst::draw(const float (*proj)[4])
{
	draw_with_scale_and_offset(proj, 1.0, 0.0);
}


void
StencilSunburst::draw(const float (*proj)[4])
{
	glEnable(GL_STENCIL_TEST);
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

	glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	glUseProgram(prog);
	glUniformMatrix4fv(proj_loc, 1, GL_TRUE, &proj[0][0]);
	glBindVertexArray(vao);
	for (int i = 0; i < num_tris; ++i) {
		glStencilFunc(GL_ALWAYS, i+1, 0xff);
		glUniform1f(rotation_loc, M_PI * 2.0 * i / num_tris);
		glDrawArrays(GL_TRIANGLES, 0, 3);
	}

	glDisable(GL_STENCIL_TEST);
}


DepthSunburst::DepthSunburst(bool compute_depth)
{
	this->compute_depth = compute_depth;
}


void
DepthSunburst::draw(const float (*proj)[4])
{
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(prog);
	glUniformMatrix4fv(proj_loc, 1, GL_TRUE, &proj[0][0]);
	glBindVertexArray(vao);
	for (int i = 0; i < num_tris; ++i) {
		/* Draw triangles in a haphazard order so we can
		 * verify that depth comparisons sort them out
		 * properly.
		 */
		int triangle_to_draw = (i * 3) % num_tris;

		/* Note: with num_tris == 7, this causes us to draw
		 * triangles at depths of 3/4, 1/2, -1/4, 0, 1/4, 1/2,
		 * and 3/4.
		 */
		glUniform1f(compute_depth ? frag_depth_loc : vert_depth_loc,
			    float(num_tris - triangle_to_draw * 2 - 1)
			    / (num_tris + 1));

		glUniform1f(rotation_loc,
			    M_PI * 2.0 * triangle_to_draw / num_tris);
		glDrawArrays(GL_TRIANGLES, 0, 3);
	}

	glDisable(GL_DEPTH_TEST);
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
	  srgb(srgb)
{
}

void
Test::init(int num_samples, bool small, bool combine_depth_stencil,
	   int pattern_width, int pattern_height, int supersample_factor)
{
	this->num_samples = num_samples;
	this->pattern_width = pattern_width;
	this->pattern_height = pattern_height;
	this->supersample_factor = supersample_factor;

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
			  which_buffers, GL_NEAREST);
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
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
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
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
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
 * Convert from sRGB color space to linear color space, using the
 * formula from the GL 3.0 spec, section 4.1.8 (sRGB Texture Color
 * Conversion).
 */
float
decode_srgb(float x)
{
	if (x <= 0.0405)
		return x / 12.92;
	else
		return pow((x + 0.055) / 1.055, 2.4);
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

	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
			glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
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
					ref = decode_srgb(ref);
					test = decode_srgb(test);
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
	    int supersample_factor)
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
		   pattern_height, supersample_factor);
	return test;
}

/**
 * Convert the image into a format that can be easily understood by
 * visual inspection, and display it on the screen.
 *
 * Luminance and intensity values are converted to a grayscale value.
 * Alpha values are visualized by blending the image with a grayscale
 * checkerboard.
 *
 * Pass image_count = 0 to disable drawing multiple images to window
 * system framebuffer.
 */
void
visualize_image(float *img, GLenum base_internal_format,
		int image_width, int image_height,
		int image_count, bool rhs)
{
	unsigned components = piglit_num_components(base_internal_format);
	float *visualization =
		(float *) malloc(sizeof(float)*3*image_width*image_height);
	for (int y = 0; y < image_height; ++y) {
		for (int x = 0; x < image_width; ++x) {
			float r = 0, g = 0, b = 0, a = 1;
			float *pixel =
				&img[(y * image_width + x) * components];
			switch (base_internal_format) {
			case GL_ALPHA:
				a = pixel[0];
				break;
			case GL_RGBA:
				a = pixel[3];
				/* Fall through */
			case GL_RGB:
				b = pixel[2];
				/* Fall through */
			case GL_RG:
				g = pixel[1];
				/* Fall through */
			case GL_RED:
				r = pixel[0];
				break;
			case GL_LUMINANCE_ALPHA:
				a = pixel[1];
				/* Fall through */
			case GL_INTENSITY:
			case GL_LUMINANCE:
				r = pixel[0];
				g = pixel[0];
				b = pixel[0];
				break;
			}
			float checker = ((x ^ y) & 0x10) ? 0.75 : 0.25;
			r = r * a + checker * (1 - a);
			g = g * a + checker * (1 - a);
			b = b * a + checker * (1 - a);
			visualization[(y * image_width + x) * 3] = r;
			visualization[(y * image_width + x) * 3 + 1] = g;
			visualization[(y * image_width + x) * 3 + 2] = b;
		}
	}
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glUseProgram(0);

	/* To simultaneously display multiple images on window system
	 * framebuffer.
	 */
	if(image_count) {
		/* Use glWindowPos to directly update x, y coordinates of
		 * current raster position without getting transformed by
		 * modelview projection matrix and viewport-to-window
		 * transform.
		 */
		glWindowPos2f(rhs ? image_width : 0,
			      (image_count - 1) * image_height);
	}
	else {
		glRasterPos2f(rhs ? 0 : -1, -1);
	}
	glDrawPixels(image_width, image_height, GL_RGB, GL_FLOAT,
		     visualization);
	free(visualization);
}
