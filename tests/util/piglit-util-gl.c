/*
 * Copyright (c) The Piglit project 2007
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * on the rights to use, copy, modify, merge, publish, distribute, sub
 * license, and/or sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT.  IN NO EVENT SHALL
 * VA LINUX SYSTEM, IBM AND/OR THEIR SUPPLIERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE
 * USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#if defined(_WIN32)
#include <windows.h>
#endif

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>

#include "piglit-util-gl-common.h"


GLint piglit_ARBfp_pass_through = 0;

unsigned
piglit_num_components(GLenum base_format)
{
	switch (base_format) {
	case GL_ALPHA:
	case GL_DEPTH_COMPONENT:
	case GL_INTENSITY:
	case GL_LUMINANCE:
	case GL_RED:
		return 1;
	case GL_DEPTH_STENCIL:
	case GL_LUMINANCE_ALPHA:
	case GL_RG:
		return 2;
	case GL_RGB:
		return 3;
	case GL_RGBA:
		return 4;
	default:
		printf("Unknown num_components for %s\n",
		       piglit_get_gl_enum_name(base_format));
		piglit_report_result(PIGLIT_FAIL);
		return 0;
	}
}

/**
 * Read a pixel from the given location and compare its RGBA value to the
 * given expected values.
 *
 * Print a log message if the color value deviates from the expected value.
 * \return true if the color values match, false otherwise
 */
int piglit_probe_pixel_rgba(int x, int y, const float* expected)
{
	GLfloat probe[4];
	int i;
	GLboolean pass = GL_TRUE;

	glReadPixels(x, y, 1, 1, GL_RGBA, GL_FLOAT, probe);

	for(i = 0; i < 4; ++i) {
		if (fabs(probe[i] - expected[i]) > piglit_tolerance[i]) {
			pass = GL_FALSE;
		}
	}

	if (pass)
		return 1;

	printf("Probe at (%i,%i)\n", x, y);
	printf("  Expected: %f %f %f %f\n", expected[0], expected[1], expected[2], expected[3]);
	printf("  Observed: %f %f %f %f\n", probe[0], probe[1], probe[2], probe[3]);

	return 0;
}

int piglit_probe_pixel_rgb_silent(int x, int y, const float* expected, float *out_probe)
{
	GLfloat probe[3];
	int i;
	GLboolean pass = GL_TRUE;

	glReadPixels(x, y, 1, 1, GL_RGB, GL_FLOAT, probe);

	for(i = 0; i < 3; ++i) {
		if (fabs(probe[i] - expected[i]) > piglit_tolerance[i]) {
			pass = GL_FALSE;
		}
	}

	if (out_probe)
		memcpy(out_probe, probe, sizeof(probe));

	return pass;
}

int piglit_probe_pixel_rgba_silent(int x, int y, const float* expected, float *out_probe)
{
	GLfloat probe[4];
	int i;
	GLboolean pass = GL_TRUE;

	glReadPixels(x, y, 1, 1, GL_RGBA, GL_FLOAT, probe);

	for(i = 0; i < 4; ++i) {
		if (fabs(probe[i] - expected[i]) > piglit_tolerance[i]) {
			pass = GL_FALSE;
		}
	}

	if (out_probe)
		memcpy(out_probe, probe, sizeof(probe));

	return pass;
}

int
piglit_probe_rect_rgba(int x, int y, int w, int h, const float *expected)
{
	int i, j, p;
	GLfloat *probe;
	GLfloat *pixels = malloc(w*h*4*sizeof(float));

	glReadPixels(x, y, w, h, GL_RGBA, GL_FLOAT, pixels);

	for (j = 0; j < h; j++) {
		for (i = 0; i < w; i++) {
			probe = &pixels[(j*w+i)*4];

			for (p = 0; p < 4; ++p) {
				if (fabs(probe[p] - expected[p]) >= piglit_tolerance[p]) {
					printf("Probe at (%i,%i)\n", x+i, y+j);
					printf("  Expected: %f %f %f %f\n",
					       expected[0], expected[1], expected[2], expected[3]);
					printf("  Observed: %f %f %f %f\n",
					       probe[0], probe[1], probe[2], probe[3]);

					free(pixels);
					return 0;
				}
			}
		}
	}

	free(pixels);
	return 1;
}

int
piglit_probe_rect_rgba_int(int x, int y, int w, int h, const int *expected)
{
	int i, j, p;
	GLint *probe;
	GLint *pixels = malloc(w*h*4*sizeof(int));

	glReadPixels(x, y, w, h, GL_RGBA_INTEGER, GL_INT, pixels);

	for (j = 0; j < h; j++) {
		for (i = 0; i < w; i++) {
			probe = &pixels[(j*w+i)*4];

			for (p = 0; p < 4; ++p) {
				if (fabs(probe[p] - expected[p]) >= piglit_tolerance[p]) {
					printf("Probe at (%d,%d)\n", x+i, y+j);
					printf("  Expected: %d %d %d %d\n",
					       expected[0], expected[1], expected[2], expected[3]);
					printf("  Observed: %d %d %d %d\n",
					       probe[0], probe[1], probe[2], probe[3]);

					free(pixels);
					return 0;
				}
			}
		}
	}

	free(pixels);
	return 1;
}

int
piglit_probe_rect_rgba_uint(int x, int y, int w, int h,
			    const unsigned int *expected)
{
	int i, j, p;
	GLuint *probe;
	GLuint *pixels = malloc(w*h*4*sizeof(unsigned int));

	glReadPixels(x, y, w, h, GL_RGBA_INTEGER, GL_UNSIGNED_INT, pixels);

	for (j = 0; j < h; j++) {
		for (i = 0; i < w; i++) {
			probe = &pixels[(j*w+i)*4];

			for (p = 0; p < 4; ++p) {
				if (fabs(probe[p] - expected[p]) >= piglit_tolerance[p]) {
					printf("Probe at (%d,%d)\n", x+i, y+j);
					printf("  Expected: %u %u %u %u\n",
					       expected[0], expected[1], expected[2], expected[3]);
					printf("  Observed: %u %u %u %u\n",
					       probe[0], probe[1], probe[2], probe[3]);

					free(pixels);
					return 0;
				}
			}
		}
	}

	free(pixels);
	return 1;
}

static void
print_pixel(const float *pixel, unsigned components)
{
	int p;
	for (p = 0; p < components; ++p)
		printf(" %f", pixel[p]);
}

/**
 * Compute the appropriate tolerance for comparing images of the given
 * base format.
 */
void
piglit_compute_probe_tolerance(GLenum format, float *tolerance)
{
	int num_components, component;
	switch (format) {
	case GL_LUMINANCE_ALPHA:
		tolerance[0] = piglit_tolerance[0];
		tolerance[1] = piglit_tolerance[3];
		break;
	case GL_ALPHA:
		tolerance[0] = piglit_tolerance[3];
		break;
	default:
		num_components = piglit_num_components(format);
		for (component = 0; component < num_components; ++component)
			tolerance[component] = piglit_tolerance[component];
		break;
	}
}

/**
 * Compare two in-memory floating-point images.
 */
int
piglit_compare_images_color(int x, int y, int w, int h, int num_components,
			    const float *tolerance,
			    const float *expected_image,
			    const float *observed_image)
{
	int i, j, p;
	for (j = 0; j < h; j++) {
		for (i = 0; i < w; i++) {
			const float *expected =
				&expected_image[(j*w+i)*num_components];
			const float *probe =
				&observed_image[(j*w+i)*num_components];

			for (p = 0; p < num_components; ++p) {
				if (fabs(probe[p] - expected[p])
				    >= tolerance[p]) {
					printf("Probe at (%i,%i)\n", x+i, y+j);
					printf("  Expected:");
					print_pixel(expected, num_components);
					printf("\n  Observed:");
					print_pixel(probe, num_components);
					printf("\n");

					return 0;
				}
			}
		}
	}

	return 1;
}

/**
 * Compare the contents of the current read framebuffer with the given
 * in-memory floating-point image.
 */
int
piglit_probe_image_color(int x, int y, int w, int h, GLenum format,
			 const float *image)
{
	int c = piglit_num_components(format);
	GLfloat *pixels = malloc(w*h*c*sizeof(float));
	float tolerance[4];
	int result;

	piglit_compute_probe_tolerance(format, tolerance);

	if (format == GL_INTENSITY) {
		/* GL_INTENSITY is not allowed for ReadPixels so
		 * substitute GL_LUMINANCE.
		 */
		format = GL_LUMINANCE;
	}

	glReadPixels(x, y, w, h, format, GL_FLOAT, pixels);

	result = piglit_compare_images_color(x, y, w, h, c, tolerance, image,
					     pixels);

	free(pixels);
	return result;
}

int
piglit_probe_image_rgb(int x, int y, int w, int h, const float *image)
{
	return piglit_probe_image_color(x, y, w, h, GL_RGB, image);
}

int
piglit_probe_image_rgba(int x, int y, int w, int h, const float *image)
{
	return piglit_probe_image_color(x, y, w, h, GL_RGBA, image);
}

/**
 * Read a pixel from the given location and compare its RGB value to the
 * given expected values.
 *
 * Print a log message if the color value deviates from the expected value.
 * \return true if the color values match, false otherwise
 */
int piglit_probe_pixel_rgb(int x, int y, const float* expected)
{
	GLfloat probe[3];
	int i;
	GLboolean pass = GL_TRUE;

	glReadPixels(x, y, 1, 1, GL_RGB, GL_FLOAT, probe);


	for(i = 0; i < 3; ++i) {
		if (fabs(probe[i] - expected[i]) > piglit_tolerance[i]) {
			pass = GL_FALSE;
		}
	}

	if (pass)
		return 1;

	printf("Probe at (%i,%i)\n", x, y);
	printf("  Expected: %f %f %f\n", expected[0], expected[1], expected[2]);
	printf("  Observed: %f %f %f\n", probe[0], probe[1], probe[2]);

	return 0;
}

int
piglit_probe_rect_rgb(int x, int y, int w, int h, const float *expected)
{
	int i, j, p;
	GLfloat *probe;
	GLfloat *pixels = malloc(w*h*3*sizeof(float));

	glReadPixels(x, y, w, h, GL_RGB, GL_FLOAT, pixels);

	for (j = 0; j < h; j++) {
		for (i = 0; i < w; i++) {
			probe = &pixels[(j*w+i)*3];

			for (p = 0; p < 3; ++p) {
				if (fabs(probe[p] - expected[p]) >= piglit_tolerance[p]) {
					printf("Probe at (%i,%i)\n", x+i, y+j);
					printf("  Expected: %f %f %f\n",
					       expected[0], expected[1], expected[2]);
					printf("  Observed: %f %f %f\n",
					       probe[0], probe[1], probe[2]);

					free(pixels);
					return 0;
				}
			}
		}
	}

	free(pixels);
	return 1;
}

int
piglit_probe_rect_rgb_silent(int x, int y, int w, int h, const float *expected)
{
	int i, j, p;
	GLfloat *probe;
	GLfloat *pixels = malloc(w*h*3*sizeof(float));

	glReadPixels(x, y, w, h, GL_RGB, GL_FLOAT, pixels);

	for (j = 0; j < h; j++) {
		for (i = 0; i < w; i++) {
			probe = &pixels[(j*w+i)*3];

			for (p = 0; p < 3; ++p) {
				if (fabs(probe[p] - expected[p]) >= piglit_tolerance[p]) {
					free(pixels);
					return 0;
				}
			}
		}
	}

	free(pixels);
	return 1;
}

/**
 * Read a pixel from the given location and compare its depth value to the
 * given expected value.
 *
 * Print a log message if the depth value deviates from the expected value.
 * \return true if the depth value matches, false otherwise
 */
int piglit_probe_pixel_depth(int x, int y, float expected)
{
	GLfloat probe;
	GLfloat delta;

	glReadPixels(x, y, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &probe);

	delta = probe - expected;
	if (fabs(delta) < 0.01)
		return 1;

	printf("Probe at (%i,%i)\n", x, y);
	printf("  Expected: %f\n", expected);
	printf("  Observed: %f\n", probe);

	return 0;
}

int piglit_probe_rect_depth(int x, int y, int w, int h, float expected)
{
	int i, j;
	GLfloat *probe;
	GLfloat *pixels = malloc(w*h*sizeof(float));

	glReadPixels(x, y, w, h, GL_DEPTH_COMPONENT, GL_FLOAT, pixels);

	for (j = 0; j < h; j++) {
		for (i = 0; i < w; i++) {
			probe = &pixels[j*w+i];

			if (fabs(*probe - expected) >= 0.01) {
				printf("Probe at (%i,%i)\n", x+i, y+j);
				printf("  Expected: %f\n", expected);
				printf("  Observed: %f\n", *probe);

				free(pixels);
				return 0;
			}
		}
	}

	free(pixels);
	return 1;
}

int piglit_probe_pixel_stencil(int x, int y, unsigned expected)
{
	GLuint probe;
	glReadPixels(x, y, 1, 1, GL_STENCIL_INDEX, GL_UNSIGNED_INT, &probe);

	if (probe == expected)
		return 1;

	printf("Probe at (%i, %i)\n", x, y);
	printf("  Expected: %u\n", expected);
	printf("  Observed: %u\n", probe);

	return 0;
}

int piglit_probe_rect_stencil(int x, int y, int w, int h, unsigned expected)
{
	int i, j;
	GLuint *pixels = malloc(w*h*sizeof(GLuint));

	glReadPixels(x, y, w, h, GL_STENCIL_INDEX, GL_UNSIGNED_INT, pixels);

	for (j = 0; j < h; j++) {
		for (i = 0; i < w; i++) {
			GLuint probe = pixels[j * w + i];
			if (probe != expected) {
				printf("Probe at (%i, %i)\n", x + i, y + j);
				printf("  Expected: %u\n", expected);
				printf("  Observed: %u\n", probe);
				free(pixels);
				return 0;
			}
		}
	}

	free(pixels);
	return 1;
}

/**
 * Read a texel rectangle from the given location and compare its RGBA value to
 * the given expected values.
 *
 * Print a log message if the color value deviates from the expected value.
 * \return true if the color values match, false otherwise
 */
int piglit_probe_texel_rect_rgba(int target, int level, int x, int y,
				 int w, int h, const float* expected)
{
	GLfloat *buffer;
	GLfloat *probe;
	int i, j, p;
	GLint width;
	GLint height;

	glGetTexLevelParameteriv(target, level, GL_TEXTURE_WIDTH, &width);
	glGetTexLevelParameteriv(target, level, GL_TEXTURE_HEIGHT, &height);
	buffer = malloc(width * height * 4 * sizeof(GLfloat));

	glGetTexImage(target, level, GL_RGBA, GL_FLOAT, buffer);

	assert(x >= 0);
	assert(y >= 0);
	assert(x+w <= width);
	assert(y+h <= height);

	for (j = y; j < y+h; ++j) {
		for (i = x; i < x+w; ++i) {
			probe = &buffer[(j * width + i) * 4];

			for (p = 0; p < 4; ++p) {
				if (fabs(probe[p] - expected[p]) >= piglit_tolerance[p]) {
					printf("Probe at (%i,%i)\n", i, j);
					printf("  Expected: %f %f %f %f\n",
					       expected[0], expected[1], expected[2], expected[3]);
					printf("  Observed: %f %f %f %f\n",
					       probe[0], probe[1], probe[2], probe[3]);

					free(buffer);
					return 0;
				}
			}
		}
	}

	free(buffer);
	return 1;
}

/**
 * Read a texel from the given location and compare its RGBA value to the
 * given expected values.
 *
 * Print a log message if the color value deviates from the expected value.
 * \return true if the color values match, false otherwise
 */
int piglit_probe_texel_rgba(int target, int level, int x, int y,
			    const float* expected)
{
	return piglit_probe_texel_rect_rgba(target, level, x, y, 1, 1,
					    expected);
}

/**
 * Read a texel rectangle from the given location and compare its RGB value to
 * the given expected values.
 *
 * Print a log message if the color value deviates from the expected value.
 * \return true if the color values match, false otherwise
 */
int piglit_probe_texel_rect_rgb(int target, int level, int x, int y,
				int w, int h, const float* expected)
{
	GLfloat *buffer;
	GLfloat *probe;
	int i, j, p;
	GLint width;
	GLint height;

	glGetTexLevelParameteriv(target, level, GL_TEXTURE_WIDTH, &width);
	glGetTexLevelParameteriv(target, level, GL_TEXTURE_HEIGHT, &height);
	buffer = malloc(width * height * 3 * sizeof(GLfloat));

	glGetTexImage(target, level, GL_RGB, GL_FLOAT, buffer);

	assert(x >= 0);
	assert(y >= 0);
	assert(x+w <= width);
	assert(y+h <= height);

	for (j = y; j < y+h; ++j) {
		for (i = x; i < x+w; ++i) {
			probe = &buffer[(j * width + i) * 3];

			for (p = 0; p < 3; ++p) {
				if (fabs(probe[p] - expected[p]) >= piglit_tolerance[p]) {
					printf("Probe at (%i,%i)\n", i, j);
					printf("  Expected: %f %f %f\n",
					       expected[0], expected[1], expected[2]);
					printf("  Observed: %f %f %f\n",
					       probe[0], probe[1], probe[2]);

					free(buffer);
					return 0;
				}
			}
		}
	}

	free(buffer);
	return 1;
}

/**
 * Read a texel from the given location and compare its RGB value to the
 * given expected values.
 *
 * Print a log message if the color value deviates from the expected value.
 * \return true if the color values match, false otherwise
 */
int piglit_probe_texel_rgb(int target, int level, int x, int y,
			   const float *expected)
{
	return piglit_probe_texel_rect_rgb(target, level, x, y, 1, 1, expected);
}

int piglit_probe_rect_halves_equal_rgba(int x, int y, int w, int h)
{
	int i, j, p;
	GLfloat probe1[4];
	GLfloat probe2[4];
	GLubyte *pixels = malloc(w*h*4*sizeof(GLubyte));

	glReadPixels(x, y, w, h, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

	for (j = 0; j < h; j++) {
		for (i = 0; i < w / 2; i++) {
			GLubyte *pixel1 = &pixels[4 * (j * w + i)];
			GLubyte *pixel2 = &pixels[4 * (j * w + w / 2 + i)];

			for (p = 0; p < 4; ++p) {
				probe1[p] = pixel1[p] / 255.0f;
				probe2[p] = pixel2[p] / 255.0f;
			}

			for (p = 0; p < 4; ++p) {
				if (fabs(probe1[p] - probe2[p]) >= piglit_tolerance[p]) {
					printf("Probe at (%i,%i)\n", x+i, x+j);
					printf("  Left: %f %f %f %f\n",
					       probe1[0], probe1[1], probe1[2], probe1[3]);
					printf("  Right: %f %f %f %f\n",
					       probe2[0], probe2[1], probe2[2], probe2[3]);

					free(pixels);
					return 0;
				}
			}
		}
	}

	free(pixels);
	return 1;
}

int piglit_use_fragment_program(void)
{
	static const char source[] =
		"!!ARBfp1.0\n"
		"MOV	result.color, fragment.color;\n"
		"END\n"
		;

	glewInit();
	if (!piglit_is_extension_supported("GL_ARB_fragment_program"))
		return 0;

	piglit_ARBfp_pass_through =
		piglit_compile_program(GL_FRAGMENT_PROGRAM_ARB, source);

	return (piglit_ARBfp_pass_through != 0);
}

void piglit_require_fragment_program(void)
{
	if (!piglit_use_fragment_program()) {
		printf("GL_ARB_fragment_program not supported.\n");
		piglit_report_result(PIGLIT_SKIP);
		exit(1);
	}
}

int piglit_use_vertex_program(void)
{
	glewInit();
	return piglit_is_extension_supported("GL_ARB_vertex_program");
}

void piglit_require_vertex_program(void)
{
	if (!piglit_use_vertex_program()) {
		printf("GL_ARB_vertex_program not supported.\n");
		piglit_report_result(PIGLIT_SKIP);
		exit(1);
	}
}

GLuint piglit_compile_program(GLenum target, const char* text)
{
	GLuint program;
	GLint errorPos;

	glGenProgramsARB(1, &program);
	glBindProgramARB(target, program);
	glProgramStringARB(
			target,
			GL_PROGRAM_FORMAT_ASCII_ARB,
			strlen(text),
			(const GLubyte *)text);
	glGetIntegerv(GL_PROGRAM_ERROR_POSITION_ARB, &errorPos);
	if (glGetError() != GL_NO_ERROR || errorPos != -1) {
		int l = FindLine(text, errorPos);
		int a;

		fprintf(stderr, "Compiler Error (pos=%d line=%d): %s\n",
			errorPos, l,
			(char *) glGetString(GL_PROGRAM_ERROR_STRING_ARB));

		for (a=-10; a<10; a++)
		{
			if (errorPos+a < 0)
				continue;
			if (errorPos+a >= strlen(text))
				break;
			fprintf(stderr, "%c", text[errorPos+a]);
		}
		fprintf(stderr, "\nin program:\n%s", text);
		piglit_report_result(PIGLIT_FAIL);
	}
	if (!glIsProgramARB(program)) {
		fprintf(stderr, "glIsProgramARB failed\n");
		piglit_report_result(PIGLIT_FAIL);
	}

	return program;
}

void
piglit_escape_exit_key(unsigned char key, int x, int y)
{
	(void) x;
	(void) y;
	switch (key) {
		case 27:
			exit(0);
			break;
	}
	glutPostRedisplay();
}

/**
 * Convenience function to draw a triangle.
 */
GLvoid
piglit_draw_triangle(float x1, float y1, float x2, float y2,
		     float x3, float y3)
{
	piglit_draw_triangle_z(0.0, x1, y1, x2, y2, x3, y3);
}

/**
 * Convenience function to draw a triangle at a given depth.
 */
GLvoid
piglit_draw_triangle_z(float z, float x1, float y1, float x2, float y2,
		     float x3, float y3)
{
	float verts[3][4];

	verts[0][0] = x1;
	verts[0][1] = y1;
	verts[0][2] = z;
	verts[0][3] = 1.0;
	verts[1][0] = x2;
	verts[1][1] = y2;
	verts[1][2] = z;
	verts[1][3] = 1.0;
	verts[2][0] = x3;
	verts[2][1] = y3;
	verts[2][2] = z;
	verts[2][3] = 1.0;

	glVertexPointer(4, GL_FLOAT, 0, verts);
	glEnableClientState(GL_VERTEX_ARRAY);

	glDrawArrays(GL_TRIANGLES, 0, 3);

	glDisableClientState(GL_VERTEX_ARRAY);
}

/**
 * Convenience function to draw an axis-aligned rectangle.
 */
GLvoid
piglit_draw_rect(float x, float y, float w, float h)
{
	float verts[4][4];

	verts[0][0] = x;
	verts[0][1] = y;
	verts[0][2] = 0.0;
	verts[0][3] = 1.0;
	verts[1][0] = x + w;
	verts[1][1] = y;
	verts[1][2] = 0.0;
	verts[1][3] = 1.0;
	verts[2][0] = x + w;
	verts[2][1] = y + h;
	verts[2][2] = 0.0;
	verts[2][3] = 1.0;
	verts[3][0] = x;
	verts[3][1] = y + h;
	verts[3][2] = 0.0;
	verts[3][3] = 1.0;

	glVertexPointer(4, GL_FLOAT, 0, verts);
	glEnableClientState(GL_VERTEX_ARRAY);

	glDrawArrays(GL_QUADS, 0, 4);

	glDisableClientState(GL_VERTEX_ARRAY);
}

/**
 * Convenience function to draw an axis-aligned rectangle.
 */
GLvoid
piglit_draw_rect_z(float z, float x, float y, float w, float h)
{
	float verts[4][4];

	verts[0][0] = x;
	verts[0][1] = y;
	verts[0][2] = z;
	verts[0][3] = 1.0;
	verts[1][0] = x + w;
	verts[1][1] = y;
	verts[1][2] = z;
	verts[1][3] = 1.0;
	verts[2][0] = x + w;
	verts[2][1] = y + h;
	verts[2][2] = z;
	verts[2][3] = 1.0;
	verts[3][0] = x;
	verts[3][1] = y + h;
	verts[3][2] = z;
	verts[3][3] = 1.0;

	glVertexPointer(4, GL_FLOAT, 0, verts);
	glEnableClientState(GL_VERTEX_ARRAY);

	glDrawArrays(GL_QUADS, 0, 4);

	glDisableClientState(GL_VERTEX_ARRAY);
}

/**
 * Convenience function to draw an axis-aligned rectangle
 * with texture coordinates.
 */
GLvoid
piglit_draw_rect_tex(float x, float y, float w, float h,
                     float tx, float ty, float tw, float th)
{
	float verts[4][4];
	float tex[4][2];

	verts[0][0] = x;
	verts[0][1] = y;
	verts[0][2] = 0.0;
	verts[0][3] = 1.0;
	tex[0][0] = tx;
	tex[0][1] = ty;
	verts[1][0] = x + w;
	verts[1][1] = y;
	verts[1][2] = 0.0;
	verts[1][3] = 1.0;
	tex[1][0] = tx + tw;
	tex[1][1] = ty;
	verts[2][0] = x + w;
	verts[2][1] = y + h;
	verts[2][2] = 0.0;
	verts[2][3] = 1.0;
	tex[2][0] = tx + tw;
	tex[2][1] = ty + th;
	verts[3][0] = x;
	verts[3][1] = y + h;
	verts[3][2] = 0.0;
	verts[3][3] = 1.0;
	tex[3][0] = tx;
	tex[3][1] = ty + th;

	glVertexPointer(4, GL_FLOAT, 0, verts);
	glTexCoordPointer(2, GL_FLOAT, 0, tex);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	glDrawArrays(GL_QUADS, 0, 4);

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
}


/**
 * Convenience function to configure an abitrary orthogonal projection matrix
 */
void
piglit_gen_ortho_projection(double left, double right, double bottom,
			    double top, double near_val, double far_val,
			    GLboolean push)
{
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
	if (push)
		glPushMatrix();
        glOrtho(left, right, bottom, top, near_val, far_val);

        glMatrixMode(GL_MODELVIEW);
	if (push)
		glPushMatrix();
        glLoadIdentity();
}


/**
 * Convenience function to configure projection matrix for window coordinates
 */
void
piglit_ortho_projection(int w, int h, GLboolean push)
{
        /* Set up projection matrix so we can just draw using window
         * coordinates.
         */
	piglit_gen_ortho_projection(0, w, 0, h, -1, 1, push);
}

/**
 * Convenience function to configure frustum projection.
 */
void
piglit_frustum_projection(GLboolean push, double l, double r, double b,
			  double t, double n, double f)
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	if (push)
		glPushMatrix();
	glFrustum(l, r, b, t, n, f);

	glMatrixMode(GL_MODELVIEW);
	if (push)
		glPushMatrix();
	glLoadIdentity();
}



/**
 * Generate a checkerboard texture
 *
 * \param tex                Name of the texture to be used.  If \c tex is
 *                           zero, a new texture name will be generated.
 * \param level              Mipmap level the checkerboard should be written to
 * \param width              Width of the texture image
 * \param height             Height of the texture image
 * \param horiz_square_size  Size of each checkerboard tile along the X axis
 * \param vert_square_size   Size of each checkerboard tile along the Y axis
 * \param black              RGBA color to be used for "black" tiles
 * \param white              RGBA color to be used for "white" tiles
 *
 * A texture with alternating black and white squares in a checkerboard
 * pattern is generated.  The texture data is written to LOD \c level of
 * the texture \c tex.
 *
 * If \c tex is zero, a new texture created.  This texture will have several
 * texture parameters set to non-default values:
 *
 *  - S and T wrap modes will be set to \c GL_CLAMP_TO_BORDER.
 *  - Border color will be set to { 1.0, 0.0, 0.0, 1.0 }.
 *  - Min and mag filter will be set to \c GL_NEAREST.
 *
 * \return
 * Name of the texture.  In addition, this texture will be bound to the
 * \c GL_TEXTURE_2D target of the currently active texture unit.
 */
GLuint
piglit_checkerboard_texture(GLuint tex, unsigned level,
			    unsigned width, unsigned height,
			    unsigned horiz_square_size,
			    unsigned vert_square_size,
			    const float *black, const float *white)
{
	static const GLfloat border_color[4] = { 1.0, 0.0, 0.0, 1.0 };
	unsigned i;
	unsigned j;

	float *const tex_data = malloc(width * height * (4 * sizeof(float)));
	float *texel = tex_data;

	for (i = 0; i < height; i++) {
		const unsigned row = i / vert_square_size;

		for (j = 0; j < width; j++) {
			const unsigned col = j / horiz_square_size;

			if ((row ^ col) & 1) {
				memcpy(texel, white, 4 * sizeof(float));
			} else {
				memcpy(texel, black, 4 * sizeof(float));
			}

			texel += 4;
		}
	}


	if (tex == 0) {
		glGenTextures(1, &tex);

		glBindTexture(GL_TEXTURE_2D, tex);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
				GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
				GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
				GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,
				GL_CLAMP_TO_BORDER);
		glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR,
				 border_color);
	} else {
		glBindTexture(GL_TEXTURE_2D, tex);
	}

	glTexImage2D(GL_TEXTURE_2D, level, GL_RGBA, width, height, 0, GL_RGBA,
		     GL_FLOAT, tex_data);

	return tex;
}

/**
 * Generates a 8x8 mipmapped texture whose layers contain solid r, g, b, and w.
 */
GLuint
piglit_miptree_texture()
{
	GLfloat *data;
	int size, i, level;
	GLuint tex;
	const float color_wheel[4][4] = {
		{1, 0, 0, 1}, /* red */
		{0, 1, 0, 1}, /* green */
		{0, 0, 1, 1}, /* blue */
		{1, 1, 1, 1}, /* white */
	};

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
			GL_NEAREST_MIPMAP_NEAREST);

	for (level = 0; level < 4; ++level) {
		size = 8 >> level;

		data = malloc(size*size*4*sizeof(GLfloat));
		for (i = 0; i < size * size; ++i) {
			memcpy(data + 4 * i, color_wheel[level],
			       4 * sizeof(GLfloat));
		}
		glTexImage2D(GL_TEXTURE_2D, level, GL_RGBA,
			     size, size, 0, GL_RGBA, GL_FLOAT, data);
		free(data);
	}
	return tex;
}

/**
 * Generates a texture with the given internalFormat, w, h with a
 * teximage of r, g, b w quadrants.
 *
 * Note that for compressed teximages, where the blocking would be
 * problematic, we assign the whole layers at w == 4 to red, w == 2 to
 * green, and w == 1 to blue.
 */
GLuint
piglit_rgbw_texture(GLenum format, int w, int h, GLboolean mip,
		    GLboolean alpha, GLenum basetype)
{
	GLfloat *data;
	int size, x, y, level;
	GLuint tex;
	float red[4]   = {1.0, 0.0, 0.0, 0.0};
	float green[4] = {0.0, 1.0, 0.0, 0.25};
	float blue[4]  = {0.0, 0.0, 1.0, 0.5};
	float white[4] = {1.0, 1.0, 1.0, 1.0};

	if (!alpha) {
		red[3] = 1.0;
		green[3] = 1.0;
		blue[3] = 1.0;
		white[3] = 1.0;
	}

	switch (basetype) {
	case GL_UNSIGNED_NORMALIZED:
		break;

	case GL_SIGNED_NORMALIZED:
		for (x = 0; x < 4; x++) {
			red[x] = red[x] * 2 - 1;
			green[x] = green[x] * 2 - 1;
			blue[x] = blue[x] * 2 - 1;
			white[x] = white[x] * 2 - 1;
		}
		break;

	case GL_FLOAT:
		for (x = 0; x < 4; x++) {
			red[x] = red[x] * 10 - 5;
			green[x] = green[x] * 10 - 5;
			blue[x] = blue[x] * 10 - 5;
			white[x] = white[x] * 10 - 5;
		}
		break;

	default:
		assert(0);
	}

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	if (mip) {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
				GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
				GL_LINEAR_MIPMAP_NEAREST);
	} else {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
				GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
				GL_NEAREST);
	}
	data = malloc(w * h * 4 * sizeof(GLfloat));

	for (level = 0, size = w > h ? w : h; size > 0; level++, size >>= 1) {
		for (y = 0; y < h; y++) {
			for (x = 0; x < w; x++) {
				const float *color;

				if (x < w / 2 && y < h / 2)
					color = red;
				else if (y < h / 2)
					color = green;
				else if (x < w / 2)
					color = blue;
				else
					color = white;

				switch (format) {
				case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:
				case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
				case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:
				case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:
				case GL_COMPRESSED_RGB_FXT1_3DFX:
				case GL_COMPRESSED_RGBA_FXT1_3DFX:
				case GL_COMPRESSED_RED_RGTC1:
				case GL_COMPRESSED_SIGNED_RED_RGTC1:
				case GL_COMPRESSED_RG_RGTC2:
				case GL_COMPRESSED_SIGNED_RG_RGTC2:
				case GL_COMPRESSED_SRGB_S3TC_DXT1_EXT:
				case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT:
				case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT:
				case GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT:
					if (size == 4)
						color = red;
					else if (size == 2)
						color = green;
					else if (size == 1)
						color = blue;
					break;
				default:
					break;
				}

				memcpy(data + (y * w + x) * 4, color,
				       4 * sizeof(float));
			}
		}
		glTexImage2D(GL_TEXTURE_2D, level,
			     format,
			     w, h, 0,
			     GL_RGBA, GL_FLOAT, data);

		if (!mip)
			break;

		if (w > 1)
			w >>= 1;
		if (h > 1)
			h >>= 1;
	}
	free(data);
	return tex;
}

GLuint
piglit_depth_texture(GLenum target, GLenum internalformat, int w, int h, int d, GLboolean mip)
{
	void *data;
	float *f = NULL, *f2 = NULL;
	unsigned int  *i = NULL;
	int size, x, y, level, layer;
	GLuint tex;
	GLenum type, format;

	glGenTextures(1, &tex);
	glBindTexture(target, tex);
	glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	if (mip) {
		glTexParameteri(target, GL_TEXTURE_MAG_FILTER,
				GL_LINEAR);
		glTexParameteri(target, GL_TEXTURE_MIN_FILTER,
				GL_LINEAR_MIPMAP_NEAREST);
	} else {
		glTexParameteri(target, GL_TEXTURE_MAG_FILTER,
				GL_NEAREST);
		glTexParameteri(target, GL_TEXTURE_MIN_FILTER,
				GL_NEAREST);
	}
	data = malloc(w * h * 4 * sizeof(GLfloat));

	if (internalformat == GL_DEPTH_STENCIL_EXT ||
	    internalformat == GL_DEPTH24_STENCIL8_EXT) {
		format = GL_DEPTH_STENCIL_EXT;
		type = GL_UNSIGNED_INT_24_8_EXT;
		i = data;
	} else if (internalformat == GL_DEPTH32F_STENCIL8) {
		format = GL_DEPTH_STENCIL;
		type = GL_FLOAT_32_UNSIGNED_INT_24_8_REV;
		f2 = data;
	} else {
		format = GL_DEPTH_COMPONENT;
		type = GL_FLOAT;
		f = data;
	}

	for (level = 0, size = w > h ? w : h; size > 0; level++, size >>= 1) {
		for (y = 0; y < h; y++) {
			for (x = 0; x < w; x++) {
				float val = (float)(x) / (w - 1);
				if (f)
					f[y * w + x] = val;
				else if (f2)
					f2[(y * w + x)*2] = val;
				else
					i[y * w + x] = 0xffffff00 * val;
			}
		}

		switch (target) {
		case GL_TEXTURE_1D:
			glTexImage1D(target, level,
				     internalformat,
				     w, 0,
				     format, type, data);
			break;

		case GL_TEXTURE_1D_ARRAY:
		case GL_TEXTURE_2D:
		case GL_TEXTURE_RECTANGLE:
			glTexImage2D(target, level,
				     internalformat,
				     w, h, 0,
				     format, type, data);
			break;

		case GL_TEXTURE_2D_ARRAY:
			glTexImage3D(target, level,
				     internalformat,
				     w, h, d, 0,
				     format, type, NULL);
			for (layer = 0; layer < d; layer++) {
				glTexSubImage3D(target, level,
						0, 0, layer, w, h, 1,
						format, type, data);
			}
			break;

		default:
			assert(0);
		}

		if (!mip)
			break;

		if (w > 1)
			w >>= 1;
		if (h > 1)
			h >>= 1;
	}
	free(data);
	return tex;
}

/**
 * Require transform feedback.
 *
 * Transform feedback may either be provided by GL 3.0 or
 * EXT_transform_feedback.
 */
void
piglit_require_transform_feedback(void)
{
	if (!(piglit_get_gl_version() >= 30 ||
	      piglit_is_extension_supported("GL_EXT_transform_feedback"))) {
		printf("Transform feedback not supported.\n");
		piglit_report_result(PIGLIT_SKIP);
		exit(1);
	}
}
