/*
 * Copyright © 2011 Intel Corporation
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
 * values for a variety of shaders.
 *
 * \author Ian Romanick
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_RGB | PIGLIT_GL_VISUAL_DOUBLE;

PIGLIT_GL_TEST_CONFIG_END

struct attribute {
	/** Name of the attribute. */
	const char *name;

	/**
	 * This attribute must be active in the linked shader.
	 *
	 * Some attributes must be active and some may or may not be active
	 * (becuase a clever compiler could optimize them away.  Attributes
	 * that must not be active should not be listed in
	 * \c test::attributes.
	 */
	bool must_be_active;

	/**
	 * Expected (array) size of the attribute.
	 *
	 * \note
	 * Attribute arrays aren't added until GLSL 1.50.
	 */
	GLint size;

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

static const struct test glsl110_tests[] = {
	/* The first group of tests tries all the possible types for vertex
	 * shader inputs.
	 */
	{
		"attribute float vertex;\n"
		"void main() { gl_Position = vec4(vertex); }",
		{
			{ "vertex",    true,  1, GL_FLOAT },
			{ NULL, }
		}
	},
	{
		"attribute vec2 vertex;\n"
		"void main() { gl_Position = vertex.xyxy; }",
		{
			{ "vertex",    true,  1, GL_FLOAT_VEC2 },
			{ NULL, }
		}
	},
	{
		"attribute vec3 vertex;\n"
		"void main() { gl_Position = vertex.xyzx; }",
		{
			{ "vertex",    true,  1, GL_FLOAT_VEC3 },
			{ NULL, }
		}
	},
	{
		"attribute vec4 vertex;\n"
		"void main() { gl_Position = vertex; }",
		{
			{ "vertex",    true,  1, GL_FLOAT_VEC4 },
			{ NULL, }
		}
	},
	{
		"attribute mat2 vertex;\n"
		"void main() { gl_Position = vertex[0].xyxy; }",
		{
			{ "vertex",    true,  1, GL_FLOAT_MAT2 },
			{ NULL, }
		}
	},
	{
		"attribute mat3 vertex;\n"
		"void main() { gl_Position = vertex[0].xyzx; }",
		{
			{ "vertex",    true,  1, GL_FLOAT_MAT3 },
			{ NULL, }
		}
	},
	{
		"attribute mat4 vertex;\n"
		"void main() { gl_Position = vertex[0]; }",
		{
			{ "vertex",    true,  1, GL_FLOAT_MAT4 },
			{ NULL, }
		}
	},


	/* Try using each of the built-in attributes one at a time.  Only the
	 * first two glMultiTexCoord attributes are checked because that's all
	 * an implementation is required to support.
	 */
	{
		"void main() { gl_Position = gl_Color; }",
		{
			{ "gl_Color",  true,  1, GL_FLOAT_VEC4 },
			{ NULL, }
		}
	},
	{
		"void main() { gl_Position = gl_SecondaryColor; }",
		{
			{ "gl_SecondaryColor", true,  1, GL_FLOAT_VEC4 },
			{ NULL, }
		}
	},
	{
		"void main() { gl_Position = gl_Normal.xyzx; }",
		{
			{ "gl_Normal", true,  1, GL_FLOAT_VEC3 },
			{ NULL, }
		}
	},
	{
		"void main() { gl_Position = gl_Vertex; }",
		{
			{ "gl_Vertex", true,  1, GL_FLOAT_VEC4 },
			{ NULL, }
		}
	},
	{
		"void main() { gl_Position = gl_MultiTexCoord0; }",
		{
			{ "gl_MultiTexCoord0", true,  1, GL_FLOAT_VEC4 },
			{ NULL, }
		}
	},
	{
		"void main() { gl_Position = gl_MultiTexCoord1; }",
		{
			{ "gl_MultiTexCoord1", true,  1, GL_FLOAT_VEC4 },
			{ NULL, }
		}
	},
	{
		"void main() { gl_Position = vec4(gl_FogCoord); }",
		{
			{ "gl_FogCoord", true,  1, GL_FLOAT },
			{ NULL, }
		}
	},

	/* Try various cases of using / not using some user-defined attributes
	 * and some built-in attributes.
	 */
	{
		"attribute vec4 not_used;\n"
		"void main() { gl_Position = gl_Vertex; }",
		{
			{ "gl_Vertex", true,  1, GL_FLOAT_VEC4 },
			{ NULL, }
		}
	},
	{
		"attribute vec4 vertex;\n"
		"void main() { gl_Position = vertex + gl_Vertex; }",
		{
			{ "gl_Vertex", true,  1, GL_FLOAT_VEC4 },
			{ "vertex",    true,  1, GL_FLOAT_VEC4 },
			{ NULL, }
		}
	},
	{
		"attribute vec4 vertex;\n"
		"void main() {\n"
		"    gl_Position = vertex;\n"
		"    if (false) gl_Position = gl_Vertex;\n"
		"}",
		{
			{ "gl_Vertex", false, 1, GL_FLOAT_VEC4 },
			{ "vertex",    true,  1, GL_FLOAT_VEC4 },
			{ NULL, }
		}
	},
	{
		"attribute vec4 vertex;\n"
		"attribute vec2 alternate;\n"
		"uniform bool use_alternate;\n"
		"void main() {\n"
		"    gl_Position = vertex;\n"
		"    if (use_alternate) gl_Position = alternate.xyxy;\n"
		"}",
		{
			{ "vertex",    true,  1, GL_FLOAT_VEC4 },
			{ "alternate", true,  1, GL_FLOAT_VEC2 },
			{ NULL, }
		}
	},

	/* The built-in function ftransform should also mark gl_Vertex as used.
	 */
	{
		"void main() { gl_Position = ftransform(); }",
		{
			{ "gl_Vertex", true,  1, GL_FLOAT_VEC4 },
			{ NULL, }
		}
	},
};

static const struct test glsl120_tests[] = {
	/* Try all the possible types for vertex shader inputs.  Note that
	 * this only checks the types that were added in GLSL 1.20.
	 *
	 * Since GLSL 1.20 doesn't add any new built-in attributes, there are
	 * no other tests added in the GLSL 1.20 group.
	 */
	{
		"#version 120\n"
		"attribute mat2x3 vertex;\n"
		"void main() { gl_Position = vertex[0].xxxx; }",
		{
			{ "vertex",    true,  1, GL_FLOAT_MAT2x3 },
			{ NULL, }
		}
	},
	{
		"#version 120\n"
		"attribute mat2x4 vertex;\n"
		"void main() { gl_Position = vertex[0].xxxx; }",
		{
			{ "vertex",    true,  1, GL_FLOAT_MAT2x4 },
			{ NULL, }
		}
	},
	{
		"#version 120\n"
		"attribute mat3x2 vertex;\n"
		"void main() { gl_Position = vertex[0].xxxx; }",
		{
			{ "vertex",    true,  1, GL_FLOAT_MAT3x2 },
			{ NULL, }
		}
	},
	{
		"#version 120\n"
		"attribute mat3x4 vertex;\n"
		"void main() { gl_Position = vertex[0].xxxx; }",
		{
			{ "vertex",    true,  1, GL_FLOAT_MAT3x4 },
			{ NULL, }
		}
	},
	{
		"#version 120\n"
		"attribute mat4x2 vertex;\n"
		"void main() { gl_Position = vertex[0].xxxx; }",
		{
			{ "vertex",    true,  1, GL_FLOAT_MAT4x2 },
			{ NULL, }
		}
	},
	{
		"#version 120\n"
		"attribute mat4x3 vertex;\n"
		"void main() { gl_Position = vertex[0].xxxx; }",
		{
			{ "vertex",    true,  1, GL_FLOAT_MAT4x3 },
			{ NULL, }
		}
	},
};

static const struct test glsl130_tests[] = {
	/* Try all the possible types for vertex shader inputs.  Note that
	 * this only checks the types that were added in GLSL 1.30.
	 *
	 * Since GLSL 1.30 doesn't add any new built-in attributes, there are
	 * no other tests added in the GLSL 1.30 group.
	 */
	{
		"#version 130\n"
		"in int vertex;\n"
		"void main() { gl_Position = vec4(vertex); }",
		{
			{ "vertex",    true,  1, GL_INT },
			{ NULL, }
		}
	},
	{
		"#version 130\n"
		"in uint vertex;\n"
		"void main() { gl_Position = vec4(vertex); }",
		{
			{ "vertex",    true,  1, GL_UNSIGNED_INT },
			{ NULL, }
		}
	},
	{
		"#version 130\n"
		"in ivec2 vertex;\n"
		"void main() { gl_Position = vec4(vertex.x); }",
		{
			{ "vertex",    true,  1, GL_INT_VEC2 },
			{ NULL, }
		}
	},
	{
		"#version 130\n"
		"in uvec2 vertex;\n"
		"void main() { gl_Position = vec4(vertex.x); }",
		{
			{ "vertex",    true,  1, GL_UNSIGNED_INT_VEC2 },
			{ NULL, }
		}
	},
	{
		"#version 130\n"
		"in ivec3 vertex;\n"
		"void main() { gl_Position = vec4(vertex.x); }",
		{
			{ "vertex",    true,  1, GL_INT_VEC3 },
			{ NULL, }
		}
	},
	{
		"#version 130\n"
		"in uvec3 vertex;\n"
		"void main() { gl_Position = vec4(vertex.x); }",
		{
			{ "vertex",    true,  1, GL_UNSIGNED_INT_VEC3 },
			{ NULL, }
		}
	},
	{
		"#version 130\n"
		"in ivec4 vertex;\n"
		"void main() { gl_Position = vec4(vertex.x); }",
		{
			{ "vertex",    true,  1, GL_INT_VEC4 },
			{ NULL, }
		}
	},
	{
		"#version 130\n"
		"in uvec4 vertex;\n"
		"void main() { gl_Position = vec4(vertex.x); }",
		{
			{ "vertex",    true,  1, GL_UNSIGNED_INT_VEC4 },
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
		GLint vert =
			piglit_compile_shader_text(GL_VERTEX_SHADER,
						   tests[i].code);
		GLint prog = piglit_link_simple_program(vert, 0);
		GLint num_attr;
		unsigned visited_count[64];
		unsigned j;
		bool shader_dumped = false;

		memset(visited_count, 0, sizeof(visited_count));

		/* From page 93 (page 109 of the PDF) says:
		 *
		 *     "An attribute variable (either conventional or generic)
		 *     is considered active if it is determined by the
		 *     compiler and linker that the attribute may be accessed
		 *     when the shader is executed. Attribute variables that
		 *     are declared in a vertex shader but never used will not
		 *     count against the limit. In cases where the compiler
		 *     and linker cannot make a conclusive determination, an
		 *     attribute will be considered active."
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
					"Attribute `%s' should not be active "
					"but is.\n", name_buf);
				pass = false;
				continue;
			}

			attr = &tests[i].attributes[attr_idx];
			if (visited_count[attr_idx] != 0) {
				DUMP_SHADER(tests[i].code);
				fprintf(stderr,
					"Attribute `%s' listed multiple times "
					"in active list.\n", name_buf);
				pass = false;
			} else if (attr->size != size) {
				DUMP_SHADER(tests[i].code);
				fprintf(stderr,
					"Attribute `%s' should have size %d, "
					"but had size %d.\n",
					name_buf, attr->size, size);
				pass = false;
			} else if (attr->type != type) {
				DUMP_SHADER(tests[i].code);
				fprintf(stderr,
					"Attribute `%s' should have type "
					"0x%04x, but had type 0x%04x.\n",
					name_buf, attr->type, type);
				pass = false;
			}

			visited_count[attr_idx]++;
		}

		for (j = 0; tests[i].attributes[j].name != NULL; j++) {
			if (tests[i].attributes[j].must_be_active
			    && visited_count[j] == 0) {
				DUMP_SHADER(tests[i].code);
				fprintf(stderr,
					"Attribute `%s' should have been "
					"active but wasn't.\n",
					tests[i].attributes[j].name);
				pass = false;
			}
		}
	}

	return pass;
}

void usage_and_fail(const char *name)
{
	fprintf(stderr, "Usage: %s [110|120|130]\n", name);
	piglit_report_result(PIGLIT_FAIL);
}

void piglit_init(int argc, char **argv)
{
	bool pass = true;
	unsigned i;

	if (argc == 1) {
		usage_and_fail(argv[0]);
	}

	for (i = 1; i < argc; i++) {
		if (strcmp("110", argv[i]) == 0) {
			pass = do_test(glsl110_tests,
				       ARRAY_SIZE(glsl110_tests))
				&& pass;
		} else if (strcmp("120", argv[i]) == 0) {
			piglit_require_GLSL_version(120);
			pass = do_test(glsl120_tests,
				       ARRAY_SIZE(glsl120_tests))
				&& pass;
		} else if (strcmp("130", argv[i]) == 0) {
			piglit_require_GLSL_version(130);
			pass = do_test(glsl130_tests,
				       ARRAY_SIZE(glsl130_tests))
				&& pass;
		} else {
			usage_and_fail(argv[0]);
		}
	}

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}
