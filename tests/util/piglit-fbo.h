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
 * \file piglit-fbo.h
 * This file declares classes to initialize a framebuffer object as per piglit
 * test's requirements.
 */

#include "piglit-util-gl.h"
#include "math.h"

namespace piglit_util_fbo {
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

		/**
		 * Useful if attach_texture is true and color buffer is
		 * non-multisample. Specifies the format that should be used
		 * for the color buffer, or GL_NONE if no color buffer should
		 * be used. Defaults to GL_RGBA.
		 */
		GLenum color_format;

		/**
		 * Internalformat that should be used for the color buffer, or
		 * GL_NONE if no color buffer should be used.  Defaults to
		 * GL_RGBA.
		 */
		GLenum color_internalformat;

		/**
		 * Internalformat that should be used for the depth buffer, or
		 * GL_NONE if no depth buffer should be used.  Ignored if
		 * combine_depth_stencil is true.  Defaults to
		 * GL_DEPTH_COMPONENT24.
		 */
		GLenum depth_internalformat;

		/**
		 * Internalformat that should be used for the stencil buffer,
		 * or GL_NONE if no stencil buffer should be used.  Ignored if
		 * combine_depth_stencil is true.  Defaults to
		 * GL_STENCIL_INDEX8.
		 */
		GLenum stencil_internalformat;
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

		void set_samples(int num_samples);
		void setup(const FboConfig &new_config);
		bool try_setup(const FboConfig &new_config);

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

	private:
		void generate_gl_objects();
		void attach_color_renderbuffer(const FboConfig &config);
		void attach_color_texture(const FboConfig &config);
		void attach_multisample_color_texture(const FboConfig &config);

		/**
		 * True if generate_gl_objects has been called and color_tex,
		 * color_rb, depth_rb, and stencil_rb have been initialized.
		 */
		bool gl_objects_generated;
	};
}
