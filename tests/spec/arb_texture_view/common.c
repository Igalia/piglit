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

#include "piglit-util-gl-common.h"
#include <stdarg.h>

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
