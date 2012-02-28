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
 * \file max-texture-size.c
 * Verify that large textures are handled properly in mesa driver.
 *
 * This test works by calling glTexImage1D/2D/3D and glTexSubImage1D/2D/3D
 * functions with different texture targets. Each texture target is tested
 * with maximum supported texture size.
 * All the calls to glTexImage2D() and glTexSubImage2D() should ensure no
 * segmentation fault / assertion failure in mesa driver.
 *
 * This test case reproduces the errors reported in:
 * 1. https://bugs.freedesktop.org/show_bug.cgi?id=44970
 *    Use GL_TEXTURE_2D and GL_RGBA16
 *
 * 2. https://bugs.freedesktop.org/show_bug.cgi?id=46303
 *
 * GL_OUT_OF_MEMORY is an expected GL error in this test case.
 *
 * \Author Anuj Phogat <anuj.phogat@gmail.com>
 */

#include "piglit-util.h"
#define COLOR_COMPONENTS 4 /*GL_RGBA*/

int piglit_width = 100, piglit_height = 100;
int piglit_window_mode = GLUT_RGBA | GLUT_DOUBLE;

GLenum target[] = {
	GL_TEXTURE_1D,
	GL_TEXTURE_2D,
	GL_TEXTURE_RECTANGLE,
	GL_TEXTURE_CUBE_MAP,
	GL_TEXTURE_3D,
	};

GLenum internalformat[] = {
	GL_RGBA8,
	GL_RGBA16,
	GL_RGBA32F,
	};

static GLenum
getMaxTarget(GLenum target)
{
	switch(target) {
	case GL_TEXTURE_1D:
	case GL_TEXTURE_2D:
		return GL_MAX_TEXTURE_SIZE;
	case GL_TEXTURE_3D:
		return GL_MAX_3D_TEXTURE_SIZE;
	case GL_TEXTURE_CUBE_MAP_ARB:
		return GL_MAX_CUBE_MAP_TEXTURE_SIZE_ARB;
	case GL_TEXTURE_RECTANGLE:
		return GL_MAX_RECTANGLE_TEXTURE_SIZE;
	case GL_RENDERBUFFER_EXT:
		return GL_MAX_RENDERBUFFER_SIZE_EXT;
	default:
		printf ("Invalid texture target\n");
		return 0;
	}
}

static GLenum
getProxyTarget(GLenum target)
{
	switch(target) {
	case GL_TEXTURE_1D:
		return GL_PROXY_TEXTURE_1D;
	case GL_TEXTURE_2D:
		return GL_PROXY_TEXTURE_2D;
	case GL_TEXTURE_3D:
		return GL_PROXY_TEXTURE_3D;
	case GL_TEXTURE_CUBE_MAP:
		return GL_PROXY_TEXTURE_CUBE_MAP;
	case GL_TEXTURE_RECTANGLE:
		return GL_PROXY_TEXTURE_RECTANGLE;
	default:
		printf ("No proxy target for this texture target\n");
		return 0;
	}
}

static bool
isValidTexSize(GLenum target, GLenum internalFormat, int sideLength)
{
	GLint texWidth;
	GLenum proxyTarget = getProxyTarget(target);

	switch(proxyTarget) {
	case GL_PROXY_TEXTURE_1D:
		glTexImage1D(proxyTarget, 0, internalFormat, sideLength, 0,
			     GL_RGBA, GL_FLOAT, 0);
		break;
	case GL_PROXY_TEXTURE_2D:
	case GL_PROXY_TEXTURE_RECTANGLE:
	case GL_PROXY_TEXTURE_CUBE_MAP_ARB:
		glTexImage2D(proxyTarget, 0, internalFormat, sideLength,
			     sideLength, 0, GL_RGBA, GL_FLOAT, 0);
		break;
	case GL_PROXY_TEXTURE_3D:
		glTexImage3D(proxyTarget, 0, internalFormat, sideLength,
			     sideLength, sideLength, 0, GL_RGBA, GL_FLOAT, 0);
		break;
	default:
		printf ("Invalid  proxy texture target\n");
	}

	glGetTexLevelParameteriv(proxyTarget, 0, GL_TEXTURE_WIDTH, &texWidth);
	return texWidth == sideLength;
}

static GLfloat *
initTexData (GLenum target, int sideLength)
{
	uint64_t nPixels;
	if (target == GL_TEXTURE_1D)
		nPixels = (uint64_t)(sideLength);
	else if (target == GL_TEXTURE_2D ||
		 target == GL_TEXTURE_RECTANGLE ||
		 target == GL_TEXTURE_CUBE_MAP)
		nPixels = (uint64_t)(sideLength) *
			  (uint64_t)(sideLength);
	else if (target == GL_TEXTURE_3D)
		nPixels = (uint64_t)(sideLength) *
			  (uint64_t)(sideLength) *
			  (uint64_t)(sideLength);

	/* Allocate sufficiently large data array and initialize to zero */
	return ((GLfloat *) calloc(nPixels * COLOR_COMPONENTS, sizeof(float)));
}

static GLboolean
ValidateTexSize (GLenum target,  GLenum internalformat, bool useProxy)
{
	int maxSide, k;
	GLfloat *pixels = NULL;
	GLenum err = GL_NO_ERROR;

	/* Query the largest supported texture size */
	glGetIntegerv(getMaxTarget(target), &maxSide);

	if (!useProxy) {
		printf("%s, Internal Format = %s, Largest Texture Size ="
		       " %d\n", piglit_get_gl_enum_name(target),
		       piglit_get_gl_enum_name(internalformat),
		       maxSide);
		/* Allocate and initialize texture data array */
		pixels = initTexData(target, maxSide);

		if ( pixels == NULL) {
			printf("Error allocating texture data array\n");
			piglit_report_result(PIGLIT_SKIP);
		}
	}
	else {
		/* Compute largest supported texture size using proxy textures */
		while(isValidTexSize(target, internalformat, maxSide))
			maxSide *= 2;
		/* First unsupported size */
		while(!isValidTexSize(target, internalformat, maxSide))
			maxSide /= 2;
		while(isValidTexSize(target, internalformat, maxSide))
			maxSide += 1;
		/* Last supported texture size */
		maxSide -= 1;
		printf("%s, Internal Format = %s, Largest Texture Size ="
		       " %d\n", piglit_get_gl_enum_name(getProxyTarget(target)),
		       piglit_get_gl_enum_name(internalformat),
		       maxSide);
	}

	switch (target) {
	case GL_TEXTURE_1D:
		if (!useProxy) {
			glTexImage1D(target, 0, internalformat, maxSide,
				     0, GL_RGBA, GL_FLOAT, NULL);

			err = glGetError();
			/* Report a GL error other than GL_OUT_OF_MEMORY */
			if (err != GL_NO_ERROR && err != GL_OUT_OF_MEMORY) {
				free(pixels);
				printf("Unexpected GL error: 0x%x\n", err);
				return false;
			}

			glTexSubImage1D(target, 0, 0, maxSide/2, GL_RGBA,
					GL_FLOAT, pixels);

			err = glGetError();
			/* Report a GL error other than GL_OUT_OF_MEMORY */
			if (err != GL_NO_ERROR && err != GL_OUT_OF_MEMORY) {
				free(pixels);
				printf("Unexpected GL error: 0x%x\n", err);
				return false;
			}
		}
		else {
			glTexImage1D(GL_PROXY_TEXTURE_1D, 0, internalformat,
				     maxSide, 0, GL_RGBA, GL_FLOAT, NULL);

			err = glGetError();
			/* Report a GL error other than GL_OUT_OF_MEMORY */
			if (err != GL_NO_ERROR && err != GL_OUT_OF_MEMORY) {
				printf("Unexpected GL error: 0x%x\n", err);
				return false;
			}
		}
		break;

	case GL_TEXTURE_2D:
		if(!useProxy) {
			glTexImage2D(target, 0, internalformat, maxSide,
				     maxSide, 0, GL_RGBA, GL_FLOAT, NULL);

			err = glGetError();
			/* Report a GL error other than GL_OUT_OF_MEMORY */
			if (err != GL_NO_ERROR && err != GL_OUT_OF_MEMORY) {
				free(pixels);
				printf("Unexpected GL error: 0x%x\n", err);
				return false;
			}

			glTexSubImage2D(target, 0, 0, 0, maxSide/2, maxSide/2,
					GL_RGBA, GL_FLOAT, pixels);

			err = glGetError();
			/* Report a GL error other than GL_OUT_OF_MEMORY */
			if (err != GL_NO_ERROR && err != GL_OUT_OF_MEMORY) {
				free(pixels);
				printf("Unexpected GL error: 0x%x\n", err);
				return false;
			}
		}

		else {
			glTexImage2D(GL_PROXY_TEXTURE_2D, 0, internalformat,
				     maxSide, maxSide, 0, GL_RGBA, GL_FLOAT,
				     NULL);

			err = glGetError();
			/* Report a GL error other than GL_OUT_OF_MEMORY */
			if (err != GL_NO_ERROR && err != GL_OUT_OF_MEMORY) {
				printf("Unexpected GL error: 0x%x\n", err);
				return false;
			}
		}
		break;

	case GL_TEXTURE_RECTANGLE:
		glTexImage2D(target, 0, internalformat, maxSide,
			     maxSide, 0, GL_RGBA, GL_FLOAT, NULL);

		err = glGetError();
		/* Report a GL error other than GL_OUT_OF_MEMORY */
		if (err != GL_NO_ERROR && err != GL_OUT_OF_MEMORY) {
			printf("Unexpected GL error: 0x%x\n", err);
			return false;
		}
		break;

	case GL_TEXTURE_3D:
		//printf("Width = %d, Height = %d, Depth =  %d\n", maxSide,
		//       maxSide, maxSide);
		if(!useProxy) {
			glTexImage3D(target, 0, internalformat, maxSide,
				     maxSide, maxSide, 0, GL_RGBA, GL_FLOAT,
				     NULL);

			err = glGetError();
			/* Report a GL error other than GL_OUT_OF_MEMORY */
			if (err != GL_NO_ERROR && err != GL_OUT_OF_MEMORY) {
				printf("Unexpected GL error: 0x%x\n", err);
				free(pixels);
				return false;
			}

			glTexSubImage3D(target, 0, 0, 0, 0, maxSide/2,
					maxSide/2, maxSide/2, GL_RGBA,
					GL_FLOAT, pixels);
			err = glGetError();
			/* Report a GL error other than GL_OUT_OF_MEMORY */
			if (err != GL_NO_ERROR && err != GL_OUT_OF_MEMORY) {
				free(pixels);
				printf("Unexpected GL error: 0x%x\n", err);
				return false;
			}
		}
		else {
			glTexImage3D(GL_PROXY_TEXTURE_3D, 0, internalformat,
				     maxSide, maxSide, maxSide, 0, GL_RGBA,
				     GL_FLOAT, NULL);

			err = glGetError();
			if (err == GL_OUT_OF_MEMORY)
				return true;

			/* Report a GL error other than GL_OUT_OF_MEMORY */
			if (err != GL_NO_ERROR) {
				printf("Unexpected GL error: 0x%x\n", err);
				return false;
			}
		}
		break;

	case GL_TEXTURE_CUBE_MAP_ARB:
		if (!useProxy) {
			for (k = 0; k < 6; k++) {
				glTexImage2D(
				GL_TEXTURE_CUBE_MAP_POSITIVE_X + k,
				0, internalformat, maxSide, maxSide, 0,
				GL_RGBA, GL_FLOAT, NULL);

				err = glGetError();
				/* Report a GL error other than GL_OUT_OF_MEMORY */
				if (err != GL_NO_ERROR && err != GL_OUT_OF_MEMORY) {
					printf("Unexpected GL error: 0x%x\n", err);
					free(pixels);
					return false;
				}
			}

			for (k = 0; k < 6; k++) {
				glTexSubImage2D(
				GL_TEXTURE_CUBE_MAP_POSITIVE_X + k,
				0, 0, 0, maxSide/2, maxSide/2, GL_RGBA,
				GL_FLOAT, pixels);

				err = glGetError();
				if (err == GL_OUT_OF_MEMORY) {
					free(pixels);
					return true;
				}

				/* Report a GL error other than GL_OUT_OF_MEMORY */
				if (err != GL_NO_ERROR && err != GL_OUT_OF_MEMORY) {
					printf("Unexpected GL error: 0x%x\n", err);
					free(pixels);
					return false;
				}
			}
		}
		else {
			glTexImage2D(GL_PROXY_TEXTURE_CUBE_MAP, 0,
			internalformat, maxSide, maxSide, 0,
			GL_RGBA, GL_FLOAT, NULL);

			err = glGetError();
			/* Report a GL error other than GL_OUT_OF_MEMORY */
			if (err != GL_NO_ERROR && err != GL_OUT_OF_MEMORY) {
				printf("Unexpected GL error: 0x%x\n", err);
				return false;
			}
		}
		break;
	}
	if (pixels)
		free(pixels);
	/* If execution reaches this point, return true */
	return true;
}

void
piglit_init(int argc, char **argv)
{
	GLuint tex;
	GLboolean pass = true;
	int i, j, useProxy;

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0, piglit_width, 0, piglit_height, -1, 1);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glClearColor(0.2, 0.2, 0.2, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	for (useProxy = 1; useProxy >= 0; useProxy--) {
		for ( i = 0; i < ARRAY_SIZE(target); i++) {

			if(!useProxy) {
				glGenTextures(1, &tex);
				glBindTexture(target[i], tex);
				glTexParameteri(target[i],
						GL_TEXTURE_MIN_FILTER,
						GL_NEAREST);
				glTexParameteri(target[i],
						GL_TEXTURE_MAG_FILTER,
						GL_NEAREST);
			}

			for (j = 0; j < ARRAY_SIZE(internalformat); j++) {
				/* Skip floating point formats if
				 * GL_ARB_texture_float is not supported
				 */
				if ((internalformat[j] == GL_RGBA16F ||
				    internalformat[j] == GL_RGBA32F) &&
				    !piglit_is_extension_supported(
				    "GL_ARB_texture_float"))
					continue;
				/* Test using proxy textures */
				 pass = ValidateTexSize (target[i],
							 internalformat[j],
							 useProxy)
					&& pass;
			}

			if (!useProxy)
				glDeleteTextures(1, &tex);
		}
	}
	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}

enum
piglit_result
piglit_display(void)
{
	return PIGLIT_FAIL;
}
