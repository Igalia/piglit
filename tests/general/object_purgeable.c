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

#include "piglit-util-gl.h"
#include "object_purgeable.h"

#define FAIL_ON_ERROR(string)						\
	do {								\
		const GLenum err = glGetError();			\
		if (err != GL_NO_ERROR) {				\
			fprintf(stderr, "%s generated error 0x%04x\n", 	\
				string, err);				\
			pass = GL_FALSE;				\
		}							\
	} while (0)

#define EXPECT_AN_ERROR(string, expected)				\
	do {								\
		const GLenum err = glGetError();			\
		if (err != expected) {					\
			fprintf(stderr, "%s generated error 0x%04x, "	\
				"but error 0x%04x (%s) was expected\n",	\
				string, err, expected, # expected);	\
			pass = GL_FALSE;				\
		}							\
	} while (0)

void
init_ObjectPurgeableAPI(void)
{
	piglit_require_extension("GL_APPLE_object_purgeable");
}


/**
 * Format for error messages when an unexpected value is received.
 */
static const char expected_fmt[] =
	"%s:%s: expected 0x%04x (%s), got 0x%04x\n";


/**
 * Check the setting and querying purgeability on object 0 generates errors.
 */
GLboolean
test_DefaultObject(GLenum objectType)
{
	GLboolean pass = GL_TRUE;
	GLint param;

	/* From the GL_APPLE_object_purgeable spec:
	 *
	 *     "INVALID_VALUE is generated if the <name> parameter of
	 *      ObjectUnpurgeableAPPLE or ObjectUnpurgeableAPPLE is zero."
	 */
	glObjectPurgeableAPPLE(objectType, 0, GL_VOLATILE_APPLE);
	EXPECT_AN_ERROR("glObjectPurgeableAPPLE", GL_INVALID_VALUE);

	glObjectUnpurgeableAPPLE(objectType, 0, GL_RETAINED_APPLE);
	EXPECT_AN_ERROR("glObjectUnpurgeableAPPLE", GL_INVALID_VALUE);

	/* From the GL_APPLE_object_purgeable spec:
	 *
	 *     "INVALID_VALUE is generated if the <name> parameter of
	 *      GetObjectParameterivAPPLE is zero."
	 */
	glGetObjectParameterivAPPLE(objectType, 0, GL_PURGEABLE_APPLE,
					&param);
	EXPECT_AN_ERROR("glGetObjectParameterivAPPLE", GL_INVALID_VALUE);

	return pass;
}


GLboolean
test_ObjectpurgeableAPPLE(GLenum objectType, GLuint name, GLenum option)
{
	GLboolean pass = GL_TRUE;
	GLenum ret;

	ret = glObjectPurgeableAPPLE(objectType, name, option);
	FAIL_ON_ERROR("glObjectPurgeableAPPLE");

	switch (option) {
	case GL_VOLATILE_APPLE:
		/* From the GL_APPLE_object_purgeable spec:
		 *
		 *     "If ObjectPurgeableAPPLE is called with an <option> of
		 *     VOLATILE_APPLE, then ObjectPurgeableAPPLE will also
		 *     return the value VOLATILE_APPLE."
		 */
		if (ret != GL_VOLATILE_APPLE) {
			fprintf(stderr, expected_fmt,
				"glObjectPurgeableAPPLE", "GL_VOLATILE_APPLE",
				GL_VOLATILE_APPLE, "GL_VOLATILE_APPLE",
				ret);
			pass = GL_FALSE;
		}
		break;

	case GL_RELEASED_APPLE:
		/* From the GL_APPLE_object_purgeable spec:
		 *
		 *     "If ObjectPurgeableAPPLE is called with an <option> of
		 *     RELEASED_APPLE, then ObjectPurgeableAPPLE may return
		 *     either the value RELEASED_APPLE or the value
		 *     VOLATILE_APPLE."
		 */
		if (ret != GL_VOLATILE_APPLE && ret != GL_RELEASED_APPLE) {
			fprintf(stderr, expected_fmt,
				"glObjectPurgeableAPPLE", "GL_RELEASED_APPLE",
				GL_VOLATILE_APPLE,
				"GL_VOLATILE_APPLE or GL_RELEASED_APPLE",
				ret);
			pass = GL_FALSE;
		}
		break;
	}

	/* From the GL_APPLE_object_purgeable spec:
	 *
	 *     "Calling ObjectPurgeableAPPLE with either option sets
	 *     PURGEABLE_APPLE to TRUE..."
	 */
	if (!test_GetObjectParameterivAPPLE(objectType, name, GL_TRUE)) {
		fprintf(stderr,
			"Object marked purgeable is not set to purgeable\n");
		pass = GL_FALSE;
	}

	/* From the GL_APPLE_object_purgeable spec:
	 *
	 *     "If ObjectPurgeableAPPLE is called and PURGEABLE_APPLE is
	 *     already TRUE, the error INVALID_OPERATION is generated."
	 */
	glObjectPurgeableAPPLE(objectType, name, option);
	EXPECT_AN_ERROR("glObjectPurgeableAPPLE", GL_INVALID_OPERATION);

	return pass;
}


GLboolean
test_ObjectunpurgeableAPPLE(GLenum objectType, GLuint name, GLenum option)
{
	GLboolean pass = GL_TRUE;
	GLenum ret;

	ret = glObjectUnpurgeableAPPLE(objectType, name, option);
	FAIL_ON_ERROR("glObjectUnpurgeableAPPLE");

	switch (option) {
	case GL_RETAINED_APPLE:
		/* From the GL_APPLE_object_purgeable spec:
		 *
		 *     "If ObjectUnpurgeableAPPLE is called with an <option> of
		 *     RETAINED_APPLE, then ObjectPurgeableAPPLE may return
		 *     either the value RETAINED_APPLE or the value
		 *     UNDEFINED_APPLE."
		 */
		if (ret != GL_RETAINED_APPLE && ret != GL_UNDEFINED_APPLE) {
			fprintf(stderr, expected_fmt,
				"glObjectUnpurgeableAPPLE", "GL_RETAINED_APPLE",
				GL_RETAINED_APPLE,
				"GL_RETAINED_APPLE or GL_UNDEFINED_APPLE",
				ret);
			pass = GL_FALSE;
		}
		break;

	case GL_UNDEFINED_APPLE:
		/* From the GL_APPLE_object_purgeable spec:
		 *
		 *     "If ObjectUnpurgeableAPPLE is called with the <option>
		 *     set to UNDEFINED_APPLE, then ObjectUnpurgeableAPPLE will
		 *     return the value UNDEFINED_APPLE."
		 */
		if (ret != GL_UNDEFINED_APPLE) {
			fprintf(stderr, expected_fmt,
				"glObjectUnpurgeableAPPLE", "GL_UNDEFINED_APPLE",
				GL_UNDEFINED_APPLE, "GL_UNDEFINED_APPLE",
				ret);
			pass = GL_FALSE;
		}
		break;
	}

	/* From the GL_APPLE_object_purgeable spec:
	 *
	 *     "Calling ObjectUnpurgeableAPPLE with either option sets
	 *     PURGEABLE_APPLE to FALSE..."
	 */
	if (!test_GetObjectParameterivAPPLE(objectType, name, GL_FALSE)) {
		fprintf(stderr, "Object marked unpurgeable is not set to "
			"unpurgeable\n");
		pass = GL_FALSE;
	}

	/* From the GL_APPLE_object_purgeable spec:
	 *
	 *     "If ObjectUnpurgeableAPPLE is called and PURGEABLE_APPLE is
	 *     already FALSE, the error INVALID_OPERATION is returned."
	 */
	glObjectUnpurgeableAPPLE(objectType, name, option);
	EXPECT_AN_ERROR("glObjectPurgeableAPPLE", GL_INVALID_OPERATION);

	return pass;
}


GLboolean
test_GetObjectParameterivAPPLE(GLenum objectType, GLuint name, GLenum expect)
{
	GLboolean pass = GL_TRUE;
	GLint param;

	glGetObjectParameterivAPPLE(objectType, name, GL_PURGEABLE_APPLE,
					&param);
	FAIL_ON_ERROR("glGetObjectParameterivAPPLE");

	if (param != expect) {
		fprintf(stderr, expected_fmt,
			"glGetObjectParameterivAPPLE", "GL_PURGEABLE_APPLE",
			expect, expect ? "GL_TRUE" : "GL_FALSE",
			param);
		pass = GL_FALSE;
	}

	return pass;
}


GLboolean test_Purgeable(GLuint object, GLenum type)
{
	GLboolean pass = GL_TRUE;

	glGetError();

	if (!test_DefaultObject(type)) {
		fprintf(stderr, "Default object tests failed.\n");
		pass = GL_FALSE;
	}

	if (!test_GetObjectParameterivAPPLE(type, object, GL_FALSE)) {
		fprintf(stderr, "Default state test failed.\n");
		pass = GL_FALSE;
	}

	if (!test_ObjectpurgeableAPPLE(type, object, GL_VOLATILE_APPLE)) {
		pass = GL_FALSE;
	}

	if (!test_ObjectunpurgeableAPPLE(type, object, GL_RETAINED_APPLE)) {
		pass = GL_FALSE;
	}

	if (!test_ObjectpurgeableAPPLE(type, object, GL_RELEASED_APPLE)) {
		pass = GL_FALSE;
	}

	if (!test_ObjectunpurgeableAPPLE(type, object, GL_UNDEFINED_APPLE)) {
		pass = GL_FALSE;
	}

	return pass;
}
