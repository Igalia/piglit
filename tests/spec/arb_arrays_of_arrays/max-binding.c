/*
 * Copyright © 2015 Intel Corporation
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

/** @file max-binding.c
 *
 * Test that using more than the maximum number of suported interface block,
 * sampler, or atomic bindings fails with a compile error.
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_core_version = 32;

	config.window_width = 1;
	config.window_height = 1;
	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;

PIGLIT_GL_TEST_CONFIG_END

static bool
generate_and_compile_shader(GLuint stage, const char *src_template,
			    unsigned binding)
{
	char *src;
	int ret;

	ret = asprintf(&src, src_template, binding);
	assert(ret);

	ret = piglit_compile_shader_text_nothrow(stage, src);

	free(src);
	return ret;
}

static bool
run_test_sampler_max_bindings(unsigned binding)
{
	const char *src_template = "#version 150\n"
		"#extension GL_ARB_arrays_of_arrays : enable\n"
		"#extension GL_ARB_shading_language_420pack : enable\n"
		"layout(binding=%d) uniform sampler2D sampler[2][2];\n"
		"in vec2 coord;"
		"\n"
		"void main() {\n"
		"  gl_FragColor = texture2D(sampler[1][1], coord) + texture2D(sampler[0][1], coord);"
		"}\n";
	return generate_and_compile_shader(GL_FRAGMENT_SHADER, src_template,
					   binding);
}

static bool
run_test_interface_max_bindings(unsigned binding)
{
        const char *src_template = "#version 150\n"
		"#extension GL_ARB_arrays_of_arrays : enable\n"
		"#extension GL_ARB_shading_language_420pack : enable\n"
		"layout(binding=%d) uniform ArraysOfArraysBlock\n"
		"{\n"
		"  float a;"
		"} i[2][2];\n"
		"void main() {\n"
		"  gl_Position = vec4(i[0][0].a, i[0][1].a, i[1][0].a, i[1][1].a);"
		"}\n";
	return generate_and_compile_shader(GL_VERTEX_SHADER, src_template,
					   binding);
}

static bool
run_test_ac_fragment_max_bindings(unsigned binding)
{
	const char *src_template = "#version 150\n"
		"#extension GL_ARB_shader_atomic_counters : enable\n"
		"#extension GL_ARB_arrays_of_arrays : enable\n"
		"\n"
		"layout(binding=%d) uniform atomic_uint x[2][2];"
		"\n"
		"void main() {\n"
		"}\n";
	return generate_and_compile_shader(GL_FRAGMENT_SHADER, src_template,
					   binding);
}

static bool
run_test_ac_vertex_max_bindings(unsigned binding)
{
        const char *src_template = "#version 150\n"
		"#extension GL_ARB_shader_atomic_counters : enable\n"
		"#extension GL_ARB_arrays_of_arrays : enable\n"
		"\n"
		"in vec4 position;\n"
		"layout(binding=%d) uniform atomic_uint x[2][2];"
		"\n"
		"void main() {\n"
		"       gl_Position = position;\n"
		"}\n";
	return generate_and_compile_shader(GL_VERTEX_SHADER, src_template,
					   binding);
}

static void
subtest_fail(enum piglit_result *status, char *name)
{
	piglit_report_subtest_result(PIGLIT_FAIL, name);
	*status = PIGLIT_FAIL;
}

void
piglit_init(int argc, char **argv)
{
	enum piglit_result status = PIGLIT_PASS;
	int max_ab_binding;
	int max_samp_binding;
	int max_ifc_blk_binding;

	/* assume the templates use this number of elements */
	int array_elements = 4;

	if (piglit_is_extension_supported("GL_ARB_shader_atomic_counters")) {
		glGetIntegerv(GL_MAX_ATOMIC_COUNTER_BUFFER_BINDINGS,
			      &max_ab_binding);
	}
	glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS,
		      &max_samp_binding);
	glGetIntegerv(GL_MAX_UNIFORM_BUFFER_BINDINGS, &max_ifc_blk_binding);

	/* require 3.2 for interface block support */
	piglit_require_gl_version(32);
	piglit_require_extension("GL_ARB_arrays_of_arrays");
	piglit_require_extension("GL_ARB_shading_language_420pack");

	/* text above max limit */
	if(run_test_interface_max_bindings(max_ifc_blk_binding-(array_elements-1)))
		subtest_fail(&status, "Test interface block binding above "
				      "maximum number of uniform "
				      "buffer bindings");

	/* test max boundary */
	if(!run_test_interface_max_bindings(max_ifc_blk_binding-array_elements))
		subtest_fail(&status, "Test interface block binding on "
				      "boundary of maximum number of uniform "
				      "buffer bindings");

	/* text above max limit */
	if(run_test_sampler_max_bindings(max_samp_binding-(array_elements-1)))
		subtest_fail(&status, "Test sampler binding above "
				      "maximum number of texture "
				      "unit bindings");

	/* test max boundary */
	if(!run_test_sampler_max_bindings(max_samp_binding-array_elements))
		subtest_fail(&status, "Test sampler binding on boundary "
				      "of maximum number of texure "
				      "unit bindings");

	if (piglit_is_extension_supported("GL_ARB_shader_atomic_counters")) {
		/* text above max limit */
		if(run_test_ac_fragment_max_bindings(max_ab_binding))
			subtest_fail(&status, "Fragment shader test above "
					      "maximum number of atomic "
					      "counter bindings");

		if(run_test_ac_vertex_max_bindings(max_ab_binding))
			subtest_fail(&status, "Vertex shader test above "
					      "maximum number of atomic "
					      "counter bindings");
		/* test max boundary */
		if(!run_test_ac_fragment_max_bindings(max_ab_binding-1))
			subtest_fail(&status, "Fragment shader test boundary "
					      "of maximum number of atomic "
					      "counter bindings");

		if(!run_test_ac_vertex_max_bindings(max_ab_binding-1))
			subtest_fail(&status, "Vertex shader test boundary "
					      "of maximum number of atomic "
					      "counter bindings");
	}

	piglit_report_result(status);
}

enum piglit_result
piglit_display(void)
{
	return PIGLIT_PASS;
}
