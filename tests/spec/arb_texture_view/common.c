/*
 * Copyright © 2013 LunarG, Inc.
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
 *
 * Author: Jon Ashburn <jon@lunarg.com>
 */

#include "piglit-util-gl.h"
#include <stdarg.h>

const GLubyte Colors[][8] = {
		{127,	0,   0, 255,  0, 10, 20,  0},
		{  0, 127,   0, 255,  0,  0, 80, 90},
		{  0,	0, 127, 255, 25,  0,  0, 60},
		{  0, 127, 127, 255, 15, 15,  0,  0},
		{127,	0, 127, 255,  0,  2, 50,  0},
		{127, 127,   0, 255, 80, 10, 70, 20},
		{255,	0,   0, 255, 60,  0, 40, 30},
		{  0, 255,   0, 255, 50, 20,  2, 40},
		{  0,	0, 255, 255, 40,  0,  1,  0},
		{  0, 255, 255, 255, 30,  5,  3,  8},
		{255,	0, 255, 255, 20, 18,  4,  7},
		{255, 255,   0, 255,  10, 24, 77, 67},
		{255, 255, 255, 255,  5,  33, 88, 44}
};

/**
 * Create a single-color image. Up to 64 bits per pixel depending upon bytes
 */
GLubyte *
create_solid_image(GLint w, GLint h, GLint d, const unsigned int bytes,
		   const unsigned int idx)
{
	GLubyte *buf = (GLubyte *) malloc(w * h * d * bytes);
	int i,j;

	if (buf == NULL || idx > (sizeof(Colors) / bytes - 1)) {
		free(buf);
		return NULL;
	}
	for (i = 0; i < w * h * d; i++) {
		for (j = 0; j < bytes; j++) {
			buf[i*bytes+j] = Colors[idx][j];
		}
	}
	return buf;
}

/**
 * This function takes an array of valid and invalid GLenums.  The invalid
 * enums array starts fully populated and the valid  array is empty.
 * It adds the varargs GLenum values to the valid array and removes  them
 * from the invalid array.
 * @param numInvalid  the size of array invalid
 * An variable argument equal to zero will signal the end of
 * the variable parameters.
 */
unsigned int
update_valid_arrays(GLenum *valid, GLenum *invalid, unsigned int numInvalid,
		    ... )
{
	va_list args;
	GLenum val;
	unsigned int j, num;

	va_start(args, numInvalid);
	val = va_arg(args, GLenum);
	num = 0;
	while (val) {
		valid[num++] = val;
		/* remove the valid enum from the invalid array */
		for (j= 0; j < numInvalid; j++) {
			if (invalid[j] == val)
				invalid[j] = 0;
		}
		val = va_arg(args, GLenum);
	}
	va_end(args);
	return num;
}

/**
 *  Draw a textured quad, sampling only the given depth  of the 3D texture.
 *  Use shader pipeline.
 */
void
draw_3d_depth(float x, float y, float w, float h, int depth)
{
	const GLfloat vertices[12] =  {x, y, 0.0,
				 x+w, y, 0.0,
				 x+w, y+h, 0.0,
				 x, y+h, 0.0};
	const GLfloat texcoords[12] = {0.0, 0.0, depth,
				 1.0, 0.0, depth,
				 1.0, 1.0, depth,
				 0.0, 1.0, depth};

	glVertexPointer(3, GL_FLOAT, 0, vertices);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(3, GL_FLOAT, 0, texcoords);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}
