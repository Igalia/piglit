/*
 * Copyright © 2018 Timothy Arceri
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
 * Verify that commands added in ARB_viewport_array are compiled into display
 * lists.
 */

#include "piglit-util-gl.h"
#include <stdarg.h>

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 32;
	config.window_visual = PIGLIT_GL_VISUAL_RGBA | PIGLIT_GL_VISUAL_DOUBLE;
	config.khr_no_error_support = PIGLIT_NO_ERRORS;

PIGLIT_GL_TEST_CONFIG_END

#define MIN_VP 16

enum mode {
	set_scalar,
	set_vector,
	set_array_of_vectors,
	get_and_compare
};

enum function_type {
	viewport,
	scissor,
	depth
};

#define GEN_BUFFERS(type, n, c, div)				\
		type outbuf[n * c];				\
		unsigned jjj;					\
								\
		for (jjj = 0; jjj < (n * c); jjj++) {		\
			outbuf[jjj] = (type) value++;		\
			outbuf[jjj] /= div;			\
		}

#define SET_SCALAR_INDICES2(type, func, n, c)			\
	do {							\
		/* depth values are clamped between 0 and 1.0	\
		 * so we divide by 100.				\
		 */						\
		GEN_BUFFERS(type, n, c, 100.0);			\
		gl ## func					\
			(i,					\
			 outbuf[0],				\
			 outbuf[1]);				\
	} while (0)

#define SET_SCALAR_INDICES4(type, func, n, c)			\
	do {							\
		GEN_BUFFERS(type, n, c, 1);			\
		gl ## func					\
			(i,					\
			 outbuf[0],				\
			 outbuf[1],				\
			 outbuf[2],				\
			 outbuf[3]);				\
	} while (0)

#define SET_VECTOR_INDICES(type, func, n, c)			\
	do {							\
		GEN_BUFFERS(type, n, c, 1);			\
		gl ## func					\
			(i, outbuf);				\
	} while (0)

#define SET_ARRAY_INDICES(type, func, n, c, div)		\
	do {							\
		GEN_BUFFERS(type, n, c, div);			\
		gl ## func					\
			(0, c, outbuf);				\
	} while (0)

#define GET_AND_COMPARE_INDICES(type, func, getmode, n, c, div)	\
	do {							\
		type inbuf[n * c];				\
		GEN_BUFFERS(type, n, c, div);			\
								\
		/* Mesa converts the depth double to a float */	\
		if (sizeof(type) == sizeof(GLclampd)) {		\
			for (unsigned j = 0; j < (n * c); j++) {\
				float tmp = outbuf[j];		\
				outbuf[j] = tmp;		\
			}					\
		}						\
								\
		glGet ## func					\
			(					\
			 getmode				\
			 , i, inbuf);				\
								\
		if (memcmp(inbuf, outbuf,			\
			   sizeof(type) * n * c) != 0) {	\
			printf("        index %d data "		\
			       "does not match.\n",		\
			       i);				\
			pass = false;				\
		}						\
	} while (0)

bool
process_indices(unsigned base_value, enum mode m, enum function_type f_type)
{
	bool pass = true;

	unsigned value = base_value;
	unsigned count = m == set_array_of_vectors ? 1 : MIN_VP;

	for (unsigned i = 0; i < count; i++) {

		switch (m) {
		case set_scalar:
			switch (f_type) {
			case viewport:
				SET_SCALAR_INDICES4(GLfloat, ViewportIndexedf, 4, 1);
				break;
			case scissor:
				SET_SCALAR_INDICES4(GLint, ScissorIndexed, 4, 1);
				break;
			case depth:
				SET_SCALAR_INDICES2(GLclampd, DepthRangeIndexed, 2, 1);
				break;
			}

			break;
		case set_vector:
			switch (f_type) {
			case viewport:
				SET_VECTOR_INDICES(GLfloat, ViewportIndexedfv, 4, 1);
				break;
			case scissor:
				SET_VECTOR_INDICES(GLint, ScissorIndexedv, 4, 1);
				break;
			case depth:
				printf("Error: Depth has no vector setter");
				break;
			}

			break;
		case set_array_of_vectors:
			switch (f_type) {
			case viewport:
				SET_ARRAY_INDICES(GLfloat, ViewportArrayv, 4, MIN_VP, 1);
				break;
			case scissor:
				SET_ARRAY_INDICES(GLint, ScissorArrayv, 4, MIN_VP, 1);
				break;
			case depth:
				SET_ARRAY_INDICES(GLclampd, DepthRangeArrayv, 2, MIN_VP, 100.0);
				break;
			}

			break;
		case get_and_compare:
			switch (f_type) {
			case viewport:
				GET_AND_COMPARE_INDICES(GLfloat, Floati_v, GL_VIEWPORT, 4, 1, 1);
				break;
			case scissor:
				GET_AND_COMPARE_INDICES(GLint, Integeri_v, GL_SCISSOR_BOX, 4, 1, 1);
				break;
			case depth:
				GET_AND_COMPARE_INDICES(GLclampd, Doublei_v, GL_DEPTH_RANGE, 2, 1, 100.0);
				break;
			}
			break;
		}
	}

	return pass;
}

void
piglit_init(int argc, char **argv)
{
	bool pass = true;
	GLint maxVP;

	piglit_require_extension("GL_ARB_viewport_array");

	glGetIntegerv(GL_MAX_VIEWPORTS, &maxVP);
	if (!piglit_check_gl_error(GL_NO_ERROR) || maxVP < MIN_VP)
		piglit_report_result(PIGLIT_FAIL);

	static const struct {
		GLenum list_mode;
		enum mode setter_mode;
		const char *setter_mode_name;
		enum function_type f_type;
		unsigned base_value;
	} tests[] = {
		{
			GL_COMPILE,
			set_scalar, "viewport scalar", viewport,
			5
		},
		{
			GL_COMPILE,
			set_vector, "viewport vector", viewport,
			7
		},
		{
			GL_COMPILE,
			set_array_of_vectors, "viewport array of vectors",
			viewport,
			7
		},
		{
			GL_COMPILE_AND_EXECUTE,
			set_scalar, "viewport scalar", viewport,
			11
		},
		{
			GL_COMPILE_AND_EXECUTE,
			set_vector, "viewport vector", viewport,
			13
		},
		{
			GL_COMPILE_AND_EXECUTE,
			set_array_of_vectors, "viewport array of vectors",
			viewport,
			15
		},
		{
			GL_COMPILE,
			set_scalar, "scissor scalar", scissor,
			5
		},
		{
			GL_COMPILE,
			set_vector, "scissor vector", scissor,
			7
		},
		{
			GL_COMPILE,
			set_array_of_vectors, "scissor array of vectors",
			scissor,
			7
		},
		{
			GL_COMPILE_AND_EXECUTE,
			set_scalar, "scissor scalar", scissor,
			11
		},
		{
			GL_COMPILE_AND_EXECUTE,
			set_vector, "scissor vector", scissor,
			13
		},
		{
			GL_COMPILE_AND_EXECUTE,
			set_array_of_vectors, "scissor array of vectors",
			scissor,
			15
		},
		{
			GL_COMPILE,
			set_scalar, "depth scalar", depth,
			5
		},
		{
			GL_COMPILE,
			set_array_of_vectors, "depth array", depth,
			7
		},
		{
			GL_COMPILE_AND_EXECUTE,
			set_scalar, "depth scalar", depth,
			11
		},
		{
			GL_COMPILE_AND_EXECUTE,
			set_array_of_vectors, "depth array", depth,
			13
		}
	};

	GLuint list = glGenLists(1);

	for (unsigned i = 0; i < ARRAY_SIZE(tests); i++) {
		const unsigned post_compile_base_value =
			(tests[i].list_mode == GL_COMPILE)
			? 0 : tests[i].base_value;

		printf("    %s: %s mode\n",
		       piglit_get_gl_enum_name(tests[i].list_mode),
		       tests[i].setter_mode_name);

		printf("        pre-initialize\n");
		pass = process_indices(0, tests[i].setter_mode, tests[i].f_type) && pass;
		pass = process_indices(0, get_and_compare, tests[i].f_type) && pass;

		glNewList(list, tests[i].list_mode);
		printf("        compiling\n");
		pass = process_indices(tests[i].base_value,
				       tests[i].setter_mode, tests[i].f_type) && pass;
		glEndList();

		printf("        post-compile verify\n");
		pass = process_indices(post_compile_base_value,
				       get_and_compare, tests[i].f_type) && pass;

		/* Reset the values back.  This is useful if GL_COMPILE
		 * executed the commands and for GL_COMPILE_AND_EXECUTE.  We
		 * want to know that glCallList changed things.
		 */
		printf("        restore original values\n");
		pass = process_indices(0, tests[i].setter_mode, tests[i].f_type) && pass;
		pass = process_indices(0, get_and_compare, tests[i].f_type) && pass;

		printf("        post-glCallList verify\n");
		glCallList(list);
		pass = process_indices(tests[i].base_value,
				       get_and_compare, tests[i].f_type) && pass;
	}

	glDeleteLists(list, 1);

	pass = piglit_check_gl_error(GL_NO_ERROR) && pass;

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}

enum piglit_result
piglit_display(void)
{
	return PIGLIT_FAIL;
}
