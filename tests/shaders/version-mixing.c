/*
 * Copyright © 2013 Intel Corporation
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
 */

/** \file
 *
 * Test that any desktop GLSL version may be linked with any other
 * desktop GLSL version.
 *
 * From the GLSL 4.30 spec, section 3.3 (Preprocessor):
 *
 *     "Shaders for the core or compatibility profiles that declare
 *     different versions can be linked together."
 *
 * This is a deliberate relaxation of the cross-version linking rules
 * from previous versions of the GLSL spec (which prohibited some
 * combinations of GLSL versions from being linked together).  It was
 * made because existing implementations didn't follow the old
 * cross-version linking rules (see Khronos bug 8463).  So it seems
 * reasonable to expect all implementations to follow the new relaxed
 * rules.
 *
 * This test can be run in the following ways:
 *
 * - "interstage" checks that a vertex shader of one version can be
 * linked with a fragment shader of another version.
 *
 * - "intrastage" checks that two vertex shaders of different versions
 * can be linked together.
 *
 * - "vs-gs" checks that a vertex shader of one version can be linked
 * with a geometry shader of another version.
 */

#include "piglit-util-gl.h"

static enum test_type {
	test_type_interstage,
	test_type_intrastage,
	test_type_vs_gs,
} test_type;

static void parse_params();

PIGLIT_GL_TEST_CONFIG_BEGIN

	piglit_gl_process_args(&argc, argv, &config);
	parse_params(argc, argv);
	if (test_type == test_type_vs_gs) {
		config.supports_gl_compat_version = 32;
		config.supports_gl_core_version = 32;
	} else {
		config.supports_gl_compat_version = 10;
		config.supports_gl_core_version = 31;
	}

PIGLIT_GL_TEST_CONFIG_END

static const char *interstage_vs =
	"#version %d\n"
	"\n"
	"void main()\n"
	"{\n"
	"  gl_Position = vec4(0.0);\n"
	"}\n";

static const char *interstage_gs =
	"#version %d\n"
	"\n"
	"layout(triangles) in;\n"
	"layout(triangle_strip, max_vertices = 3) out;\n"
	"void main()\n"
	"{\n"
	"  for (int i = 0; i < 3; i++) {\n"
	"    gl_Position = gl_in[i].gl_Position;\n"
	"    EmitVertex();\n"
	"  }\n"
	"}\n";

static const char *interstage_fs =
	"#version %d\n"
	"\n"
	"void main()\n"
	"{\n"
	"  gl_FragColor = vec4(0.0);\n"
	"}\n";

static const char *intrastage_vs1 =
	"#version %d\n"
	"\n"
	"void f();\n"
	"void main()\n"
	"{\n"
	"  f();\n"
	"}\n";

static const char *intrastage_vs2 =
	"#version %d\n"
	"\n"
	"void f()\n"
	"{\n"
	"  gl_Position = vec4(0.0);\n"
	"}\n";

static int all_glsl_versions[] = {
	110, 120, 130, 140, 150, 330, 400, 410, 420, 430, 440
};


static void
print_usage_and_exit(const char *prog_name)
{
	printf("Usage: %s <subtest>\n"
	       "  where <subtest> is one of:\n"
	       "    interstage: test interstage linking (vs-to-fs)\n"
	       "    intrastage: test intrastage linking (vs-to-vs)\n"
	       "    vs-gs: test interstage linking (vs-to-gs)\n",
	       prog_name);
	piglit_report_result(PIGLIT_FAIL);
}


static int
get_max_glsl_version()
{
	bool es;
	int major, minor;
	piglit_get_glsl_version(&es, &major, &minor);
	if (es) {
		printf("This test should only be run on desktop GL.\n");
		piglit_report_result(PIGLIT_FAIL);
	}
	return 100 * major + minor;
}


/**
 * Try compiling a shader of type \c target, whose string is formed by
 * applying \c version to \c shader_template, and attach it to \c
 * prog.  On success, return true.  If there is a problem, print an
 * error message using \c shader_desc to describe the shader, and
 * return false.
 */
static bool
try_attach_shader(GLuint prog, const char *shader_desc, GLenum target,
		  const char *shader_template, int version)
{
	char *shader_text = NULL;
	GLuint shader = glCreateShader(target);
	GLint ok;

	asprintf(&shader_text, shader_template, version);
	glShaderSource(shader, 1, (const GLchar **) &shader_text, NULL);
	free(shader_text);
	glCompileShader(shader);
	glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
	if (!ok) {
		printf("%s failed to compile.\n", shader_desc);
		glDeleteShader(shader);
		return false;
	}
	glAttachShader(prog, shader);
	glDeleteShader(shader);
	return true;
}


/**
 * Test interstage linking between VS and FS, and print a message
 * describing the result, and return true if compilation and linking
 * succeeded.
 */
static bool
test_interstage(int version_vs, int version_other, bool use_gs)
{
	GLuint prog = glCreateProgram();
	GLint ok;

	if (!try_attach_shader(prog, "vertex shader", GL_VERTEX_SHADER,
			       interstage_vs, version_vs)) {
		glDeleteProgram(prog);
		return false;
	}
	if (use_gs) {
		if (version_other < 150) {
			printf("Not tested (GS requires GLSL 1.50).\n");
			glDeleteProgram(prog);
			return true;
		}
		if (!try_attach_shader(prog, "geometry shader",
				       GL_GEOMETRY_SHADER, interstage_gs,
				       version_other)) {
			glDeleteProgram(prog);
			return false;
		}
	} else {
		if (!try_attach_shader(prog, "fragment shader",
				       GL_FRAGMENT_SHADER, interstage_fs,
				       version_other)) {
			glDeleteProgram(prog);
			return false;
		}
	}
	glLinkProgram(prog);
	glGetProgramiv(prog, GL_LINK_STATUS, &ok);
	if (ok)
		printf("Success.\n");
	else
		printf("Link failed.\n");
	return ok;
}


/**
 * Test intrastage linking between two VS shaders, and print a message
 * describing the result, and return true if compilation and linking
 * succeeded.
 */
static bool
test_intrastage(int version_vs1, int version_vs2)
{
	GLuint prog = glCreateProgram();
	GLint ok;

	if (!try_attach_shader(prog, "vertex shader 1", GL_VERTEX_SHADER,
			       intrastage_vs1, version_vs1)) {
		glDeleteProgram(prog);
		return false;
	}
	if (!try_attach_shader(prog, "vertex shader 2", GL_VERTEX_SHADER,
			       intrastage_vs2, version_vs2)) {
		glDeleteProgram(prog);
		return false;
	}
	glLinkProgram(prog);
	glGetProgramiv(prog, GL_LINK_STATUS, &ok);
	if (ok)
		printf("Success.\n");
	else
		printf("Link failed.\n");
	return ok;
}


void
parse_params(int argc, char **argv)
{
	if (argc != 2)
		print_usage_and_exit(argv[0]);
	if (strcmp(argv[1], "interstage") == 0)
		test_type = test_type_interstage;
	else if (strcmp(argv[1], "intrastage") == 0)
		test_type = test_type_intrastage;
	else if (strcmp(argv[1], "vs-gs") == 0)
		test_type = test_type_vs_gs;
	else
		print_usage_and_exit(argv[0]);
}


void
piglit_init(int argc, char **argv)
{
	int i, j;
	bool pass = true;
	int max_glsl_version;

	piglit_require_GLSL();
	max_glsl_version = get_max_glsl_version();

	for (i = 0; i < ARRAY_SIZE(all_glsl_versions); i++) {
		if (all_glsl_versions[i] > max_glsl_version)
			continue;
		for (j = 0; j < ARRAY_SIZE(all_glsl_versions); j++) {
			if (all_glsl_versions[j] > max_glsl_version)
				continue;
			printf("Testing versions %d and %d: ",
			       all_glsl_versions[i], all_glsl_versions[j]);
			switch (test_type) {
			case test_type_interstage:
				pass = test_interstage(all_glsl_versions[i],
						       all_glsl_versions[j],
						       false /* use_gs */)
					&& pass;
				break;
			case test_type_vs_gs:
				pass = test_interstage(all_glsl_versions[i],
						       all_glsl_versions[j],
						       true /* use_gs */)
					&& pass;
				break;
			case test_type_intrastage:
				pass = test_intrastage(all_glsl_versions[i],
						       all_glsl_versions[j])
					&& pass;
				break;
			default:
				/* Should never occur */
				piglit_report_result(PIGLIT_FAIL);
				break;
			}
		}
	}

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}


enum piglit_result
piglit_display(void)
{
	/* Should never be reached */
	return PIGLIT_FAIL;
}
