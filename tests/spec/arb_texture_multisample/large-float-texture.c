/*
 * Copyright (c) 2017 VMware, Inc.
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

/** @file large-float-texture.c
 *
 * Test large MSAA float textures.  In particular, create/load a multisample
 * texture then read it back and compare returned values.
 * Also support array textures instead of MSAA as a sanity check / debug
 * option.
 *
 * Some drivers/GPUs may fail this test.  NVIDIA, for example, appears to
 * only store the MSAA coverage info, not the sample colors, for samples
 * beyond the 8th sample.  We may tune the way this test operates over time
 * to be more useful.  Maybe we should test all MSAA samples/pixels (2x,
 * 4x, 8x, etc).
 *
 * See code for command line arguments.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN
	config.supports_gl_compat_version = 30;
	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;
PIGLIT_GL_TEST_CONFIG_END


static bool verbose = false;


/**
 * Try to create a GL_RGBA32F/16F texture of the given size, samples.
 * Return 0 if failure.
 */
static GLuint
create_texture(GLenum target, GLenum intFormat,
	       GLsizei width, GLsizei height, GLuint numSamples)
{
	GLuint tex;

	assert(intFormat == GL_RGBA32F || intFormat == GL_RGBA16F);

	if (verbose) {
		printf("Trying %d x %d  %d samples/layers\n",
		       width, height, numSamples);
	}

	glGenTextures(1, &tex);
	glBindTexture(target, tex);
	if (target == GL_TEXTURE_2D_MULTISAMPLE) {
		glTexImage2DMultisample(target,
					numSamples, intFormat, width, height,
					GL_FALSE); /* fixedsamplelocations */
	}
	else {
		/* instead of samples per pixel, use 'samples' layers */
		assert(target == GL_TEXTURE_2D_ARRAY);
		glTexStorage3D(target, 1, intFormat, width, height,
			       numSamples);
	}

	if (glGetError() != GL_NO_ERROR) {
		/* some error */
		glDeleteTextures(1, &tex);
		tex = 0;
	}

	return tex;
}


/**
 * Find the max working texture size.
 */
static GLuint
create_texture_max_size(GLenum target, GLenum intFormat,
			GLsizei *width, GLsizei *height,
			GLuint numSamples)
{
	GLint w, h;
	GLuint tex = 0;

	w = *width;
	h = *height;
	while (w >= 1 && h >= 1) {
		tex = create_texture(target, intFormat, w, h, numSamples);
		if (tex) {
			/* done! */
			*width = w;
			*height = h;
			break;
		}
		/* try smaller size */
		if (h >= w) {
			h /= 2;
		}
		else {
			w /= 2;
		}
	}

	return tex;
}


/**
 * Create an FBO which wraps the given texture.
 */
static GLuint
create_fbo(GLuint tex, GLenum texTarget)
{
	GLuint fbo;

	assert(texTarget == GL_TEXTURE_2D_MULTISAMPLE ||
	       texTarget == GL_TEXTURE_2D_ARRAY ||
	       texTarget == GL_TEXTURE_2D);

	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	if (texTarget == GL_TEXTURE_2D_MULTISAMPLE ||
	    texTarget == GL_TEXTURE_2D) {
		glFramebufferTexture2D(GL_FRAMEBUFFER,
				       GL_COLOR_ATTACHMENT0,
				       texTarget, tex, 0);  /* 0=level */
	}
	else {
		assert(texTarget == GL_TEXTURE_2D_ARRAY);
		glFramebufferTextureLayer(GL_FRAMEBUFFER,
					  GL_COLOR_ATTACHMENT0,
					  tex, 0, 0);
	}

	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE) {
		if (verbose) {
			printf("Failed to create FBO! (status = %s)\n",
			       piglit_get_gl_enum_name(status));
		}
		glDeleteFramebuffers(1, &fbo);
		fbo = 0;
	}

	return fbo;
}


/**
 * Load the GL_RGBA32F/16F MSAA (or array) texture with known values.
 * The color components are basically:
 *  x = texcoord in [0, 1]  (this can get fuzzy for large texture widths)
 *  y = fragcoord.y MOD 16 in [0, 15]
 *  z = -curSample in [-(numSamples-1), 0]
 *  w = curSample in [0, numSamples-1]
 */
static void
load_texture_image(GLenum target, GLuint fbo, GLuint tex,
		   GLsizei width, GLsizei height, GLuint numSamples,
		   GLfloat valueScale)
{
	static const char *vs_text =
		"#version 130\n"
		"out vec4 texcoord;\n"
		"void main() {\n"
		"  texcoord = gl_MultiTexCoord0;\n"
		"  gl_Position = gl_Vertex;\n"
		"}\n";
	static const char *fs_text =
		"#version 130\n"
		"out vec4 color;\n"
		"in vec4 texcoord;\n"
		"uniform int curSample;\n"
		"uniform float valueScale;\n"
		"void main() {\n"
		"   float x = texcoord.x; \n"
		"   float y = float(int(gl_FragCoord.y) % 16) / 16.0; \n"
		"   float z = -curSample; \n"
		"   float w = curSample; \n"
		"   color = valueScale * vec4(x, y, z, w); \n"
		"}\n";

	GLuint prog;

	prog = piglit_build_simple_program(vs_text, fs_text);

	assert(prog);
	assert(numSamples <= 32);

	glUseProgram(prog);

	GLint curSampleUniform = glGetUniformLocation(prog, "curSample");
	assert(curSampleUniform >= 0);

	GLint valueScaleUniform = glGetUniformLocation(prog, "valueScale");
	glUniform1f(valueScaleUniform, valueScale);

	if (target == GL_TEXTURE_2D_MULTISAMPLE) {
		glEnable(GL_SAMPLE_MASK);
		glEnable(GL_MULTISAMPLE);
	}

	GLint samp;
	for (samp = 0; samp < numSamples; samp++) {
		if (verbose) {
			printf("Render sample/layer %d\n", samp);
		}

		glUniform1i(curSampleUniform, samp);

		/* choose sample or layer to write to */
		if (target == GL_TEXTURE_2D_MULTISAMPLE) {
			glSampleMaski(0, 1u << samp);
		}
		else {
			assert(target == GL_TEXTURE_2D_ARRAY);
			glFramebufferTextureLayer(GL_FRAMEBUFFER,
						  GL_COLOR_ATTACHMENT0,
						  tex, 0, samp);
		}

		/* Full framebuffer rect */
		piglit_draw_rect_tex(-1, -1, 2, 2,
				     0, 0, 1, 1);
	}

	if (target == GL_TEXTURE_2D_MULTISAMPLE) {
		glDisable(GL_SAMPLE_MASK);
		glDisable(GL_MULTISAMPLE);
	}

	glDeleteProgram(prog);
}


/**
 * Create simple 2D, GL_RGBA32F texture of given size.
 */
static GLuint
create_float4_tex(GLsizei width, GLsizei height)
{
	GLuint tex;

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, width, height);

	if (glGetError() != GL_NO_ERROR) {
		glDeleteTextures(1, &tex);
		tex = 0;
	}
	return tex;
}


/**
 * Create the shader program needed for extracting texels from an
 * MSAA (or array) texture.
 */
static GLuint
create_readback_program(GLenum target)
{
	static const char *fs_text_msaa =
		"#version 130\n"
		"#extension GL_ARB_texture_multisample : enable\n"
		"out vec4 color;\n"
		"uniform sampler2DMS tex;\n"
		"uniform int sample;\n"
		"void main() {\n"
		"  ivec2 coord = ivec2(gl_FragCoord.xy);\n"
		"  color = texelFetch(tex, coord, sample);\n"
		"}\n";

	static const char *fs_text_array =
		"#version 130\n"
		"out vec4 color;\n"
		"uniform sampler2DArray tex;\n"
		"uniform int sample;\n"
		"void main() {\n"
		"  ivec2 coord = ivec2(gl_FragCoord.xy);\n"
		"  color = texelFetch(tex, ivec3(coord, sample), 0);\n"
		"}\n";

	const char *fs_text = target == GL_TEXTURE_2D_MULTISAMPLE ?
		fs_text_msaa : fs_text_array;

	GLuint prog = piglit_build_simple_program(NULL, fs_text);

	assert(prog);

	return prog;
}

/**
 * Extract a slice or per-sample image from the src_tex.
 */
static void
extract_texture_image(GLuint readbackProg,
		      GLuint src_tex, GLsizei width, GLsizei height,
		      GLuint sample)
{
	glUseProgram(readbackProg);

	GLint texUniform = glGetUniformLocation(readbackProg, "tex");
	GLint sampleUniform = glGetUniformLocation(readbackProg, "sample");

	/* Create texture to put results into, and wrap it in an FBO.
	 * The shader will extract the sample from the MSAA texture (or
	 * array layer) and write the results into a destination texture/FBO.
	 */
	GLuint dst_tex = create_float4_tex(width, height);
	GLuint dst_fbo = create_fbo(dst_tex, GL_TEXTURE_2D);

	glUniform1i(texUniform, 0);  // unit 0
	glUniform1i(sampleUniform, sample);

	glBindFramebuffer(GL_FRAMEBUFFER, dst_fbo);

	piglit_draw_rect_tex(-1, -1, 2, 2,
			     0, 0, 1, 1);

	glBindTexture(GL_TEXTURE_2D, dst_tex);

	/* Now, the extracted image is available both as dst_tex
	 * and as the current FBO.
	 */
}


/**
 * Test if two float[4] values are nearly equal.
 */
static bool
nearly_equal(const GLfloat x[4], const GLfloat y[4])
{
	/* XXX this tolerance was chosen emperically */
	const float maxRelDiff = 0.0005;
	int i;
	for (i = 0; i < 4; i++) {
		float diff = x[i] - y[i];
		float ax = fabsf(x[i]), ay = fabsf(y[i]);
		if (diff > MAX2(ax, ay) * maxRelDiff) {
			return false;
		}
	}

	return true;
}


/**
 * To record +/- max difference between expected and rendered results.
 */
struct error_info
{
	float min_error[4], max_error[4];
	float avg_error[4];
	unsigned num_fail;
};


static void
init_error_info(struct error_info *err)
{
	int i;
	for (i = 0; i < 4; i++) {
		err->min_error[i] = 1e20;
		err->max_error[i] = -1e20;
		err->avg_error[i] = 0.0;
	}
	err->num_fail = 0;
}


static bool
nonzero(const float a[4])
{
	return a[0] != 0.0 ||a[1] != 0.0 ||a[2] != 0.0 ||a[3] != 0.0;
}


static void
finish_and_print_error_info(struct error_info *err,
			    GLsizei width, GLsizei height)
{
	int i;
	for (i = 0; i < 4; i++) {
		err->avg_error[i] /= width * height;
	}
	if (verbose ||
	    nonzero(err->min_error) ||
	    nonzero(err->max_error) ||
	    nonzero(err->avg_error)) {
		printf("Min error: %g %g %g %g\n",
		       err->min_error[0], err->min_error[1],
		       err->min_error[2], err->min_error[3]);
		printf("Max error: %g %g %g %g\n",
		       err->max_error[0], err->max_error[1],
		       err->max_error[2], err->max_error[3]);
		printf("Avg error: %g %g %g %g\n",
		       err->avg_error[0], err->avg_error[1],
		       err->avg_error[2], err->avg_error[3]);
		printf("num_fail: %u\n", err->num_fail);
	}
}


static void
update_error_info(struct error_info *err,
		  const GLfloat a[4], const GLfloat b[4])
{
	bool fail = false;
	int i;

	for (i = 0; i < 4; i++) {
		float delta = a[i] - b[i];
		err->min_error[i] = MIN2(err->min_error[i], delta);
		err->max_error[i] = MAX2(err->max_error[i], delta);

		err->avg_error[i] += fabsf(delta);

		if (delta != 0.0f) {
			fail = true;
		}
	}

	err->num_fail += (unsigned) fail;
}


static unsigned
texel_size(GLenum intFormat)
{
	switch (intFormat) {
	case GL_RGBA16F:
		return 4 * sizeof(GLhalf);
	case GL_RGBA32F:
		return 4 * sizeof(GLfloat);
	default:
		assert(!"Unexpected texture format");
		return 0;
	}
}


/**
 * Read back all texture samples, compare to reference.
 */
static bool
validate_texture_image(GLenum target,
		       GLenum intFormat,
		       GLuint readbackProg,
		       GLuint src_tex,
		       GLsizei width, GLsizei height, GLuint numSamples,
		       GLfloat valueScale)
{
	/*
	 * Note: this is a little more complicated than just mallocing
	 * a buffer of size width * height * numSamples * texelSize
	 * because we could easily exceed 4GB.  So, we read back the image
	 * in stripes no larger than 512MB.
	 */
	const size_t buffer_size = 512 * 1024 * 1024;  // 512 MB
	GLfloat *buffer = malloc(buffer_size);
	assert(buffer);

	const int bytesPerRow = width * numSamples * texel_size(intFormat);
	const int stripeHeight = MIN2(buffer_size / bytesPerRow, height);

	bool pass = true;
	float fwidth = (float) width;
	GLuint samp;

	glBindTexture(target, src_tex);

	for (samp = 0; samp < numSamples; samp++) {
		struct error_info err;

		init_error_info(&err);

		if (verbose) {
			printf("Checking sample/layer %d\n", samp);
		}

		extract_texture_image(readbackProg, src_tex,
				      width, height, samp);

		GLint i, j, numFail = 0;

		for (j = 0; j < height; j++) {

			if (j % stripeHeight == 0) {
				/* read a stripe */
				if (height == stripeHeight) {
					/* get whole texture with GetTexImage */
					glGetTexImage(GL_TEXTURE_2D, 0,
						      GL_RGBA, GL_FLOAT,
						      buffer);
				}
				else {
					/* use glReadPixels to get a stripe */
					glReadPixels(0, j, width, stripeHeight,
						     GL_RGBA, GL_FLOAT, buffer);
				}
			}

			for (i = 0; i < width; i++) {
				int row = j % stripeHeight;
				const GLfloat *texel =
					buffer + (width * row + i) * 4;
				GLfloat expected[4];

				/* [0] is texcoord at center of fragment */
				expected[0] = i / fwidth + (0.5f / fwidth);
				/* [1] is fragcoord.y MOD 16 / 16.0 */
				expected[1] = (j % 16) / 16.0;
				expected[2] = -1.0 * samp;
				expected[3] = samp;

				expected[0] *= valueScale;
				expected[1] *= valueScale;
				expected[2] *= valueScale;
				expected[3] *= valueScale;

				update_error_info(&err, texel, expected);

				if (!nearly_equal(texel, expected)) {
					printf("Fail at %d, %d:\n", i, j);
					printf("  Expected %g, %g, %g, %g\n",
					       expected[0], expected[1],
					       expected[2], expected[3]);
					printf("  Found %g, %g, %g, %g\n",
					       texel[0], texel[1],
					       texel[2], texel[3]);
					pass = false;
					numFail++;
					if (numFail >= 5) {
						goto end;
					}
				}
			}
		}
		finish_and_print_error_info(&err, width, height);
	}

end:
	free(buffer);

	return pass;
}


void
piglit_init(int argc, char **argv)
{
	GLenum target = GL_TEXTURE_2D_MULTISAMPLE;
	GLenum intFormat = GL_RGBA32F;
	GLint samples = -1;  /* or array slices */
	GLint maxSize = -1;
	GLsizei width = -1, height = -1;
	GLfloat valueScale = 1.0;
	int i;

	for (i = 1; i < argc; i++) {
		if (strcmp(argv[i], "--array") == 0) {
			target = GL_TEXTURE_2D_ARRAY;
		}
		else if (strcmp(argv[i], "--samples") == 0) {
			i++;
			samples = atoi(argv[i]);
			assert(samples > 0);
		}
		else if (strcmp(argv[i], "--texsize") == 0) {
			i++;
			maxSize = atoi(argv[i]);
			assert(maxSize > 0);
		}
		else if (strcmp(argv[i], "--width") == 0) {
			i++;
			width = atoi(argv[i]);
			assert(width > 0);
		}
		else if (strcmp(argv[i], "--height") == 0) {
			i++;
			height = atoi(argv[i]);
			assert(height > 0);
		}
		else if (strcmp(argv[i], "--scale") == 0) {
			i++;
			valueScale = atof(argv[i]);
			assert(valueScale > 0.0);
		}
		else if (strcmp(argv[i], "--fp16") == 0) {
			intFormat = GL_RGBA16F;
		}
		else if (strcmp(argv[i], "--verbose") == 0) {
			verbose = true;
		}
		else {
			printf("Unknown option %s\n", argv[i]);
			piglit_report_result(PIGLIT_FAIL);
		}
	}

	piglit_require_extension("GL_ARB_texture_float");
	piglit_require_extension("GL_ARB_texture_multisample");
	piglit_require_GLSL_version(130);

	if (maxSize == -1) {
		glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxSize);
	}

	if (samples == -1) {
		if (target == GL_TEXTURE_2D_MULTISAMPLE) {
			glGetIntegerv(GL_MAX_COLOR_TEXTURE_SAMPLES, &samples);
		}
		else {
			samples = 16;  /* 16 texture array layers */
		}
	}

	GLuint tex, fbo = 0;

	if (width == -1 || height == -1) {
		width = height = maxSize;
	}

	while (width > 1 && height > 1) {
		tex = create_texture_max_size(target, intFormat,
					      &width, &height, samples);

		if (!tex) {
			printf("Failed to create MSAA texture\n");
			piglit_report_result(PIGLIT_FAIL);
		}

		fbo = create_fbo(tex, target);
		if (!fbo) {
			/* texture creation worked, but FBO failed.
			 * Try smaller texture.
			 */
			glDeleteTextures(1, &tex);
			if (height >= width) {
				height /= 2;
			}
			else {
				width /= 2;
			}
		}
		else {
			break;
		}
	}

	if (!fbo) {
		printf("Failed to create FBO\n");
		piglit_report_result(PIGLIT_SKIP);
	}

	GLint64 mbytes = (GLint64) width * height * samples
		* texel_size(intFormat) / (1024 * 1024);

	const char *formatName = piglit_get_gl_enum_name(intFormat);
	if (target == GL_TEXTURE_2D_ARRAY) {
		printf("Created %d x %d %d-layer %s texture/FBO"
		       " (%lld MB)\n",
		       width, height, samples, formatName,
		       (long long int) mbytes);
	}
	else {
		printf("Created %d x %d %d-sample MSAA %s texture/FBO"
		       " (%lld MB)\n",
		       width, height, samples, formatName,
		       (long long int) mbytes);
	}

	GLuint readbackProg = create_readback_program(target);

	glViewport(0, 0, width, height);
	glClampColor(GL_CLAMP_READ_COLOR, GL_FALSE);
	glClampColor(GL_CLAMP_FRAGMENT_COLOR, GL_FALSE);

	if (verbose) {
		printf("Loading...\n");
	}

	load_texture_image(target, fbo, tex, width, height,
			   samples, valueScale);

	if (verbose) {
		printf("Validating...\n");
	}

	bool pass = validate_texture_image(target, intFormat, readbackProg, tex,
					   width, height, samples, valueScale);

	if (!piglit_check_gl_error(GL_NO_ERROR)) {
		pass = false;
	}

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}


enum piglit_result
piglit_display(void)
{
	/* should never get here */
	return PIGLIT_FAIL;
}
