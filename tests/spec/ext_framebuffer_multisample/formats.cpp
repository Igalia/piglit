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
 * \file formats.cpp
 *
 * Verify the proper functioning of multisample antialiasing for all
 * possible buffer formats.
 *
 * This test operates by rendering an MSAA image twice: once in a
 * standard RGBA buffer (the behaviour of which is well tested by the
 * other MSAA tests), and once in a buffer with some other format.
 * Then it blits both images to corresponding single-sample buffers
 * and uses glReadPixels to make sure the same image was drawn in both
 * cases (to within the expected tolerance considering the bit depth
 * of the two images).
 *
 * Finally, the images that were compared are drawn on screen to make
 * it easier to diagnose failures.
 */

#include "common.h"
#include "../../fbo/fbo-formats.h"

PIGLIT_GL_TEST_MAIN(
    512 /*window_width*/,
    256 /*window_height*/,
    GLUT_DOUBLE | GLUT_RGBA | GLUT_ALPHA)

namespace {

const int pattern_width = 256; const int pattern_height = 256;

int num_samples;

ColorGradientSunburst *test_pattern_vec4;
ColorGradientSunburst *test_pattern_ivec4;
ColorGradientSunburst *test_pattern_uvec4;


/**
 * This class encapsulates the code necessary to draw the test pattern
 * in either the reference GL_RGBA format or the format under test,
 * downsample it, read the rendered pixels into memory, and draw a
 * visualization of the result.
 */
class PatternRenderer
{
public:
	bool try_setup(GLenum internalformat);
	void set_piglit_tolerance();
	void draw();
	float *read_image(GLenum base_format);

	/**
	 * Number of bits in each color channel.  E.g. color_bits[2]
	 * == number of bits in blue color channel.
	 */
	GLint color_bits[4];

	/**
	 * Type of data in the color buffer.  E.g. GL_FLOAT,
	 * GL_UNSIGNED_NORMALIZED, or GL_UNSIGNED_INT.
	 */
	GLenum component_type;

	/**
	 * ColorGradientSunburst object that will be used to draw the
	 * test pattern.
	 */
	ColorGradientSunburst *test_pattern;

	Fbo fbo_msaa;
	Fbo fbo_downsampled;
};


/**
 * Try to set up the necessary framebuffers to render to the given
 * MSAA format.  Return false if one or more of the framebuffers is
 * incomplete.
 */
bool
PatternRenderer::try_setup(GLenum internalformat)
{
	FboConfig config_downsampled(0, pattern_width, pattern_height);
	config_downsampled.color_internalformat = internalformat;

	FboConfig config_msaa = config_downsampled;
	config_msaa.num_samples = num_samples;

	if (!(fbo_downsampled.try_setup(config_downsampled) &&
	      fbo_msaa.try_setup(config_msaa)))
		return false;

	glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo_downsampled.handle);
	glGetFramebufferAttachmentParameteriv(
		GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
		GL_FRAMEBUFFER_ATTACHMENT_RED_SIZE, &color_bits[0]);
	glGetFramebufferAttachmentParameteriv(
		GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
		GL_FRAMEBUFFER_ATTACHMENT_GREEN_SIZE, &color_bits[1]);
	glGetFramebufferAttachmentParameteriv(
		GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
		GL_FRAMEBUFFER_ATTACHMENT_BLUE_SIZE, &color_bits[2]);
	glGetFramebufferAttachmentParameteriv(
		GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
		GL_FRAMEBUFFER_ATTACHMENT_ALPHA_SIZE, &color_bits[3]);
	glGetFramebufferAttachmentParameteriv(
		GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
		GL_FRAMEBUFFER_ATTACHMENT_COMPONENT_TYPE,
		(GLint *) &component_type);

	switch (component_type) {
	case GL_INT:
		test_pattern = test_pattern_ivec4;
		break;
	case GL_UNSIGNED_INT:
		test_pattern = test_pattern_uvec4;
		break;
	case GL_UNSIGNED_NORMALIZED:
	case GL_FLOAT:
		test_pattern = test_pattern_vec4;
		break;
	default:
		printf("Unrecognized component type: %s\n",
		       piglit_get_gl_enum_name(component_type));
		piglit_report_result(PIGLIT_FAIL);
	}

	return true;
}


/**
 * Set the piglit tolerance appropriately based on the number of bits
 * in each channel.
 */
void PatternRenderer::set_piglit_tolerance()
{
	int tolerance_bits[4];

	for (int i = 0; i < 4; ++i) {
		if (color_bits[i] == 0) {
			/* For channels that have 0 bits, test to 8
			 * bits precision so we can verify that the
			 * blit puts in the appropriate value.
			 */
			tolerance_bits[i] = 8;
		} else if (color_bits[i] > 8) {
			/* For channels that have >8 bits, test to 8
			 * bits precision because we only use an 8-bit
			 * reference image.
			 */
			tolerance_bits[i] = 8;
		} else {
			tolerance_bits[i] = color_bits[i];
		}
	}

	piglit_set_tolerance_for_bits(tolerance_bits[0], tolerance_bits[1],
				      tolerance_bits[2], tolerance_bits[3]);
}


/**
 * Draw the test pattern into the MSAA framebuffer, and then blit it
 * to the downsampled FBO to force an MSAA resolve.
 */
void
PatternRenderer::draw()
{
	/* Draw into the MSAA fbo */
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo_msaa.handle);
	fbo_msaa.set_viewport();
	test_pattern->draw(TestPattern::no_projection);

	/* Blit to the downsampled fbo, forcing the image to be downsampled */
	glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo_msaa.handle);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo_downsampled.handle);
	glBlitFramebuffer(0, 0, pattern_width, pattern_height,
			  0, 0, pattern_width, pattern_height,
			  GL_COLOR_BUFFER_BIT, GL_NEAREST);
}


/**
 * Return the integer base format corresponding to a given base
 * format.
 */
GLenum
integer_base_format(GLenum base_format)
{
	switch (base_format) {
	case GL_RED:
		return GL_RED_INTEGER_EXT;
	case GL_RG:
		return GL_RG_INTEGER;
	case GL_RGB:
		return GL_RGB_INTEGER;
	case GL_RGBA:
		return GL_RGBA_INTEGER;
	case GL_ALPHA:
		return GL_ALPHA_INTEGER;
	case GL_LUMINANCE:
		return GL_LUMINANCE_INTEGER_EXT;
	case GL_LUMINANCE_ALPHA:
		return GL_LUMINANCE_ALPHA_INTEGER_EXT;
	default:
		printf("Unexpected integer base_format: %s\n",
		       piglit_get_gl_enum_name(base_format));
		piglit_report_result(PIGLIT_FAIL);
		return 0;
	}
}


/**
 * Read the image from the downsampled FBO into a newly allocated
 * array of floats and return it.
 */
float *
PatternRenderer::read_image(GLenum base_format)
{
	unsigned components = piglit_num_components(base_format);
	unsigned array_size = components*pattern_width*pattern_height;
	float *image = (float *) malloc(sizeof(float)*array_size);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo_downsampled.handle);
	if (base_format == GL_INTENSITY) {
		/* GL_INTENSITY is not allowed for ReadPixels so
		 * substitute GL_LUMINANCE.
		 */
		base_format = GL_LUMINANCE;
	}
	if (component_type == GL_INT || component_type == GL_UNSIGNED_INT) {
		int *tmp = (int *) malloc(sizeof(int)*array_size);
		glReadPixels(0, 0, pattern_width, pattern_height,
			     integer_base_format(base_format),
			     component_type, tmp);
		if (component_type == GL_INT) {
			for (unsigned i = 0; i < array_size; ++i)
				image[i] = tmp[i];
		} else {
			for (unsigned i = 0; i < array_size; ++i)
				image[i] = (unsigned) tmp[i];
		}
		free(tmp);
	} else {
		glReadPixels(0, 0, pattern_width, pattern_height, base_format,
			     GL_FLOAT, image);
	}
	return image;
}


/**
 * PatternRenderer used to render the image under test.
 */
PatternRenderer test_renderer;


/**
 * PatternRenderer used to render the reference image (in GL_RGBA
 * format).
 */
PatternRenderer ref_renderer;


/**
 * Convert the image into a format that can be easily understood by
 * visual inspection, and display it on the screen.
 *
 * Luminance and intensity values are converted to a grayscale value.
 * Alpha values are visualized by blending the image with a grayscale
 * checkerboard.
 */
void
visualize_image(float *img, GLenum base_internal_format, bool rhs)
{
	unsigned components = piglit_num_components(base_internal_format);
	float *visualization =
		(float *) malloc(sizeof(float)*3*pattern_width*pattern_height);
	for (int y = 0; y < pattern_height; ++y) {
		for (int x = 0; x < pattern_width; ++x) {
			float r = 0, g = 0, b = 0, a = 1;
			float *pixel =
				&img[(y * pattern_width + x) * components];
			switch (base_internal_format) {
			case GL_ALPHA:
				a = pixel[0];
				break;
			case GL_RGBA:
				a = pixel[3];
				/* Fall through */
			case GL_RGB:
				r = pixel[0];
				g = pixel[1];
				b = pixel[2];
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
			visualization[(y * pattern_width + x) * 3] = r;
			visualization[(y * pattern_width + x) * 3 + 1] = g;
			visualization[(y * pattern_width + x) * 3 + 2] = b;
		}
	}
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glViewport(0, 0, piglit_width, piglit_height);
	glUseProgram(0);
	glRasterPos2f(rhs ? 0 : -1, -1);
	glDrawPixels(pattern_width, pattern_height, GL_RGB, GL_FLOAT,
		     visualization);
	free(visualization);
}


/**
 * Transform the reference image (which is in GL_RGBA format) to an
 * expected image for a given base internal format, using the the
 * transformation described in the GL 3.0 spec, table 3.15 (Conversion
 * from RGBA, depth, and stencil pixel components to internal texture,
 * table, or filter components).  In short, the mapping is as follows:
 *
 * base_internal_format  mapping
 * GL_ALPHA              A -> A
 * GL_LUMINANCE          R -> L
 * GL_LUMINANCE_ALPHA    R,A -> L,A
 * GL_INTENSITY          R -> I
 * GL_RED                R -> R
 * GL_RG                 R,G -> R,G
 * GL_RGB                R,G,B -> R,G,B
 * GL_RGBA               R,G,B,A -> R,G,B,A
 */
float *
compute_expected_image(const float *ref_image, GLenum base_internal_format)
{
	unsigned components = piglit_num_components(base_internal_format);
	unsigned num_pixels = pattern_width*pattern_height;
	unsigned size = sizeof(float)*components*num_pixels;
	float *expected_image = (float *) malloc(size);
	for (unsigned i = 0; i < num_pixels; ++i) {
		float *expected = &expected_image[components*i];
		const float *ref = &ref_image[4*i];
		for (unsigned j = 0; j < components; ++j) {
			switch (base_internal_format) {
			case GL_ALPHA:
				expected[j] = ref[3];
				break;
			case GL_LUMINANCE_ALPHA:
				expected[j] = ref[j ? 3 : 0];
				break;
			default:
				expected[j] = ref[j];
				break;
			}
		}
	}
	return expected_image;
}


/**
 * Test a given internal format.
 */
enum piglit_result
test_format(const struct format_desc *format)
{
	bool pass = true;

	/* Caller messes with the clear color.  Reset it to the
	 * default.
	 */
	glClearColor(0, 0, 0, 0);

	printf("Testing %s\n", format->name);

	/* Set up the framebuffers for rendering the reference image.
	 * This shouldn't fail.
	 */
	bool setup_success = ref_renderer.try_setup(GL_RGBA);
	if (!piglit_check_gl_error(GL_NO_ERROR)) {
		printf("Error setting up reference renderbuffers\n");
		return PIGLIT_FAIL;
	}
	if (!setup_success) {
		printf("Reference framebuffer combination is unsupported\n");
		return PIGLIT_FAIL;
	}

	/* Set up the framebuffers for rendering the test image.  This
	 * might fail if the format we're testing isn't supported as a
	 * render target, and that's ok.
	 *
	 * Note: in order to be sure we test all formats which the
	 * implementations supports as render targets, we try all of
	 * them, even formats that the spec doesn't define as
	 * color-renderable (e.g. GL_LUMINANCE8, which is supported as
	 * a render target format by some drivers even though it's not
	 * officially color-renderable).  If we tried to request a
	 * color-renderable format and it wasn't supported, we would
	 * expect the framebuffer to be incomplete.  If we tried to
	 * request a non-color-renderable format and it wasn't
	 * supported, we might have received a GL error.  In either
	 * case just skip to the next format.
	 */
	setup_success = test_renderer.try_setup(format->internalformat);
	if (glGetError() != GL_NO_ERROR) {
		printf("Error setting up test renderbuffers\n");
		return PIGLIT_SKIP;
	}
	if (!setup_success) {
		printf("Unsupported framebuffer combination\n");
		return PIGLIT_SKIP;
	}

	/* Draw test and reference images, and read them into memory */
	test_renderer.set_piglit_tolerance();
	test_renderer.draw();
	float *test_image =
		test_renderer.read_image(format->base_internal_format);
	ref_renderer.draw();
	float *ref_image = ref_renderer.read_image(GL_RGBA);

	/* Compute the expected image from the reference image */
	float *expected_image =
		compute_expected_image(ref_image,
				       format->base_internal_format);

	/* Check that the test image was correct */
	unsigned num_components =
		piglit_num_components(format->base_internal_format);
	float tolerance[4];
	piglit_compute_probe_tolerance(format->base_internal_format,
				       tolerance);
	pass = piglit_compare_images_color(0, 0, pattern_width, pattern_height,
					   num_components, tolerance,
					   expected_image, test_image);

	/* Show both the test and expected images on screen so that
	 * the user can diagnose problems.
	 */
	visualize_image(test_image, format->base_internal_format, false);
	visualize_image(expected_image, format->base_internal_format, true);

	/* Finally, if any error occurred, count that as a failure. */
	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

	free(test_image);
	free(ref_image);
	free(expected_image);

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}


void
print_usage_and_exit(char *prog_name)
{
	printf("Usage: %s <num_samples>\n", prog_name);
	piglit_report_result(PIGLIT_FAIL);
}


extern "C" void
piglit_init(int argc, char **argv)
{
	if (argc < 2)
		print_usage_and_exit(argv[0]);
	char *endptr = NULL;
	num_samples = strtol(argv[1], &endptr, 0);
	if (endptr != argv[1] + strlen(argv[1]))
		print_usage_and_exit(argv[0]);

	piglit_require_gl_version(30);

	/* Skip the test if num_samples > GL_MAX_SAMPLES */
	GLint max_samples;
	glGetIntegerv(GL_MAX_SAMPLES, &max_samples);
	if (num_samples > max_samples)
		piglit_report_result(PIGLIT_SKIP);

	fbo_formats_init_test_set(0 /* core formats */,
				  GL_TRUE /* print_options */);
	test_pattern_vec4 = new ColorGradientSunburst(GL_UNSIGNED_NORMALIZED);
	test_pattern_vec4->compile();
	test_pattern_ivec4 = new ColorGradientSunburst(GL_INT);
	test_pattern_ivec4->compile();
	test_pattern_uvec4 = new ColorGradientSunburst(GL_UNSIGNED_INT);
	test_pattern_uvec4->compile();
}

extern "C" enum piglit_result
piglit_display()
{
	return fbo_formats_display(test_format);
}

};
