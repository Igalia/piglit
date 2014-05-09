/*
 * Copyright © 2013 Intel Corporation
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
 * \file piglit-fbo.cpp
 *
 * This file defines the functions which can be utilized to develop
 * new piglit test cases. These functions initialize a framebuffer
 * object based on paramaters passed.
 */
#include "piglit-fbo.h"
using namespace piglit_util_fbo;

FboConfig::FboConfig(int num_samples, int width, int height)
	: num_samples(num_samples),
	  num_rb_attachments(1),
	  num_tex_attachments(0),
	  width(width),
	  height(height),
	  combine_depth_stencil(true),
	  attach_texture(false),
	  color_format(GL_RGBA),
	  color_internalformat(GL_RGBA),
	  depth_internalformat(GL_DEPTH_COMPONENT24),
	  stencil_internalformat(GL_STENCIL_INDEX8)
{
	memset(rb_attachment, 0, PIGLIT_MAX_COLOR_ATTACHMENTS * sizeof(GLuint));
	memset(tex_attachment, 0, PIGLIT_MAX_COLOR_ATTACHMENTS * sizeof(GLuint));

	/* Set default values for single renderbuffer and texture attachment. */
	rb_attachment[0] = GL_COLOR_ATTACHMENT0;
	tex_attachment[0] = GL_COLOR_ATTACHMENT0;
}

Fbo::Fbo()
	: config(0, 0, 0), /* will be overwritten on first call to setup() */
	  handle(0),
	  color_tex(0),
	  color_rb(0),
	  depth_rb(0),
	  stencil_rb(0),
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
Fbo::attach_color_renderbuffer(const FboConfig &config)
{
	glBindRenderbuffer(GL_RENDERBUFFER, color_rb);
	glRenderbufferStorageMultisample(GL_RENDERBUFFER,
					 config.num_samples,
					 config.color_internalformat,
					 config.width,
					 config.height);
	glFramebufferRenderbuffer(GL_DRAW_FRAMEBUFFER,
				  GL_COLOR_ATTACHMENT0,
				  GL_RENDERBUFFER, color_rb);
}

void
Fbo::attach_color_texture(const FboConfig &config)
{
	glBindTexture(GL_TEXTURE_RECTANGLE, color_tex);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MIN_FILTER,
			GL_NEAREST);
	glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MAG_FILTER,
			GL_NEAREST);
	glTexImage2D(GL_TEXTURE_RECTANGLE,
		     0 /* level */,
		     config.color_internalformat,
		     config.width,
		     config.height,
		     0 /* border */,
		     config.color_format /* format */,
		     GL_BYTE /* type */,
		     NULL /* data */);
	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER,
			       GL_COLOR_ATTACHMENT0,
			       GL_TEXTURE_RECTANGLE,
			       color_tex,
			       0 /* level */);
}

void
Fbo::attach_multisample_color_texture(const FboConfig &config)
{
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, color_tex);
	glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE,
				config.num_samples,
				config.color_internalformat,
				config.width,
				config.height,
				GL_TRUE /* fixed sample locations */);

	glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER,
			       GL_COLOR_ATTACHMENT0,
			       GL_TEXTURE_2D_MULTISAMPLE,
			       color_tex,
			       0 /* level */);
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
	GLint max_attachments;
	GLint requested_attachments = new_config.num_rb_attachments +
		new_config.num_tex_attachments;
	glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &max_attachments);

	if (requested_attachments > max_attachments) {
		printf("Number of color attachments are not supported by the"
		       " implementation.\nattachments requested = %d,"
		       " max attachments supported = %d\n",
		       requested_attachments, max_attachments);
		piglit_report_result(PIGLIT_SKIP);
	}

	if (!try_setup(new_config)) {
		printf("Framebuffer not complete\n");
		piglit_report_result(PIGLIT_SKIP);
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
			attach_color_renderbuffer(new_config);
		} else if (config.num_samples == 0) {
			attach_color_texture(new_config);
			piglit_require_extension("GL_ARB_texture_rectangle");
		} else {
			piglit_require_extension("GL_ARB_texture_multisample");
			attach_multisample_color_texture(new_config);
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

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, piglit_winsys_fbo);

	return success;
}

void
Fbo::set_viewport()
{
	glViewport(0, 0, config.width, config.height);
}
