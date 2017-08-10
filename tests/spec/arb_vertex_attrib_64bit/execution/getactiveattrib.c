/*
 * Copyright © 2016 Intel Corporation
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
 * \file getactiveattrib.c
 * Verify that glGetActiveAttrib and GL_ACTIVE_ATTRIBUTES return the expected
 * values for the new tokens defined at ARB_vertex_attrib_64bit specification.
 *
 * This is based on tests/general/getactiveattrib.c
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN
	config.supports_gl_compat_version = 32;
	config.supports_gl_core_version = 32;

	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;
	config.khr_no_error_support = PIGLIT_NO_ERRORS;
PIGLIT_GL_TEST_CONFIG_END

struct attribute {
	/** Name of the attribute. */
	const char *name;

	/** Expected GLSL type of the attribute. */
	GLenum type;
};

struct test {
	const char *code;

	/**
	 * List of attributes to be checked
	 *
	 * \note
	 * The list is terminated by an attribute with a \c NULL \c name
	 * pointer.
	 */
	struct attribute attributes[16];
};

static const struct test vertex_attrib_64bit_tests[] = {
	/* Try all the possible types for vertex shader inputs added
	 * at the spec. They could be added as 13 attributes on the
	 * same vertex shader, but we would need to get them all
	 * active. It is easier to read this way.
	 */
	{
		"#version 150\n"
		"#extension GL_ARB_vertex_attrib_64bit: require\n"
		"#extension GL_ARB_gpu_shader_fp64: require\n"
		"in double _double;\n"
		"void main() { gl_Position = vec4(float(_double)); }\n",
		{
			{ "_double",	GL_DOUBLE },
			{ NULL, }
		}
	},
	{
		"#version 150\n"
		"#extension GL_ARB_vertex_attrib_64bit: require\n"
		"#extension GL_ARB_gpu_shader_fp64: require\n"
		"in dvec2 _dvec2;\n"
		"void main() { gl_Position = vec4(float(_dvec2.x)); }\n",
		{
			{ "_dvec2",	GL_DOUBLE_VEC2 },
			{ NULL, }
		}
	},
	{
		"#version 150\n"
		"#extension GL_ARB_vertex_attrib_64bit: require\n"
		"#extension GL_ARB_gpu_shader_fp64: require\n"
		"in dvec3 _dvec3;\n"
		"void main() { gl_Position = vec4(float(_dvec3.x)); }\n",
		{
			{ "_dvec3",	GL_DOUBLE_VEC3 },
			{ NULL, }
		}
	},
	{
		"#version 150\n"
		"#extension GL_ARB_vertex_attrib_64bit: require\n"
		"#extension GL_ARB_gpu_shader_fp64: require\n"
		"in dvec4 _dvec4;\n"
		"void main() { gl_Position = vec4(float(_dvec4.x)); }\n",
		{
			{ "_dvec4",	GL_DOUBLE_VEC4 },
			{ NULL, }
		}
	},
	{
		"#version 150\n"
		"#extension GL_ARB_vertex_attrib_64bit: require\n"
		"#extension GL_ARB_gpu_shader_fp64: require\n"
		"in dmat2 _dmat2;\n"
		"void main() { gl_Position = vec4(float(_dmat2[0][0])); }\n",
		{
			{ "_dmat2",	GL_DOUBLE_MAT2 },
			{ NULL, }
		}
	},
	{
		"#version 150\n"
		"#extension GL_ARB_vertex_attrib_64bit: require\n"
		"#extension GL_ARB_gpu_shader_fp64: require\n"
		"in dmat3 _dmat3;\n"
		"void main() { gl_Position = vec4(float(_dmat3[0][0])); }\n",
		{
			{ "_dmat3",	GL_DOUBLE_MAT3 },
			{ NULL, }
		}
	},
	{
		"#version 150\n"
		"#extension GL_ARB_vertex_attrib_64bit: require\n"
		"#extension GL_ARB_gpu_shader_fp64: require\n"
		"in dmat4 _dmat4;\n"
		"void main() { gl_Position = vec4(float(_dmat4[0][0])); }\n",
		{
			{ "_dmat4",	GL_DOUBLE_MAT4 },
			{ NULL, }
		}
	},
	{
		"#version 150\n"
		"#extension GL_ARB_vertex_attrib_64bit: require\n"
		"#extension GL_ARB_gpu_shader_fp64: require\n"
		"in dmat2x3 _dmat2x3;\n"
		"void main() { gl_Position = vec4(float(_dmat2x3[0][0])); }\n",
		{
			{ "_dmat2x3",	GL_DOUBLE_MAT2x3 },
			{ NULL, }
		}
	},
	{
		"#version 150\n"
		"#extension GL_ARB_vertex_attrib_64bit: require\n"
		"#extension GL_ARB_gpu_shader_fp64: require\n"
		"in dmat2x4 _dmat2x4;\n"
		"void main() { gl_Position = vec4(float(_dmat2x4[0][0])); }\n",
		{
			{ "_dmat2x4",	GL_DOUBLE_MAT2x4 },
			{ NULL, }
		}
	},
	{
		"#version 150\n"
		"#extension GL_ARB_vertex_attrib_64bit: require\n"
		"#extension GL_ARB_gpu_shader_fp64: require\n"
		"in dmat3x2 _dmat3x2;\n"
		"void main() { gl_Position = vec4(float(_dmat3x2[0][0])); }\n",
		{
			{ "_dmat3x2",	GL_DOUBLE_MAT3x2 },
			{ NULL, }
		}
	},
	{
		"#version 150\n"
		"#extension GL_ARB_vertex_attrib_64bit: require\n"
		"#extension GL_ARB_gpu_shader_fp64: require\n"
		"in dmat3x4 _dmat3x4;\n"
		"void main() { gl_Position = vec4(float(_dmat3x4[0][0])); }\n",
		{
			{ "_dmat3x4",	GL_DOUBLE_MAT3x4 },
			{ NULL, }
		}
	},
	{
		"#version 150\n"
		"#extension GL_ARB_vertex_attrib_64bit: require\n"
		"#extension GL_ARB_gpu_shader_fp64: require\n"
		"in dmat4x2 _dmat4x2;\n"
		"void main() { gl_Position = vec4(float(_dmat4x2[0][0])); }\n",
		{
			{ "_dmat4x2",	GL_DOUBLE_MAT4x2 },
			{ NULL, }
		}
	},
	{
		"#version 150\n"
		"#extension GL_ARB_vertex_attrib_64bit: require\n"
		"#extension GL_ARB_gpu_shader_fp64: require\n"
		"in dmat4x3 _dmat4x3;\n"
		"void main() { gl_Position = vec4(float(_dmat4x3[0][0])); }\n",
		{
			{ "_dmat4x3",	GL_DOUBLE_MAT4x3 },
			{ NULL, }
		}
	},
};

enum piglit_result
piglit_display(void)
{
	return PIGLIT_FAIL;
}

int
find_attrib(const struct attribute *attribs, const char *name)
{
	unsigned i;

	for (i = 0; attribs[i].name != NULL; i++) {
		if (strcmp(attribs[i].name, name) == 0)
			return (int) i;
	}

	return -1;
}

#define DUMP_SHADER(code)						\
	do {								\
		if (!shader_dumped) {					\
			fprintf(stderr, "\nFailing shader:\n%s\n\n",	\
				code);					\
			shader_dumped = true;				\
		}							\
	} while (false)

bool
do_test(const struct test *tests, unsigned num_tests)
{
	bool pass = true;
	unsigned i;

	for (i = 0; i < num_tests; i++) {
		GLint prog = piglit_build_simple_program(tests[i].code, 0);
		GLint num_attr;
		unsigned visited_count[64];
		unsigned j;
		bool shader_dumped = false;

		memset(visited_count, 0, sizeof(visited_count));

		/* From page 93 (page 109 of the PDF) says:
		 *
		 *     "An attribute variable (either conventional or generic)
		 *      is considered active if it is determined by the
		 *      compiler and linker that the attribute may be accessed
		 *      when the shader is executed. Attribute variables that
		 *      are declared in a vertex shader but never used will not
		 *      count against the limit. In cases where the compiler
		 *      and linker cannot make a conclusive determination, an
		 *      attribute will be considered active."
		 *
		 * Compare the set of active attributes against the list of
		 * expected active attributes.
		 */
		glGetProgramiv(prog, GL_ACTIVE_ATTRIBUTES, &num_attr);

		for (j = 0; j < num_attr; j++) {
			const struct attribute *attr;
			char name_buf[256];
			int attr_idx;
			GLsizei name_len;
			GLint size;
			GLenum type;

			glGetActiveAttrib(prog, j,
					  sizeof(name_buf),
					  &name_len,
					  &size,
					  &type,
					  name_buf);
			attr_idx = find_attrib(tests[i].attributes, name_buf);

			/* If the named attribute is not in the list for the
			 * test, then it must not be active.
			 */
			if (attr_idx < 0) {
				DUMP_SHADER(tests[i].code);
				fprintf(stderr,
					"Attribute `%s' should not be active"
					" but is.\n", name_buf);
				pass = false;
				continue;
			}

			attr = &tests[i].attributes[attr_idx];
			if (visited_count[attr_idx] != 0) {
				DUMP_SHADER(tests[i].code);
				fprintf(stderr,
					"Attribute `%s' listed multiple times"
					" in active list.\n", name_buf);
				pass = false;
			} else if (attr->type != type) {
				DUMP_SHADER(tests[i].code);
				fprintf(stderr,
					"Attribute `%s' should have type"
					" %s, but had type %s.\n",
					name_buf, piglit_get_gl_enum_name(attr->type),
					piglit_get_gl_enum_name(type));
				pass = false;
			}

			visited_count[attr_idx]++;
		}

		for (j = 0; tests[i].attributes[j].name != NULL; j++) {
			if (visited_count[j] == 0) {
				DUMP_SHADER(tests[i].code);
				fprintf(stderr,
					"Attribute `%s' should have been"
					" active but wasn't.\n",
					tests[i].attributes[j].name);
				pass = false;
			}
		}
	}

	return pass;
}

void piglit_init(int argc, char **argv)
{
	bool pass = true;

	piglit_require_extension("GL_ARB_vertex_attrib_64bit");

	pass = do_test(vertex_attrib_64bit_tests,
		       ARRAY_SIZE(vertex_attrib_64bit_tests));

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}
