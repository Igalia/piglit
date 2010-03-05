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

#include "piglit-util.h"
#include "object_purgeable.h"

PFNGLOBJECTPURGEABLEAPPLEPROC pglObjectPurgeableAPPLE = NULL;
PFNGLOBJECTUNPURGEABLEAPPLEPROC pglObjectUnpurgeableAPPLE = NULL;
PFNGLGETOBJECTPARAMETERIVAPPLEPROC pglGetObjectParameterivAPPLE = NULL;

#define FAIL_ON_ERROR(string)						\
	do {								\
		const GLenum err = glGetError();			\
		if (err != GL_NO_ERROR) {				\
			fprintf(stderr, "%s generated error 0x%04x\n", 	\
				string, err);				\
			pass = GL_FALSE;				\
		}							\
	} while (0)

void
init_ObjectPurgeableAPI(void)
{
	piglit_require_extension("GL_APPLE_object_purgeable");

	pglObjectPurgeableAPPLE = (PFNGLOBJECTPURGEABLEAPPLEPROC)
		piglit_get_proc_address("glObjectPurgeableAPPLE");
	pglObjectUnpurgeableAPPLE = (PFNGLOBJECTUNPURGEABLEAPPLEPROC)
		piglit_get_proc_address("glObjectUnpurgeableAPPLE");
	pglGetObjectParameterivAPPLE = (PFNGLGETOBJECTPARAMETERIVAPPLEPROC)
		piglit_get_proc_address("glGetObjectParameterivAPPLE");
}


GLboolean
test_ObjectpurgeableAPPLE(GLenum objectType, GLuint name, GLenum option)
{
	GLboolean pass = GL_TRUE;
	GLenum ret;

	ret = (*pglObjectPurgeableAPPLE)(objectType, name, option);
	FAIL_ON_ERROR("pglObjectpurgeableAPPLE");

	switch (option) {
	case GL_VOLATILE_APPLE:
		if (ret != GL_VOLATILE_APPLE) {
			fprintf(stderr,
				"GL_VOLATILE_APPLE should be returned when "
				"call purgeable with GL_VOLATILE_APPLE\n");
			pass = GL_FALSE;
		}
		break;

	case GL_RELEASED_APPLE:
		if (ret != GL_VOLATILE_APPLE && ret != GL_RELEASED_APPLE) {
			fprintf(stderr,
				"GL_VOLATILE_APPLE or GL_RELEASED_APPLE should "
				"be returned when call purgeable with "
				"GL_RELEASED_APPLE\n");
			pass = GL_FALSE;
		}
		break;
	}

	return pass;
}


GLboolean
test_ObjectunpurgeableAPPLE(GLenum objectType, GLuint name, GLenum option)
{
	GLboolean pass = GL_TRUE;
	GLenum ret;

	ret = (*pglObjectUnpurgeableAPPLE)(objectType, name, option);
	FAIL_ON_ERROR("pglObjectunpurgeableAPPLE");

	switch (option) {
	case GL_RETAINED_APPLE:
		if (ret != GL_RETAINED_APPLE && ret != GL_UNDEFINED_APPLE) {
			fprintf(stderr,
				"GL_RETAINED_APPLE or GL_UNDEFINED_APPLE "
				"should be returned when call purgeable with "
				"GL_RETAINED_APPLE\n");
			pass = GL_FALSE;
		}
		break;

	case GL_UNDEFINED_APPLE:
		if (ret != GL_UNDEFINED_APPLE) {
			fprintf(stderr, "GL_UNDEFINED_APPLE should be returned "
				"when call purgeable with "
				"GL_UNDEFINED_APPLE\n");
			pass = GL_FALSE;
		}
		break;
	}

	return pass;
}


GLboolean
test_GetObjectParameterivAPPLE(GLenum objectType, GLuint name, GLenum expect)
{
	GLboolean pass = GL_TRUE;
	GLint param;

	(*pglGetObjectParameterivAPPLE)(objectType, name, GL_PURGEABLE_APPLE,
					&param);
	FAIL_ON_ERROR("glGetObjectParameterivAPPLE");

	if (param != expect) {
		fprintf(stderr, "GL_PURGEABLE_APPLE state is 0x%04x, should be "
			"0x%04x\n",
			param, expect);
		pass = GL_FALSE;
	}

	return pass;
}


GLboolean test_Purgeable(GLuint object, GLenum type)
{
    GLboolean pass = GL_TRUE;

    glGetError();

    if (test_GetObjectParameterivAPPLE(type, object, GL_FALSE) != GL_TRUE) {
        fprintf(stderr, "Default GL_PURGEABLE_APPLE state should GL_FALSE for texture object\n");
        pass = GL_FALSE;
    }

    if (test_ObjectpurgeableAPPLE(type, object, GL_VOLATILE_APPLE) != GL_TRUE) {
        fprintf(stderr, "Error when mark object %x to purgeable(GL_VOLATILE_APPLE)\n", object);
        pass = GL_FALSE;
    }

    if (test_GetObjectParameterivAPPLE(type, object, GL_TRUE) != GL_TRUE) {
        fprintf(stderr, "Object %x is not set to purgeable\n", object);
        pass = GL_FALSE;
    }

    if (test_ObjectunpurgeableAPPLE(type, object, GL_RETAINED_APPLE) != GL_TRUE) {
        fprintf(stderr, "Error when mark object %x to unpurgeable(GL_RETAINED_APPLE)\n", object);
        pass = GL_FALSE;
    }

    if (test_GetObjectParameterivAPPLE(type, object, GL_FALSE) != GL_TRUE) {
        fprintf(stderr, "Object %x is not set to unpurgeable\n", object);
        pass = GL_FALSE;
    }

    if (test_ObjectpurgeableAPPLE(type, object, GL_RELEASED_APPLE) != GL_TRUE) {
        fprintf(stderr, "Error when mark object %x to purgeable(GL_RELEASED_APPLE)\n", object);
        pass = GL_FALSE;
    }

    if (test_GetObjectParameterivAPPLE(type, object, GL_TRUE) != GL_TRUE) {
        fprintf(stderr, "Object %x is not set to purgeable\n", object);
        pass = GL_FALSE;
    }

    if (test_ObjectunpurgeableAPPLE(type, object, GL_UNDEFINED_APPLE) != GL_TRUE) {
        fprintf(stderr, "Error when mark object %x to unpurgeable(GL_UNDEFINED_APPLE)\n", object);
        pass = GL_FALSE;
    }

    if (test_GetObjectParameterivAPPLE(type, object, GL_FALSE) != GL_TRUE) {
        fprintf(stderr, "Object %x is not set to unpurgeable\n", object);
        pass = GL_FALSE;
    }

    return pass;
}
