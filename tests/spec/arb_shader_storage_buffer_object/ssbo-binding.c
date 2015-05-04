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
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

/** @file shaderstorageblockbinding.c
 *
 * From GL_ARB_shader_storage_buffer_object:
 * "After a program is linked, the command
 *
 *     void ShaderStorageBlockBinding(uint program, uint storageBlockIndex,
 *                                    uint storageBlockBinding);
 *
 *   changes the active shader storage block with an assigned index of
 *   <storageBlockIndex> in program object <program>.  The error INVALID_VALUE
 *   is generated if <storageBlockIndex> is not an active shader storage block
 *   index in <program>, or if <storageBlockBinding> is greater than or equal
 *   to the value of MAX_SHADER_STORAGE_BUFFER_BINDINGS. If successful,
 *   ShaderStorageBlockBinding specifies that <program> will use the data
 *   store of the buffer object bound to the binding point
 *   <storageBlockBinding> to read and write the values of the buffer
 *   variables in the shader storage block identified by <storageBlockIndex>."
 */

#include "piglit-util-gl.h"

PIGLIT_GL_TEST_CONFIG_BEGIN

	config.supports_gl_compat_version = 10;

	config.window_visual = PIGLIT_GL_VISUAL_DOUBLE | PIGLIT_GL_VISUAL_RGBA;

PIGLIT_GL_TEST_CONFIG_END

static GLuint prog;

static const char frag_shader_text[] =
	"#extension GL_ARB_shader_storage_buffer_object : require\n"
	"\n"
	"buffer ssbo_a { vec4 a; };\n"
	"\n"
	"void main()\n"
	"{\n"
	"	gl_FragColor = a;\n"
	"}\n";

void
piglit_init(int argc, char **argv)
{
	bool pass = true;
	int max_binding;
	int index;

	piglit_require_extension("GL_ARB_shader_storage_buffer_object");
	piglit_require_extension("GL_ARB_program_interface_query");

	prog = piglit_build_simple_program(NULL, frag_shader_text);

	glGetIntegerv(GL_MAX_SHADER_STORAGE_BUFFER_BINDINGS, &max_binding);
	printf("MAX_SHADER_STORAGE_BUFFER_BINDINGS: %d\n", max_binding);

	index = glGetProgramResourceIndex(prog,
					  GL_SHADER_STORAGE_BLOCK, "ssbo_a");
	if (!piglit_check_gl_error(0)) {
		pass = false;
	}
	printf("Shader storage block \"ssbo_a\" index: %d\n", index);

	printf("Test binding value: %d\n", 0);
	glShaderStorageBlockBinding(prog, index, 0);
	if (!piglit_check_gl_error(0)) {
		pass = false;
	}
	printf("Test binding value: %d\n", max_binding - 1);
	glShaderStorageBlockBinding(prog, index, max_binding - 1);
	if (!piglit_check_gl_error(0)) {
		pass = false;
	}

	/* The error INVALID_VALUE is generated if <storageBlockIndex> is not
	 * an active shader storage block index in <program>, or if
	 * <storageBlockBinding> is greater than or equal to the value of
	 * MAX_SHADER_STORAGE_BUFFER_BINDINGS.
	 */
	printf("Test binding value: %d\n", max_binding);
	glShaderStorageBlockBinding(prog, index, max_binding);
	if (!piglit_check_gl_error(GL_INVALID_VALUE)) {
		pass = false;
	}

	/* The error INVALID_VALUE is generated if <storageBlockIndex> is not
	 * an active shader storage block index in <program>
	 */
	printf("Test invalid index: %d\n", index + 1);
	glShaderStorageBlockBinding(prog, index + 1, 0);
	if (!piglit_check_gl_error(GL_INVALID_VALUE)) {
		pass = false;
	}

	piglit_report_result(pass ? PIGLIT_PASS : PIGLIT_FAIL);
}

enum piglit_result piglit_display(void)
{
	/* UNREACHED */
	return PIGLIT_FAIL;
}
