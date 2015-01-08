/* BEGIN_COPYRIGHT -*- glean -*-
 *
 * Copyright (C) 2007, 2014  Intel Corporation
 * Copyright (C) 1999  Allen Akin  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY
 * KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 * PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
 * AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * END_COPYRIGHT
 */

/** @file pbo.c
 *
 * Test OpenGL Extension GL_ARB_pixel_buffer_object
 *
 * Authors:
 * Shuang He <shuang.he@intel.com>
 * Adapted to Piglit by Laura Ekstrand <laura@jlekstrand.net>, November 2014.
 */

#include "piglit-util-gl.h"

#define WINSIZE 100
#define BUFFER_OFFSET(i) ((char *)NULL + (i))

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_RGBA |
		PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

void
piglit_init(int argc, char **argv)
{
	piglit_require_extension("GL_ARB_pixel_buffer_object");

	piglit_ortho_projection(piglit_width, piglit_height, GL_FALSE);
}

static void
report_failure(const char *msg, const int line)
{
	printf("FAILURE: %s (at pbo.c: %d)\n", msg, line);
}

#define REPORT_FAILURE(MSG) report_failure(MSG, __LINE__)

#define TEXSIZE 64

enum piglit_result
test_sanity(void)
{
	GLuint pbs[1];
	GLuint pb_binding;

	glGetIntegerv(GL_PIXEL_UNPACK_BUFFER_BINDING_ARB,
		      (GLint *) & pb_binding);
	if (pb_binding != 0) {
		REPORT_FAILURE("Failed to bind unpack pixel buffer object");
		return PIGLIT_FAIL;
	}

	glGetIntegerv(GL_PIXEL_PACK_BUFFER_BINDING_ARB,
		      (GLint *) & pb_binding);
	if (pb_binding != 0) {
		REPORT_FAILURE("Failed to bind pack pixel buffer object");
		return PIGLIT_FAIL;
	}

	glGenBuffersARB(1, pbs);

	if (glIsBufferARB(pbs[0]) != GL_FALSE) {
		REPORT_FAILURE("glIsBufferARB failed");
		return PIGLIT_FAIL;
	}

	glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, pbs[0]);
	glGetIntegerv(GL_PIXEL_UNPACK_BUFFER_BINDING_ARB,
		      (GLint *) & pb_binding);
	glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, 0);
	if (pb_binding != pbs[0]) {
		REPORT_FAILURE("Failed to bind unpack pixel buffer object");
		return PIGLIT_FAIL;
	}

	glBindBufferARB(GL_PIXEL_PACK_BUFFER_ARB, pbs[0]);
	glGetIntegerv(GL_PIXEL_PACK_BUFFER_BINDING_ARB,
		      (GLint *) & pb_binding);
	glBindBufferARB(GL_PIXEL_PACK_BUFFER_ARB, 0);
	if (pb_binding != pbs[0]) {
		REPORT_FAILURE("Failed to bind unpack pixel buffer object");
		return PIGLIT_FAIL;
	}

	glDeleteBuffersARB(1, pbs);

	if (glIsBufferARB(pbs[0]) == GL_TRUE) {
		REPORT_FAILURE("glIsBufferARB failed");
		return PIGLIT_FAIL;
	}

	return PIGLIT_PASS;
}

enum piglit_result
test_draw_pixels(void)
{
	int use_unpack;
	int use_pack;
	GLuint pb_pack[1];
	GLuint pb_unpack[1];
	GLubyte buf[WINSIZE * WINSIZE * 4];
	GLubyte t[TEXSIZE * TEXSIZE * 4];
	int i, j;
	GLubyte * pbo_pack_mem = NULL;
	GLubyte black[4] = { 0, 0, 0, 255 };
	bool pass = true;
	GLubyte expected[WINSIZE * WINSIZE * 4];

	glBindBuffer(GL_PIXEL_UNPACK_BUFFER_ARB, 0);
	glBindBuffer(GL_PIXEL_PACK_BUFFER_ARB, 0);

	for (use_unpack = 0; use_unpack < 2; use_unpack++) {
		for (use_pack = 0; use_pack < 2; use_pack++) {
			GLubyte *pbo_mem = NULL;
			glClearColor(0.0, 0.0, 0.0, 1.0);
			glClear(GL_COLOR_BUFFER_BIT);
			if (use_unpack) {
				glGenBuffersARB(1, pb_unpack);
				glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB,
						pb_unpack[0]);
				glBufferDataARB(GL_PIXEL_UNPACK_BUFFER_ARB,
						TEXSIZE * TEXSIZE * 4 *
						sizeof(GLubyte), NULL,
						GL_STREAM_DRAW);
			}
			if (use_unpack) {
				pbo_mem = (GLubyte *) glMapBufferARB(
						GL_PIXEL_UNPACK_BUFFER_ARB,
						GL_WRITE_ONLY);
			}
			else {
				pbo_mem = t;
			}

			/* Fill the buffer */
			for (i = 0; i < TEXSIZE; i++)
				for (j = 0; j < TEXSIZE; j++) {
					int idx = 4 * (i * TEXSIZE + j);
					pbo_mem[idx + 0] = i % 256;
					pbo_mem[idx + 1] = i % 256;
					pbo_mem[idx + 2] = i % 256;
					pbo_mem[idx + 3] = 0;
				}

			if (use_unpack) {
				glUnmapBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB);
				glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB,
						0);
			}

			/* Draw the buffer */
			if (use_unpack) {
				glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB,
						pb_unpack[0]);
				glDrawPixels(TEXSIZE, TEXSIZE, GL_BGRA,
					     GL_UNSIGNED_BYTE, NULL);
				glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB,
						0);
			}
			else {
				glDrawPixels(TEXSIZE, TEXSIZE, GL_BGRA,
					     GL_UNSIGNED_BYTE, pbo_mem);
			}

			if (!piglit_automatic)
				piglit_present_results();

			/* Check the result */
			if (use_pack) {
				glGenBuffersARB(1, pb_pack);
				glBindBufferARB(GL_PIXEL_PACK_BUFFER_ARB,
						pb_pack[0]);
				glBufferDataARB(GL_PIXEL_PACK_BUFFER_ARB,
						WINSIZE * WINSIZE * 4 *
						sizeof(GL_UNSIGNED_BYTE),
						NULL, GL_STREAM_DRAW);
				glReadPixels(0, 0, WINSIZE, WINSIZE,
					     GL_BGRA, GL_UNSIGNED_BYTE, NULL);
				pbo_pack_mem = (GLubyte *) glMapBufferARB(
						GL_PIXEL_PACK_BUFFER_ARB,
						GL_READ_ONLY);
			}
			else {
				pbo_pack_mem = buf;
				glReadPixels(0, 0, WINSIZE, WINSIZE,
					     GL_BGRA,
					     GL_UNSIGNED_BYTE, pbo_pack_mem);
			}

			/* Make expected. */
			for (j = 0; j < WINSIZE; j++) {
				for (i = 0; i < WINSIZE; i++) {
					int idx = (j * WINSIZE + i) * 4;
					if (i < TEXSIZE && j < TEXSIZE) {
						expected[idx + 0] = j % 256;
						expected[idx + 1] = j % 256;
						expected[idx + 2] = j % 256;
						expected[idx + 3] = 0;
					}
					else {
						expected[idx + 0] = black[0];
						expected[idx + 1] = black[1];
						expected[idx + 2] = black[2];
						expected[idx + 3] = black[3];
					}
				}
			}

			pass &= piglit_compare_images_ubyte(0, 0, WINSIZE,
							    WINSIZE,
							    expected,
							    pbo_pack_mem);

			if (use_pack) {
				glBindBuffer(GL_PIXEL_PACK_BUFFER_ARB, 0);
				glDeleteBuffersARB(1, pb_pack);
			}

			if (use_unpack) {
				glBindBuffer(GL_PIXEL_UNPACK_BUFFER_ARB, 0);
				glDeleteBuffersARB(1, pb_unpack);
			}

		}
	}

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}


enum piglit_result
test_pixel_map(void)
{
	int use_unpack;
	int use_pack;
	GLuint pb_pack[1];
	GLuint pb_unpack[1];
	int i;
	int size;
	int max;
	GLushort *pbo_mem;

	glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, 0);
	glBindBufferARB(GL_PIXEL_PACK_BUFFER_ARB, 0);

	glGetIntegerv(GL_MAX_PIXEL_MAP_TABLE, &max);

	for (use_pack = 0; use_pack < 2; use_pack++) {
		for (use_unpack = 0; use_unpack < 2;
		   use_unpack++) {
			glClearColor(0.0, 0.0, 0.0, 1.0);
			glClear(GL_COLOR_BUFFER_BIT);
			if (use_unpack) {
				glGenBuffersARB(1, pb_unpack);
				glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB,
						pb_unpack[0]);
				glBufferDataARB(GL_PIXEL_UNPACK_BUFFER_ARB,
						max * sizeof(GLushort), NULL,
						GL_STREAM_DRAW);
			}
			pbo_mem = NULL;
			if (use_unpack) {
				pbo_mem = (GLushort *) glMapBufferARB(
						GL_PIXEL_UNPACK_BUFFER_ARB,
						GL_WRITE_ONLY);
			}
			else {
				pbo_mem = (GLushort *)
					malloc(sizeof(GLushort) * max);
			}
			for (i = 0; i < max; i++)
				pbo_mem[i] = max - i - 1;

			if (use_unpack) {
				glUnmapBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB);
				glPixelMapusv(GL_PIXEL_MAP_R_TO_R, max, NULL);
				glPixelMapusv(GL_PIXEL_MAP_G_TO_G, max, NULL);
				glPixelMapusv(GL_PIXEL_MAP_B_TO_B, max, NULL);
				glPixelMapusv(GL_PIXEL_MAP_A_TO_A, max, NULL);
				glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB,
						0);
			}
			else {
				glPixelMapusv(GL_PIXEL_MAP_R_TO_R, max,
					      pbo_mem);
				glPixelMapusv(GL_PIXEL_MAP_G_TO_G, max,
					      pbo_mem);
				glPixelMapusv(GL_PIXEL_MAP_B_TO_B, max,
					      pbo_mem);
				glPixelMapusv(GL_PIXEL_MAP_A_TO_A, max,
					      pbo_mem);
				free(pbo_mem);
			}


			glGetIntegerv(GL_PIXEL_MAP_R_TO_R_SIZE, &size);
			if (size != max) {
				REPORT_FAILURE("glPixelMap failed");
				return PIGLIT_FAIL;
			}
			glPixelTransferi(GL_MAP_COLOR, GL_FALSE);

			/* Read back pixel map */
			if (use_pack) {
				glGenBuffersARB(1, pb_pack);
				glBindBufferARB(GL_PIXEL_PACK_BUFFER_ARB,
						pb_pack[0]);
				glBufferDataARB(GL_PIXEL_PACK_BUFFER_ARB,
						max * sizeof(GLushort),
						NULL, GL_STREAM_DRAW);
				glGetPixelMapusv(GL_PIXEL_MAP_R_TO_R, NULL);
				pbo_mem = (GLushort *) glMapBufferARB(
						GL_PIXEL_PACK_BUFFER_ARB,
						GL_READ_ONLY);
			}
			else {
				pbo_mem = (GLushort *)
					malloc(sizeof(GLushort) * max);
				glGetPixelMapusv(GL_PIXEL_MAP_R_TO_R, pbo_mem);
			}

			for (i = 0; i < max; i++) {
				if (pbo_mem[i] != (255 - i)) {
					REPORT_FAILURE("get PixelMap failed");
					return PIGLIT_FAIL;
				}
			}


			if (use_pack) {
				glUnmapBufferARB(GL_PIXEL_PACK_BUFFER_ARB);
				glBindBufferARB(GL_PIXEL_PACK_BUFFER_ARB, 0);
				glDeleteBuffersARB(1, pb_pack);
			}
			else {
				free(pbo_mem);
			}

			if (use_unpack) {
				glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB,
						0);
				glDeleteBuffersARB(1, pb_unpack);
			}

			if (!piglit_automatic)
				piglit_present_results();

		}
	}

	return PIGLIT_PASS;
}

enum piglit_result
test_bitmap(void)
{
	GLuint pb_unpack[1];
	GLuint pb_pack[1];
	int use_unpack = 1;
	int use_pack = 0;
	GLubyte bitmap[TEXSIZE * TEXSIZE / 8];
	GLfloat buf[WINSIZE * WINSIZE * 3];
	GLfloat white[3] = { 1.0, 1.0, 1.0 };
	GLfloat black[3] = { 0.0, 0.0, 0.0 };
	int i, j;
	GLubyte *pbo_unpack_mem = NULL;
	GLfloat *pbo_pack_mem = NULL;
	GLfloat expected[WINSIZE * WINSIZE * 3];
	float tolerance[4];
	bool pass = true;

	glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, 0);
	glBindBufferARB(GL_PIXEL_PACK_BUFFER_ARB, 0);

	for (use_pack = 0; use_pack < 2; use_pack++) {
		for (use_unpack = 0; use_unpack < 2;
		   use_unpack++) {
			glClearColor(0.0, 0.0, 0.0, 1.0);
			glClear(GL_COLOR_BUFFER_BIT);

			if (use_unpack) {
				glGenBuffersARB(1, pb_unpack);
				glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB,
						pb_unpack[0]);
				glBufferDataARB(GL_PIXEL_UNPACK_BUFFER_ARB,
						TEXSIZE * TEXSIZE,
						NULL, GL_STREAM_DRAW);
				pbo_unpack_mem = (GLubyte *) glMapBufferARB(
						GL_PIXEL_UNPACK_BUFFER_ARB,
						GL_WRITE_ONLY);
			}
			else {
				pbo_unpack_mem = bitmap;
			}

			for (i = 0; i < TEXSIZE * TEXSIZE / 8; i++) {
				pbo_unpack_mem[i] = 0xAA; /* Binary 10101010 */
			}


			glColor4f(1.0, 1.0, 1.0, 0.0);
			glRasterPos2f(0.0, 0.0);
			if (use_unpack) {
				glUnmapBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB);
				/* Draw white into every other pixel,
				 * for a white/black checkerboard. */
				glBitmap(TEXSIZE, TEXSIZE, 0, 0, 0, 0, NULL);
				glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB,
						0);
			}
			else {
				glBitmap(TEXSIZE, TEXSIZE, 0, 0, 0, 0,
					 pbo_unpack_mem);
			}

			if (!piglit_automatic)
				piglit_present_results();

			/* Check the result */
			if (use_pack) {
				glGenBuffersARB(1, pb_pack);
				glBindBufferARB(GL_PIXEL_PACK_BUFFER_ARB,
						pb_pack[0]);
				glBufferDataARB(GL_PIXEL_PACK_BUFFER_ARB,
						WINSIZE * WINSIZE *
						4 * sizeof(GLfloat), NULL,
						GL_STREAM_DRAW);
				glReadPixels(0, 0, WINSIZE, WINSIZE,
					     GL_RGB, GL_FLOAT,
					     NULL);
				pbo_pack_mem =
					(GLfloat *) glMapBufferARB(
						GL_PIXEL_PACK_BUFFER_ARB,
						GL_READ_ONLY);
			}
			else {
				pbo_pack_mem = buf;
				glReadPixels(0, 0, WINSIZE, WINSIZE,
					     GL_RGB, GL_FLOAT, pbo_pack_mem);
			}

			/* Compute expected and compare it to the result. */
			for (j = 0; j < WINSIZE; j++) {
				for (i = 0; i < WINSIZE; i++) {
					int idx = (j * WINSIZE + i) * 3;
					if ((i & 1) || (i >= TEXSIZE) ||
					   (j >= TEXSIZE)) {
						expected[idx + 0] = black[0];
						expected[idx + 1] = black[1];
						expected[idx + 2] = black[2];
					}
					else {
						expected[idx + 0] = white[0];
						expected[idx + 1] = white[1];
						expected[idx + 2] = white[2];
					}
				}
			}
			piglit_compute_probe_tolerance(GL_RGB, &tolerance[0]);
			pass &= piglit_compare_images_color(0, 0, WINSIZE,
							    WINSIZE, 3,
							    tolerance,
							    expected,
							    pbo_pack_mem);

			if (use_pack) {
				glUnmapBuffer(GL_PIXEL_PACK_BUFFER_ARB);
				glBindBuffer(GL_PIXEL_PACK_BUFFER_ARB, 0);
				glDeleteBuffersARB(1, pb_pack);
			}

			if (use_unpack) {
				glBindBuffer(GL_PIXEL_UNPACK_BUFFER_ARB, 0);
				glDeleteBuffersARB(1, pb_unpack);
			}
		}
	}
	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

#define TEXTURE_SIZE TEXSIZE * TEXSIZE * 3
#define WINDOW_SIZE WINSIZE * WINSIZE * 3

enum piglit_result
test_tex_image(void)
{
	bool pass = true;

	int break_pbo_cow, break_tex_cow; /* cow = copy on write */
	int use_unpack, use_pack;
	GLuint unpack_pb[1];
	GLuint pack_pb[1];
	GLenum pack = GL_PIXEL_PACK_BUFFER_ARB;
	GLenum unpack = GL_PIXEL_UNPACK_BUFFER_ARB;
	GLfloat t1[TEXTURE_SIZE];
	GLfloat t2[TEXTURE_SIZE];
	GLfloat *pbo_mem = NULL;
	int i, j;
	GLfloat green[3] = { 1.0, 1.0, 0.0 };
	GLfloat black[3] = { 0.0, 0.0, 0.0 };
	GLfloat buf[WINDOW_SIZE];
	GLfloat exp_tex[TEXTURE_SIZE];
	GLfloat exp_win[WINDOW_SIZE];
	GLfloat tolerance[4];

	piglit_compute_probe_tolerance(GL_RGB, tolerance);

	glBindBufferARB(unpack, 0);
	glBindBufferARB(pack, 0);

	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	for (use_pack = 0; use_pack < 2; use_pack++) {
		for (use_unpack = 0; use_unpack < 2;
			  use_unpack++) {
			for (break_pbo_cow = 0; break_pbo_cow < use_unpack + 1;
				  break_pbo_cow++) {
				for (break_tex_cow = 0;
					  break_tex_cow < use_unpack + 1;
					  break_tex_cow++) {
					if (use_unpack) {
						glGenBuffersARB(1, unpack_pb);
						glBindBufferARB(unpack,
						   unpack_pb[0]);
						glBufferDataARB(unpack,
							TEXTURE_SIZE *
							sizeof(GLfloat),
							NULL, GL_STREAM_DRAW);
					}

					glTexParameteri(GL_TEXTURE_2D,
							GL_TEXTURE_MIN_FILTER,
							GL_NEAREST);
					glTexParameteri(GL_TEXTURE_2D,
							GL_TEXTURE_MAG_FILTER,
							GL_NEAREST);

					if (use_unpack) {
						pbo_mem = (GLfloat *)
							glMapBufferARB(unpack,
							GL_WRITE_ONLY);
					}
					else {
						pbo_mem = t1;
					}

					for (i = 0; i < TEXTURE_SIZE/3; i++) {
						pbo_mem[3 * i] = 1.0;
						pbo_mem[3 * i + 1] = 1.0;
						pbo_mem[3 * i + 2] = 0.0;
					}

					if (use_unpack) {
						glUnmapBufferARB(unpack);
						glTexImage2D(GL_TEXTURE_2D, 0,
							     GL_RGB, TEXSIZE,
							     TEXSIZE, 0,
							     GL_RGB, GL_FLOAT,
							     NULL);
						glBindBufferARB(unpack, 0);
					}
					else
						glTexImage2D(GL_TEXTURE_2D, 0,
							     GL_RGB, TEXSIZE,
							     TEXSIZE, 0,
							     GL_RGB, GL_FLOAT,
							     pbo_mem);

					if (use_unpack && break_pbo_cow) {
						glBindBufferARB(unpack,
							        unpack_pb[0]);
						pbo_mem = (GLfloat *)
							 glMapBufferARB(
							    unpack,
							    GL_WRITE_ONLY);
						for (i = 0; i < TEXTURE_SIZE; i++)
							pbo_mem[i] = 0.2;
						glUnmapBufferARB(unpack);
						glBindBufferARB(unpack, 0);
					}

					if (use_unpack && break_tex_cow) {
						GLfloat temp[3];
						for (i = 0; i < 3; i++)
							temp[i] = 0.8;
						glTexSubImage2D(GL_TEXTURE_2D,
								0, 0, 0, 1, 1,
								GL_RGB,
								GL_FLOAT,
								temp);
					}

					/* Check PBO's content */
					if (use_unpack) {
						glBindBufferARB(unpack,
							        unpack_pb[0]);
						pbo_mem = (GLfloat *)
							 glMapBuffer(unpack,
							 GL_READ_ONLY);
						if (break_pbo_cow) {
							for (i = 0; i < TEXTURE_SIZE; i++)
								if (fabsf(pbo_mem[i] - 0.2f) > tolerance[0]) {
									REPORT_FAILURE
										("PBO modified by someone else, "
										 "there must be something wrong");
									return PIGLIT_FAIL;
								}
						}
						glUnmapBufferARB(unpack);
						glBindBufferARB(unpack, 0);
					}


					/* Read texture back */
					if (use_pack) {
						glGenBuffersARB(1, pack_pb);
						glBindBufferARB(pack, pack_pb[0]);
						glBufferDataARB(pack,
								TEXTURE_SIZE *
								sizeof(GLfloat),
								NULL, GL_STREAM_DRAW);
						glGetTexImage(GL_TEXTURE_2D,
							      0, GL_RGB,
							      GL_FLOAT, NULL);
						pbo_mem = (GLfloat *)
							 glMapBufferARB(pack,
							 GL_READ_ONLY);
					}
					else {
						glGetTexImage(GL_TEXTURE_2D,
							      0, GL_RGB,
							      GL_FLOAT, t2);
						pbo_mem = t2;
					}

					/* Check texture image */
					for (i = 0; i < TEXTURE_SIZE/3; i++) {
						int idx = i * 3;
						if (i == 0 && break_tex_cow
						   && use_unpack) {
							exp_tex[idx + 0] = 0.8;
							exp_tex[idx + 1] = 0.8;
							exp_tex[idx + 2] = 0.8;
						}
						else {
							exp_tex[idx + 0] = 1.0;
							exp_tex[idx + 1] = 1.0;
							exp_tex[idx + 2] = 0.0;
						}
					}
					pass &= piglit_compare_images_color(0,
							0, TEXSIZE,
							TEXSIZE, 3,
							tolerance, exp_tex,
							pbo_mem);

					if (use_pack) {
						glUnmapBufferARB(pack);
						glBindBufferARB(pack, 0);
						glDeleteBuffersARB(1, pack_pb);
					}
					if (use_unpack) {
						glDeleteBuffersARB(1, unpack_pb);
					}

					glEnable(GL_TEXTURE_2D);
					glBegin(GL_POLYGON);
					glTexCoord2f(0, 0);
					glVertex2f(0, 0);
					glTexCoord2f(1, 0);
					glVertex2f(TEXSIZE, 0);
					glTexCoord2f(1, 1);
					glVertex2f(TEXSIZE, TEXSIZE);
					glTexCoord2f(0, 1);
					glVertex2f(0, TEXSIZE);
					glEnd();
					glDisable(GL_TEXTURE_2D);

					glReadPixels(0, 0, WINSIZE, WINSIZE,
						     GL_RGB, GL_FLOAT,
						     buf);

					for (j = 0; j < WINSIZE; j++) {
						for (i = 0; i < WINSIZE; i++) {
							int idx = (j * WINSIZE + i) * 3;
							if (i == 0 && j == 0
							    && break_tex_cow
							    && use_unpack) {
								exp_win[idx + 0] = 0.8;
								exp_win[idx + 1] = 0.8;
								exp_win[idx + 2] = 0.8;
							}
							else if (i < TEXSIZE && j < TEXSIZE) {
								exp_win[idx + 0] = green[0];
								exp_win[idx + 1] = green[1];
								exp_win[idx + 2] = green[2];
							}
							else {
								exp_win[idx + 0] = black[0];
								exp_win[idx + 1] = black[1];
								exp_win[idx + 2] = black[2];
							}
						}
					}
					pass &= piglit_compare_images_color(0,
							0, WINSIZE,
							WINSIZE, 3,
							tolerance, exp_win,
							buf);
				}
			}
		}
	}

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

enum piglit_result
test_tex_sub_image(void)
{
	GLuint pbs[1];
	GLfloat t[TEXSIZE * TEXSIZE * 3];
	int i, j;
	int use_unpack = 0;
	GLfloat green[3] = { 0.0, 1.0, 0.0 };
	GLfloat black[3] = { 0.0, 0.0, 0.0 };
	GLfloat *pbo_mem = NULL;
	GLfloat buf[WINSIZE * WINSIZE * 3];
	bool pass = true;
	GLfloat expected[WINSIZE * WINSIZE * 3];
	GLfloat tolerance[4];
	piglit_compute_probe_tolerance(GL_RGB, &tolerance[0]);

	glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, 0);
	glBindBufferARB(GL_PIXEL_PACK_BUFFER_ARB, 0);

	for (use_unpack = 0; use_unpack < 2; use_unpack++) {
		pbo_mem = NULL;
		glClearColor(0.0, 0.0, 0.0, 1.0);
		glClear(GL_COLOR_BUFFER_BIT);

		if (use_unpack) {
			glGenBuffersARB(1, pbs);
			glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, pbs[0]);
			glBufferDataARB(GL_PIXEL_UNPACK_BUFFER_ARB,
					TEXSIZE * TEXSIZE * 3 *
					sizeof(GLfloat), NULL, GL_STREAM_DRAW);
			glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, 0);
		}

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, TEXSIZE, TEXSIZE, 0, GL_RGB,
						 GL_FLOAT, NULL);

		if (use_unpack) {
			glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, pbs[0]);
			pbo_mem = (GLfloat *) glMapBufferARB(
					GL_PIXEL_UNPACK_BUFFER_ARB,
					GL_WRITE_ONLY);
		}
		else {
			pbo_mem = t;
		}

		for (i = 0; i < TEXSIZE * TEXSIZE; i++) {
			pbo_mem[3 * i] = 0.0;
			pbo_mem[3 * i + 1] = 1.0;
			pbo_mem[3 * i + 2] = 0.0;
		}

		if (use_unpack) {
			glUnmapBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB);
			glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, TEXSIZE,
					TEXSIZE, GL_RGB, GL_FLOAT, NULL);
			glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, 0);
		}
		else
			glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, TEXSIZE,
					TEXSIZE, GL_RGB, GL_FLOAT, pbo_mem);

		glEnable(GL_TEXTURE_2D);
		glBegin(GL_POLYGON);
		glTexCoord2f(0, 0);
		glVertex2f(0, 0);
		glTexCoord2f(1, 0);
		glVertex2f(10, 0);
		glTexCoord2f(1, 1);
		glVertex2f(10, 10);
		glTexCoord2f(0, 1);
		glVertex2f(0, 10);
		glEnd();
		glDisable(GL_TEXTURE_2D);

		glReadPixels(0, 0, WINSIZE, WINSIZE, GL_RGB, GL_FLOAT, buf);

		for (j = 0; j < WINSIZE; j++) {
			for (i = 0; i < WINSIZE; i++) {
				int idx = (j * WINSIZE + i) * 3;
				if (i < 10 && j < 10) {
					expected[idx + 0] = green[0];
					expected[idx + 1] = green[1];
					expected[idx + 2] = green[2];
				}
				else {
					expected[idx + 0] = black[0];
					expected[idx + 1] = black[1];
					expected[idx + 2] = black[2];
				}
			}
		}
		pass &= piglit_compare_images_color(0, 0, WINSIZE,
						    WINSIZE, 3, tolerance,
						    expected, buf);
	}

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

enum piglit_result
test_polygon_stip(void)
{
	int use_unpack = 0;
	int use_pack = 0;
	GLuint unpack_pb[1];
	GLuint pack_pb[1];
	GLubyte t1[32 * 32 / 8];
	GLubyte t2[32 * 32 / 8];
	GLubyte *pbo_mem = NULL;
	int i, j;
	GLfloat white[3] = { 1.0, 1.0, 1.0 };
	GLfloat black[3] = { 0.0, 0.0, 0.0 };
	GLfloat buf[WINSIZE * WINSIZE * 3];
	bool pass = true;
	GLfloat expected[WINSIZE * WINSIZE * 3];
	GLfloat tolerance[4];

	piglit_compute_probe_tolerance(GL_RGB, &tolerance[0]);

	glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, 0);
	glBindBufferARB(GL_PIXEL_PACK_BUFFER_ARB, 0);

	for (use_unpack = 0; use_unpack < 2; use_unpack++) {
		for (use_pack = 0; use_pack < 2; use_pack++) {
			glClearColor(0.0, 0.0, 0.0, 1.0);
			glClear(GL_COLOR_BUFFER_BIT);

			if (use_unpack) {
				glGenBuffersARB(1, unpack_pb);
				glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB,
						unpack_pb[0]);
				glBufferDataARB(GL_PIXEL_UNPACK_BUFFER_ARB,
						32 * 32 / 8, NULL,
						GL_STREAM_DRAW);
				pbo_mem = (GLubyte *) glMapBufferARB(
						GL_PIXEL_UNPACK_BUFFER_ARB,
						GL_WRITE_ONLY);
			}
			else {
				pbo_mem = t1;
			}

			/* Fill in the stipple pattern */
			for (i = 0; i < 32 * 32 / 8; i++) {
				pbo_mem[i] = 0xAA; /* Checkerboard */
			}

			if (use_unpack) {
				glUnmapBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB);
				glPolygonStipple(NULL);
			}
			else {
				glPolygonStipple(pbo_mem);
			}

			/* Read back the stipple pattern */
			if (use_pack) {
				glGenBuffersARB(1, pack_pb);
				glBindBufferARB(GL_PIXEL_PACK_BUFFER_ARB,
					pack_pb[0]);
				glBufferDataARB(GL_PIXEL_PACK_BUFFER_ARB,
						32 * 32 / 8, NULL,
						GL_STREAM_DRAW);
				glGetPolygonStipple(NULL);
				pbo_mem = (GLubyte *) glMapBufferARB(
					GL_PIXEL_PACK_BUFFER_ARB,
					GL_READ_ONLY);
			}
			else {
				glGetPolygonStipple(t2);
				pbo_mem = t2;
			}

			for (i = 0; i < 32 * 32 / 8; i++) {
				if (pbo_mem[i] != 0xAA) {
					REPORT_FAILURE("glGetPolygonStipple failed");
					return PIGLIT_FAIL;
				}
			}


			if (use_unpack) {
				glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, 0);
				glDeleteBuffersARB(1, unpack_pb);
			}
			if (use_pack) {
				glBindBufferARB(GL_PIXEL_PACK_BUFFER_ARB, 0);
				glDeleteBuffersARB(1, pack_pb);
			}

			glEnable(GL_POLYGON_STIPPLE);
			glColor4f(1.0, 1.0, 1.0, 0.0);
			glBegin(GL_POLYGON);
			glVertex2f(0, 0);
			glVertex2f(10, 0);
			glVertex2f(10, 10);
			glVertex2f(0, 10);
			glEnd();

			glDisable(GL_POLYGON_STIPPLE);

			/* Check the result */
			glReadPixels(0, 0, WINSIZE, WINSIZE, GL_RGB, GL_FLOAT, buf);

			for (j = 0; j < WINSIZE; j++) {
				for (i = 0; i < WINSIZE; i++) {
					int idx = (j * WINSIZE + i) * 3;
					if (!(i & 1) && i < 10 && j < 10) {
						expected[idx + 0] = white[0];
						expected[idx + 1] = white[1];
						expected[idx + 2] = white[2];
					}
					else {
						expected[idx + 0] = black[0];
						expected[idx + 1] = black[1];
						expected[idx + 2] = black[2];
					}
				}
			}
			pass &= piglit_compare_images_color(0, 0, WINSIZE,
						            WINSIZE, 3,
							    tolerance,
							    expected, buf);

		}
	}

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

enum piglit_result
test_error_handling(void)
{
	GLuint fbs[1];

	glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, 0);
	glBindBufferARB(GL_PIXEL_PACK_BUFFER_ARB, 0);

	/* glDrawPixels raises an error when the buffer is too small */
	glGenBuffersARB(1, fbs);
	glBindBufferARB(GL_PIXEL_UNPACK_BUFFER, fbs[0]);
	glBufferDataARB(GL_PIXEL_UNPACK_BUFFER_ARB, 32 * 32 * 4, NULL,
						 GL_STREAM_DRAW);
	glDrawPixels(32, 32 + 1, GL_BGRA, GL_UNSIGNED_BYTE, NULL);
	if (glGetError() != GL_INVALID_OPERATION)
		return PIGLIT_FAIL;

	glDeleteBuffersARB(1, fbs);
	glBindBufferARB(GL_PIXEL_UNPACK_BUFFER, 0);

	/* test that glReadPixels into too small of buffer raises error */
	glGenBuffersARB(1, fbs);
	glBindBufferARB(GL_PIXEL_PACK_BUFFER, fbs[0]);
	glBufferDataARB(GL_PIXEL_PACK_BUFFER_ARB, 32 * 32 * 4, NULL,
						 GL_STREAM_DRAW);
	glReadPixels(0, 0, 32, 32 + 1, GL_BGRA, GL_UNSIGNED_BYTE, NULL);
	if (glGetError() != GL_INVALID_OPERATION)
		return PIGLIT_FAIL;

	glDeleteBuffersARB(1, fbs);
	glBindBufferARB(GL_PIXEL_PACK_BUFFER, 0);

	return PIGLIT_PASS;
}

struct test_func {
	enum piglit_result (*func) (void);
	char *name;
};

enum piglit_result
piglit_display(void)
{
	int i = 0;
	enum piglit_result result = PIGLIT_PASS;
	enum piglit_result subtest;
	static struct test_func funcs[] = {
		{test_sanity,		  "test_sanity"},
		{test_draw_pixels,	  "test_draw_pixels"},
		{test_pixel_map,	  "test_pixel_map"},
		{test_bitmap,		  "test_bitmap"},
		{test_tex_image,	  "test_tex_image"},
		{test_tex_sub_image,	  "test_tex_sub_image"},
		{test_polygon_stip,	  "test_polygon_stip"},
		{test_error_handling,     "test_error_handling"},
		{NULL, ""}	 /* End of list sentinal */
	};

	while (funcs[i].func)
	{
		subtest = funcs[i].func();
		piglit_report_subtest_result(subtest, "%s",
					     funcs[i].name);
		if (subtest == PIGLIT_FAIL)
			result = PIGLIT_FAIL;
		i++;
	}

	return result;
}
