/*
 * Copyright © 2019 Advanced Micro Devices, Inc.
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

/** @file image_functions.c
 *
 * Test the new GLSL image functions.
 *
 * Two categories of tests in this file:
 *   - for the functions existing both in ARB_shader_image_load_store and
 *     EXT_shader_image_load_store, we simply build a program to verify that
 *     they're available.
 *   - for the 2 functions that only exist in EXT (atomicIncWrap and atomicDecWrap),
 *     we verify their behavior.
 */

#include "piglit-util-gl.h"

#define WRAP_VALUE 13
#define TEX_WIDTH 50

PIGLIT_GL_TEST_CONFIG_BEGIN

config.supports_gl_core_version = 32;
config.khr_no_error_support = PIGLIT_NO_ERRORS;

PIGLIT_GL_TEST_CONFIG_END

struct test_data {
	const char* function_name;
	int expected_value;
	const char* type;
	GLuint (*create_texture) (void);
	void (*read_texture) (void* data, size_t s);
};

static const char* vs =
	"#version 150\n"
	"#extension GL_EXT_shader_image_load_store : enable\n"
	"in vec4 position;\n"
	"void main() {\n"
	"gl_Position = position;\n"
	"}\n";

static GLuint
create_texture()
{
	static int data[TEX_WIDTH];
	GLuint texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_1D, texture);

	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	memset(data, 0, sizeof(data));
	glTexImage1D(GL_TEXTURE_1D, 0, GL_R32I, TEX_WIDTH, 0, GL_RED_INTEGER, GL_INT, data);

	return texture;
}

static void
read_texture(void* data, size_t s)
{
	glGetTexImage(GL_TEXTURE_1D, 0,
		      GL_RED_INTEGER,
		      GL_INT,
		      data);
}

static GLuint
create_buffer_texture()
{
	static int data[TEX_WIDTH];
	GLuint texture, buffer;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_BUFFER, texture);

	memset(data, 0, sizeof(data));

	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferStorage(GL_ARRAY_BUFFER, sizeof(data), data, GL_MAP_READ_BIT);

	glTexBuffer(GL_TEXTURE_BUFFER, GL_R32I, buffer);

	return texture;
}

static void
read_buffer_texture(void* data, size_t s)
{
	GLint buffer;
	glGetIntegerv(GL_TEXTURE_BINDING_BUFFER, &buffer);
	glGetBufferSubData(GL_ARRAY_BUFFER, 0, s, data);
	glDeleteBuffers(1, (GLuint*) &buffer);
}

static enum piglit_result
run_test(void * _data)
{
	static const char* fs_template =
		"#version 150\n"
		"#extension GL_EXT_shader_image_load_store : enable\n"
		"uniform int wrap_value;\n"
		"layout(size1x32) uniform %s image;\n"
		"void main() {\n"
		"   %s(image, 0, wrap_value); \n"
		"}\n";

	char fs[1024];
	bool pass = true;
	const struct test_data* test = (const struct test_data*) _data;

	sprintf(fs, fs_template, test->type, test->function_name);

	GLint program = piglit_build_simple_program(vs, fs);
	GLint image_location = glGetUniformLocation(program, "image");
	GLint wrap_location = glGetUniformLocation(program, "wrap_value");
	GLuint texture = test->create_texture();
	GLint read_back[TEX_WIDTH];

	glBindImageTextureEXT(0, texture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32I);

	glUseProgram(program);
	glUniform1i(image_location, 0);
	glUniform1i(wrap_location, WRAP_VALUE);

	piglit_draw_rect(-1, -1, 2.0, 2.0);

	glMemoryBarrier(GL_TEXTURE_UPDATE_BARRIER_BIT |
			GL_BUFFER_UPDATE_BARRIER_BIT |
			GL_PIXEL_BUFFER_BARRIER_BIT |
			GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

	test->read_texture(read_back, sizeof(read_back));

	/* The first component of the first pixel has been written to by all invocations */
	pass = pass && read_back[0] == test->expected_value;
	/* All other pixels/components should be untouched */
	for (int i = 1; i < TEX_WIDTH; i++) {
		pass = pass && read_back[i] == 0;
	}

	glDeleteTextures(1, &texture);
	glDeleteProgram(program);
	return pass && piglit_check_gl_error(GL_NO_ERROR) ?
		PIGLIT_PASS : PIGLIT_FAIL;
}

static enum piglit_result
run_compile_test(void * data)
{
	/* Type of the uniform variable used in imageStore */
	static const char* v_value_type[] = { "vec4", "ivec4", "uvec4" };
	/* Type of the uniform variable used in the atomic operations */
	static const char* i_value_type[] = { "int", "int", "uint" };
	/* Layout qualifier for the image */
	static const char* qualifiers[] = {
		"size1x8", "size1x16", "size1x32", "size2x32", "size4x32"
	};
	/* 'coord' variable */
	static const char* coords[] = {
		"int coord = 0",
		"ivec2 coord = ivec2(0)",
		"ivec3 coord = ivec3(0)",
	};
	/* Types of the image variable using a 'int' for the coord */
	static const char* types_int[] = {
		"image1D", "iimage1D", "uimage1D", "imageBuffer", NULL,
	};
	/* Types of the image variable using a 'ivec2' for the coord */
	static const char* types_ivec2[] = {
		"image2D", "iimage2D", "uimage2D",
		"image2DRect", "iimage2DRect", "uimage2DRect",
		"image1DArray", "iimage1DArray", "uimage1DArray",
		NULL
	};
	/* Types of the image variable using a 'ivec3' for the coord */
	static const char* types_ivec3[] = {
		"image3D", "iimage3D", "uimage3D",
		"imageCube", "iimageCube", "uimageCube",
		"image2DArray", "iimage2DArray", "uimage2DArray",
		"imageCubeArray", "iimageCubeArray", "uimageCubeArray",
		NULL
	};
	/* Fragment shader template */
	static const char* fs_template =
		"#version 150\n"
		"#extension GL_EXT_shader_image_load_store : enable\n"
		"uniform int wrap_value;\n"
		"uniform %s v_value;\n"
		"uniform %s i_value;\n"
		"layout(%s) uniform %s image;\n"
		"void main() {\n"
		"   %s;\n"
		"   %s;\n"
		"}\n";

	char fs[2048];
	bool pass = true;

	static const char** types[] = {
		types_int, types_ivec2, types_ivec3
	};

	const bool atomicOp = strstr(data, "Atomic") != NULL;

	for (int i = 0; i < ARRAY_SIZE(qualifiers); i++) {
		for (int j = 0; j < ARRAY_SIZE(types); j++) {
			for (int k = 0; types[j][k]; k++) {
				bool floatImageType = strncmp(types[j][k], "image", 5) == 0;

				/* skip atomic + float */
				if (atomicOp && floatImageType)
					continue;

				/* skip atomic + not-1x32 */
				if (atomicOp && i != 2)
					continue;

				/* Build the fragment template */
				sprintf(fs, fs_template,
					v_value_type[k % 3],
					i_value_type[k % 3],
					qualifiers[i],
					types[j][k],
					coords[j],
					(char*) data);

				/* And verify we can build the program */
				GLint program = piglit_build_simple_program(vs, fs);
				pass = pass && piglit_check_gl_error(GL_NO_ERROR);
				glDeleteProgram(program);
			}
		}
	}

	return pass ? PIGLIT_PASS : PIGLIT_FAIL;
}

static unsigned compute_imageAtomicDecWrap(int num_exec, unsigned wrap)
{
	unsigned value = 0;
	/* The EXT_shader_image_load_store spec says:
	 *
	 *    imageAtomicDecWrap() computes a new value by subtracting one from the
	 *    contents of the selected texel, and then forcing the result to <wrap>-1 if
	 *    the original value read from the selected texel was either zero or greater
	 *    than <wrap>.  These functions support only 32-bit unsigned integer
	 *    operands.
	 */
	for (int i = 0; i < num_exec; i++) {
		if (value == 0 || value > wrap) {
			value = wrap;
		} else {
			value -= 1;
		}
	}
	return value;
}

void
piglit_init(int argc, char **argv)
{
	struct test_data test_data[] = {
		{
			"imageAtomicIncWrap",
			/* The EXT_shader_image_load_store spec says:
			 *
			 *     imageAtomicIncWrap() computes a new value by adding one to the contents of
			 *     the selected texel, and then forcing the result to zero if and only if the
			 *     incremented value is greater than or equal to <wrap>.
			 *
			 * So it's equivalent to a modulo.
			 */
			(piglit_width * piglit_height) % WRAP_VALUE,
			"iimage1D",
			create_texture,
			read_texture
		},
		{
			"imageAtomicIncWrap",
			(piglit_width * piglit_height) % WRAP_VALUE,
			"iimageBuffer",
			create_buffer_texture,
			read_buffer_texture
		},
		{
			"imageAtomicDecWrap",
			compute_imageAtomicDecWrap(piglit_width * piglit_height, WRAP_VALUE),
			"iimage1D",
			create_texture,
			read_texture
		},
		{
			"imageAtomicDecWrap",
			compute_imageAtomicDecWrap(piglit_width * piglit_height, WRAP_VALUE),
			"iimageBuffer",
			create_buffer_texture,
			read_buffer_texture
		},
	};
	const struct piglit_subtest tests[] =
	{
		{
			"imageAtomicIncWrap iimage1D",
			NULL,
			run_test,
			&test_data[0],
		},
		{
			"imageAtomicIncWrap iimageBuffer",
			NULL,
			run_test,
			&test_data[1],
		},
		{
			"imageAtomicDecWrap iimage1D",
			NULL,
			run_test,
			&test_data[2],
		},
		{
			"imageAtomicDecWrap iimageBuffer",
			NULL,
			run_test,
			&test_data[3],
		},
		/* Compile only tests */
		{
			"imageLoad",
			NULL,
			run_compile_test,
			"imageLoad(image, coord)",
		},
		{
			"imageStore",
			NULL,
			run_compile_test,
			"imageStore(image, coord, v_value)",
		},
		{
			"imageAtomicAdd",
			NULL,
			run_compile_test,
			"imageAtomicAdd(image, coord, i_value)",
		},
		{
			"imageAtomicMin",
			NULL,
			run_compile_test,
			"imageAtomicMin(image, coord, i_value)",
		},
		{
			"imageAtomicMax",
			NULL,
			run_compile_test,
			"imageAtomicMax(image, coord, i_value)",
		},
		{
			"imageAtomicAnd",
			NULL,
			run_compile_test,
			"imageAtomicAnd(image, coord, i_value)",
		},
		{
			"imageAtomicOr",
			NULL,
			run_compile_test,
			"imageAtomicOr(image, coord, i_value)",
		},
		{
			"imageAtomicXor",
			NULL,
			run_compile_test,
			"imageAtomicXor(image, coord, i_value)",
		},
		{
			"imageAtomicExchange",
			NULL,
			run_compile_test,
			"imageAtomicExchange(image, coord, i_value)",
		},
		{
			"imageAtomicCompSwap",
			NULL,
			run_compile_test,
			"imageAtomicCompSwap(image, coord, i_value, i_value)",
		},
		{
			"imageAtomicIncWrap",
			NULL,
			run_compile_test,
			"imageAtomicIncWrap(image, coord, i_value)",
		},
		{
			"imageAtomicDecWrap",
			NULL,
			run_compile_test,
			"imageAtomicDecWrap(image, coord, i_value)",
		},
		{0},
	};

	piglit_require_extension("GL_EXT_shader_image_load_store");

	enum piglit_result result = PIGLIT_PASS;

	result = piglit_run_selected_subtests(
		tests,
		NULL,
		0,
		PIGLIT_PASS);

	piglit_report_result(result);
}

enum piglit_result
piglit_display(void)
{
		return PIGLIT_FAIL;
}
