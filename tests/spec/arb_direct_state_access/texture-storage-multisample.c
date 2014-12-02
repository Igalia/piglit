/*
 * Copyright © 2013 Chris Forbes
 * Copyright 2014 Intel Corporation
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
 */

/** @file texture-storage-multisample.c
 *
 * Based on arb_texture_storage_multisample/tex-storage.c by Chris Forbes and
 * piglit_multisample_texture in piglit_util_gl.c by Jason Ekstrand.
 * Adapted to test glTextureStorage2DMultisample and
 * glTextureStorage3DMultisample by Laura Ekstrand (laura@jlekstrand.net).
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 30;

	config.window_visual = PIGLIT_GL_VISUAL_RGBA |
			       PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

static const char multisample_texture_vs_source[] =
"#version 130\n"
"in vec2 vertex;\n"
"out vec2 tex_coords;\n"
"void main()\n"
"{\n"
"	tex_coords = vertex;\n"
"	vec2 pos = (vertex.xy * 2) - vec2(1, 1);\n"
"	gl_Position = vec4(pos, 0, 1);\n"
"}\n";

static const char multisample_texture_fs_source[] =
"#version 130\n"
"#extension GL_ARB_sample_shading : enable\n"
"in vec2 tex_coords;\n"
"uniform sampler2DArray tex;\n"
"uniform int tex_depth;\n"
"uniform int z;\n"
"void main()\n"
"{\n"
"	int layer = (gl_SampleID * tex_depth) + z;\n"
"	gl_FragColor = texture(tex, vec3(tex_coords, layer));\n"
"}\n";

/**
 * Uploads an arbitrary multisample texture.
 *
 * This function acts like glTexSub*Image for multisample textures.
 * For the texture given, it assumes that glTexImage[23]DMultisample or
 * glTex*Storage[23]DMultisample has already been called to establish the 
 * storage.
 *
 * When this function returns, multisample texture will be bound to the
 * currently active texture.
 *
 * \param tex		 Texture name for a previously initialized texture.
 * \param target         either GL_TEXTURE_2D_MULTISAMPLE or
 *                       GL_TEXTURE2D_MULTISAMPLE_ARRAY
 * \param internalformat a renderable color format accepted by
 *                       glTexImage2DMultisample
 * \param width          texture width
 * \param height         texture height
 * \param depth          texture depth.  If target is
 *                       GL_TEXTURE_2D_MULTISAMPLE, this must be 1.
 * \param samples        the number of samples
 * \param format         format of the pixel data
 * \param type           type of the pixel data
 * \param data           pixel data with which to fill the texture
 *
 */
void
texture_sub_image_multisample(GLenum tex, GLenum target,
			      GLenum internalFormat, unsigned width,
			      unsigned height, unsigned depth,
			      unsigned samples, GLenum format, GLenum type,
			      void *data)
{
	static GLuint prog = 0;
	static GLint tex_loc, tex_depth_loc, z_loc;
	static GLuint fbo, array_tex;
	static const float verts[] = {
		0.0, 0.0,
		0.0, 1.0,
		1.0, 1.0,
		1.0, 1.0,
		1.0, 0.0,
		0.0, 0.0
	};
	unsigned z;

	struct {
		GLint active_tex;
		GLint draw_fbo;
		GLint prog;
		GLint viewport[4];
		GLboolean arb_sample_shading;
		GLfloat min_sample_shading;
		GLint clamp_fragment_color;
	} backup;

	piglit_require_extension("GL_ARB_texture_multisample");
	piglit_require_extension("GL_ARB_sample_shading");

	if (target == GL_TEXTURE_2D_MULTISAMPLE) {
		assert(depth == 1);
	} else if (target == GL_TEXTURE_2D_MULTISAMPLE_ARRAY) {
	} else {
		assert(!"Invalid texture target");
		return;
	}

	if (prog == 0) {
		/* First-run setup */
		prog = piglit_build_simple_program_unlinked(
			multisample_texture_vs_source,
			multisample_texture_fs_source);
		glBindAttribLocation(prog, 0, "vertex");
		glLinkProgram(prog);
		if (!piglit_link_check_status(prog)) {
			prog = 0;
			return;
		}

		tex_loc = glGetUniformLocation(prog, "tex");
		tex_depth_loc = glGetUniformLocation(prog, "tex_depth");
		z_loc = glGetUniformLocation(prog, "z");

		glGenFramebuffers(1, &fbo);
		glGenTextures(1, &array_tex);
	}

	/* Backup client values so we can restore them later */
	glGetIntegerv(GL_ACTIVE_TEXTURE, &backup.active_tex);
	glGetIntegerv(GL_CURRENT_PROGRAM, &backup.prog);
	glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &backup.draw_fbo);
	glGetIntegerv(GL_CLAMP_FRAGMENT_COLOR, &backup.clamp_fragment_color);
	glGetIntegerv(GL_VIEWPORT, backup.viewport);
	glGetBooleanv(GL_SAMPLE_SHADING_ARB, &backup.arb_sample_shading);
	glGetFloatv(GL_MIN_SAMPLE_SHADING_VALUE_ARB, &backup.min_sample_shading);

	glEnable(GL_SAMPLE_SHADING_ARB);
	glMinSampleShadingARB(1.0f);

	glBindTexture(GL_TEXTURE_2D_ARRAY, array_tex);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, internalFormat, width, height,
		     depth * samples, 0, format, type, data);

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
	glViewport(0, 0, width, height);

	glClampColor(GL_CLAMP_FRAGMENT_COLOR, GL_FALSE);

	glUseProgram(prog);
	glUniform1i(tex_loc, backup.active_tex - GL_TEXTURE0);
	glUniform1i(tex_depth_loc, depth);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, verts);

	if (target == GL_TEXTURE_2D_MULTISAMPLE) {
		glUniform1i(z_loc, 0);
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER,
				       GL_COLOR_ATTACHMENT0,
				       target, tex, 0);
		if (glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER) !=
		    GL_FRAMEBUFFER_COMPLETE)
			return;

		glDrawArrays(GL_TRIANGLES, 0, 6);
	} else {
		for (z = 0; z < depth; ++z) {
			glUniform1i(z_loc, z);
			glFramebufferTextureLayer(GL_DRAW_FRAMEBUFFER,
						  GL_COLOR_ATTACHMENT0,
						  tex, 0, z);
			if (glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER) !=
			    GL_FRAMEBUFFER_COMPLETE)
				return;

			glDrawArrays(GL_TRIANGLES, 0, 6);
		}
	}

	glDisableVertexAttribArray(0);

	/* Restore values for the client */
	if (!backup.arb_sample_shading)
		glDisable(GL_SAMPLE_SHADING_ARB);
	glMinSampleShadingARB(backup.min_sample_shading);

	glUseProgram(backup.prog);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, backup.draw_fbo);
	glViewport(backup.viewport[0], backup.viewport[1],
		   backup.viewport[2], backup.viewport[3]);
	glBindTexture(target, tex);
	glClampColor(GL_CLAMP_FRAGMENT_COLOR, backup.clamp_fragment_color);
}

static bool
check_non_generated_texture(void)
{
	bool pass = true;

	/* Attempting to call TextureStorage* for a texture name that was not
	 * generated by OpenGL must fail with INVALID_OPERATION
	 * (OpenGL 4.5 core spec 30.10.2014, beginning of Section 8.19
	 * Immutable-Format Texture Images).
	 */
	glTextureStorage2DMultisample(250, 4, GL_RGBA8, 64, 64, GL_TRUE);
	pass &= piglit_check_gl_error(GL_INVALID_OPERATION);

	piglit_report_subtest_result(pass ? PIGLIT_PASS : PIGLIT_FAIL,
		"non-generated texture name");
	return pass;
}

static bool
check_unsized_format(void)
{
	bool pass = true;

	/* Attempting to call TextureStorage* with an unsized internalformat
	 * must fail with INVALID_ENUM (OpenGL 4.5 core spec 30.10.2014,
	 * beginning of Section 8.19 Immutable-Format Texture Images)
	 */

	GLuint tex;
	glCreateTextures(GL_TEXTURE_2D_MULTISAMPLE, 1, &tex);
	glTextureStorage2DMultisample(tex, 4, GL_RGBA, 64, 64, GL_TRUE);

	/* unsized formats may not be used with TexStorage* */
	pass &= piglit_check_gl_error(GL_INVALID_ENUM);
	piglit_report_subtest_result(pass ? PIGLIT_PASS : PIGLIT_FAIL,
		"unsized-format");
	return pass;
}

static bool
check_immutable(void)
{
	bool pass = true;
	GLuint tex;
	GLint param;

	glCreateTextures(GL_TEXTURE_2D_MULTISAMPLE, 1, &tex);
	/* specify storage for the texture, and mark it immutable-format */
	glTextureStorage2DMultisample(tex, 4, GL_RGBA8, 64, 64, GL_TRUE);
	pass &= piglit_check_gl_error(GL_NO_ERROR);

	/* should now have TEXTURE_IMMUTABLE_FORMAT */
	glGetTextureParameteriv(tex, GL_TEXTURE_IMMUTABLE_FORMAT, &param);

	if (!piglit_check_gl_error(GL_NO_ERROR)) {
		pass = false;
		printf("failed to fetch texture parameter"
		       " TEXTURE_IMMUTABLE_FORMAT\n");
	}

	if (param != GL_TRUE) {
		pass = false;
		printf("expected TEXTURE_IMMUTABLE_FORMAT to be true,"
		       " got %d\n", param);
	}

	/* calling Tex*Storage* again on the same texture should fail */
	glTextureStorage2DMultisample(tex, 4, GL_RGBA8, 32, 32, GL_TRUE);
	if (!piglit_check_gl_error(GL_INVALID_OPERATION)) {
		pass = false;
		printf("expected respecifying an immutable-format texture"
		       " (with TexStorage*Multisample) to fail\n");
	}

	/* calling TexImage2DMultisample should fail too */
	glBindTextureUnit(0, tex);
	glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE,
				4, GL_RGBA8, 32, 32, GL_TRUE);

	if (!piglit_check_gl_error(GL_INVALID_OPERATION)) {
		pass = false;
		printf("expected respecifying an immutable-format texture"
		       " (with TexImage*Multisample) to fail\n");
	}

	piglit_report_subtest_result(pass ? PIGLIT_PASS : PIGLIT_FAIL,
				     "immutable");
	return pass;
}


void
piglit_init(int argc, char **argv)
{
	piglit_require_extension("GL_ARB_direct_state_access");
	piglit_require_extension("GL_ARB_texture_storage_multisample");
}

enum piglit_result
piglit_display(void)
{
	enum piglit_result result = PIGLIT_PASS;
	if (!check_non_generated_texture())
		result = PIGLIT_FAIL;
	if (!check_immutable())
		result = PIGLIT_FAIL;
	if (!check_unsized_format())
		result = PIGLIT_FAIL;

	return result;
}
