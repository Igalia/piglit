/*
 * Copyright © 2010 Intel Corporation
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
 *
 * Authors:
 *    Shuang He <shuang.he@intel.com>
 *    Ian Romanick <ian.d.romanick@intel.com>
 */

#ifndef OBJECT_PURGEABLE_H

extern void init_ObjectPurgeableAPI(void);

extern GLboolean test_ObjectpurgeableAPPLE(GLenum objectType, GLuint name,
					   GLenum option);

extern GLboolean test_ObjectunpurgeableAPPLE(GLenum objectType, GLuint name,
					     GLenum option);

extern GLboolean test_GetObjectParameterivAPPLE(GLenum objectType, GLuint name,
						GLenum expect);

extern GLboolean test_Purgeable(GLuint object, GLenum type);

#endif /* OBJECT_PURGEABLE_H */
